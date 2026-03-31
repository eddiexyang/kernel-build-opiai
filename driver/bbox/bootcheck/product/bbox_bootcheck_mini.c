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

#include "bbox_bootcheck_mini.h"

#include <linux/securec.h>
#include "bootparam/bbox_boot_param.h"
#include "common/bbox_print.h"
#include "device/bbox_pub_mini.h"
#include "rdr/rdr_common.h"
#include "rdr/rdr_dump_core.h"
#include "rdr/rdr_module_core.h"

#define EXCEPID_LPM_HARDWARE_PUM            0xA8192061
#define EXCEPID_LPM_HARDWARE_TSENSOR        0xA8191F0F
#define EXCEPID_OS_PANIC                    0xA8040001
#define EXCEPID_OS_COMM                     0xA6040001
#define EXCEPID_TEE_PANIC                   0xA82A0000
#define EXCEPID_ATF_PANIC                   0xA8340000
#define EXCEPID_OS_REBOOT                   0xA8040000
#define EXCEPID_ABNORMAL_NOTICE             0xA4040003
#define LPM_HARDWARE_DDR(type)              (0xA8190000 + ((u32)DDR_FATAL_EXCEPTION * 0x100) + (type))

STATIC struct bbox_bootcheck_einfo g_bootcheck_einfo[] = {
    {EXCEPID_ABNORMAL_NOTICE,       ABNORMAL_EXCEPTION,     BBOX_OS},
    {EXCEPID_LPM_HARDWARE_TSENSOR,  TSENSOR_EXCEPTION,      BBOX_LPM},
    {EXCEPID_LPM_HARDWARE_PUM,      PMU_EXCEPTION,          BBOX_LPM},
    {0,                             DDR_FATAL_EXCEPTION,    BBOX_LPM},
    {0,                             LPM_EXCEPTION,          BBOX_LPM},
    {EXCEPID_OS_PANIC,              OS_PANIC,               BBOX_OS},
    {EXCEPID_OS_COMM,               OS_COMM,                BBOX_OS},
    {EXCEPID_TEE_PANIC,             TEE_EXCEPTION,          BBOX_TEEOS},
    {EXCEPID_ATF_PANIC,             ATF_EXCEPTION,          BBOX_TF},
    {EXCEPID_OS_REBOOT,             DEVICE_LTO_EXCEPTION,   BBOX_OS},
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
 * @brief       : save lpm3 sram data to host by hdc
 * @param [in]  : u32 devid                 device id
 * @param [in]  : struct bbox_time *time    time stamp
 * @return      : <0 failure; =0 success
 */
s32 bbox_bootcheck_send_sram_data(u32 devid, const struct bbox_time *time)
{
    u8 etype;
    s32 ret;
    u32 excepid;
    char *vaddr = NULL;
    const struct bbox_config_data *config = bbox_get_config();
    struct rdr_exception_msg_info info = {0};

    BB_CHECK_PTR(time, return BBOX_FAILURE, "invalid param, time is NULL.\n");

    vaddr = (char *)bbox_ioremap(config->ddr_mntn.addr, config->ddr_mntn.length);
    if (vaddr == NULL) {
        BB_PRINT_ERR("ioremap lpm sram addr failed.\n");
        return BBOX_FAILURE;
    }

    etype = bbox_get_reboot_type(devid);
    excepid = LPM_HARDWARE_DDR(bbox_get_sub_etype(0));
    RDR_SET_SUBMIT_INFO(info, devid, excepid, BBOX_LPM, etype, *time);
    rdr_save_history_log_for_bootcheck(&info);
    ret = bbox_submit_sram_message(&info, (const char *)vaddr, (u32)config->ddr_mntn.length);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("submit sram message failed\n");
    }

    bbox_iounmap((void *)vaddr);
    return ret;
}

/*
 * @brief       : get tsensor id
 * @param [in]  : char *buffer      buffer
 * @param [in]  : u32 length        buffer size
 * @return      : <0 failure; >=0 success
 */
STATIC s32 bbox_get_tsensor_info(char *buffer, u32 length)
{
    // 返回值为1-4，对应id为0-3, 所以需要减1
    return sprintf_s(buffer, length, "tsensor id:%d.\n", (bbox_get_sub_etype(0) - 1));
}

/*
 * @brief       : save registers data to host
 * @param [in]  : u32 devid                  device id
 * @param [in]  : struct bbox_time *time     time stamp
 * @return      : <0 failure; =0 success
 */
