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

#include "bbox_bootcheck.h"

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/syscalls.h>
#include <linux/securec.h>
#include "bootparam/bbox_boot_param.h"
#include "communication/bbox_channel_status.h"
#include "communication/bbox_communication.h"
#include "communication/bbox_message.h"
#include "common/bbox_sys_api.h"
#include "config/bbox_config.h"
#include "device/bbox_pub.h"
#include "device/bbox_pub_cloud.h"
#include "hdr_bootcheck.h"
#include "cdr_bootcheck.h"
#include "os_adapter/bbox_adapter.h"
#include "rdr/rdr_common.h"
#include "rdr/rdr_dump_core.h"
#include "rdr/rdr_field_core.h"
#include "rdr/rdr_module_core.h"
#include "register/bbox_register.h"
#include "bbox_inner.h"
#include "bbox_common.h"

STATIC struct bbox_bootcheck_start_info g_start_info = { { 0, 0 }, 0 };

/*
 * @brief       : get start up time
 * @return      : NA
 */
STATIC void bbox_bootcheck_start_info(void)
{
    char date[DATATIME_MAXLEN] = {0};
    struct bbox_time tm = {0, 0};

    bbox_get_monotonic_clock(&tm);
    bbox_get_systime(&g_start_info.start);
    g_start_info.start.tv_sec  = (u64)((g_start_info.start.tv_sec > (u64)tm.tv_sec) ?
                                       (g_start_info.start.tv_sec - (u64)tm.tv_sec) : 0U);

    bbox_get_date(&g_start_info.start, date, DATATIME_MAXLEN);
    g_start_info.time_seq = bbox_get_time_seq();
    BB_PRINT_INFO("start time: %s UTC.\n", date);
    return;
}

/*
 * @brief       : get dump type
 * @param [in]  : u32 devid         device id
 * @param [out] : u8 *dump_type     dump type
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_bootcheck_get_dump_type(u32 devid, u8 *dump_type)
{
    u8 reboot_type;

    BB_CHECK_EXP_ACT((devid >= bbox_get_device_num()), return BBOX_FAILURE,
        "invalid parameter, devid is %u.\n", devid);
    BB_CHECK_PTR(dump_type, return BBOX_FAILURE, "invalid parameter, dump_type is NULL.\n");

    reboot_type = bbox_get_reboot_type(devid);
    switch (reboot_type) {
        case DEVICE_COLDBOOT:
        case ABNORMAL_EXCEPTION:
        case DEVICE_LTO_EXCEPTION:
            *dump_type = (u8)BBOX_FETCH_NULL;
            break;
        case PMU_EXCEPTION:
            *dump_type = (u8)BBOX_FETCH_REGISTERS;
            break;
        case TSENSOR_EXCEPTION:
            *dump_type = (u8)((u8)BBOX_FETCH_REGISTERS | (u8)BBOX_FETCH_DDR);
            break;
        case DDR_FATAL_EXCEPTION:
            *dump_type = (u8)BBOX_FETCH_SRAM;
            break;
        case TEE_EXCEPTION:
            *dump_type = (u8)BBOX_FETCH_TEE;
            break;
        case ATF_EXCEPTION:
            *dump_type = (u8)BBOX_FETCH_ATF;
            break;
        case SD_EXCEPTION:
            *dump_type = (u8)BBOX_FETCH_SD;
            break;
        default:
            *dump_type = (u8)BBOX_FETCH_DDR;
            break;
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : save lpfw sram data to host by hdc
 * @param [in]  : u32 devid                  device id
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_bootcheck_send_sram(u32 devid)
{
    struct bbox_time tm;
    tm.tv_sec = g_start_info.start.tv_sec;
    tm.tv_nsec = g_start_info.start.tv_nsec + g_start_info.time_seq;
    return bbox_bootcheck_send_sram_data(devid, &tm);
}

/*
 * @brief       : send register info to host;
 *                if pmu ddr buffer contains no valid data, return directly
 * @param [in]  : u32 devid                  device id
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_bootcheck_send_registers(u32 devid)
{
    struct bbox_time tm;
    tm.tv_sec = g_start_info.start.tv_sec;
    tm.tv_nsec = g_start_info.start.tv_nsec + g_start_info.time_seq;
    return bbox_bootcheck_send_registers_data(devid, &tm);
}

/*
 * @brief       : compose device-x last bbox ddr
 * @param [in]  : u32 devid             device id
 * @param [in]  : char *data            date buffer
 * @param [in]  : u32 data_len          data length
 * @return      : hdc buffer addr
 */
