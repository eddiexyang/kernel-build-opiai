/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * Description:
 * Author: huawei
 * Create: 2022-08-13
 */

#include "rdr_exception_core.h"
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/securec.h>
#include <linux/securectype.h>
#include "rdr_core.h"
#include "common/bbox_print.h"
#include "bbox_platform.h"
#include "bbox_inner.h"

STATIC LIST_HEAD(g_rdr_exception_list);
STATIC DEFINE_SPINLOCK(g_rdr_exception_list_lock);

/*
 * @brief       : exception callback function
 * @param [in]  : const struct bbox_exception_info *p_exce_info    exception info
 * @param [in]  : u32 excepid,              exception id;
 * @param [in]  : const char *logpath       log path
 * @return      : != 0 fail; = 0 success
 */
void rdr_exception_callback(const struct bbox_exception_info *p_exce_info, u32 excepid)
{
    BB_CHECK_PTR(p_exce_info, return, "invalid param, p_exce_info is NULL.\n");

    if (p_exce_info->e_callback != NULL) {
        (p_exce_info->e_callback)(excepid, NULL);
    }
}

/*
 * @brief       : check exception id
 * @param [in]  : u32 excepid,       exception id;
 * @param [in]  : u32 excepid_end        exception end id
 * @return      : exception id
 *                = 0 fail; > 0 success
 */
STATIC u32 rdr_exception_check_excepid(u32 excepid, u32 excepid_end)
{
    lock_flag_t flags = 0;
    const struct rdr_exception_node *node = NULL;
    struct list_head *cur = NULL;
    struct list_head *next = NULL;

    spin_lock_irqsave(&g_rdr_exception_list_lock, flags);
    list_for_each_safe(cur, next, &g_rdr_exception_list) {
        node = (struct rdr_exception_node *)list_entry(cur, struct rdr_exception_node, e_list);
        if (node == NULL) {
            continue;
        }
        if (((excepid >= node->info.e_excepid) && (excepid <= node->info.e_excepid_end)) ||
            ((excepid_end >= node->info.e_excepid) && (excepid_end <= node->info.e_excepid_end))) {
            spin_unlock_irqrestore(&g_rdr_exception_list_lock, flags);
            return excepid;
        }
    }
    spin_unlock_irqrestore(&g_rdr_exception_list_lock, flags);
    return 0;
}

/*
 * @brief       : check exception info
 * @param [in]  : bbox_exception_info_s *e    exception info
 * @return      : != 0 fail; = 0 success
 */
