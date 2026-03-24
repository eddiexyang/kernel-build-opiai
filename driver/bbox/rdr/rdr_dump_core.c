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

#include "rdr_dump_core.h"

#include <linux/securec.h>
#include "common/bbox_print.h"
#include "communication/bbox_message.h"
#include "device/bbox_pub_cloud.h"
#include "rdr_common.h"
#include "rdr_field_core.h"
#include "rdr_module_core.h"
#include "bbox_inner.h"
#ifdef CFG_FEATURE_MDC_CLOCK_CONFIG
#include <linux/virt_wall_time.h>
#include "bootparam/bbox_boot_param.h"
#endif

#define GET_MAGIC(buffer) (*(u32*)(uintptr_t)(buffer))

STATIC DEFINE_SPINLOCK(g_rdr_log_lock);

/*
 * @brief       : save log to rdr log buffer
 * @param [in]  : const struct rdr_log_record *record       log record
 * @return      : NA
 */
STATIC void rdr_save_history_log(const struct rdr_log_record *record)
{
    int ret;
    u8 idx;
    struct rdr_struct_s *pbb_tmp = NULL;

    BB_CHECK_PTR(record, return, "invalid param, record is NULL.\n");
    pbb_tmp = rdr_field_get_pbb();
    idx = pbb_tmp->log_info.next_valid_index;
#ifndef BBOX_UT
    if (idx >= RDR_LOG_BUFFER_NUM) {
        BB_PRINT_ERR("Invalid param, idx=%u.\n", idx);
        return;
    }
#endif
    pbb_tmp->log_info.next_valid_index++;
    pbb_tmp->log_info.next_valid_index %= RDR_LOG_BUFFER_NUM;
    if (pbb_tmp->log_info.log_num < RDR_LOG_BUFFER_NUM) {
        pbb_tmp->log_info.log_num++;
    }

    ret = memcpy_s(&(pbb_tmp->log_info.log_buffer[idx]), sizeof(struct rdr_log_record),
                   record, sizeof(struct rdr_log_record));
    BB_CHECK_MEMCPY(ret, return);
}

/*
 * @brief       : save undef exception to rdr reserve buffer
 * @param [in]  : struct rdr_syserr_param_s *param          exception param info
 * @return      : NA
 */
void rdr_save_history_log_for_undef_exception(const struct rdr_syserr_param_s *param)
{
    struct rdr_log_record record;
    lock_flag_t flags = 0;

    BB_CHECK_PTR(param, return, "invalid param, param is NULL.\n");
    record.record_type = RDR_RECORD_UNDEFINE_EXCEPTION;
    record.record.devid = param->devid;
    record.record.excepid = param->excepid;
    record.record.arg = param->arg;
    bbox_get_date(&(param->tm), record.record.date, DATATIME_MAXLEN);
    spin_lock_irqsave(&g_rdr_log_lock, flags);
    rdr_save_history_log(&record);
    spin_unlock_irqrestore(&g_rdr_log_lock, flags);
}

/*
 * @brief       : save bootcheck log to rdr reserve buffer
 * @param [in]  : struct rdr_exception_msg_info *info   exception message info
 * @return      : NA
 */
void rdr_save_history_log_for_bootcheck(const struct rdr_exception_msg_info *info)
{
    struct rdr_log_record record;
    lock_flag_t flags = 0;

    BB_CHECK_PTR(info, return, "invalid param, info is NULL.\n");
    record.record_type = RDR_RECORD_DEFINE_EXCEPTION;
    record.record.devid = info->devid;
    record.record.excepid = info->excepid;
    record.record.e_from_core = info->coreid;
    record.record.e_exce_type = info->etype;
    bbox_get_date(&info->tm, record.record.date, DATATIME_MAXLEN);
    spin_lock_irqsave(&g_rdr_log_lock, flags);
    rdr_save_history_log(&record);
    spin_unlock_irqrestore(&g_rdr_log_lock, flags);
}

/*
 * @brief       : save oom log to rdr reserve buffer
 * @param [in]  : u32 devid                     device id
 * @param [in]  : u8 coreid                     module id
 * @param [in]  : u8 etype                      exception type
 * @param [in]  : u32 excepid                   exception id
 * @param [in]  : struct bbox_time *tm          time
 * @return      : NA
 */