STATIC s32 bbox_bootcheck_compose_bbox_data(u32 devid, char *data, u32 data_len)
{
    s32 i;
    s32 ret;
    u64 offset;
    u64 md = 0;
    u64 reserved = 0;
    const struct rdr_struct_s *tmp = NULL;
    struct bbox_module_result retinfo = {0, 0};

    BB_CHECK_PTR(data, return BBOX_FAILURE, "invalid param, data is NULL.\n");
    BB_CHECK_EXP_ACT(data_len == 0, return BBOX_FAILURE, "invalid param, length:%u, devid:%u.\n", data_len, devid);

    ret = bbox_config_get_rdr(&reserved, NULL);
    BB_CHECK_EXP_ACT(ret != BBOX_SUCCESS, return BBOX_FAILURE, "get rdr phymem addr failed.\n");

    // copy rdr head
    tmp = rdr_field_get_tmppbb();
    ret = memcpy_s(data, data_len, tmp, RDR_BASEINFO_SIZE);
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);

    for (i = 0; i < (s32)BBOX_AREA_MAXIMUM; i++) {
        ret = rdr_field_get_areainfo(tmp->area_info[i].coreid, &retinfo);
        if (ret == BBOX_SUCCESS) {
            offset = ((retinfo.log_addr >= reserved) ? (retinfo.log_addr - reserved) : 0U);
            BB_CHECK_EXP_ACT(offset >= data_len, return BBOX_FAILURE,
                             "offset[%pK] of area %d out of range.\n", (void *)(uintptr_t)offset, i);
            if (tmp->area_info[i].coreid == (u8)BBOX_OS) {
                // copy ap area
                ret = memcpy_s(&data[offset], (u32)(data_len - offset), &((const char *)tmp)[offset], retinfo.log_len);
                BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);
            }

            // copy module area
            ret = rdr_module_get_vaddr(devid, tmp->area_info[i].coreid, &md);
            if (ret > 0) {
                ret = memcpy_s(&data[offset], (u32)(data_len - offset), (void *)(uintptr_t)md, retinfo.log_len);
                rdr_module_free_vaddr(md);
                md = 0;
                BB_CHECK_EXP_ACT(ret != EOK, return BBOX_FAILURE,
                    "memcpy_s area(%hhu) failed.\n", tmp->area_info[i].coreid);
            }
        }
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : save last bbox ddr to host
 * @param [in]  : u32 devid                  device id
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_bootcheck_send_bbox(u32 devid)
{
    u32 len;
    char *data = NULL;
    const struct rdr_struct_s *tmp = NULL;
    struct rdr_exception_msg_info info = {0};

    len = (u32)rdr_field_get_pbb_size();
    tmp = rdr_field_get_tmppbb();
    if ((tmp->top_head.magic != FILE_MAGIC) || (bbox_excepid_check(tmp->base_info.excepid) == false)) {
        BB_PRINT_INFO("[device-%u] no have exception data, reason is %u, magic = 0x%x, excepId = 0x%x.\n",
            devid, bbox_get_reboot_type(devid), tmp->top_head.magic, tmp->base_info.excepid);
        return BBOX_SUCCESS;
    } else {
        BB_PRINT_WARN("[device-%u] has exception. (excepId=0x%x, coreExcepId=0x%x).\n",
                      devid, tmp->base_info.excepid, tmp->core_base_info.excepid);
        if (tmp->core_base_info.excepid != 0) {
            info.coreid = tmp->core_base_info.e_core;
            info.excepid = tmp->core_base_info.excepid;
            info.tm.tv_sec = tmp->core_base_info.tm.tv_sec;
            info.tm.tv_nsec = tmp->core_base_info.tm.tv_nsec;
            info.etype = tmp->core_base_info.e_type;
            info.devid = devid;
        } else {
            info.coreid = tmp->base_info.e_core;
            info.excepid = tmp->base_info.excepid;
            info.tm.tv_sec = tmp->base_info.tm.tv_sec;
            info.tm.tv_nsec = tmp->base_info.tm.tv_nsec;
            info.etype = tmp->base_info.e_type;
            info.devid = devid;
        }
    }

    if (devid == DEFAULT_DEVICE_ID) {
        // device-0 just send ddr of device-0
        return bbox_submit_ddr_message(&info, (const char *)tmp, len);
    } else {
        s32 ret;
        data = (char *)bbox_vmalloc(len);
        if (data == NULL) {
            BB_PRINT_ERR("get buffer failed. excepid[0x%x], coreid[%hhu], len[%u].\n",
                         info.excepid, info.coreid, len);
            return BBOX_FAILURE;
        }
        ret = bbox_bootcheck_compose_bbox_data(devid, data, len);
        if (ret != BBOX_SUCCESS) {
            BB_PRINT_ERR("compose last baseinfo failed.\n");
            bbox_vfree(data);
            return BBOX_FAILURE;
        }

        ret = bbox_submit_ddr_message(&info, (const char *)data, len);
        bbox_vfree(data);
        return ret;
    }
}

