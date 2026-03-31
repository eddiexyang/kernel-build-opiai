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

#include "rdr_dfx_core.h"
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/securec.h>
#include <linux/securectype.h>
#include "rdr_dump_core.h"
#include "rdr_core.h"
#include "rdr_exception_core.h"
#include "rdr_module_core.h"
#include "common/bbox_print.h"
#include "bbox_inner.h"


/*
 * @brief       : increase reference count
 * @param [in]  : dev_id                        device id
 * @param [in]  : u32 excep_id                  exception id
 * @param [in]  : const excep_time *tm          exception time
 * @param [in]  : u32 arg                       arg
 * @return      : <0 failure; ==0 success
 */
s32 rdr_dfx_add_exception(u32 devid, u32 excepid, const struct excep_time *tm, u32 arg)
{
    u8 coreid;
    s32 ret;

    coreid = bbox_excepid_get_coreid(excepid);
    if (bbox_check_coreid_valid(coreid) == BBOX_FAILURE) {
        BB_PRINT_ERR("invalid coreid : %hhx\n", coreid);
        return BBOX_FAILURE;
    }

    ret = rdr_exception_inc_reference(excepid);
    if (ret != BBOX_SUCCESS) {
        struct rdr_syserr_param_s param;
        if (tm == NULL) {
            RDR_SYSERR_SET_PARAM(param, excepid, devid, arg, 0, 0);
        } else {
            RDR_SYSERR_SET_PARAM(param, excepid, devid, arg, tm->tv_sec, tm->tv_usec * KILO);
        }
        rdr_save_history_log_for_undef_exception(&param);
        return BBOX_FAILURE;
    }

    ret = rdr_module_inc_reference(coreid);
    if (ret != BBOX_SUCCESS) {
        rdr_exception_dec_reference(excepid);
    }

    return ret;
}

/*
 * @brief       : decrease reference count
 * @param [in]  : u32 excepid       exception id
 * @return      : NA
 */
void rdr_dfx_del_exception(u32 excepid)
{
    u8 coreid = bbox_excepid_get_coreid(excepid);
    if (bbox_check_coreid_valid(coreid) == BBOX_FAILURE) {
        return;
    }

    rdr_exception_dec_reference(excepid);
    rdr_module_dec_reference(coreid);
}

STATIC LIST_HEAD(g_rdr_excepid_list);
STATIC DEFINE_SPINLOCK(g_rdr_excepid_list_lock);
/*
 * @brief       : add device error code to excepid list
 * @param [in]  : u32 devid                    device id
 * @param [in]  : u32 excepid                  exception id
 * @return      : <0 failure; ==0 success
 */
s32 rdr_add_device_errorcode(u32 devid, u32 excepid, const struct bbox_exception_info *info)
{
    s32 ret;
    s32 exist = BBOX_FALSE;
    struct list_head *cur = NULL;
    struct list_head *next = NULL;
    struct rdr_excepid_param_s *new_node = NULL;
    struct rdr_excepid_param_s *e_cur = NULL;

    BB_CHECK_PTR(info, return BBOX_FAILURE, "invalid param, info is NULL!\n");
    BB_CHECK_EXP_ACT(devid >= DEVICE_MAX_NUM, return BBOX_FAILURE, "invalid param, device is %u!\n", devid);

    spin_lock(&g_rdr_excepid_list_lock);
    list_for_each_safe(cur, next, &g_rdr_excepid_list) {
        e_cur = (struct rdr_excepid_param_s *)list_entry(cur, struct rdr_excepid_param_s, excepid_list);
        if ((e_cur != NULL) && (e_cur->excepid == excepid)) {
            e_cur->count++;
            e_cur->status[devid] = DEVICE_EXCEPTION;
            exist = BBOX_TRUE;
            break;
        }
    }

    if (exist == BBOX_TRUE) {
        spin_unlock(&g_rdr_excepid_list_lock);
        return BBOX_SUCCESS;
    }

    new_node = (struct rdr_excepid_param_s *)bbox_kmalloc(sizeof(struct rdr_excepid_param_s));
    if (new_node == NULL) {
        spin_unlock(&g_rdr_excepid_list_lock);
        BB_PRINT_ERR("malloc rdr_excepid_param_s failed.\n");
        return BBOX_FAILURE;
    }

    new_node->count = 1;
    new_node->excepid = excepid;
    new_node->status[devid] = DEVICE_EXCEPTION;
    ret = memcpy_s(new_node->desc, BBOX_EXCEPTIONDESC_MAXLEN, info->e_desc, BBOX_EXCEPTIONDESC_MAXLEN);
    list_add_tail(&new_node->excepid_list, &g_rdr_excepid_list);
    spin_unlock(&g_rdr_excepid_list_lock);
    BB_CHECK_RET(ret != EOK, "[%s][%4d] memcpy_s failed.\n", __func__, __LINE__);
    BB_PRINT_INFO("add exception code, excepid:[0x%x].\n", excepid);
    return BBOX_SUCCESS;
}

