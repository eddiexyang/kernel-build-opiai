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

#include "bbox_bootcheck_cloud.h"

#include "bootparam/bbox_boot_param.h"
#include "common/bbox_print.h"
#include "device/bbox_pub_cloud.h"
#include "rdr/rdr_common.h"
#include "rdr/rdr_dump_core.h"
#include "rdr/rdr_module_core.h"

#define EXCEPID_OS_PANIC                    0xA8040001
#define EXCEPID_OS_COMM                     0xA6040001
#define EXCEPID_TEE_PANIC                   0xA82A0000
#define EXCEPID_LPFW_HBL                    0xA82F3905
#define EXCEPID_LPFW_HARDWARE_PMU           0xA82F2061
#define EXCEPID_ABNORMAL_NOTICE             0xA4040003

STATIC struct bbox_bootcheck_einfo g_bootcheck_einfo[] = {
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
    BB_CHECK_PTR(table, return BBOX_FAILURE, "Invalid param, table is NULL.\n");

    table->list = g_bootcheck_einfo;
    table->size = (u32)(sizeof(g_bootcheck_einfo) / sizeof(struct bbox_bootcheck_einfo));
    return BBOX_SUCCESS;
}

/*
 * @brief       : save ddr mntn data to host
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
 * @brief       : send register info to host;
 *                if pmu ddr buffer contains no valid data, return directly
 * @param [in]  : u32 devid                  device id
 * @param [in]  : struct bbox_time *time     time stamp
 * @return      : <0 failure; =0 success
 */
s32 bbox_bootcheck_send_registers_data(u32 devid, const struct bbox_time *time)
{
    u8 etype;
    s32 ret;
    u32 data_len;
    u64 p_addr;
    struct bbox_lpfw_pmu_info *pmu_info = NULL;
    struct rdr_exception_msg_info info = {0};

    BB_PRINT_START();
    BB_CHECK_PTR(time, return BBOX_FAILURE, "Invalid param, time is NULL.\n");

    etype = bbox_get_reboot_type(devid);
    if (etype != (u8)PMU_EXCEPTION) {
        BB_PRINT_ERR("no need to dump register: 0x%hhx.\n", etype);
        return BBOX_FAILURE;
    }
    p_addr = bbox_get_device_addr(devid, LPFW_PMU_DDR_BASE_ADDRESS);
    pmu_info = (struct bbox_lpfw_pmu_info *)bbox_ioremap(p_addr, LPFW_PMU_DDR_BASE_LEN);
    if (pmu_info == NULL) {
        BB_PRINT_PN("bbox map pmu registers buffer failed.\n");
        return BBOX_FAILURE;
    }

    if (pmu_info->pmu_count == 0) {
        BB_PRINT_INFO("no pmu info in ddr[%u].\n", devid);
        bbox_iounmap(pmu_info);
        pmu_info = NULL;
        return BBOX_SUCCESS;
    } else if (pmu_info->pmu_count > LPFW_PMU_EVENT_MAX) {
        BB_PRINT_ERR("pmu event count(%u) exceed the limit: %d.\n", pmu_info->pmu_count, LPFW_PMU_EVENT_MAX);
        bbox_iounmap(pmu_info);
        pmu_info = NULL;
        return BBOX_FAILURE;
    }
    data_len = (u32)sizeof(pmu_info->pmu_count) + (u32)sizeof(struct pum_event) * pmu_info->pmu_count;
    RDR_SET_SUBMIT_INFO_EX1(info, EXCEPID_LPFW_HARDWARE_PMU, BBOX_LPFW, etype, *time);
    rdr_save_history_log_for_bootcheck(&info);
    ret = bbox_submit_pmu_message(&info, (const char *)pmu_info, data_len);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("submit registers message failed.\n");
    }

    bbox_iounmap(pmu_info);
    pmu_info = NULL;
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
    u8 etype;
    s32 ret;
    u32 len;
    u64 vaddr = 0;
    u64 read_addr;
    struct rdr_exception_msg_info info = {0};
    struct rdr_ddr_module_info_s *l_info = NULL;

    BB_CHECK_PTR(time, return BBOX_FAILURE, "Invalid param, time is NULL.\n");
    ret = rdr_module_get_vaddr(devid, BBOX_TEEOS, &vaddr);
    if ((ret <= 0) || (vaddr == 0)) {
        BB_PRINT_ERR("get module vaddr failed with %d.", ret);
        return BBOX_FAILURE;
    }
    len = (u32)ret;
    read_addr = vaddr;
    l_info = (struct rdr_ddr_module_info_s *)(uintptr_t)vaddr;
    if ((l_info->magic == MODULE_MAGIC) &&
        (((u64)l_info->e_info_offset + l_info->e_info_len) <= len)) {
        read_addr += l_info->e_info_offset;
        len = l_info->e_info_len;
    }

    etype = bbox_get_reboot_type(devid);
    RDR_SET_SUBMIT_INFO(info, devid, EXCEPID_TEE_PANIC, BBOX_TEEOS, etype, *time);
    rdr_save_history_log_for_bootcheck(&info);
    ret = bbox_submit_module_message(&info, (char *)(uintptr_t)read_addr, len);
    BB_CHECK_RET(ret != BBOX_SUCCESS, "bootcheck send tee data failed with ret(%d).", ret);
    rdr_module_free_vaddr(vaddr);
    return ret;
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

