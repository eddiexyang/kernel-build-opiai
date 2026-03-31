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

#include "rdr_interface_mini.h"

#include <linux/securec.h>
#include "bootparam/bbox_boot_param.h"
#include "common/bbox_print.h"
#include "device/bbox_pub_mini.h"
#include "rdr/rdr_common.h"
#include "rdr/rdr_core.h"
#include "rdr/rdr_field_core.h"
#include "rdr/rdr_module_core.h"

// compatible with old interfaces
STATIC struct rdr_module_ops g_rdr_module_ops[BBOX_COREID_MAX];
STATIC u64 g_module_vaddr[BBOX_COREID_MAX] = {0};

/*
 * @brief       : dump callback func.
 * @param [in]  : u32 excepid       exception id
 * @param [in]  : u8 coreid         module id
 * @param [in]  : u8 etype          exception type
 * @param [in]  : excep_time tm     exception time
 * @return      : NA
 */
STATIC void rdr_dump_done(u32 excepid, u8 coreid, u8 etype, excep_time tm)
{
    struct bbox_dump_done_ops_info info;
    info.devid = 0;
    info.excepid = excepid;
    info.coreid = coreid;
    info.etype = etype;
    info.time.tv_sec = tm.tv_sec;
    info.time.tv_usec = tm.tv_usec;
    bbox_dump_done(&info);
}

/*
 * @brief       : module dump interface, compatible with old interfaces
 * @param [in]  : struct bbox_dump_ops_info *info  dump info
 * @param [in]  : bbox_dump_done_ops done  callback function, unused
 * @return      : NA
 */
STATIC void rdr_module_dump(const struct bbox_dump_ops_info *info, bbox_dump_done_ops done)
{
    BB_CHECK_PTR(info, return, "invalid param, info is NULL.\n");
    BB_CHECK_EXP_ACT((info->devid >= bbox_get_device_num()), return, "invalid parameter, dev is %u.\n", info->devid);
    BB_CHECK_EXP_ACT((bbox_check_coreid_valid(info->coreid) != BBOX_SUCCESS), return,
                     "invalid parameter, coreid is %hhu.\n", info->coreid);
    UNUSED(done);

    if (g_rdr_module_ops[info->coreid].ops_dump != NULL) {
        char path[PATH_MAXLEN] = {0};
        g_rdr_module_ops[info->coreid].ops_dump(info->excepid, info->etype, info->coreid, info->time,
                                                (char *)path, rdr_dump_done);
    }
}

/*
 * @brief       : module reset interface, compatible with old interfaces
 * @param [in]  : struct bbox_reset_ops_info *info   module reset info
 * @return      : NA
 */
STATIC void rdr_module_reset(const struct bbox_reset_ops_info *info)
{
    BB_CHECK_PTR(info, return, "invalid param, info is NULL.\n");
    if ((bbox_check_coreid_valid(info->coreid) == BBOX_SUCCESS) && (g_rdr_module_ops[info->coreid].ops_reset != NULL)) {
        g_rdr_module_ops[info->coreid].ops_reset(info->excepid, info->etype, info->coreid);
    }
}

/*
 * @brief       : register module, compatible with old interfaces
 * @param [in]  : u8 core_id                                    core id
 * @param [in]  : const struct rdr_module_ops *ops              module register info
 * @param [out] : struct rdr_register_module_result *retinfo    return info
 * @return      : <0 failure; >=0 success
 */
s32 rdr_register_module_ops(u8 coreid, const struct rdr_module_ops *ops, struct rdr_register_module_result *retinfo)
{
    s32 ret;
    const void *vaddr = NULL;
    struct bbox_module_result info = {0, 0};
    struct bbox_module_info bbox_ops;

    BB_CHECK_PTR(ops, return BBOX_FAILURE, "invalid param, ops is NULL, coreid is %hhu.\n", coreid);
    BB_CHECK_PTR(retinfo, return BBOX_FAILURE, "invalid param, retinfo is NULL, coreid: is %hhu.\n", coreid);
    BB_CHECK_EXP_ACT((bbox_check_coreid_valid(coreid) == BBOX_FAILURE), return BBOX_FAILURE,
        "invalid param, coreid is %hhu.\n", coreid);

    bbox_ops.coreid = coreid;
    bbox_ops.ops_dump = rdr_module_dump;
    bbox_ops.ops_reset = rdr_module_reset;
    ret = bbox_register_module(&bbox_ops, &info);
    if (ret == BBOX_SUCCESS) {
        vaddr = rdr_map(DEFAULT_DEVICE_ID, (phys_addr_t)info.log_addr, info.log_len);
        if (vaddr == NULL) {
            (void)bbox_unregister_module(coreid);
            return BBOX_FAILURE;
        }

        retinfo->log_vaddr = (u64)(uintptr_t)vaddr;
        retinfo->log_len = info.log_len;
        g_module_vaddr[coreid] = retinfo->log_vaddr;
        g_rdr_module_ops[coreid].ops_dump = ops->ops_dump;
        g_rdr_module_ops[coreid].ops_reset = ops->ops_reset;
    }

    return ret;
}
EXPORT_SYMBOL(rdr_register_module_ops);

