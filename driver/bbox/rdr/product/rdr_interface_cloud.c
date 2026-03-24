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

#include "rdr_interface_cloud.h"

#include <linux/securec.h>
#include "bootparam/bbox_boot_param.h"
#include "common/bbox_print.h"
#include "device/bbox_pub.h"
#include "device/bbox_pub_cloud.h"
#include "rdr/rdr_common.h"
#include "rdr/rdr_module_core.h"

// compatible with old interfaces
STATIC struct bbox_module_ops g_bbox_module_ops[BBOX_COREID_MAX];

/*
 * @brief       : dump callback func. when module dump done, call it
 * @param [in]  : u32 devid                 device id
 * @param [in]  : u8 coreid                 module id
 * @param [in]  : u32 excepid               exception id
 * @param [in]  : u8 etype                  exception type
 * @param [in]  : const excep_time *tm      exception time
 * @return      : NA
 */
STATIC void boox_dump_done_cloud(u32 dev_id, u8 core_id, u32 excep_id, u8 etype, const excep_time *tm)
{
    struct bbox_dump_done_ops_info info = {0};
    BB_CHECK_PTR(tm, return, "invalid param, time is NULL.\n");

    info.devid = dev_id;
    info.excepid = excep_id;
    info.coreid = core_id;
    info.etype = etype;
    info.time = *tm;
    bbox_dump_done(&info);
}

/*
 * @brief       : module dump interface, compatible with old interfaces
 * @param [in]  : struct bbox_dump_ops_info *info  dump info
 * @param [in]  : bbox_dump_done_ops done  callback function, unused
 * @return      : NA
 */
STATIC void bbox_module_dump(const struct bbox_dump_ops_info *info, bbox_dump_done_ops done)
{
    BB_CHECK_PTR(info, return, "invalid param, info is NULL.\n");
    BB_CHECK_EXP_ACT((info->devid >= bbox_get_device_num()), return, "invalid parameter, dev is %u.\n", info->devid);
    BB_CHECK_EXP_ACT((bbox_check_coreid_valid(info->coreid) != BBOX_SUCCESS), return,
                     "invalid parameter, coreid is %hhu.\n", info->coreid);
    UNUSED(done);

    if (g_bbox_module_ops[info->coreid].ops_dump != NULL) {
        g_bbox_module_ops[info->coreid].ops_dump(info->devid, info->coreid, info->excepid,
                                                 info->etype, &info->time, info->arg, boox_dump_done_cloud);
    }
}

/*
 * @brief       : module reset interface, compatible with old interfaces
 * @param [in]  : struct bbox_reset_ops_info *info    module reset info
 * @return      : NA
 */
STATIC void bbox_module_reset(const struct bbox_reset_ops_info *info)
{
    BB_CHECK_PTR(info, return, "invalid param, info is NULL.\n");

    if ((bbox_check_coreid_valid(info->coreid) == BBOX_SUCCESS) &&
        (g_bbox_module_ops[info->coreid].ops_reset != NULL)) {
        g_bbox_module_ops[info->coreid].ops_reset(info->devid, info->coreid, info->excepid, info->etype);
    }
}

/*
 * @brief       : register module
 * @param [in]  : u8 core_id                                    core id
 * @param [in]  : const struct bbox_module_ops* ops             module register info
 * @param [out] : struct bbox_register_module_result* retinfo   return info
 * @return      : <0 failure; >=0 success
 */
s32 bbox_register_module_ops(u8 core_id,
                             const struct bbox_module_ops *ops,
                             struct bbox_register_module_result *retinfo)
{
    s32 ret;
    struct bbox_module_result info = {0, 0};
    struct bbox_module_info bbox_ops;

    BB_CHECK_PTR(ops, return BBOX_FAILURE, "invalid param, ops is NULL, coreid is %hhu.\n", core_id);
    BB_CHECK_PTR(retinfo, return BBOX_FAILURE, "invalid param, retinfo is NULL, coreid is %hhu.\n", core_id);
    BB_CHECK_EXP_ACT((bbox_check_coreid_valid(core_id) == BBOX_FAILURE), return BBOX_FAILURE,
        "invalid param, coreid is %hhu.\n", core_id);

    bbox_ops.coreid = core_id;
    bbox_ops.ops_dump = bbox_module_dump;
    bbox_ops.ops_reset = bbox_module_reset;
    ret = bbox_register_module(&bbox_ops, &info);
    if (ret == BBOX_SUCCESS) {
        retinfo->log_addr = info.log_addr;
        retinfo->log_len = info.log_len;
        g_bbox_module_ops[core_id].ops_dump = ops->ops_dump;
        g_bbox_module_ops[core_id].ops_reset = ops->ops_reset;
    }

    return ret;
}
EXPORT_SYMBOL(bbox_register_module_ops);

/*
 * @brief       : unregister module
 * @param [in]  : u8 core_id,    core id;
 * @return      : <0 failure; ==0 success
 */
s32 bbox_unregister_module_ops(u8 core_id)
{
    return bbox_unregister_module(core_id);
}
EXPORT_SYMBOL(bbox_unregister_module_ops);

/*
 * @brief       : Latest report exception interface
 * @param [in]  : u32 dev_id                    device id
 * @param [in]  : u32 excep_id                  exception id
 * @param [in]  : const excep_time *timestamp   exception time
 * @param [in]  : u32 arg                       exception arg
 * @return      : NA
 */
void bbox_system_error(u32 dev_id, u32 excep_id, const excep_time *timestamp, u32 arg)
{
    struct bbox_report_info info = {0};
    BB_CHECK_PTR(timestamp, return, "invalid param, timestamp is NULL.\n");

    info.devid = dev_id;
    info.excepid = excep_id;
    info.time = *timestamp;
    info.arg = arg;
    (void)bbox_exception_report(&info);
    return;
}
EXPORT_SYMBOL(bbox_system_error);