/*
 * @brief       : send bios excption to host
 * @param [in]  : const struct exc_module_info_s *info  excption info
 * @param [in]  : u32 devid                             device id
 * @param [in]  : const struct bbox_time *tm            time stamp
 * @param [in]  : const char *data                      excetion data
 * @param [in]  : u32 len                               data length
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_bootcheck_send_bios_data(const struct exc_module_info_s *info,
                                         u32 devid, const struct bbox_time *tm,
                                         const char *data, u32 len)
{
    struct rdr_exception_msg_info msg_info = {0};

    BB_CHECK_PTR(info, return BBOX_FAILURE, "invalid param, operate bios data, info is NULL.\n");
    BB_CHECK_PTR(data, return BBOX_FAILURE, "invalid param, operate bios data, data is NULL.\n");
    BB_CHECK_PTR(tm, return BBOX_FAILURE, "invalid param, operate bios data, time is NULL.\n");

    if (((u64)info->e_info_offset + (u64)info->e_info_len) <= (u64)len) {
        msg_info.coreid = BBOX_BIOS;
        msg_info.etype = BIOS_EXCEPTION;
        msg_info.excepid = info->cur_info.e_excepid;
        msg_info.devid = devid;
        msg_info.tm.tv_sec = tm->tv_sec;
        msg_info.tm.tv_nsec = tm->tv_nsec;
        rdr_save_history_log_for_bootcheck(&msg_info);
        return bbox_submit_module_message(&msg_info, &data[info->e_info_offset], info->e_info_len);
    }

    BB_PRINT_ERR("param invalid! offset[%u], len[%u].\n", info->e_info_offset, info->e_info_len);
    return BBOX_FAILURE;
}

/*
 * @brief       : check bios stage data
 * @param [in]  : u32 devid         device id
 * @param [in]  : char *data        data
 * @param [in]  : u32 length        data length
 * @return      : NA
 */
STATIC void bbox_bootcheck_check_bios_stage(u32 devid, const char *data, u32 length)
{
    s32 ret;
    struct bbox_time tm = {0, 0};
    struct exc_module_info_s *info = (struct exc_module_info_s *)data;

    BB_CHECK_EXP_CTRL(BB_PRINT_INFO, (info->magic != MODULE_MAGIC), return,
        "[device-%u] bios no has exception data, magic(%u).\n", devid, info->magic);
    BB_CHECK_EXP_CTRL(BB_PRINT_INFO, (info->e_excep_valid != MODULE_VALID), return,
        "[device-%u] bios no has exception data, valid(%hu).\n", devid, info->e_excep_valid);
    BB_CHECK_EXP_CTRL(BB_PRINT_INFO, bbox_excepid_get_coreid(info->cur_info.e_excepid) != (u8)BBOX_BIOS, return,
        "[device-%u] bios no has exception data, excepid(%u).\n", devid, info->cur_info.e_excepid);

    tm.tv_sec = g_start_info.start.tv_sec;
    tm.tv_nsec = g_start_info.start.tv_nsec;
    bbox_update_time_seq(&tm);

    ret = bbox_bootcheck_send_bios_data(info, devid, &tm, data, length);
    BB_CHECK_RET(ret != BBOX_SUCCESS, "[device-%u] bootcheck send bios data failed with ret(%d).\n", devid, ret);
}

STATIC bool bbox_bootcheck_feature_bios_stage(void)
{
    return bbox_check_feature(FEATURE_BOOTCHECK_BIOS_STAGE);
}

/*
 * @brief       : check bios excption
 * @return      : NA
 */
