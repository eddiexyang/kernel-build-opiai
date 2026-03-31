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

#include "bbox_bootcheck_mdc.h"

#include "bootparam/bbox_boot_param.h"
#include "common/bbox_print.h"
#include "device/bbox_pub.h"
#include "rdr/rdr_common.h"
#include "rdr/rdr_dump_core.h"
#include "rdr/rdr_module_core.h"

#define EXCEPID_OS_PANIC                    0xA8040001U
#define EXCEPID_OS_COMM                     0xA6040001U
#define EXCEPID_ATF_PANIC                   0xA8340000U
#define EXCEPID_SD_PANIC                    0xA8400000U
#define EXCEPID_LPM_HARDWARE_PUM            0xA8192061U

STATIC struct bbox_bootcheck_einfo g_bootcheck_einfo[] = {
    {EXCEPID_OS_PANIC,              OS_PANIC,               BBOX_OS},
    {EXCEPID_OS_COMM,               OS_COMM,                BBOX_OS},
};

static inline u32 bbox_bootcheck_lpm_get_excepid(u8 type)
{
    const u32 basic = 0xA8190000U; // excepid basic
    const u32 offset = 0x100;
    return (basic + ((u32)DDR_FATAL_EXCEPTION * offset) + type);
}

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
 * @param [in]  : struct bbox_time *tm      time stamp
 * @return      : <0 failure; =0 success
 */
s32 bbox_bootcheck_send_sram_data(u32 devid, const struct bbox_time *tm)
{
    u8 etype;
    s32 ret;
    u32 excepid;
    const char *vaddr = NULL;
    const struct bbox_config_data *config = bbox_get_config();
    struct rdr_exception_msg_info info = {0};

    BB_PRINT_START();
    BB_CHECK_PTR(tm, return BBOX_FAILURE, "Invalid param, time is NULL.\n");

    if (devid >= bbox_get_device_num()) {
        BB_PRINT_ERR("invalid device id: %u.\n", devid);
        return BBOX_FAILURE;
    }

    vaddr = (const char *)bbox_ioremap(bbox_get_device_addr(devid, config->ddr_mntn.addr),
                                       (u32)config->ddr_mntn.length);
    if (vaddr == NULL) {
        BB_PRINT_ERR("map sram addr failed.\n");
        return BBOX_FAILURE;
    }

    etype = bbox_get_reboot_type(devid);
    excepid = bbox_bootcheck_lpm_get_excepid(bbox_get_sub_etype(0));
    RDR_SET_SUBMIT_INFO(info, devid, excepid, BBOX_LPM, etype, *tm);
    rdr_save_history_log_for_bootcheck(&info);
    ret = bbox_submit_sram_message(&info, vaddr, (u32)config->ddr_mntn.length);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("submit sram message failed\n");
    }

    bbox_iounmap((const void *)vaddr);
    BB_PRINT_END();
    return ret;
}

/*
 * @brief       : send register info to host
 * @param [in]  : u32 devid                 device id
 * @param [in]  : struct bbox_time *tm      time stamp
 * @return      : <0 failure; =0 success
 */
s32 bbox_bootcheck_send_registers_data(u32 devid, const struct bbox_time *tm)
{
    u8 etype;
    s32 ret;
    const char *vaddr = NULL;
    struct rdr_exception_msg_info info = {0};

    BB_PRINT_START();
    BB_CHECK_PTR(tm, return BBOX_FAILURE, "Invalid param, time is NULL.\n");

    if (devid >= bbox_get_device_num()) {
        BB_PRINT_ERR("invalid device id: %u.\n", devid);
        return BBOX_FAILURE;
    }

    vaddr = (const char *)bbox_ioremap(bbox_get_device_addr(devid, LPM_PMU_DDR_BASE_ADDRESS),
                                       LPM_PMU_DDR_BASE_LEN);
    if (vaddr == NULL) {
        BB_PRINT_ERR("map reg addr failed.\n");
        return BBOX_FAILURE;
    }

    etype = bbox_get_reboot_type(devid);
    RDR_SET_SUBMIT_INFO(info, devid, EXCEPID_LPM_HARDWARE_PUM, BBOX_LPM, etype, *tm);
    rdr_save_history_log_for_bootcheck(&info);
    ret = bbox_submit_pmu_message(&info, vaddr, LPM_PMU_DDR_BASE_LEN);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("submit reg message failed\n");
    }

    bbox_iounmap((void *)vaddr);
    BB_PRINT_END();
    return ret;
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
    struct rdr_exception_msg_info info = {0};
    BB_CHECK_PTR(time, return BBOX_FAILURE, "invalid param, time is NULL.\n");

    RDR_SET_SUBMIT_INFO(info, devid, EXCEPID_ATF_PANIC, BBOX_TF, ATF_EXCEPTION, *time);
    rdr_save_history_log_for_bootcheck(&info);
    return bbox_bootcheck_get_proxy_module(&info);
}

/*
 * @brief       : get sd ddr, send to host
 * @param [in]  : u32 devid     device id
 * @param [in]  : struct bbox_time *tm      time stamp
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_bootcheck_send_sd_data(u32 devid, const struct bbox_time *tm)
{
    s32 ret;
    u32 len;
    u64 vaddr = 0;
    struct rdr_exception_msg_info info = {0};

    BB_CHECK_PTR(tm, return BBOX_FAILURE, "Invalid param, time is NULL.\n");
    ret = rdr_module_get_vaddr(devid, BBOX_AOS_SD, &vaddr);
    if ((ret <= 0) || (vaddr == 0)) {
        BB_PRINT_ERR("get module vaddr failed with %d.", ret);
        return BBOX_FAILURE;
    }

    len = (u32)ret;
    RDR_SET_SUBMIT_INFO(info, devid, EXCEPID_SD_PANIC, BBOX_AOS_SD, SD_EXCEPTION, *tm);
    rdr_save_history_log_for_bootcheck(&info);
    ret = bbox_submit_module_message(&info, (char *)(uintptr_t)vaddr, len);
    BB_CHECK_RET(ret != BBOX_SUCCESS, "bootcheck send sd data failed with ret(%d).", ret);
    rdr_module_free_vaddr(vaddr);
    return ret;
}

/*
 * @brief       : send cdr to remote
 * @param [in]  : u32 devid                     device id
 * @param [in]  : node_free_notify_ptr notify   send callback
 * @param [in]  : struct bbox_time *tm          timestamp
 * @param [in]  : char *data                    data
 * @param [in]  : u32 len                       data length
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_bootcheck_send_cdr_data(u32 devid, const node_free_notify_ptr notify,
    const struct bbox_time *tm, const char *data, u32 len)
{
    UNUSED(devid);
    UNUSED(notify);
    UNUSED(tm);
    UNUSED(data);
    UNUSED(len);
    return BBOX_SUCCESS;
}