void rdr_save_history_log_for_oom(u32 devid, u8 coreid, u8 etype, u32 excepid, const struct bbox_time *tm)
{
    struct rdr_log_record record;
    struct rdr_struct_s *pbb_tmp = NULL;
    lock_flag_t flags = 0;

    BB_CHECK_PTR(tm, return, "invalid param, time is NULL.\n");

    record.record_type = RDR_RECORD_DEFINE_EXCEPTION;
    record.record.devid = devid;
    record.record.excepid = excepid;
    record.record.e_from_core = coreid;
    record.record.e_exce_type = etype;
    record.record.tm.tv_sec = tm->tv_sec;
    record.record.tm.tv_nsec = tm->tv_nsec;
    bbox_get_date(tm, record.record.date, DATATIME_MAXLEN);

    pbb_tmp = rdr_field_get_pbb();
    spin_lock_irqsave(&g_rdr_log_lock, flags);
    pbb_tmp->log_info.event_flag |= (u16)EVENT_OOM_TRIGGER;
    rdr_save_history_log(&record);
    spin_unlock_irqrestore(&g_rdr_log_lock, flags);
}

/*
 * @brief       : save comm log to rdr log buffer
 * @param [in]  : u32 devid                         device id
 * @param [in]  : struct bbox_exception_info *info  exception info
 * @param [in]  : struct bbox_time *tm              time
 * @return      : NA
 */
void rdr_save_history_log_for_comm(u32 devid, const struct bbox_exception_info *info,
                                   const struct bbox_time *tm)
{
    struct rdr_log_record record;
    lock_flag_t flags = 0;

    BB_CHECK_PTR(info, return, "invalid param, info is NULL.\n");
    BB_CHECK_PTR(tm, return, "invalid param, time is NULL.\n");

    record.record_type = RDR_RECORD_DEFINE_EXCEPTION;
    record.record.devid = devid;
    record.record.excepid = info->e_excepid;
    record.record.e_exce_type = info->e_exce_type;
    record.record.e_from_core = info->e_from_core;
    bbox_get_date(tm, record.record.date, DATATIME_MAXLEN);

    spin_lock_irqsave(&g_rdr_log_lock, flags);
    rdr_save_history_log(&record);
    spin_unlock_irqrestore(&g_rdr_log_lock, flags);
}

/*
 * @brief       : save runtime log to rdr log buffer
 * @param [in]  : u32 devid                                device id
 * @param [in]  : const struct bbox_exception_info *info   exception info
 * @param [in]  : const struct bbox_time *tm               date in time seq
 * @return      : NA
 */
void rdr_save_history_log_for_runtime(u32 devid, const struct bbox_exception_info *info,
                                      const struct bbox_time *tm)
{
    u8 is_reset;
    struct rdr_log_record record;
    lock_flag_t flags = 0;

    BB_CHECK_PTR(info, return, "invalid param, info is NULL.\n");
    BB_CHECK_PTR(tm, return, "invalid param, time is NULL.\n");

    is_reset = bbox_excepid_check_excep_class(info->e_excepid) ? BBOX_FALSE : BBOX_TRUE;
    if (is_reset == BBOX_TRUE) {
        record.record_type = RDR_RECORD_RESET_EXCEPTION;
    } else {
        record.record_type = RDR_RECORD_DEFINE_EXCEPTION;
    }
    record.record.devid = devid;
    record.record.excepid = info->e_excepid;
    record.record.e_exce_type = info->e_exce_type;
    record.record.e_from_core = info->e_from_core;
    bbox_get_date(tm, record.record.date, DATATIME_MAXLEN);

    spin_lock_irqsave(&g_rdr_log_lock, flags);
    rdr_save_history_log(&record);
    spin_unlock_irqrestore(&g_rdr_log_lock, flags);
}

/*
 * @brief       : save panic log to rdr log buffer
 * @param [in]  : u32 devid                                device id
 * @param [in]  : const struct bbox_exception_info *info   exception info
 * @param [in]  : const struct bbox_time *tm               date in time seq
 * @return      : NA
 */
void rdr_save_history_log_for_panic(u32 devid, const struct bbox_exception_info *info,
                                    const struct bbox_time *tm)
{
    struct rdr_log_record log_record;

    BB_CHECK_PTR(info, return, "invalid param, info is NULL.\n");
    BB_CHECK_PTR(tm, return, "invalid param, time is NULL.\n");

    log_record.record_type = RDR_RECORD_DEFINE_EXCEPTION;
    log_record.record.devid = devid;
    log_record.record.excepid = info->e_excepid;
    log_record.record.e_exce_type = info->e_exce_type;
    log_record.record.e_from_core = info->e_from_core;
    bbox_get_date(tm, log_record.record.date, DATATIME_MAXLEN);
    rdr_save_history_log(&log_record);
}