STATIC s32 rdr_exception_check_info(const bbox_exception_info_s *e)
{
    if (e == NULL) {
        BB_PRINT_ERR("invaild parameter.\n");
        return BBOX_FAILURE;
    }

    if (e->e_process_priority >= (u8)BBOX_PPRI_MAX) {
        BB_PRINT_ERR("invaild e_process_priority\n");
        return BBOX_FAILURE;
    }
    if (e->e_reboot_priority >= (u8)BBOX_REBOOT_MAX) {
        BB_PRINT_ERR("invaild e_reboot_priority\n");
        return BBOX_FAILURE;
    }
    if (e->e_notify_core_mask > bbox_coreid_mask((u8)BBOX_CORE_MAX - 1U)) {
        BB_PRINT_ERR("invaild notify core mask[%llu]\n", e->e_notify_core_mask);
        return BBOX_FAILURE;
    }
    if (e->e_reset_core_mask > bbox_coreid_mask((u8)BBOX_CORE_MAX - 1U)) {
        BB_PRINT_ERR("invaild reset core mask[%llu]\n", e->e_reset_core_mask);
        return BBOX_FAILURE;
    }
    if (e->e_from_core >= (u8)BBOX_CORE_MAX) {
        BB_PRINT_ERR("invaild e_from_core\n");
        return BBOX_FAILURE;
    }
    if ((e->e_reentrant != (u32)BBOX_REENTRANT_ALLOW) &&
        (e->e_reentrant != (u32)BBOX_REENTRANT_DISALLOW)) {
        BB_PRINT_ERR("invaild e_reentrant:%d\n", e->e_reentrant);
        return BBOX_FAILURE;
    }

    if (bbox_excepid_compare_coreid(e->e_excepid, e->e_from_core) != true) {
        BB_PRINT_ERR("invaild e_excepid\n");
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : get exception node
 * @param [in]  : u32 excepid,     exception id;
 * @return      : NULL: fail; other: success
 */
STATIC struct rdr_exception_node *rdr_exception_get_node(u32 excepid)
{
    u32 e_excepid;
    lock_flag_t flags = 0;
    struct list_head *cur = NULL;
    struct list_head *next = NULL;

    e_excepid = bbox_excepid_transf_class(excepid);
    spin_lock_irqsave(&g_rdr_exception_list_lock, flags);
    list_for_each_safe(cur, next, &g_rdr_exception_list) {
        struct rdr_exception_node *node =
            (struct rdr_exception_node *)list_entry(cur, struct rdr_exception_node, e_list);
        if (node == NULL) {
            continue;
        }
        if ((node->info.e_excepid <= e_excepid) && (node->info.e_excepid_end >= e_excepid)) {
            spin_unlock_irqrestore(&g_rdr_exception_list_lock, flags);
            return node;
        }
    }
    spin_unlock_irqrestore(&g_rdr_exception_list_lock, flags);

    return NULL;
}

/*
 * @brief       : get exception info
 * @param [in]  : u32 excepid     exception id;
 * @return      : NULL: fail; other: success
 */
struct bbox_exception_info *rdr_exception_get_info(u32 excepid)
{
    struct rdr_exception_node *node = rdr_exception_get_node(excepid);
    if (node != NULL) {
        return &node->info;
    } else {
        return NULL;
    }
}

/*
 * @brief       : get exception type
 * @param [in]  : u32 excepid      exception id;
 * @return      : BBOX_EXCEPTION_REASON_INVALID: fail; other: success
 */
u8 rdr_exception_get_etype(u32 excepid)
{
    const struct rdr_exception_node *node = rdr_exception_get_node(excepid);
    if (node != NULL) {
        return node->info.e_exce_type;
    } else {
        return BBOX_EXCEPTION_REASON_INVALID;
    }
}

/*
 * @brief       : print one exception info
 * @param [in]  : struct bbox_exception_info *e    exception info
 * @return      : NULL
 */
void rdr_exception_print_info(struct bbox_exception_info *e)
{
    if (e == NULL) {
        BB_PRINT_ERR("invaild parameter.\n");
        return;
    }
    e->e_desc[BBOX_EXCEPTIONDESC_MAXLEN - 1] = 0U;
    e->e_from_module[BBOX_MODULE_NAME_LEN - 1] = 0U;

    BB_PRINT_INFO(" excepid:        [0x%x]\n", e->e_excepid);
    BB_PRINT_INFO(" excepid_end:    [0x%x]\n", e->e_excepid_end);
    BB_PRINT_INFO(" process_pri:    [0x%hhx]\n", e->e_process_priority);
    BB_PRINT_INFO(" reboot_pri:     [0x%hhx]\n", e->e_reboot_priority);
    BB_PRINT_INFO(" notify_core_mk: [0x%llx]\n", e->e_notify_core_mask);
    BB_PRINT_INFO(" reset_core_mk:  [0x%llx]\n", e->e_reset_core_mask);
    BB_PRINT_INFO(" reentrant:      [0x%hhx]\n", e->e_reentrant);
    BB_PRINT_INFO(" exce_type:      [0x%hhx]\n", e->e_exce_type);
    BB_PRINT_INFO(" from_core:      [0x%hhx]\n", e->e_from_core);
    BB_PRINT_INFO(" from_module:    [%s]\n", e->e_from_module);
    BB_PRINT_INFO(" desc:           [%s]\n", e->e_desc);
    BB_PRINT_INFO(" callback:       [%s]\n", (e->e_callback == NULL) ? "NO" : "YES");
}

/*
 * @brief       : set reference count of the exception
 * @param [in]  : u32 excepid       exception id
 * @param [in]  : int is_inc        increase reference
 * @return      : != 0 fail; = 0 success
 */
STATIC s32 rdr_exception_set_reference(u32 excepid, s32 is_inc)
{
    u32 e_excepid;
    lock_flag_t flags = 0;
    struct rdr_exception_node *node = NULL;
    struct list_head *cur = NULL;
    struct list_head *next = NULL;

    e_excepid = bbox_excepid_transf_class(excepid);
    spin_lock_irqsave(&g_rdr_exception_list_lock, flags);
    list_for_each_safe(cur, next, &g_rdr_exception_list) {
        node = (struct rdr_exception_node *)list_entry(cur, struct rdr_exception_node, e_list);
        if (node == NULL) {
            continue;
        }
        if ((node->info.e_excepid <= e_excepid) && (node->info.e_excepid_end >= e_excepid)) {
            if ((is_inc == BBOX_TRUE) && (node->pause == BBOX_TRUE)) {
                spin_unlock_irqrestore(&g_rdr_exception_list_lock, flags);
                BB_PRINT_INFO("exception[0x%x] ready to go offline.\n", e_excepid);
                return BBOX_FAILURE;
            } else if ((is_inc == BBOX_TRUE) && (node->reference < RDR_EXCEPTION_REENTRANT_NUM_MAX)) {
                node->reference++;
            } else if ((is_inc == BBOX_FALSE) && (node->reference > 0)) {
                node->reference--;
            } else {
                spin_unlock_irqrestore(&g_rdr_exception_list_lock, flags);
                BB_PRINT_INFO("exception[0x%x] count(%u) out of range[0 - %d].\n",
                              e_excepid, node->reference, RDR_EXCEPTION_REENTRANT_NUM_MAX);
                return BBOX_FAILURE;
            }
            spin_unlock_irqrestore(&g_rdr_exception_list_lock, flags);
            return BBOX_SUCCESS;
        }
    }
    spin_unlock_irqrestore(&g_rdr_exception_list_lock, flags);
    return BBOX_FAILURE;
}

/*
 * @brief       : increase reference count of the exception
 * @param [in]  : u32 excepid       exception id
 * @return      : != 0 fail; = 0 success
 */
s32 rdr_exception_inc_reference(u32 excepid)
{
    return rdr_exception_set_reference(excepid, BBOX_TRUE);
}

/*
 * @brief       : decrease reference count of the exception
 * @param [in]  : u32 excepid       exception id
 * @return      : NA
 */
void rdr_exception_dec_reference(u32 excepid)
{
    s32 ret = rdr_exception_set_reference(excepid, BBOX_FALSE);
    BB_CHECK_RET(ret != BBOX_SUCCESS, "decrease reference count of the exception[0x%x] failed.\n", excepid);
}

/*
 * @brief       : register exception
 * @param [in]  : struct rdr_exception_node *e    exception node
 * @return      : NA
 */
STATIC void rdr_exception_add_node(struct rdr_exception_node *node)
{
    lock_flag_t flags = 0;

    BB_CHECK_PTR(node, return, "invalid param, node is NULL.\n");
    spin_lock_irqsave(&g_rdr_exception_list_lock, flags);
    list_add_tail(&(node->e_list), &g_rdr_exception_list);
    spin_unlock_irqrestore(&g_rdr_exception_list_lock, flags);
}

/*
 * @brief       : free exception list
 * @return      : NA
 */
STATIC void rdr_exception_free_list(void)
{
    lock_flag_t flags = 0;
    const struct rdr_exception_node *node = NULL;
    struct list_head *cur = NULL;
    struct list_head *next = NULL;

    spin_lock_irqsave(&g_rdr_exception_list_lock, flags);
    list_for_each_safe(cur, next, &g_rdr_exception_list) {
        node = (const struct rdr_exception_node *)list_entry(cur, struct rdr_exception_node, e_list);
        if (node == NULL) {
            continue;
        }
        list_del(cur);
        BBOX_KFREE(node);
    }
    spin_unlock_irqrestore(&g_rdr_exception_list_lock, flags);
}

/*
 * @brief       : rdr exception init
 * @return      : != 0 fail; = 0 success
 */
s32 rdr_exception_init(void)
{
    return BBOX_SUCCESS;
}

/*
 * @brief       : rdr exception exit
 * @return      : NA
 */
void rdr_exception_exit(void)
{
    rdr_exception_free_list();
}

/*
 * @brief       : latest register exception
 * @param [in]  : struct bbox_exception_info *e    exception info
 * @return      : e_excepid
 *                == 0 fail; >0 success
 */
u32 bbox_register_exception(const struct bbox_exception_info *e)
{
    s32 ret;
    u32 excepid_end;
    struct rdr_exception_node *node = NULL;

    BB_CHECK_PTR(e, return 0, "invalid param, e is NULL.\n");
    BB_CHECK_EXP_ACT(!rdr_init_done(), return 0, "rdr hasn't been inited!\n");

    // check excepid & excepid_end region
    excepid_end = e->e_excepid_end;
    if ((e->e_excepid_end == 0) || (e->e_excepid_end < e->e_excepid)) {
        BB_PRINT_INFO("excepid[0x%x ~ 0x%x], but excepid end is invalid. modify excepid_end = [0x%x].\n",
                      e->e_excepid, e->e_excepid_end, e->e_excepid);
        excepid_end = e->e_excepid;
    }

    if (rdr_exception_check_excepid(e->e_excepid, excepid_end) != 0) {
        BB_PRINT_INFO("excepid exist already\n");
        return 0;
    }

    if (rdr_exception_check_info(e) != 0) {
        BB_PRINT_ERR("exception info invalid.\n");
        return 0;
    }

    node = (struct rdr_exception_node*)bbox_kmalloc(sizeof(struct rdr_exception_node));
    if (node == NULL) {
        BB_PRINT_PN("bbox_kmalloc failed for e_tpye_info\n");
        return 0;
    }

    ret = memcpy_s(&node->info, sizeof(struct bbox_exception_info), e, sizeof(struct bbox_exception_info));
    if (ret != EOK) {
        BB_PRINT_ERR("[%s][%4d] memcpy_s failed.\n", __func__, __LINE__);
        BBOX_KFREE(node);
        return 0;
    }

    node->reference = 0;
    node->pause = BBOX_FALSE;
    node->info.e_excepid_end = excepid_end;

    rdr_exception_add_node(node);
    return node->info.e_excepid_end;
}
EXPORT_SYMBOL(bbox_register_exception);

/*
 * @brief       : latest unregister exception
 * @param [in]  : u32 excepid,     exception id;
 * @return      : < 0 fail; ==0 success
 */
s32 bbox_unregister_exception(u32 excepid)
{
    s32 loop = BBOX_FALSE;
    lock_flag_t flags = 0;
    struct rdr_exception_node *node = NULL;
    struct list_head *cur = NULL;
    struct list_head *next = NULL;

    BB_CHECK_EXP_ACT(!rdr_init_done(), return BBOX_FAILURE, "rdr hasn't been inited!\n");
    while (1) {
        spin_lock_irqsave(&g_rdr_exception_list_lock, flags);
        list_for_each_safe(cur, next, &g_rdr_exception_list) {
            node = (struct rdr_exception_node *)list_entry(cur, struct rdr_exception_node, e_list);
            if ((node == NULL) || (excepid < node->info.e_excepid) || (excepid > node->info.e_excepid_end)) {
                continue;
            }

            if (node->reference == 0) {
                list_del(cur);
                BBOX_KFREE(node);
                spin_unlock_irqrestore(&g_rdr_exception_list_lock, flags);
                return BBOX_SUCCESS;
            } else {
                loop = BBOX_TRUE;
                node->pause = BBOX_TRUE;
                break;
            }
        }
        spin_unlock_irqrestore(&g_rdr_exception_list_lock, flags);

        if (loop == BBOX_TRUE) {
            cur = &g_rdr_exception_list;
            next = cur->next;
            msleep(WAIT_MS_MIDDLE);
        } else {
            BB_PRINT_INFO("unregister exception[0x%x] success.\n", excepid);
            break;
        }
    }
    return BBOX_SUCCESS;
}
EXPORT_SYMBOL(bbox_unregister_exception);

