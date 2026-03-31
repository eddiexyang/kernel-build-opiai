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

#include "rdr_module_core.h"

#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/securec.h>
#include "bootparam/bbox_boot_param.h"
#include "communication/bbox_message.h"
#include "rdr_common.h"
#include "rdr_core.h"
#include "rdr_dump_core.h"
#include "rdr_field_core.h"

STATIC LIST_HEAD(g_rdr_module_ops_list);
STATIC DEFINE_SPINLOCK(g_rdr_module_ops_list_lock);
STATIC DEFINE_SPINLOCK(g_rdr_current_mask_lock);
STATIC u64 g_current_coremk = 0;
STATIC u32 g_current_excepid = 0;
STATIC u32 g_current_devid = 0;
STATIC u64 g_current_mask = 0;
STATIC struct bbox_time g_current_time = {0, 0};
STATIC bool g_rdr_reboot_later_flag = false;

#define OPS_DUMP(ops, devid, coreid, excepid, etype, time, arg) do { \
    struct bbox_dump_ops_info _info = { devid, excepid, coreid, etype, time, arg }; \
    (*((ops)->s_ops.ops_dump))(&_info, bbox_dump_done); \
} while (0)

static inline void bbox_ops_reset(const struct rdr_module_ops_s *ops, u32 devid,
                                  u8 coreid, u32 excepid, u8 etype)
{
    struct bbox_reset_ops_info ops_info;

    BB_CHECK_PTR(ops, return, "invalid parameter, ops is NULL.\n");
    ops_info.devid = devid;
    ops_info.excepid = excepid;
    ops_info.coreid = coreid;
    ops_info.etype = etype;
    (*(ops->s_ops.ops_reset))(&ops_info);
}

/*
 * @brief       : register module
 * @param [in]  : struct rdr_module_ops_s *ops      module register info
 * @return      : NA
 */
STATIC void rdr_module_add_node(struct rdr_module_ops_s *ops)
{
    lock_flag_t lock_flag = 0;
    const struct rdr_module_ops_s *p_info = NULL;
    struct list_head *cur = NULL;
    struct list_head *next = NULL;

    BB_CHECK_PTR(ops, return, "invalid parameter, ops is NULL.\n");

    spin_lock_irqsave(&g_rdr_module_ops_list_lock, lock_flag);
    if (list_empty(&g_rdr_module_ops_list) != 0) {
        list_add_tail(&(ops->s_list), &g_rdr_module_ops_list);
        spin_unlock_irqrestore(&g_rdr_module_ops_list_lock, lock_flag);
        return;
    }

    list_for_each_safe(cur, next, &g_rdr_module_ops_list) {
        p_info = (struct rdr_module_ops_s *)list_entry(cur, struct rdr_module_ops_s, s_list);
        if (p_info == NULL) {
            continue;
        }

        if (ops->s_ops.coreid > p_info->s_ops.coreid) {
            list_add(&(ops->s_list), cur);
            spin_unlock_irqrestore(&g_rdr_module_ops_list_lock, lock_flag);
            return;
        }
    }
    list_add_tail(&(ops->s_list), &g_rdr_module_ops_list);
    spin_unlock_irqrestore(&g_rdr_module_ops_list_lock, lock_flag);
}

/*
 * @brief       : free module list
 * @return      : NA
 */
STATIC void rdr_module_free_list(void)
{
    lock_flag_t lock_flag = 0;
    const struct rdr_module_ops_s *p_info = NULL;
    struct list_head *cur = NULL;
    struct list_head *next = NULL;

    spin_lock_irqsave(&g_rdr_module_ops_list_lock, lock_flag);
    list_for_each_safe(cur, next, &g_rdr_module_ops_list) {
        p_info = (const struct rdr_module_ops_s *)list_entry(cur, struct rdr_module_ops_s, s_list);
        if (p_info == NULL) {
            continue;
        }

        list_del(cur);
        BBOX_KFREE(p_info);
    }
    spin_unlock_irqrestore(&g_rdr_module_ops_list_lock, lock_flag);
}