STATIC s32 rdr_save_dump_for_local_module(u64 log_vaddr, u32 log_len,
                                          const struct rdr_exception_msg_info *info)
{
    u32 len;
    u64 addr;
    const struct rdr_ddr_module_info_s *l_info = (struct rdr_ddr_module_info_s *)(uintptr_t)log_vaddr;
    if (((u64)l_info->e_info_offset + (u64)l_info->e_info_len) <= (u64)log_len) {
        addr = log_vaddr + l_info->e_info_offset;
        len = l_info->e_info_len;
        return bbox_submit_module_message(info, (char *)(uintptr_t)addr, len);
    } else {
        BB_PRINT_ERR("param invalid! coreid[0x%hhx][0x%x], offset[0x%x], len[0x%x].",
                     info->coreid, log_len, l_info->e_info_offset, l_info->e_info_len);
        return BBOX_FAILURE;
    }
}

STATIC s32 rdr_save_dump_for_local_module_v1(u64 log_vaddr, u32 log_len,
                                             const struct rdr_exception_msg_info *info)
{
    const struct bbox_module_ctrl *l_info = (struct bbox_module_ctrl *)(uintptr_t)log_vaddr;
    s32 i, ret;
    s32 count = 0;
    u32 len;
    u64 addr;
    s32 e_block_num;

    e_block_num = (s32)((l_info->e_block_num > BBOX_MODULE_CTRL_NUM) ?
        BBOX_MODULE_CTRL_NUM : l_info->e_block_num);
    for (i = 0; i < e_block_num; i++) {
        const struct bbox_module_exception_ctrl *block = &l_info->block[i];
        if ((block->e_excepid == info->excepid) &&
            (block->e_clock.tv_usec == (info->tm.tv_nsec / KILO)) &&
            (block->e_clock.tv_sec == info->tm.tv_sec)) {
            if ((block->e_block_offset < sizeof(struct bbox_module_ctrl)) ||
                ((block->e_block_offset + block->e_info_len) > log_len)) {
                BB_PRINT_ERR("save dump for lmodule block(%d) failed! "
                             "invalid offset : 0x%x, len : 0x%x for excepid : 0x%x.",
                             i, block->e_block_offset, block->e_info_len, info->excepid);
                continue;
            }
            addr = log_vaddr + block->e_block_offset;
            len = block->e_info_len;
            ret = bbox_submit_module_message(info, (char *)(uintptr_t)addr, len);
            BB_CHECK_RET(ret != 0, "save dump for lmodule block(%d) failed!\n", i);
            if (ret == 0) {
                count++;
            }
        }
    }
    if (count == 0) {
        BB_PRINT_ERR("save dump for lmodule failed! Cannot find dump info for excepid: 0x%x.\n", info->excepid);
        return BBOX_FAILURE;
    }
    return BBOX_SUCCESS;
}

/*
 * @brief       : send local module data to hdc
 * @param [in]  : u32 devid                     device id
 * @param [in]  : u32 excepid                   exception id
 * @param [in]  : u8 coreid                     core id
 * @param [in]  : const struct bbox_time *tm    time stamp
 * @return      : <0: failure; =0: success
 */
STATIC s32 rdr_save_dump_for_lmodule(u32 devid, u32 excepid, u8 coreid, const struct bbox_time *tm)
{
    s32 ret;
    u32 log_len;
    u64 log_vaddr = 0;
    u32 magic;
    struct rdr_exception_msg_info info = {0};
    struct bbox_time tv;

    BB_CHECK_PTR(tm, return BBOX_FAILURE, "invalid parameter, time is NULL.\n");

    ret = rdr_module_get_vaddr(devid, coreid, &log_vaddr);
    if ((ret <= 0) || (log_vaddr == 0)) {
        BB_PRINT_ERR("get module[0x%hhx] info failed with %d!!!\n", coreid, ret);
        return BBOX_FAILURE;
    }

    bbox_get_systime(&tv);
    RDR_SET_SUBMIT_INFO_EX3(info, devid, excepid, coreid, *tm, tv);

    log_len = (u32)ret;
    magic = GET_MAGIC(log_vaddr);
    switch (magic) {
        case MODULE_MAGIC:
            ret = rdr_save_dump_for_local_module(log_vaddr, log_len, &info);
            break;
        case BBOX_MODULE_MAGIC:
            ret = rdr_save_dump_for_local_module_v1(log_vaddr, log_len, &info);
            break;
        default:
            ret = bbox_submit_module_message(&info, (char *)(uintptr_t)log_vaddr, log_len);
            break;
    }

    rdr_module_free_vaddr(log_vaddr);
    log_vaddr = 0;
    return ret;
}

