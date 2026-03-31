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

#include "bbox_bootcheck_milan.h"
#include "bootparam/bbox_boot_param.h"
#include "common/bbox_print.h"
#include "device/bbox_pub.h"
#include "rdr/rdr_common.h"
#include "rdr/rdr_dump_core.h"
#include "rdr/rdr_module_core.h"

#define EXCEPID_OS_PANIC                    0xA8040001
#define EXCEPID_OS_COMM                     0xA6040001
#define EXCEPID_TEE_PANIC                   0xA82A0000
#define EXCEPID_LPFW_HBL                    0xA6192D15
#define EXCEPID_LPFW_HARDWARE_PMU           0xA82F2061
#define EXCEPID_ABNORMAL_NOTICE             0xA4040003

struct bbox_bootcheck_einfo g_bootcheck_einfo[] = {
    {EXCEPID_OS_PANIC,              OS_PANIC,               BBOX_OS},
    {EXCEPID_OS_COMM,               OS_COMM,                BBOX_OS},
    {EXCEPID_TEE_PANIC,             TEE_EXCEPTION,          BBOX_TEEOS},
    {EXCEPID_LPFW_HARDWARE_PMU,     PMU_EXCEPTION,          BBOX_LPFW},
    {EXCEPID_ABNORMAL_NOTICE,       ABNORMAL_EXCEPTION,     BBOX_OS},
    {0,                             LPFW_EXCEPTION,         BBOX_LPFW},
};

/*
 * @brief       : get bootcheck exception table
 * @param [out] : struct bbox_bootcheck_exception_table *table          exception table
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_bootcheck_get_einfo(struct bbox_bootcheck_exception_table *table)
{
    BB_CHECK_PTR(table, return BBOX_FAILURE, "invalid param, table is NULL.\n");

    table->list = g_bootcheck_einfo;
    table->size = (u32)(sizeof(g_bootcheck_einfo) / sizeof(struct bbox_bootcheck_einfo));
    return BBOX_SUCCESS;
}

/*
 * @brief       : send ddr sram data to host
 * @param [in]  : u32 devid                 device id
 * @param [in]  : struct bbox_time *time    time stamp
 * @return      : <0 failure; =0 success
 */
s32 bbox_bootcheck_send_sram_data(u32 devid, const struct bbox_time *time)
{
    UNUSED(devid);
    UNUSED(time);
    return BBOX_SUCCESS;
}

/*
 * @brief       : send register info to host
 * @param [in]  : u32 devid                 device id
 * @param [in]  : struct bbox_time *time    time stamp
 * @return      : <0 failure; =0 success
 */
s32 bbox_bootcheck_send_registers_data(u32 devid, const struct bbox_time *time)
{
    UNUSED(devid);
    UNUSED(time);
    return BBOX_SUCCESS;
}

/*
 * @brief       : get tee ddr, send to host.
 * @param [in]  : u32 devid     device id
 * @param [in]  : struct bbox_time *time     time stamp
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_bootcheck_send_tee_data(u32 devid, const struct bbox_time *time)
{
    UNUSED(devid);
    UNUSED(time);
    return BBOX_SUCCESS;
}

/*
 * @brief       : get atf ddr, send to host
 * @param [in]  : u32 devid     device id
 * @param [in]  : struct bbox_time *time     time stamp
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_bootcheck_send_atf_data(u32 devid, const struct bbox_time *time)
{
    UNUSED(devid);
    UNUSED(time);
    return BBOX_SUCCESS;
}

/*
 * @brief       : get sd ddr, send to host
 * @param [in]  : u32 devid     device id
 * @param [in]  : struct bbox_time *time     time stamp
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_bootcheck_send_sd_data(u32 devid, const struct bbox_time *time)
{
    UNUSED(devid);
    UNUSED(time);
    return BBOX_SUCCESS;
}

/*
 * @brief       : send cdr to remote
 * @param [in]  : u32 devid                     device id
 * @param [in]  : node_free_notify_ptr notify   send callback
 * @param [in]  : struct bbox_time *time        timestamp
 * @param [in]  : char *data                    data
 * @param [in]  : u32 len                       data length
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_bootcheck_send_cdr_data(u32 devid, const node_free_notify_ptr notify,
    const struct bbox_time *time, const char *data, u32 len)
{
    struct rdr_exception_msg_info info = {0};
    BB_CHECK_PTR(time, return BBOX_FAILURE, "invalid param, time is NULL.\n");
    BB_CHECK_PTR(data, return BBOX_FAILURE, "invalid param, data is NULL.\n");
    BB_CHECK_EXP_ACT((len == 0), return BBOX_FAILURE, "invalid param, length: %u.\n", len);
    RDR_SET_SUBMIT_INFO(info, devid, EXCEPID_LPFW_HBL, BBOX_LPFW, LPFW_EXCEPTION, *time);
    rdr_save_history_log_for_bootcheck(&info);
    return bbox_submit_cdr_message(&info, notify, data, len);
}