/*
 * @brief       : check coreid format
 * @param [in]  : u8 core_id,   core id;
 * @return      : ==0 no use; >0 core_id
 */
STATIC u8 rdr_module_check_coreid(u8 core_id)
{
    lock_flag_t lock_flag = 0;
    const struct rdr_module_ops_s *p_module_ops = NULL;
    struct list_head *cur = NULL;
    struct list_head *next = NULL;

    BB_PRINT_START();
    spin_lock_irqsave(&g_rdr_module_ops_list_lock, lock_flag);
    list_for_each_safe(cur, next, &g_rdr_module_ops_list) {
        p_module_ops = (struct rdr_module_ops_s *)list_entry(cur, struct rdr_module_ops_s, s_list);
        if (p_module_ops == NULL) {
            continue;
        }
        if (core_id == p_module_ops->s_ops.coreid) {
            spin_unlock_irqrestore(&g_rdr_module_ops_list_lock, lock_flag);
            BB_PRINT_END();
            return core_id;
        }
    }
    spin_unlock_irqrestore(&g_rdr_module_ops_list_lock, lock_flag);
    BB_PRINT_END();
    return BBOX_UNDEF;
}

/*
 * @brief       : get module vaddr
 * @param [in]  : u32 devid     device id
 * @param [in]  : u8 core_id    core id
 * @param [out] : u64 *vaddr    return vaddr
 * @return      : <=0 failure; >0 module addr size
 */
s32 rdr_module_get_vaddr(u32 devid, u8 coreid, u64 *vaddr)
{
    s32 ret;
    const void *pvaddr = NULL;
    struct bbox_module_result retinfo = {0, 0};

    BB_CHECK_PTR(vaddr, return BBOX_FAILURE, "invalid parameter, vaddr is NULL.\n");
    BB_CHECK_EXP_ACT((devid >= bbox_get_device_num()), return BBOX_FAILURE, "invalid parameter, dev is %u.\n", devid);
    BB_CHECK_EXP_ACT((bbox_check_coreid_valid(coreid) != BBOX_SUCCESS), return BBOX_FAILURE,
        "invalid parameter, coreid is 0x%hhx.\n", coreid);

    ret = rdr_field_get_areainfo(coreid, &retinfo);
    if (ret == BBOX_SUCCESS) {
        pvaddr = rdr_map(devid, (phys_addr_t)retinfo.log_addr, retinfo.log_len);
        if (pvaddr != NULL) {
            *vaddr = (u64)(uintptr_t)pvaddr;
            return (s32)retinfo.log_len;
        }
    }

    return BBOX_FAILURE;
}

/*
 * @brief       : free module virtual address, match rdr_get_module_vaddr
 * @param [in]  : u64 vaddr     virtual address
 * @return      : NA
 */
void rdr_module_free_vaddr(u64 vaddr)
{
    rdr_unmap((void *)(uintptr_t)vaddr);
}

/*
 * @brief       : set reference count of the module
 * @param [in]  : u8 coreid         core id
 * @param [in]  : int is_inc        increase reference
 * @return      : != 0 fail; = 0 success
 */