/*
 * @brief       : unregister module, compatible with old interfaces
 * @param [in]  : u8 coreid     core id;
 * @return      : <0 failure; ==0 success
 */
s32 rdr_unregister_module_ops(u8 coreid)
{
    if ((bbox_check_coreid_valid(coreid) == BBOX_SUCCESS) && (g_module_vaddr[coreid] != 0)) {
        g_rdr_module_ops[coreid].ops_dump = NULL;
        g_rdr_module_ops[coreid].ops_reset = NULL;
        rdr_unmap((void *)(uintptr_t)g_module_vaddr[coreid]);
        g_module_vaddr[coreid] = 0;
        return bbox_unregister_module(coreid);
    }
    return BBOX_FAILURE;
}
EXPORT_SYMBOL(rdr_unregister_module_ops);

/*
 * @brief       : old register exception
 * @param [in]  : struct rdr_exception_info_s *e    exception info
 * @return      : e_excepid
 *                == 0 fail; >0 success
 */
u32 rdr_register_exception(const struct rdr_exception_info_s *e)
{
    s32 ret;
    bbox_exception_info_s convert_e = {0};
    BB_CHECK_PTR(e, return 0, "invalid param, exception info is NULL.\n");

    convert_e.e_excepid = e->e_excepid;
    convert_e.e_excepid_end = e->e_excepid_end;
    convert_e.e_notify_core_mask = e->e_notify_core_mask;
    convert_e.e_reset_core_mask = e->e_reset_core_mask;
    convert_e.e_exce_type = e->e_exce_type;
    convert_e.e_from_core = e->e_from_core;
    convert_e.e_process_priority = e->e_process_priority;
    convert_e.e_reboot_priority = e->e_reboot_priority;
    convert_e.e_reentrant = e->e_reentrant;
    convert_e.e_callback = (bbox_e_callback)e->e_callback;

    ret = memcpy_s(convert_e.e_from_module, BBOX_MODULE_NAME_LEN, e->e_from_module, sizeof(e->e_from_module));
    BB_CHECK_MEMCPY(ret, return 0);

    ret = memcpy_s(convert_e.e_desc, BBOX_EXCEPTIONDESC_MAXLEN, e->e_desc, sizeof(e->e_desc));
    BB_CHECK_MEMCPY(ret, return 0);

    return bbox_register_exception(&convert_e);
}
EXPORT_SYMBOL(rdr_register_exception);

/*
 * @brief       : old unregister exception
 * @param [in]  : u32 excepid,     exception id;
 * @return      : < 0 fail; >=0 success
 */
s32 rdr_unregister_exception(u32 excepid)
{
    return bbox_unregister_exception(excepid);
}
EXPORT_SYMBOL(rdr_unregister_exception);

/*
 * @brief       : get module exception trigger arg.
 * @param [in]  : u32 excepid       exception id
 * @param [in]  : u8 coreid         module id
 * @param [in]  : excep_time tm     exception time
 * @param [out] : u32 *arg          arg
 * @return      : <0: failure; =0: success
 */
s32 rdr_module_dump_get_arg(u32 excepid, u8 coreid, excep_time tm, u32 *arg)
{
    const struct rdr_struct_s *tmp = NULL;

    BB_CHECK_EXP_ACT(!rdr_init_done(), return BBOX_FAILURE, "rdr hasn't been inited!\n");

    if (arg == NULL) {
        BB_PRINT_ERR("get arg, NULL arg!!!\n");
        return BBOX_FAILURE;
    }

    if (rdr_check_curr_excepid(excepid) == BBOX_FALSE) {
        BB_PRINT_ERR("get arg, invalid excepid: 0x%x!!!\n", excepid);
        return BBOX_FAILURE;
    }

    if (rdr_check_curr_coreid(coreid) == BBOX_FALSE) {
        BB_PRINT_ERR("get arg, invalid coreid: 0x%hhx!!!\n", coreid);
        return BBOX_FAILURE;
    }

    // 时间戳比较，保证实时性
    if (rdr_check_curr_time(&tm) == BBOX_FALSE) {
        BB_PRINT_ERR("get arg, invalid time %llu.%llu!!!\n", tm.tv_sec, tm.tv_usec);
        return BBOX_FAILURE;
    }

    tmp = rdr_field_get_pbb();
    *arg = tmp->base_info.arg;
    return BBOX_SUCCESS;
}
EXPORT_SYMBOL(rdr_module_dump_get_arg);

/*
 * @brief       : the old report exception interface
 * @param [in]  : u32 excep_id                  exception id
 * @param [in]  : excep_time timestamp          exception time
 * @param [in]  : u32 arg                       exception arg
 * @return      : NA
 */
void mntn_system_error(u32 excep_id, excep_time timestamp, u32 arg)
{
    struct bbox_report_info info;
    info.devid = 0;
    info.excepid = excep_id;
    info.time = timestamp;
    info.arg = arg;
    (void)bbox_exception_report(&info);
}
EXPORT_SYMBOL(mntn_system_error);