/*
 * @brief       : del device error code from excepid list
 * @param [in]  : u32 devid                    device id
 * @param [in]  : u32 excepid                  exception id
 * @return      : <0 failure; ==0 success
 */
s32 rdr_del_device_errorcode(u32 devid, u32 excepid)
{
    struct list_head *cur = NULL;
    struct list_head *next = NULL;
    struct rdr_excepid_param_s *e_cur = NULL;

    BB_PRINT_START();
    BB_CHECK_EXP_ACT(devid >= DEVICE_MAX_NUM, return BBOX_FAILURE, "invalid param, device is %u!\n", devid);
    spin_lock(&g_rdr_excepid_list_lock);
    list_for_each_safe(cur, next, &g_rdr_excepid_list) {
        e_cur = (struct rdr_excepid_param_s *)list_entry(cur, struct rdr_excepid_param_s, excepid_list);
        if ((e_cur != NULL) && (e_cur->excepid == excepid)) {
            BB_PRINT_INFO("del excepid:[0x%x].\n", excepid);
            e_cur->status[devid] = DEVICE_NORMAL;
            break;
        }
    }
    spin_unlock(&g_rdr_excepid_list_lock);
    BB_PRINT_END();
    return BBOX_SUCCESS;
}

/*
 * @brief       : free dfx list
 * @return      : NA
 */
STATIC void rdr_dfx_free_list(void)
{
    struct list_head *cur = NULL;
    struct list_head *next = NULL;
    const struct rdr_excepid_param_s *node = NULL;

    spin_lock(&g_rdr_excepid_list_lock);
    list_for_each_safe(cur, next, &g_rdr_excepid_list) {
        node = (const struct rdr_excepid_param_s *)list_entry(cur, struct rdr_excepid_param_s, excepid_list);
        if (node != NULL) {
            list_del(cur);
            BBOX_KFREE(node);
        }
    }
    spin_unlock(&g_rdr_excepid_list_lock);
}

/*
 * @brief       : rdr dfx init
 * @return      : != 0 fail; = 0 success
 */
s32 rdr_dfx_init(void)
{
    return BBOX_SUCCESS;
}

/*
 * @brief       : rdr dfx exit
 * @return      : NA
 */
void rdr_dfx_exit(void)
{
    rdr_dfx_free_list();
}

/*
 * @brief       : get device error code
 * @param [in]  : u32 dev_id                    device id
 * @param [out] : u32 *e_code                   exception code array
 * @param [in]  : u32 e_capacity                array num, max value is 128
 * @return      : >0: error of num
 *                =0: none of error
 *                <0: faile
 */