STATIC s32 rdr_module_set_reference(u8 coreid, s32 is_inc)
{
    lock_flag_t lock_flag = 0;
    struct rdr_module_ops_s *p_info = NULL;
    struct list_head *cur = NULL;
    struct list_head *next = NULL;
    const char *name = rdr_get_exception_core(coreid);

    spin_lock_irqsave(&g_rdr_module_ops_list_lock, lock_flag);
    list_for_each_safe(cur, next, &g_rdr_module_ops_list) {
        p_info = (struct rdr_module_ops_s *)list_entry(cur, struct rdr_module_ops_s, s_list);
        if (p_info == NULL) {
            continue;
        }

        if (coreid == p_info->s_ops.coreid) {
            if ((is_inc == BBOX_TRUE) && (p_info->pause == BBOX_TRUE)) {
                spin_unlock_irqrestore(&g_rdr_module_ops_list_lock, lock_flag);
                BB_PRINT_INFO("module[%s] ready to go offline.\n", name);
                return BBOX_FAILURE;
            } else if ((is_inc == BBOX_TRUE) && (p_info->reference < RDR_MODULE_EXCEPTION_NUM_MAX)) {
                p_info->reference++;
            } else if ((is_inc == BBOX_FALSE) && (p_info->reference > 0)) {
                p_info->reference--;
            } else {
                spin_unlock_irqrestore(&g_rdr_module_ops_list_lock, lock_flag);
                BB_PRINT_INFO("module[%s] exception count(%u) out of range[0 - %d].\n",
                              name, p_info->reference, RDR_MODULE_EXCEPTION_NUM_MAX);
                return BBOX_FAILURE;
            }

            spin_unlock_irqrestore(&g_rdr_module_ops_list_lock, lock_flag);
            return BBOX_SUCCESS;
        }
    }
    spin_unlock_irqrestore(&g_rdr_module_ops_list_lock, lock_flag);
    return BBOX_FAILURE;
}

/*
 * @brief       : increase reference count of the module
 * @param [in]  : u8 coreid         core id
 * @return      : != 0 fail; = 0 success
 */
s32 rdr_module_inc_reference(u8 coreid)
{
    return rdr_module_set_reference(coreid, BBOX_TRUE);
}

/*
 * @brief       : decrease reference count of the module
 * @param [in]  : u8 coreid         core id
 * @return      : NA
 */
void rdr_module_dec_reference(u8 coreid)
{
    const char *name = rdr_get_exception_core(coreid);
    s32 ret = rdr_module_set_reference(coreid, BBOX_FALSE);
    BB_CHECK_RET(ret != BBOX_SUCCESS, "decrease reference count of the module[%s] failed.\n", name);
}

/*
 * @brief       : check the devid is current
 * @param [in]  : u32 devid     device id
 * @return      : currnet: BBOX_TRUE; not: BBOX_FALSE
 */
STATIC u8 rdr_check_curr_devid(u32 devid)
{
    return (u8)((g_current_devid == devid) ? BBOX_TRUE : BBOX_FALSE);
}

/*
 * @brief       : check dump info is current
 * @param [in]  : const struct bbox_dump_done_ops_info *info : dump info
 * @return      : current: BBOX_TRUE; not: BBOX_FALSE
 */
STATIC s32 rdr_check_curr_info(const struct bbox_dump_done_ops_info *info)
{
    if (rdr_check_curr_devid(info->devid) == BBOX_FALSE) {
        BB_PRINT_ERR("dump done: invalid devid: %u!!!\n", info->devid);
        return BBOX_FALSE;
    }

    if (rdr_check_curr_time(&info->time) == BBOX_FALSE) {
        BB_PRINT_ERR("dump done: invalid time: %llu.%llu!!!\n", info->time.tv_sec, info->time.tv_usec);
        return BBOX_FALSE;
    }

    if (rdr_check_curr_excepid(info->excepid) == BBOX_FALSE) {
        BB_PRINT_ERR("dump done: invalid excepid: 0x%x!!!\n", info->excepid);
        return BBOX_FALSE;
    }

    if (rdr_check_curr_coreid(info->coreid) == BBOX_FALSE) {
        BB_PRINT_ERR("dump done: invalid coreid: 0x%hhx!!!\n", info->coreid);
        return BBOX_FALSE;
    }

    return BBOX_TRUE;
}

/*
 * @brief       : dump callback func. when module dump done, call it
 * @param [in]  : const struct bbox_dump_done_ops_info *info    module info
 * @return      : NA
 */
void bbox_dump_done(const struct bbox_dump_done_ops_info *info)
{
    BB_CHECK_PTR(info, return, "invalid param, dump done info is NULL.\n");

    if (rdr_check_curr_info(info) != BBOX_TRUE) {
        return;
    }

    // 需要根据异常原因，特殊处理一些启动异常
    if (!rdr_get_os_status()) {
        // 非ap异常，需要实时传输至hdc
        rdr_save_dump(info->devid, info->coreid, info->excepid, info->etype, &g_current_time);
    }

    spin_lock(&g_rdr_current_mask_lock);
    g_current_mask |= (u64)bbox_coreid_mask(info->coreid);
    spin_unlock(&g_rdr_current_mask_lock);
    BB_PRINT_INFO("dump module data [%s] end! current mask:[0x%llx].\n",
                  rdr_get_exception_core(info->coreid), g_current_mask);
}

