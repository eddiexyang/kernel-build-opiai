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

#ifndef BBOX_UT
#include <linux/export.h>
#include "device/bbox_types.h"
#include "device/bbox_pub.h"
#include "device/bbox_pub_cloud.h"
#include "../common/bbox_print.h"
#include "../bbox_inner.h"
#include "../bbox_platform.h"
#include "bbox_agent.h"
#include "bbox_agent_os.h"

s32 bbox_unregister_module(u8 coreid)
{
    return BBOX_SUCCESS;
}
EXPORT_SYMBOL(bbox_unregister_module);

s32 bbox_register_module_ops(u8 core_id,
                             const struct bbox_module_ops *ops,
                             struct bbox_register_module_result *retinfo)
{
    return BBOX_SUCCESS;
}
EXPORT_SYMBOL(bbox_register_module_ops);

s32 bbox_unregister_module_ops(u8 core_id)
{
    return BBOX_SUCCESS;
}
EXPORT_SYMBOL(bbox_unregister_module_ops);

u32 bbox_register_exception(const struct bbox_exception_info *e)
{
    BB_CHECK_PTR(e, return 0, "invalid param, e is NULL.\n");
    return e->e_excepid_end;
}
EXPORT_SYMBOL(bbox_register_exception);

s32 bbox_unregister_exception(u32 excepid)
{
    return BBOX_SUCCESS;
}
EXPORT_SYMBOL(bbox_unregister_exception);

s32 bbox_register_module(const struct bbox_module_info *info, struct bbox_module_result *result)
{
    s32 ret;
    BB_CHECK_PTR(info, return BBOX_FAILURE, "invalid param, module info is NULL.\n");
    BB_CHECK_PTR(result, return BBOX_FAILURE, "invalid param, module result is NULL.\n");
    BB_CHECK_EXP_ACT((bbox_check_coreid_valid(info->coreid) != BBOX_SUCCESS), return BBOX_FAILURE,
        "invalid param. coreid: 0x%hhx.\n", info->coreid);

    ret = bbox_agent_get_areainfo(info->coreid, &result->log_addr, &result->log_len);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("core id[%hhu] no reserve space.\n", info->coreid);
        return BBOX_FAILURE;
    }
    return BBOX_SUCCESS;
}
EXPORT_SYMBOL(bbox_register_module);

/*
 * @brief       : Latest report exception interface
 * @param [in]  : struct bbox_report_info *info  report info
 * @return      : =1: disallow reentrant
 *                =0: success
 *                <0: failure
 */
s32 bbox_exception_report(const struct bbox_report_info *info)
{
    BB_CHECK_PTR(info, return BBOX_FAILURE, "invalid param, report info is NULL.\n");
    BB_CHECK_EXP_ACT((bbox_excepid_check(info->excepid) == false),
                     return BBOX_FAILURE, "invalid param, excepid: 0x%x.\n", info->excepid);

    BB_PRINT_INFO("[device-%u] blackbox receive [%s] exception at %llu.%llu, exception id is [0x%x].",
                  info->devid, bbox_agent_os_get_model_name(info->excepid),
                  info->time.tv_sec, info->time.tv_usec, info->excepid);
    return bbox_agent_msg_proc(info);
}
EXPORT_SYMBOL(bbox_exception_report);

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
#else
int bbox_agent_pub_test(void)
{
    return 0;
}
#endif