STATIC void bbox_bootcheck_check_bios_data(void)
{
    u32 i;

    if (bbox_bootcheck_feature_bios_stage() == false) {
        return;
    }

    for (i = 0; i < bbox_get_device_num(); i++) {
        u64 data_vaddr = 0;
        s32 data_length = rdr_module_get_vaddr(i, BBOX_BIOS, &data_vaddr);
        if ((data_length <= 0) || (data_vaddr == 0)) {
            BB_PRINT_ERR("get bios area info failed with ret(%d).", data_length);
            return;
        }

        bbox_bootcheck_check_bios_stage(i, (char *)(uintptr_t)data_vaddr, (u32)data_length);
        rdr_module_free_vaddr(data_vaddr);
        data_vaddr = 0;
    }
}

/*
 * @brief       : get tee ddr, send to host.
 * @param [in]  : u32 devid     device id
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_bootcheck_send_tee(u32 devid)
{
    struct bbox_time tm;
    tm.tv_sec = g_start_info.start.tv_sec;
    tm.tv_nsec = g_start_info.start.tv_nsec + g_start_info.time_seq;
    return bbox_bootcheck_send_tee_data(devid, &tm);
}

/*
 * @brief       : get atf ddr, send to host
 * @param [in]  : u32 devid     device id
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_bootcheck_send_atf(u32 devid)
{
    struct bbox_time tm;
    tm.tv_sec = g_start_info.start.tv_sec;
    tm.tv_nsec = g_start_info.start.tv_nsec + g_start_info.time_seq;
    return bbox_bootcheck_send_atf_data(devid, &tm);
}

/*
 * @brief       : get sd ddr, send to host
 * @param [in]  : u32 devid     device id
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_bootcheck_send_sd(u32 devid)
{
    struct bbox_time tm;
    tm.tv_sec = g_start_info.start.tv_sec;
    tm.tv_nsec = g_start_info.start.tv_nsec + g_start_info.time_seq;
    return bbox_bootcheck_send_sd_data(devid, &tm);
}

/*
 * @brief       : get kernel log and bbox ddr, send to host, used minirc
 * @param [in]  : u64 addr          devmem addr
 * @param [in]  : u32 len           devmem size
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_bootcheck_send_ddrdump(u64 addr, u32 len)
{
    s32 i, ret;
    u8 etype;
    u8 core_id;
    u32 exceptId;
    const char *vaddr = NULL;
    const struct ddr_bios_hand_s *head = NULL;
    struct rdr_exception_msg_info info = {0};
    struct bbox_time tm = {0, 0};

    BB_CHECK_EXP_ACT(addr == 0, return BBOX_FAILURE, "invalid param, bbox ddrdump addr is zero.\n");
    BB_CHECK_EXP_ACT(len == 0, return BBOX_FAILURE, "invalid param, bbox ddrdump len is zero.\n");

    vaddr = (char *)bbox_ioremap(addr, len);
    BB_CHECK_PTR(vaddr, return BBOX_FAILURE, "bbox map ddr dump addr failed.\n");

    head = (const struct ddr_bios_hand_s *)vaddr;
    if ((head->magic != DDR_BOOT_DUMP_MAGIC) ||
        (head->end_magic != DDR_BOOT_DUMP_END_MAGIC)) {
        BB_PRINT_ERR("invalid ddr data. magic: %x, end_magic: %x.\n", head->magic, head->end_magic);
        bbox_iounmap((void *)vaddr);
        return BBOX_FAILURE;
    }

    bbox_get_systime(&tm);
    bbox_update_time_seq(&tm);

    etype = bbox_get_reboot_type(DEFAULT_DEVICE_ID);
#ifndef BBOX_UT
    switch (etype) {
        case DEVICE_LTO_EXCEPTION:
            core_id = (u8)BBOX_OS;
            exceptId = (u32)EXCEPID_AP_REBOOT;
            break;
        case SUSPEND_FAIL:
            core_id = (u8)BBOX_COMMON;
            exceptId = (u32)EXCEPID_SUSPEND_FAIL;
            break;
        case RESUME_FAIL:
            core_id = (u8)BBOX_COMMON;
            exceptId = (u32)EXCEPID_RESUME_FAIL;
            break;
        default:
            etype = (u8)DEVICE_HOTBOOT;
            core_id = (u8)BBOX_OS;
            exceptId = (u32)EXCEPID_AP_REBOOT;
    }
#endif
    RDR_SET_SUBMIT_INFO_EX1(info, exceptId, core_id, etype, tm);
    rdr_save_history_log_for_bootcheck(&info);
    // send kernel log
    i = (s32)KERNEL_LOG - 1;
    if ((head->info[i].valid == DDR_BOOT_DUMP_VALID) &&
        (head->info[i].offset < (u64)len) &&
        (head->info[i].len < len) &&
        ((head->info[i].offset + (u64)head->info[i].len) <= (u64)len)) {
        ret = bbox_submit_klog_message(&info, &vaddr[head->info[i].offset], head->info[i].len);
        BB_CHECK_RET(ret != BBOX_SUCCESS, "bootcheck send klog failed with %d.\n", ret);
    } else {
        BB_PRINT_WARN("Invalid kernel log data. (head->info[i].valid=%u, head->info[i].offset=%llu, "
                      "head->info[i].len=%u, len=%u)\n",
                      (u32)head->info[i].valid, head->info[i].offset, head->info[i].len, len);
    }

    // send bbox ddr
    i = (s32)BBOX_DUMP - 1;
    if ((head->info[i].valid == DDR_BOOT_DUMP_VALID) &&
        (head->info[i].offset < (u64)len) &&
        (head->info[i].len < len) &&
        ((head->info[i].offset + (u64)head->info[i].len) <= (u64)len)) {
        ret = bbox_submit_ddr_message(&info, &vaddr[head->info[i].offset], head->info[i].len);
        BB_CHECK_RET(ret != BBOX_SUCCESS, "bootcheck send bbox ddr failed with %d.\n", ret);
    } else {
        BB_PRINT_WARN("Invalid ddr data. (head->info[i].valid=%u, head->info[i].offset=%llu, "
                      "head->info[i].len=%u, len=%u)\n",
                      (u32)head->info[i].valid, head->info[i].offset, head->info[i].len, len);
    }

    bbox_iounmap((void *)vaddr);
    vaddr = NULL;
    return BBOX_SUCCESS;
}

STATIC bool bbox_bootcheck_feature_ddrdump(void)
{
    return bbox_check_feature(FEATURE_BOOTCHECK_DDRDUMP);
}

/*
 * @brief       : check bios reserved previous data
 * @return      : NA
 */