/*
 * @brief       : register module
 * @param [in]  : const struct bbox_module_info* ops    module register info
 * @param [out] : struct bbox_module_result* retinfo    return info
 * @return      : <0 failure; ==0 success
 */
s32 bbox_register_module(const struct bbox_module_info *info, struct bbox_module_result *result)
{
    s32 ret;
    struct rdr_module_ops_s *p_module_ops = NULL;

    BB_PRINT_START();
    BB_CHECK_EXP_ACT(!rdr_init_done(), return BBOX_FAILURE, "rdr hasn't been inited!\n");
    BB_CHECK_PTR(info, return BBOX_FAILURE, "invalid param, module info is NULL.\n");
    BB_CHECK_PTR(result, return BBOX_FAILURE, "invalid param, module result is NULL.\n");
    BB_CHECK_EXP_ACT((bbox_check_coreid_valid(info->coreid) != BBOX_SUCCESS), return BBOX_FAILURE,
        "invalid param. coreid: 0x%hhx.\n", info->coreid);

    if ((info->ops_dump == NULL) && (info->ops_reset == NULL)) {
        BB_PRINT_PN("invalid parameter. ops.dump and ops.reset all is NULL.\n");
        return BBOX_FAILURE;
    }

    if (rdr_module_check_coreid(info->coreid) != (u8)BBOX_UNDEF) {
        BB_PRINT_ERR("core id exist already.\n");
        return BBOX_FAILURE;
    }

    ret = rdr_field_get_areainfo(info->coreid, result);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("core id[%hhu] no reserve space.\n", info->coreid);
        return BBOX_FAILURE;
    }

    p_module_ops = (struct rdr_module_ops_s *)bbox_kmalloc(sizeof(struct rdr_module_ops_s));
    if (p_module_ops == NULL) {
        BB_PRINT_PN("kmalloc module ops failed.\n");
        return BBOX_FAILURE;
    }

    p_module_ops->s_ops.coreid = info->coreid;
    p_module_ops->s_ops.ops_dump = info->ops_dump;
    p_module_ops->s_ops.ops_reset = info->ops_reset;
    p_module_ops->reference = 0;
    p_module_ops->pause = BBOX_FALSE;

    rdr_module_add_node(p_module_ops);
    BB_PRINT_INFO("register module[%s] success.\n", rdr_get_exception_core(info->coreid));

    BB_PRINT_END();
    return BBOX_SUCCESS;
}
EXPORT_SYMBOL(bbox_register_module);

/*
 * @brief       : unregister module, new interface
 * @param [in]  : u8 coreid,    core id;
 * @return      : <0 failure; ==0 success
 */
s32 bbox_unregister_module(u8 coreid)
{
    s32 loop = BBOX_FALSE;
    lock_flag_t lock_flag = 0;
    struct rdr_module_ops_s *p_module_ops = NULL;
    struct list_head *cur = NULL;
    struct list_head *next = NULL;

    BB_CHECK_EXP_ACT(!rdr_init_done(), return BBOX_FAILURE, "rdr hasn't been inited!\n");
    while (1) {
        spin_lock_irqsave(&g_rdr_module_ops_list_lock, lock_flag);
        list_for_each_safe(cur, next, &g_rdr_module_ops_list) {
            p_module_ops = (struct rdr_module_ops_s *)list_entry(cur, struct rdr_module_ops_s, s_list);
            if ((p_module_ops == NULL) || (coreid != p_module_ops->s_ops.coreid)) {
                continue;
            }
            if (p_module_ops->reference == 0) {
                list_del(cur);
                BBOX_KFREE(p_module_ops);
                spin_unlock_irqrestore(&g_rdr_module_ops_list_lock, lock_flag);
                BB_PRINT_INFO("unregister module[%s] success.\n", rdr_get_exception_core(coreid));
                return BBOX_SUCCESS;
            } else {
                p_module_ops->pause = BBOX_TRUE;
                loop = BBOX_TRUE;
                break;
            }
        }
        spin_unlock_irqrestore(&g_rdr_module_ops_list_lock, lock_flag);

        if (loop == BBOX_TRUE) {
            cur = &g_rdr_module_ops_list;
            next = cur->next;
            msleep(WAIT_MS_LONG);
        } else {
            BB_PRINT_INFO("the module[%hhu] is not registered, please check.\n", coreid);
            break;
        }
    }

    return BBOX_SUCCESS;
}
EXPORT_SYMBOL(bbox_unregister_module);