s32 bbox_bootcheck_send_registers_data(u32 devid, const struct bbox_time *time)
{
    u8 etype;
    s32 len = 0;
    s32 ret;
    u32 excepid = 0;
    char *buffer = NULL;
    struct rdr_exception_msg_info info = {0};

    BB_PRINT_START();
    BB_CHECK_PTR(time, return BBOX_FAILURE, "invalid param, time is NULL.\n");

    buffer = (char *)bbox_vmalloc(BUF_LEN_BIG);
    if (buffer == NULL) {
        BB_PRINT_PN("bbox vmalloc registers buffer failed.\n");
        return BBOX_FAILURE;
    }

    etype = bbox_get_reboot_type(devid);
    if (etype == (u8)PMU_EXCEPTION) {
        excepid = EXCEPID_LPM_HARDWARE_PUM;
        len = bbox_get_pmu_info(buffer, BUF_LEN_BIG);
    } else if (etype == (u8)TSENSOR_EXCEPTION) {
        excepid = EXCEPID_LPM_HARDWARE_TSENSOR;
        len = bbox_get_tsensor_info(buffer, BUF_LEN_BIG);
    } else {
        BB_PRINT_ERR("no need to operator: 0x%hhx.\n", etype);
        BBOX_VFREE(buffer);
        return BBOX_FAILURE;
    }

    if (len <= 0) {
        BB_PRINT_ERR("get pmu info failed!\n");
        BBOX_VFREE(buffer);
        return BBOX_FAILURE;
    }
    RDR_SET_SUBMIT_INFO(info, devid, excepid, BBOX_LPM, etype, *time);
    rdr_save_history_log_for_bootcheck(&info);
    if (etype == (u8)PMU_EXCEPTION) {
        ret = bbox_submit_pmu_message(&info, (const char *)buffer, (u32)len);
    } else {
        ret = bbox_submit_tsensor_message(&info, (const char *)buffer, (u32)len);
    }
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("submit registers message failed.\n");
    }

    BBOX_VFREE(buffer);
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

    BB_CHECK_PTR(time, return BBOX_FAILURE, "invalid param, time is NULL.\n");
    ret = rdr_module_get_vaddr(devid, BBOX_TEEOS, &vaddr);
    if ((ret <= 0) || (vaddr == 0)) {
        BB_PRINT_ERR("get module vaddr failed with %d.", ret);
        return BBOX_FAILURE;
    }

    // for mini, length devide by 2, first half for tee
    len = (u32)(ret / 2);
    read_addr = vaddr;

    l_info = (struct rdr_ddr_module_info_s *)(uintptr_t)vaddr;
    if ((l_info->magic == MODULE_MAGIC) &&
        (((u64)l_info->e_info_offset + l_info->e_info_len) <= len)) {
        BB_PRINT_INFO("get module info header, magic: 0x%x, buffer: 0x%x@0x%x, total len: 0x%x.\n",
                      l_info->magic, l_info->e_info_offset, l_info->e_info_len, len);
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
    u8 etype;
    s32 ret;
    u32 len;
    u64 vaddr = 0;
    struct rdr_exception_msg_info info = {0};

    BB_CHECK_PTR(time, return BBOX_FAILURE, "invalid param, time is NULL.\n");
    ret = rdr_module_get_vaddr(devid, BBOX_TEEOS, &vaddr);
    if ((ret <= 0) || (vaddr == 0)) {
        BB_PRINT_ERR("get module vaddr failed with %d.", ret);
        return BBOX_FAILURE;
    }
    // for mini, length divide by 2, tee for first 64k, atf for last 64k
    len = (u32)(ret / 2);
    etype = bbox_get_reboot_type(devid);
    RDR_SET_SUBMIT_INFO(info, devid, EXCEPID_ATF_PANIC, BBOX_TF, etype, *time);
    rdr_save_history_log_for_bootcheck(&info);
    ret = bbox_submit_module_message(&info, (char *)(uintptr_t)(vaddr + len), len);
    BB_CHECK_RET(ret != BBOX_SUCCESS, "bootcheck send atf data failed with ret(%d).", ret);
    rdr_module_free_vaddr(vaddr);
    return ret;
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
    UNUSED(devid);
    UNUSED(notify);
    UNUSED(time);
    UNUSED(data);
    UNUSED(len);
    return BBOX_SUCCESS;
}