STATIC void bbox_bootcheck_check_last_data(void)
{
    if (bbox_bootcheck_feature_ddrdump()) {
        u32 len = bbox_get_devmem_len();
        u64 addr = bbox_get_devmem_addr();
        if ((addr != 0) && (len != 0)) {
            s32 ret = bbox_bootcheck_send_ddrdump(addr, len);
            BB_CHECK_RET(ret != BBOX_SUCCESS, "bootcheck process reserved ddr data failed.\n");
        }
    }
}

/*
 * @brief       : check bbox ddr
 * @return      : NA
 */
STATIC void bbox_bootcheck_check_bbox_data(void)
{
    u8 dump_type = 0;
    u32 device_id;
    s32 ret;
    const struct rdr_struct_s *tmp = rdr_field_get_tmppbb();
    device_id = tmp->base_info.devid;

    if ((tmp->top_head.magic != FILE_MAGIC) ||
        (bbox_excepid_check(tmp->base_info.excepid) == false) ||
        (device_id >= bbox_get_device_num())) {
        return;
    }

    // if it has been reported, it will not be reported
    ret = bbox_bootcheck_get_dump_type(device_id, &dump_type);
    if ((dump_type & (u8)BBOX_FETCH_DDR) == (u8)BBOX_FETCH_DDR) {
        return;
    }

    // 1. channel disconnect will write regist, can be exported by hot reset
    // 2. channel no connected need this process
    if (tmp->base_info.comm_flag[device_id] == (u8)CHANNEL_STATUS_INIT) {
        BB_PRINT_INFO("bootcheck: send the exception data that the previous communication was not connected.\n");
        ret = bbox_bootcheck_send_bbox(device_id);
        BB_CHECK_RET(ret != BBOX_SUCCESS, "[device-%u] bootcheck send DDR failed.\n", device_id);
    }
}

/*
 * @brief       : check bios ddr, get bios exception, and send
 * @return      : NA
 */
STATIC void bbox_bootcheck_check_ddr_data(void)
{
    // 1. check bios exception ddr, get previous bios exception
    bbox_bootcheck_check_bios_data();

    // 2. check hdr ddr
    (void)hdr_bootcheck_process(&g_start_info.start);

    // 3. check chip ddr
    (void)cdr_bootcheck_process(&g_start_info.start);

    // 4. check bios reserved data, get last exception data
    bbox_bootcheck_check_last_data();

    // 5. check bbox module ddr
    bbox_bootcheck_check_bbox_data();
}