/*
 * @brief       : set current info
 * @param [in]  : u32 devid                    device id
 * @param [in]  : u32 excepid                  exception id
 * @param [in]  : u64 coremk                   core id mask, bit setting
 * @param [in]  : const struct bbox_time *tm   time stamp
 * @return      : NA
 */
STATIC void rdr_set_curr_info(u32 devid, u32 excepid, u64 coremk, const struct bbox_time *tm)
{
    g_current_devid = devid;
    g_current_excepid = excepid;
    g_current_coremk = coremk;
    spin_lock(&g_rdr_current_mask_lock);
    g_current_mask = 0;
    spin_unlock(&g_rdr_current_mask_lock);
    g_current_time.tv_sec = ((tm == NULL) ? 0U : tm->tv_sec);
    g_current_time.tv_nsec = ((tm == NULL) ? 0U : tm->tv_nsec);
}

/*
 * @brief       : check the coreid is current
 * @param [in]  : u8 coreid     core id
 * @return      : currnet: BBOX_TRUE; not: BBOX_FALSE
 */
u8 rdr_check_curr_coreid(u8 coreid)
{
    return (u8)(((g_current_coremk & bbox_coreid_mask(coreid)) != 0) ? BBOX_TRUE : BBOX_FALSE);
}

/*
 * @brief       : check the current exception id
 * @param [in]  : u32 excepid       exception id
 * @return      : currnet: BBOX_TRUE; not: BBOX_FALSE
 */
u8 rdr_check_curr_excepid(u32 excepid)
{
    return ((excepid == g_current_excepid) ? BBOX_TRUE : BBOX_FALSE);
}

/*
 * @brief       : check the time is current
 * @param [in]  : const excep_time *tm
 * @return      : current: BBOX_TRUE; not: BBOX_FALSE
 */
u8 rdr_check_curr_time(const excep_time *tm)
{
    if (tm == NULL) {
        return BBOX_FALSE;
    }

    // g_current_time added serial number, is ns
    // time is exception time stamp, is us
    return (u8)(((g_current_time.tv_sec == tm->tv_sec) &&
                ((g_current_time.tv_nsec / KILO) == tm->tv_usec)) ? BBOX_TRUE : BBOX_FALSE);
}

/*
 * @brief       : proxy ddr dump function, update time and submit message
 * @param [in]  : const struct bbox_dump_done_ops_info *info     dump info
 * @param [in]  : u32 excepid                                    actual exception id to report to host
 * @param [in]  : char *addr    module info                      dump addr
 * @param [in]  : u32 len                                        dump addr len
 * @return      : != 0 fail; = 0 success
 */
s32 rdr_module_ddr_dump(const struct bbox_dump_done_ops_info *info, u32 excepid, const char *addr, u32 len)
{
    struct rdr_exception_msg_info msg_info;
    if ((info == NULL) || (rdr_check_curr_info(info) != BBOX_TRUE)) {
        return BBOX_FAILURE;
    }
    bbox_set_msg_info(&msg_info, info->coreid, info->etype, excepid, info->devid);
    bbox_set_msg_time(&msg_info, &g_current_time);
    return bbox_submit_module_message(&msg_info, addr, len);
}