s32 bbox_get_device_errorcode(u32 dev_id, u32 *e_code, u32 e_capacity)
{
    s32 ret;
    u32 cnt = 0;
    u32 num = DEVICE_ERROR_CODE_NUM_MAX;  // max return 128
    struct list_head *cur = NULL;
    struct list_head *next = NULL;
    const struct rdr_excepid_param_s *e_cur = NULL;

    BB_PRINT_START();
    BB_CHECK_EXP_ACT(!rdr_init_done(), return BBOX_FAILURE, "rdr hasn't been inited!\n");
    BB_CHECK_EXP_ACT(dev_id >= DEVICE_MAX_NUM, return BBOX_FAILURE, "invalid param, device is %u!\n", dev_id);
    BB_CHECK_PTR(e_code, return BBOX_FAILURE, "invalid param, e_code is NULL.\n");
    BB_CHECK_EXP_ACT((e_capacity == 0), return BBOX_FAILURE, "invalid param, e_capacity is %u.\n", e_capacity);

    num = BBOX_MIN(num, e_capacity);
    ret = memset_s(e_code, e_capacity * sizeof(u32), 0, num * sizeof(u32));
    BB_CHECK_MEMSET(ret, return BBOX_FAILURE);

    spin_lock(&g_rdr_excepid_list_lock);
    list_for_each_safe(cur, next, &g_rdr_excepid_list) {
        if (cnt >= num) {
            break;
        }

        e_cur = (const struct rdr_excepid_param_s *)list_entry(cur, struct rdr_excepid_param_s, excepid_list);
        if ((e_cur != NULL) && (e_cur->status[dev_id] == DEVICE_EXCEPTION)) {
            e_code[cnt] = e_cur->excepid;
            cnt++;
        }
    }
    spin_unlock(&g_rdr_excepid_list_lock);
    BB_PRINT_END();
    return (s32)cnt;
}
EXPORT_SYMBOL(bbox_get_device_errorcode);

/*
 * @brief       : get the exception description
 * @param [in]  : u32 ecode             exception id
 * @param [out] : u8 *desc              string array, exception description
 * @param [in]  : u32 length            string array length
 * @return      : <0 failure; ==0 success
 */
s32 bbox_get_device_ecode_info(u32 ecode, u8 *desc, u32 length)
{
    s32 ret;
    struct list_head *cur = NULL;
    struct list_head *next = NULL;
    const struct rdr_excepid_param_s *e_cur = NULL;

    BB_CHECK_EXP_ACT(!rdr_init_done(), return BBOX_FAILURE, "rdr hasn't been inited!\n");
    BB_CHECK_PTR(desc, return BBOX_FAILURE, "invalid param, desc is NULL.\n");
    BB_CHECK_EXP_ACT((length == 0), return BBOX_FAILURE, "invalid param, length is %u.\n", length);
    BB_CHECK_EXP_ACT((bbox_excepid_check(ecode) == false),
        return BBOX_FAILURE, "invalid param, ecode is 0x%x.\n", ecode);

    spin_lock(&g_rdr_excepid_list_lock);
    list_for_each_safe(cur, next, &g_rdr_excepid_list) {
        e_cur = (const struct rdr_excepid_param_s *)list_entry(cur, struct rdr_excepid_param_s, excepid_list);
        if ((e_cur != NULL) && (e_cur->excepid == ecode)) {
            ret = memcpy_s(desc, length, e_cur->desc, BBOX_EXCEPTIONDESC_MAXLEN);
            if (ret != EOK) {
                spin_unlock(&g_rdr_excepid_list_lock);
                BB_PRINT_ERR("[%s][%4d] memcpy_s failed.\n", __func__, __LINE__);
                return BBOX_FAILURE;
            }
            desc[length - 1U] = 0U;
            spin_unlock(&g_rdr_excepid_list_lock);
            return BBOX_SUCCESS;
        }
    }
    spin_unlock(&g_rdr_excepid_list_lock);
    return BBOX_FAILURE;
}
EXPORT_SYMBOL(bbox_get_device_ecode_info);