/*
 * @brief       : send reboot reason
 * @param [in]  : u32 devid         device id
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_bootcheck_send_reboot_reason(u32 devid)
{
    u8 reboot_type = bbox_get_reboot_type(devid);
    return bbox_submit_reboot_message(devid, reboot_type, 0, 0);
}

#ifdef BBOX_SOC_PLATFORM_MDC
/*
 * @brief       : send reboot reason
 * @param [in]  : u32 devid         device id
 * @param [in]  : u8 dump_type      dump ddr type
 * @return      : !=0 failure; ==0 success
 */
STATIC void bbox_bootcheck_lastword_dump_proc(u32 devid, u8 dump_type)
{
    s32 ret;
    const struct rdr_struct_s *pbb_tmp = NULL;

    if ((dump_type & (u8)BBOX_FETCH_DDR) == (u8)BBOX_FETCH_DDR) {
        return;
    }

    pbb_tmp = rdr_field_get_tmppbb();
    if (pbb_tmp->top_head.dump_exception == 0) {
        BB_PRINT_INFO("Bootcheck lastword dump proc dump flag not set.");
        return;
    }

    BB_PRINT_INFO("Bootcheck lastword dump proc need DDR data. (device_id=%u)\n", devid);
    ret = bbox_bootcheck_send_bbox(devid);
    BB_CHECK_RET(ret != BBOX_SUCCESS, "Bootcheck send DDR failed. (ret=%d, device_id=%u)\n", ret, devid);
    return;
}
#endif

/*
 * @brief       : send data
 * @param [in]  : u32 devid         device id
 * @param [in]  : u8 dump_type      dump type
 * @return      : NA
 */
STATIC void bbox_bootcheck_send_data(u32 devid, u8 dump_type)
{
    s32 ret;
#ifdef BBOX_SOC_PLATFORM_MDC
    bbox_bootcheck_lastword_dump_proc(devid, dump_type);
#endif

    if (dump_type == (u8)BBOX_FETCH_NULL) {
        BB_PRINT_INFO("[device-%u] bootcheck normal boot.\n", devid);
        return;
    }

    if ((dump_type & (u8)BBOX_FETCH_DDR) == (u8)BBOX_FETCH_DDR) {
        BB_PRINT_INFO("[device-%u] bootcheck need DDR data.\n", devid);
        ret = bbox_bootcheck_send_bbox(devid);
        BB_CHECK_RET(ret != BBOX_SUCCESS, "[device-%u] bootcheck send DDR failed with %d.\n", devid, ret);
    }

    if ((dump_type & (u8)BBOX_FETCH_REGISTERS) == (u8)BBOX_FETCH_REGISTERS) {
        BB_PRINT_INFO("[device-%u] bootcheck need REGISTERS data.\n", devid);
        ret = bbox_bootcheck_send_registers(devid);
        BB_CHECK_RET(ret != BBOX_SUCCESS, "[device-%u] bootcheck send REGISTERS failed with %d.\n", devid, ret);
    }

    if ((dump_type & (u8)BBOX_FETCH_SRAM) == (u8)BBOX_FETCH_SRAM) {
        BB_PRINT_INFO("[device-%u] bootcheck need SRAM data.\n", devid);
        ret = bbox_bootcheck_send_sram(devid);
        BB_CHECK_RET(ret != BBOX_SUCCESS, "[device-%u] bootcheck send SRAM failed with %d.\n", devid, ret);
    }

    if ((dump_type & (u8)BBOX_FETCH_TEE) == (u8)BBOX_FETCH_TEE) {
        BB_PRINT_INFO("[device-%u] bootcheck need tee data.\n", devid);
        ret = bbox_bootcheck_send_tee(devid);
        BB_CHECK_RET(ret != BBOX_SUCCESS, "[device-%u] bootcheck send TEE failed with %d.\n", devid, ret);
    }

    if ((dump_type & (u8)BBOX_FETCH_ATF) == (u8)BBOX_FETCH_ATF) {
        BB_PRINT_INFO("[device-%u] bootcheck need atf data.\n", devid);
        ret = bbox_bootcheck_send_atf(devid);
        BB_CHECK_RET(ret != BBOX_SUCCESS, "[device-%u] bootcheck send ATF failed with %d.\n", devid, ret);
    }

    if ((dump_type & (u8)BBOX_FETCH_SD) == (u8)BBOX_FETCH_SD) {
        BB_PRINT_INFO("[device-%u] bootcheck need sd data.\n", devid);
        ret = bbox_bootcheck_send_sd(devid);
        BB_CHECK_RET(ret != BBOX_SUCCESS, "[device-%u] bootcheck send SD failed with %d.\n", devid, ret);
    }
}