/*
 * @brief       : proxy log dump function, update time and submit message
 * @param [in]  : const struct bbox_dump_done_ops_info *info     dump info
 * @param [in]  : u32 excepid                                    actual exception id to report to host
 * @param [in]  : char *addr    module info                      dump addr
 * @param [in]  : u32 len                                        dump addr len
 * @return      : != 0 fail; = 0 success
 */
s32 rdr_module_log_dump(const struct bbox_dump_done_ops_info *info, u32 excepid, const char *addr, u32 len)
{
    struct rdr_exception_msg_info msg_info;
    if (rdr_check_curr_info(info) != BBOX_TRUE) {
        return BBOX_FAILURE;
    }
    bbox_set_msg_time(&msg_info, &g_current_time);
    bbox_set_msg_info(&msg_info, info->coreid, info->etype, excepid, info->devid);
    return bbox_submit_log_message(&msg_info, addr, len);
}

/*
 * @brief       : get dump done state.
 * @param [in]  : u32 excepid
 * @return      : dump done state.
 */
u64 rdr_get_dump_result(u32 excepid)
{
    if (rdr_check_curr_excepid(excepid) == BBOX_FALSE) {
        BB_PRINT_ERR("invalid excepid :[0x%x]\n", excepid);
        return 0;
    }
    return g_current_mask;
}

/*
 * @brief       : check whether the core mask is valid in mask
 * @param [in]  : u64 mask                                     mask
 * @param [in]  : u64 coremk                                   core mask
 * @return      : true/false
 */
static inline bool bbox_check_mask_valid(u64 mask, u64 coremk)
{
    return ((mask & coremk) != 0);
}

/*
 * @brief       : call module reset function
 * @param [in]  : u32 devid                                     device id
 * @param [in]  : u32 excepid                                   exception id
 * @param [in]  : const struct bbox_exception_info *e_info     exception info
 * @return      : NA
 */
void rdr_notify_module_reset(u32 devid, u32 excepid, const struct bbox_exception_info *e_info)
{
    u64 mask;
    u64 coremk;
    lock_flag_t lock_flag = 0;
    const struct rdr_module_ops_s *p_module_ops = NULL;
    struct list_head *cur = NULL;
    struct list_head *next = NULL;

    BB_CHECK_PTR(e_info, return, "invalid parameter, e_info is NULL.\n");

    mask = e_info->e_reset_core_mask;
    spin_lock_irqsave(&g_rdr_module_ops_list_lock, lock_flag);
    list_for_each_safe(cur, next, &g_rdr_module_ops_list) {
        p_module_ops = (struct rdr_module_ops_s *)list_entry(cur, struct rdr_module_ops_s, s_list);
        if (p_module_ops == NULL) {
            continue;
        }
        coremk = bbox_coreid_mask(p_module_ops->s_ops.coreid);
        // 针对AP复位的特殊处理
        if (p_module_ops->s_ops.coreid == (u8)BBOX_OS) {
            // 如果之前有需要复位异常但因有异常未处理而搁置的复位动作并且当前链表是空的情况下
            // 或者当前异常需要AP复位并且没有异常需要处理做AP复位动作
            if (((g_rdr_reboot_later_flag || bbox_check_mask_valid(mask, coremk)) && rdr_syserr_list_empty())) {
                spin_unlock_irqrestore(&g_rdr_module_ops_list_lock, lock_flag);
                bbox_ops_reset(p_module_ops, devid, e_info->e_from_core, excepid, e_info->e_exce_type);
                spin_lock_irqsave(&g_rdr_module_ops_list_lock, lock_flag);
            } else if (bbox_check_mask_valid(mask, coremk) && !rdr_syserr_list_empty()) {
                g_rdr_reboot_later_flag = true;
            } else {
                ;
            }
        } else if (bbox_check_mask_valid(mask, coremk) && (p_module_ops->s_ops.ops_reset != NULL)) {
            spin_unlock_irqrestore(&g_rdr_module_ops_list_lock, lock_flag);
            BB_PRINT_INFO("reset module [%s] start!\n", rdr_get_exception_core(p_module_ops->s_ops.coreid));
            bbox_ops_reset(p_module_ops, devid, e_info->e_from_core, excepid, e_info->e_exce_type);
            BB_PRINT_INFO("reset module [%s] end!\n", rdr_get_exception_core(p_module_ops->s_ops.coreid));
            spin_lock_irqsave(&g_rdr_module_ops_list_lock, lock_flag);
        } else {
            ;
        }
    }
    spin_unlock_irqrestore(&g_rdr_module_ops_list_lock, lock_flag);
    return;
}