/*
 * @brief       : send ap data to hdc
 * @param [in]  : u32 devid                     device id
 * @param [in]  : u32 excepid                   exception id
 * @param [in]  : u8 coreid                     core id
 * @param [in]  : const struct bbox_time *tm    time stamp
 * @return      : <0: failure; =0: success
 */
STATIC s32 rdr_save_dump_for_ap(u32 devid, u32 excepid, u8 coreid, const struct bbox_time *tm)
{
    s32 ret;
    u32 len;
    u64 vaddr = 0;
    struct rdr_exception_msg_info info = {0};
    struct bbox_time tv;

    BB_CHECK_PTR(tm, return BBOX_FAILURE, "invalid parameter, time is NULL.\n");

    ret = rdr_module_get_vaddr(devid, coreid, &vaddr);
    if ((ret <= 0) || (vaddr == 0)) {
        BB_PRINT_ERR("get module[0x%hhx] info failed with %d!!!\n", coreid, ret);
        return BBOX_FAILURE;
    }

    len = (u32)ret;
    bbox_get_systime(&tv);
    RDR_SET_SUBMIT_INFO_EX3(info, devid, excepid, coreid, *tm, tv);

    ret = bbox_submit_module_message(&info, (char *)(uintptr_t)vaddr, len);
    rdr_module_free_vaddr(vaddr);
    vaddr = 0;
    return ret;
}

/*
 * @brief       : send exception reset data to hdc
 * @param [in]  : u32 devid                     device id
 * @param [in]  : u32 excepid                   exception id
 * @param [in]  : u8 coreid                     core id
 * @param [in]  : const struct bbox_time *tm    time stamp
 * @return      : <0: failure; =0: success
 */
s32 rdr_save_dump_for_reset(u32 devid, u32 excepid, u8 coreid, const struct bbox_time *tm)
{
    s32 ret;
    char buffer[BUF_LEN_MID] = {0};
    char date[DATATIME_MAXLEN] = {0};
    struct rdr_exception_msg_info info = {0};
    struct bbox_time tv;

    BB_CHECK_PTR(tm, return BBOX_FAILURE, "invalid parameter, time is NULL.\n");

    bbox_get_date(tm, date, DATATIME_MAXLEN);
    ret = sprintf_s(buffer, BUF_LEN_MID,
                    "exception recover.\n"
                    "module  : %s\n"
                    "device  : %u\n"
                    "excepid : 0x%x\n"
                    "coreid  : 0x%hhx\n"
                    "date    : %s UTC.",
                    rdr_get_model_name(excepid),
                    devid, excepid, coreid, date);
    BB_CHECK_SPRINTF(ret, return BBOX_FAILURE);
    buffer[BUF_LEN_MID - 1] = '\0';

    bbox_get_systime(&tv);
    RDR_SET_SUBMIT_INFO_EX3(info, devid, excepid, coreid, *tm, tv);

    return bbox_submit_module_reset_message(&info, buffer, (u32)strlen(buffer));
}

/*
 * @brief       : module dump done, process dump data, send to hdc
 * @param [in]  : u32 devid                     device id
 * @param [in]  : u32 excepid                   exception id
 * @param [in]  : u8 coreid                     core id
 * @param [in]  : u8 etype                      exception etype
 * @param [in]  : const struct bbox_time *tm    time stamp
 * @return      : NA
 */
void rdr_save_dump(u32 devid, u8 coreid, u32 excepid, u8 etype, const struct bbox_time *tm)
{
    s32 ret;

    switch (coreid) {
        case BBOX_TS:
        case BBOX_LPFW:
        case BBOX_LPM:
        case BBOX_HSM:
        case BBOX_ISP:
        case BBOX_SAFETYISLAND:
        case BBOX_MICROWATT:
            ret = BBOX_SUCCESS; // dump data has been sent before dump done.
            break;
        case BBOX_OS:
            ret = rdr_save_dump_for_ap(devid, excepid, coreid, tm);
            break;
        case BBOX_TEEOS:
        case BBOX_DVPP:
        case BBOX_DRIVER:
        case BBOX_NETWORK:
            ret = rdr_save_dump_for_lmodule(devid, excepid, coreid, tm);
            break;
        default:
            BB_PRINT_ERR("invaild coreid[0x%hhx], devid[%u], excepid[0x%x], etype[0x%hhx]\n",
                         coreid, devid, excepid, etype);
            ret = BBOX_FAILURE;
            break;
    }
    BB_CHECK_RET(ret != BBOX_SUCCESS, "device-%u save moduel(0x%hhx) dump failed with %d.\n", devid, coreid, ret);
    return;
}