/*
 * @brief       : bootcheck process data
 * @param [in]  : u32 devid     device id
 * @return      : NA
 */
STATIC void bbox_bootcheck_check_device_reboot(u32 devid)
{
    u8 dump_type = 0;
    s32 ret;

    if (devid >= DEVICE_MAX_NUM) {
        BB_PRINT_ERR("[device-%u] bootcheck invalid device id.", devid);
        return;
    }

    // 1. device-x reboot reason, notify host
    ret = bbox_bootcheck_send_reboot_reason(devid);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("[device-%u] bootcheck send reboot reason failed.", devid);
    }

    // 2. get the dump data type
    ret = bbox_bootcheck_get_dump_type(devid, &dump_type);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("[device-%u] bootcheck get data type failed.", devid);
        return;
    }

    // 3. dump data and send to host
    bbox_bootcheck_send_data(devid, dump_type);
}

/*
 * @brief       : check reboot reason for each device
 * @return      : NA
 */
STATIC void bbox_bootcheck_check_reboot_reason(void)
{
    u32 i;
    for (i = 0; i < bbox_get_device_num(); i++) {
        bbox_bootcheck_check_device_reboot(i);
    }
}

/*
 * @brief       : record the start dot in reg
 * @param [in]  : u8 stage  the start dot
 * @return      : NA
 */
STATIC void bbox_bootcheck_kernel_stage(u8 stage)
{
    if (bbox_check_feature(FEATURE_KERNEL_STAGE)) {
        bbox_record_kernel_stage(stage);
    }
}

/*
 * @brief       : bootcheck process thread
 * @param [in]  : void *arg
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_bootcheck_thread_body(void *arg)
{
    u32 num = bbox_get_device_num();
    u32 connected = 0;

    BB_PRINT_START();
    BB_PRINT_INFO("bootcheck thread enter.\n");
    UNUSED(arg);

    // 1.link to host
    BB_PRINT_INFO("============ wait for link ready start ==============\n");
    while (!bbox_channels_established(&connected)) {
        if (kthread_should_stop()) {
            BB_PRINT_INFO("bootcheck thread cancel.\n");
            return BBOX_SUCCESS;
        }
        msleep(WAIT_MS_LONG);
    }
    BB_PRINT_INFO("============ wait for link ready e n d ==============\n");
    BB_PRINT_INFO("total num: %u, connected num: %u.\n", bbox_double(num), connected);

    // 2.set kernel stage
    bbox_bootcheck_kernel_stage(STAGE_KERNEL_SUCCESS);

    // 3.start info
    bbox_bootcheck_start_info();

    // 4.check reboot reason
    bbox_bootcheck_check_reboot_reason();

    // 5.check ddr data
    bbox_bootcheck_check_ddr_data();

    // 6.free tmppbb
    rdr_field_clear_tmppbb();
    BB_PRINT_INFO("bootcheck thread exit\n");
    BB_PRINT_END();
    return BBOX_SUCCESS;
}

STATIC struct task_struct *g_bbox_bootcheck_thread = NULL;

/*
 * @brief       : bootcheck init
 * @return      : NA
 */
s32 bbox_bootcheck_init(void)
{
    g_bbox_bootcheck_thread = bbox_kthread_proc(bbox_bootcheck_thread_body, NULL, "bbox_bootcheck");
    if (g_bbox_bootcheck_thread == NULL) {
        BB_PRINT_ERR("create bootcheck thread failed.\n");
        return BBOX_FAILURE;
    }
    return BBOX_SUCCESS;
}

/*
 * @brief       : bootcheck exit
 * @return      : NA
 */
void bbox_bootcheck_exit(void)
{
    u32 connected = 0;
    if (!bbox_channels_established(&connected)) {
        KTHREAD_STOP(g_bbox_bootcheck_thread);
        BB_PRINT_INFO("close bootcheck thread.\n");
    }
}