/*
 * @brief       : call module dump function
 * @param [in]  : u32 devid                                     device id
 * @param [in]  : u32 excepid                                   exception id
 * @param [in]  : const struct bbox_time *tm                    time stamp
 * @param [in]  : u32 arg                                       arg
 * @param [in]  : const struct bbox_exception_info *e_info      exception info
 * @return      : coreid mask
 *              : =0 fail; !=0 success,
 */
u64 rdr_notify_module_dump(u32 devid, u32 excepid, const struct bbox_time *tm,
                           u32 arg, const struct bbox_exception_info *e_info)
{
    u64 ret = 0;
    u64 mask;
    u64 coremk;
    lock_flag_t lock_flag = 0;
    const struct rdr_module_ops_s *p_module_ops = NULL;
    struct list_head *cur = NULL;
    struct list_head *next = NULL;
    excep_time etime = {0, 0};

    BB_PRINT_START();
    BB_CHECK_PTR(tm, return 0, "invalid parameter, time is NULL.\n");
    BB_CHECK_PTR(e_info, return 0, "invalid parameter, e_info is NULL.\n");

    mask = e_info->e_notify_core_mask;
    etime.tv_sec = tm->tv_sec;
    etime.tv_usec = tm->tv_nsec / KILO;
    rdr_set_curr_info(devid, excepid, mask, tm);
    spin_lock_irqsave(&g_rdr_module_ops_list_lock, lock_flag);
    list_for_each_safe(cur, next, &g_rdr_module_ops_list) {
        p_module_ops = (struct rdr_module_ops_s *)list_entry(cur, struct rdr_module_ops_s, s_list);
        if (p_module_ops == NULL) {
            continue;
        }
        coremk = bbox_coreid_mask(p_module_ops->s_ops.coreid);

        // First condition: Skip modules which hadn't triggered the exception, to ensure the trigger called firstly .
        // Second condition: Skip the trigger, to avoid calling the dump-callback repeatedly.
        if (((ret == 0) && (e_info->e_from_core != p_module_ops->s_ops.coreid)) ||
            ((ret != 0) && (e_info->e_from_core == p_module_ops->s_ops.coreid))) {
            continue;
        }
        if (bbox_check_mask_valid(mask, coremk) && (p_module_ops->s_ops.ops_dump != NULL)) {
            spin_unlock_irqrestore(&g_rdr_module_ops_list_lock, lock_flag);
            BB_PRINT_INFO("dump module data [%s] start!\n", rdr_get_exception_core(p_module_ops->s_ops.coreid));
            OPS_DUMP(p_module_ops, devid, p_module_ops->s_ops.coreid, excepid, e_info->e_exce_type, etime, arg);
            spin_lock_irqsave(&g_rdr_module_ops_list_lock, lock_flag);
            ret |= coremk;
        }

        // First, to call dump-callback of module which triggered the exception.
        // After that, reset list pointer to the HEAD, and call other modules' callback.
        if (ret == coremk) {
            cur = &g_rdr_module_ops_list;
            next = cur->next;
        }
    }
    spin_unlock_irqrestore(&g_rdr_module_ops_list_lock, lock_flag);

    BB_PRINT_END();
    return ret;
}

/*
 * @brief       : rdr module init
 * @return      : != 0 fail; = 0 success
 */
s32 rdr_module_init(void)
{
    return BBOX_SUCCESS;
}

/*
 * @brief       : rdr module exit
 * @return      : NA
 */
void rdr_module_exit(void)
{
    rdr_module_free_list();
}

