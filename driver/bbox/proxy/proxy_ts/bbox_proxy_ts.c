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

#include "bbox_proxy_ts.h"

#include <asm-generic/int-ll64.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/rtc.h>
#include <linux/securec.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/syscalls.h>
#include <linux/sysfs.h>
#include <linux/uaccess.h>

#include "bbox_platform.h"
#include "bootparam/bbox_boot_param.h"
#include "common/bbox_print.h"
#include "communication/bbox_message.h"
#include "device/bbox_pub.h"
#include "device/bbox_pub_cloud.h"
#include "proxy/proxy_common/bbox_proxy_common.h"
#include "rdr/rdr_common.h"
#include "rdr/rdr_field_core.h"
#include "rdr/rdr_module_core.h"

STATIC ts_vaddr_t g_ts_vaddr[DEVICE_MAX_NUM] = {0};
STATIC ts_except_t g_ts_except_info = {0};
STATIC bbox_tsconfig g_ts_paddr_config[DEVICE_MAX_NUM] = {0};
STATIC struct task_struct *g_ts_main_task = NULL;
STATIC bool g_proxy_ts_init = false;

static inline void bbox_ts_set_dump_done(struct bbox_dump_done_ops_info *info)
{
    info->devid = g_ts_except_info.devid;
    info->coreid = BBOX_TS;
    info->excepid = g_ts_except_info.excepid;
    info->etype = g_ts_except_info.etype;
    info->time = g_ts_except_info.time;
}

static inline bool bbox_ts_check_excepid(u32 excepid)
{
    return bbox_excepid_check(excepid) && bbox_excepid_compare_coreid(excepid, BBOX_TS);
}

STATIC void bbox_ts_dump_except_header(const struct exc_module_info_s *pexcinfo, char *logbuf, u32 len)
{
    s32 ret = sprintf_s(logbuf, len,
                        "magic:                 0x%x\n"
                        "exception valid:       0x%hx\n"
                        "exception num:         %hu\n"
                        "module name:           %s\n"
                        "current time(s):       %llu\n"
                        "current time(us):      %llu\n"
                        "current exception id:  0x%x\n"
                        "current dump status:   %hu\n"
                        "current save status:   %hu\n"
                        "info offset:           %u\n"
                        "info length:           %u\n",
                        pexcinfo->magic,
                        pexcinfo->e_excep_valid,
                        pexcinfo->e_excep_num,
                        ((pexcinfo->e_from_module[0] == '\0') ? (u8 *)"UNKNOWN" : pexcinfo->e_from_module),
                        pexcinfo->cur_info.e_clock.tv_sec,
                        pexcinfo->cur_info.e_clock.tv_usec,
                        pexcinfo->cur_info.e_excepid,
                        pexcinfo->cur_info.e_dump_status,
                        pexcinfo->cur_info.e_save_status,
                        pexcinfo->e_info_offset,
                        pexcinfo->e_info_len);
    if (ret == -1) {
        BB_PRINT_ERR("save except header failed, ret(%d)\n", ret);
    }
}

STATIC s32 bbox_ts_verify_except_header(const struct exc_module_info_s *pexcinfo, u8 etype, u32 excepid)
{
    u32 total_wait_ms = 0;

    BB_CHECK_PTR(pexcinfo, return BBOX_FAILURE, "invalid param, pexcinfo is NULL.\n");
    if ((pexcinfo->magic != MODULE_MAGIC) || (pexcinfo->e_excep_valid != MODULE_VALID)) {
        BB_PRINT_ERR("magic[0x%x], valid[%hu] error\n", pexcinfo->magic, pexcinfo->e_excep_valid);
        return BBOX_FAILURE;
    }
    if (bbox_ts_check_excepid(pexcinfo->cur_info.e_excepid) == false) {
        BB_PRINT_ERR("exception id[0x%x] check failed.\n", pexcinfo->cur_info.e_excepid);
        return BBOX_FAILURE;
    }
    if (etype != (u8)HEARTBEAT_EXCEPTION) {
        while (pexcinfo->cur_info.e_dump_status == (u16)STATUS_DOING) {
            msleep(TS_WAIT_CYCLE);
            total_wait_ms += TS_WAIT_CYCLE;
            if (total_wait_ms >= TS_DUMP_WAIT_TIMEOUT) {
                break;
            }
        }
        if (excepid != pexcinfo->cur_info.e_excepid) {
            BB_PRINT_ERR("exception id inconsistent, recv[0x%x], stored[0x%x].\n",
                         excepid, pexcinfo->cur_info.e_excepid);
            return BBOX_FAILURE;
        }
    }
    if (pexcinfo->cur_info.e_dump_status == (u16)STATUS_DOING) {
        BB_PRINT_ERR("dump status error, doing status.\n");
        return BBOX_FAILURE;
    }
    return BBOX_SUCCESS;
}

/*
 * @brief       : ts runtime execption dump
 * @param [in]  : u32 devid         device id
 * @param [in]  : u32 excepid       execption id
 * @param [in]  : u8 etype          execption type
 * @return      : NA
 */
STATIC void bbox_ts_runexecpt_dump(u32 devid, u32 excepid, u8 etype)
{
    s32 ret;
    struct exc_module_info_s *pexcinfo = NULL;

    BB_CHECK_EXP_ACT((devid >= DEVICE_MAX_NUM), return, "invalid param, dev[%u].\n", devid);

    pexcinfo = (struct exc_module_info_s *)g_ts_vaddr[devid].run_except;
    BB_CHECK_PTR(pexcinfo, return, "ts pexcinfo address error.\n");

    ret = bbox_ts_verify_except_header(pexcinfo, etype, excepid);
    BB_CHECK_RET(ret == BBOX_FAILURE, "ts-proxy: exception header check failed.\n");

    pexcinfo->cur_info.e_save_status = STATUS_DOING;
    if (g_ts_except_info.pf_dumpdone != NULL) {
        struct bbox_dump_done_ops_info dump_done_info = {0};
        bbox_ts_set_dump_done(&dump_done_info);
        bbox_proxy_module_dump(&dump_done_info, g_ts_except_info.pf_dumpdone);
    }
    pexcinfo->cur_info.e_save_status = STATUS_DONE;
}

STATIC void bbox_ts_dump(const struct bbox_dump_ops_info *info, bbox_dump_done_ops fndone)
{
    BB_CHECK_PTR(info, return, "invalid param, info is NULL.\n");
    BB_CHECK_PTR(fndone, return, "invalid param, fndone is NULL, core(0x%hhx).\n", info->coreid);
    BB_CHECK_EXP_ACT(info->coreid != (u8)BBOX_TS, return, "invalid param, core(0x%hhx).\n", info->coreid);

    g_ts_except_info.devid = info->devid;
    g_ts_except_info.excepid = info->excepid;
    g_ts_except_info.etype = info->etype;
    g_ts_except_info.time.tv_sec = info->time.tv_sec;
    g_ts_except_info.time.tv_usec = info->time.tv_usec;
    g_ts_except_info.pf_dumpdone = fndone;
    up(&g_ts_except_info.sem);
}

STATIC s32 bbox_ts_register_core(u8 coreid)
{
    s32 i;
    struct bbox_module_info s_module_ops = {0};
    struct bbox_module_result register_result = {0};

    s_module_ops.coreid = coreid;
    s_module_ops.ops_dump = bbox_ts_dump;
    s_module_ops.ops_reset = NULL;

    if (bbox_register_module(&s_module_ops, &register_result) != BBOX_SUCCESS) {
        BB_PRINT_ERR("core(%hhu) register failed.\n", coreid);
        return BBOX_FAILURE;
    }

    if (register_result.log_addr == 0 || register_result.log_len == 0) {
        BB_PRINT_ERR("invalid phys addr, core(%hhu) register failed, len(%u).\n", coreid, register_result.log_len);
        (void)bbox_unregister_module(coreid);
        return BBOX_FAILURE;
    }

    for (i = 0; i < (s32)DEVICE_MAX_NUM; i++) {
        g_ts_paddr_config[i].ts_paddr[TS_MNTN_BUFFER].addr = register_result.log_addr;
        g_ts_paddr_config[i].ts_paddr[TS_MNTN_BUFFER].len = register_result.log_len;
    }

    return BBOX_SUCCESS;
}

STATIC s32 bbox_ts_register_start_exception(void)
{
    s32 ret;
    const char module_name[] = "ts";
    const char desc[] = "ts startup error";
    struct bbox_exception_info einfo = {0};

    ret = memset_s(&einfo, sizeof(einfo), 0, sizeof(einfo));
    BB_CHECK_MEMSET(ret, return BBOX_FAILURE);

    einfo.e_excepid = TS_START_FAIL_EXCEPTION;
    einfo.e_excepid_end = einfo.e_excepid;
    einfo.e_notify_core_mask = bbox_coreid_mask(BBOX_TS);
    einfo.e_reset_core_mask = bbox_coreid_mask(BBOX_TS);
    einfo.e_exce_type = STARTUP_EXCEPTION;
    einfo.e_from_core = BBOX_TS;
    einfo.e_process_priority = BBOX_CRITICAL;
    einfo.e_reboot_priority = BBOX_REBOOT_WAIT;
    einfo.e_reentrant = BBOX_REENTRANT_DISALLOW;
    ret = memcpy_s(einfo.e_from_module, BBOX_MODULE_NAME_LEN, module_name, sizeof(module_name));
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);
    ret = memcpy_s(einfo.e_desc, BBOX_EXCEPTIONDESC_MAXLEN, desc, sizeof(desc));
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);

    if (!bbox_register_exception(&einfo)) {
        BB_PRINT_ERR("ts-proxy: ts startup exception register fail.\n");
        return BBOX_FAILURE;
    }
    BB_PRINT_INFO("ts-proxy: ts startup exception register ok.\n");

    return BBOX_SUCCESS;
}

STATIC s32 bbox_ts_register_heartbeat_exception(void)
{
    s32 ret;
    const char module_name[] = "ts";
    const char desc[] = "ts heartbeat lost";
    struct bbox_exception_info einfo = {0};

    ret = memset_s(&einfo, sizeof(einfo), 0, sizeof(einfo));
    BB_CHECK_MEMSET(ret, return BBOX_FAILURE);
    einfo.e_excepid = TS_HEART_BEAT_LOST_EXCEPTION;
    einfo.e_excepid_end = einfo.e_excepid;
    einfo.e_notify_core_mask = bbox_coreid_mask(BBOX_TS);
    einfo.e_reset_core_mask = bbox_coreid_mask(BBOX_TS);
    einfo.e_exce_type = HEARTBEAT_EXCEPTION;
    einfo.e_from_core = BBOX_TS;
    einfo.e_process_priority = BBOX_MAJOR;
    einfo.e_reboot_priority = BBOX_REBOOT_WAIT;
    einfo.e_reentrant = BBOX_REENTRANT_DISALLOW;
    ret = memcpy_s(einfo.e_from_module, BBOX_MODULE_NAME_LEN, module_name, sizeof(module_name));
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);
    ret = memcpy_s(einfo.e_desc, BBOX_EXCEPTIONDESC_MAXLEN, desc, sizeof(desc));
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);

    if (!bbox_register_exception(&einfo)) {
        BB_PRINT_ERR("ts-proxy: ts heartbeat lost register fail.\n");
        return BBOX_FAILURE;
    }
    BB_PRINT_INFO("ts-proxy: ts heartbeat lost register ok.\n");

    return BBOX_SUCCESS;
}

/*
 * @brief       : get ts module info
 * @return      : module info
 */
STATIC struct exc_module_info_s *bbox_ts_get_module_info(void)
{
    struct exc_module_info_s *info = (struct exc_module_info_s *)g_ts_vaddr[0].run_except;

    if ((info == NULL) ||
        (info->magic != MODULE_MAGIC) ||
        (info->e_excep_valid != MODULE_VALID) ||
        (strlen((char *)info->e_from_module) == 0) ||
        (strlen((char *)info->e_from_module) >= BBOX_MODULE_NAME_LEN)) {
        return NULL;
    }
    return info;
}

/*
 * @brief       : register ts proxy exception
 * @param [in]  : struct exc_description_s  *e_desc       exception description
 * @param [in]  : u8 *e_from_module                       exception from module
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_ts_register_exception(const struct exc_description_s *e_desc, const u8 *e_from_module)
{
    s32 ret;
    u32 reg_result;
    struct bbox_exception_info einfo = {0};
    u32 excepid = e_desc->e_excepid;
    if ((excepid == TS_START_FAIL_EXCEPTION) || (excepid == TS_HEART_BEAT_LOST_EXCEPTION)) {
        return BBOX_SUCCESS;
    }
    ret = memset_s(&einfo, sizeof(einfo), 0, sizeof(einfo));
    BB_CHECK_MEMSET(ret, return BBOX_FAILURE);
    einfo.e_excepid = excepid;
    einfo.e_excepid_end = excepid;
    einfo.e_notify_core_mask = e_desc->e_notify_core_mask;
    einfo.e_reset_core_mask = bbox_coreid_mask(BBOX_TS);
    einfo.e_exce_type = e_desc->e_excep_type;
    einfo.e_from_core = BBOX_TS;
    einfo.e_process_priority = e_desc->e_process_level;
    einfo.e_reboot_priority = e_desc->e_reboot_priority;
    einfo.e_reentrant = e_desc->e_reentrant;
    ret = memcpy_s(einfo.e_from_module, BBOX_MODULE_NAME_LEN, e_from_module, BBOX_MODULE_NAME_LEN);
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);
    ret = memcpy_s(einfo.e_desc, BBOX_EXCEPTIONDESC_MAXLEN, e_desc->e_desc, BBOX_EXCEPTIONDESC_MAXLEN);
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);
    reg_result = bbox_register_exception(&einfo);
    if (reg_result == 0) {
        BB_PRINT_ERR("ts register exception failed, excepid[0x%x].\n", excepid);
        return BBOX_FAILURE;
    }
    return BBOX_SUCCESS;
}

/*
 * @brief       : ts register exceptions
 * @return      : <0 failure; ==0 success
 */
STATIC s32 bbox_ts_register_exceptions(void)
{
    s32 i;
    struct exc_module_info_s *info = NULL;

    info = bbox_ts_get_module_info();
    if (info == NULL) {
        return BBOX_FAILURE;
    }

    // exception register
    BB_PRINT_INFO("ts-proxy: register exception num: %hu\n", info->e_excep_num);
    for (i = 0; i < BBOX_MIN(info->e_excep_num, MODULE_EXCEPTION_REGISTER_MAXNUM); i++) {
        s32 ret = bbox_ts_register_exception(&info->e_description[i], info->e_from_module);
        if (ret != BBOX_SUCCESS) {
            BB_PRINT_ERR("ts-proxy : register exception [%u] failed.\n", info->e_description[i].e_excepid);
        }
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : unegister ts proxy exceptions
 * @return      : <0 failure; =0 success
 */
STATIC void bbox_ts_unregister_exceptions(void)
{
    s32 i;
    struct exc_module_info_s *info = NULL;

    (void)bbox_unregister_exception(TS_START_FAIL_EXCEPTION);
    (void)bbox_unregister_exception(TS_HEART_BEAT_LOST_EXCEPTION);

    info = bbox_ts_get_module_info();
    BB_CHECK_PTR(info, return, "ts-proxy: get ts module info failed.\n");

    for (i = 0; i < BBOX_MIN(info->e_excep_num, MODULE_EXCEPTION_REGISTER_MAXNUM); i++) {
        (void)bbox_unregister_exception(info->e_description[i].e_excepid);
    }

    return;
}

STATIC s32 bbox_ts_save_dump(u32 devid, u32 excepid, u8 etype, const struct bbox_time *time)
{
    s32 ret = 0;
    struct exc_module_info_s *ts_addr = NULL;
    struct rdr_exception_msg_info info = {0};

    BB_CHECK_PTR(time, return BBOX_FAILURE, "invalid param, time is NULL, devid[%u].\n", devid);
    BB_CHECK_EXP_ACT((devid >= bbox_get_device_num()), return BBOX_FAILURE, "invalid parameter. dev: %u.\n", devid);

    if (etype == (u8)HEARTBEAT_EXCEPTION) {
        char *buff = (char *)bbox_vmalloc(BUF_LEN_BIG);
        if (buff == NULL) {
            BB_PRINT_ERR("[%s], bbox_vmalloc error, len[%d].\n", __func__, BUF_LEN_BIG);
            return BBOX_FAILURE;
        }
        ts_addr = (struct exc_module_info_s *)g_ts_vaddr[devid].run_except;
        if (ts_addr == NULL) {
            BB_PRINT_PN("[%s] get ts address error\n", __func__);
            BBOX_VFREE(buff);
            return BBOX_FAILURE;
        }
        bbox_ts_dump_except_header(ts_addr, buff, BUF_LEN_BIG);
        RDR_SET_SUBMIT_INFO(info, devid, excepid, BBOX_TS, etype, *time);
        ret = bbox_submit_module_message(&info, (const char *)buff, (u32)strlen(buff));
        BBOX_VFREE(buff);
    } else if (etype == (u8)STARTUP_EXCEPTION) {
        RDR_SET_SUBMIT_INFO(info, devid, excepid, BBOX_TS, etype, *time);
        ret = bbox_submit_module_message(&info, (const char *)g_ts_vaddr[devid].start_except, TS_STARTLOG_MEM_SIZE);
    }

    return ret;
}

STATIC void bbox_ts_heartbeat_dump(u32 devid, u32 excepid, u8 etype)
{
    s32 result;
    struct exc_module_info_s *except_addr = NULL;

    BB_CHECK_EXP_ACT((devid >= bbox_get_device_num()), return, "invalid parameter. dev: %u.\n", devid);
    BB_PRINT_INFO("[device-%u] ts-proxy: recv ts heartbeat exception info.\n", devid);
    except_addr = (struct exc_module_info_s *)g_ts_vaddr[devid].run_except;
    BB_CHECK_PTR(except_addr, return, "get ts address error.\n");

    result = bbox_ts_verify_except_header(except_addr, etype, excepid);
    except_addr->cur_info.e_save_status = STATUS_DOING;

    // if exception header verify OK, re-upoload the actual exception code
    if ((result == BBOX_SUCCESS) && (except_addr->cur_info.e_excepid != TS_HEART_BEAT_LOST_EXCEPTION)) {
        struct bbox_report_info info = {0};
        info.time.tv_sec = except_addr->cur_info.e_clock.tv_sec;
        info.time.tv_usec = except_addr->cur_info.e_clock.tv_usec;
        info.devid = devid;
        info.excepid = except_addr->cur_info.e_excepid;
        info.arg = 0;
        (void)bbox_exception_report(&info);
    }

    if (g_ts_except_info.pf_dumpdone != NULL) {
        s32 ret;
        struct bbox_dump_done_ops_info dump_done_info = {0};
        struct bbox_time time;

        time.tv_sec = g_ts_except_info.time.tv_sec;
        time.tv_nsec = g_ts_except_info.time.tv_usec * KILO;
        ret = bbox_ts_save_dump(devid, excepid, etype, &time);
        BB_CHECK_RET(ret != BBOX_SUCCESS, "ts-proxy: save moduel dump failed with %d.\n", ret);
        bbox_ts_set_dump_done(&dump_done_info);
        g_ts_except_info.pf_dumpdone(&dump_done_info);
    }
    except_addr->cur_info.e_save_status = STATUS_DONE;
    return;
}

STATIC void bbox_ts_startup_dump(u32 devid, u32 excepid, u8 etype)
{
    struct bbox_dump_done_ops_info dump_done_info = {0};

    // special treatment for start-up and heartbeat exception.
    BB_PRINT_INFO("[device-%u] ts-proxy: recv ts start info.\n", devid);
    if (g_ts_except_info.pf_dumpdone != NULL) {
        s32 ret;
        struct bbox_time time;

        time.tv_sec = g_ts_except_info.time.tv_sec;
        time.tv_nsec = g_ts_except_info.time.tv_usec * KILO;
        ret = bbox_ts_save_dump(devid, excepid, etype, &time);
        BB_CHECK_RET(ret != BBOX_SUCCESS, "ts-proxy: save moduel dump failed with %d.\n", ret);
        bbox_ts_set_dump_done(&dump_done_info);
        g_ts_except_info.pf_dumpdone(&dump_done_info);
    }
}

STATIC s32 bbox_ts_main_thread(void *arg)
{
    bool register_done = false;

    BB_PRINT_INFO("ts-proxy: ts main thread enter.\n");
    UNUSED(arg);
    while (!kthread_should_stop()) {
        long rdr_jiffies = (bbox_long_t)msecs_to_jiffies(TS_WAIT_FOR_DUMP_CYCLE);
        if (!register_done && bbox_ts_register_exceptions() == BBOX_SUCCESS) {
            register_done = true;
        }
        if (down_timeout(&g_ts_except_info.sem, rdr_jiffies)) {
            continue;
        }
        if (g_ts_except_info.etype == (u8)TS_EXCEPTION) {
            bbox_ts_runexecpt_dump(g_ts_except_info.devid, g_ts_except_info.excepid, g_ts_except_info.etype);
        } else if (g_ts_except_info.etype == (u8)HEARTBEAT_EXCEPTION) {
            bbox_ts_heartbeat_dump(g_ts_except_info.devid, g_ts_except_info.excepid, g_ts_except_info.etype);
        } else {
            bbox_ts_startup_dump(g_ts_except_info.devid, g_ts_except_info.excepid, g_ts_except_info.etype);
        }
    }
    BB_PRINT_INFO("ts-proxy: ts main thread exit.\n");
    return BBOX_SUCCESS;
}


/*
 * @brief       : init ts start log buffer
 * @param [in]  : u32 devid           device id
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_ts_startlog_buff_init(u32 devid)
{
    s32 ret;
    bbox_tsconfig *config = &g_ts_paddr_config[devid];

    // kmalloc on specific node
    g_ts_vaddr[devid].start_except = kmalloc_node(TS_STARTLOG_MEM_SIZE, GFP_ATOMIC, (s32)devid);
    if (g_ts_vaddr[devid].start_except == NULL) {
        BB_PRINT_ERR("%s(): ts startlog buffer malloc failed.\n", __func__);
        return BBOX_FAILURE;
    }

    ret = memset_s(g_ts_vaddr[devid].start_except, TS_STARTLOG_MEM_SIZE, 0, TS_STARTLOG_MEM_SIZE);
    if (ret != EOK) {
        BB_PRINT_ERR("%s:%4d memset_s failed, ret(%d).\n", __func__, __LINE__, ret);
        BBOX_KFREE(g_ts_vaddr[devid].start_except);
        return BBOX_FAILURE;
    }

    config->ts_paddr[TS_MNTN_START_LOG_BUFFER].addr = (u64)virt_to_phys(g_ts_vaddr[devid].start_except);
    config->ts_paddr[TS_MNTN_START_LOG_BUFFER].len = TS_STARTLOG_MEM_SIZE;
    return BBOX_SUCCESS;
}

/*
 * @brief       : init ts proxy
 * @return      : <0 failure; =0 success
 */
s32 bbox_ts_init(void)
{
    s32 ret;

    if (g_proxy_ts_init == true) {
        BB_PRINT_INFO("ts-proxy: already initialized.\n");
        return BBOX_SUCCESS;
    }

    ret = memset_s(g_ts_vaddr, sizeof(g_ts_vaddr), 0, sizeof(g_ts_vaddr));
    BB_CHECK_MEMSET(ret, return BBOX_FAILURE);

    ret = memset_s(&g_ts_except_info, sizeof(g_ts_except_info), 0, sizeof(g_ts_except_info));
    BB_CHECK_MEMSET(ret, return BBOX_FAILURE);

    if (bbox_ts_register_core(BBOX_TS)) {
        BB_PRINT_ERR("ts-proxy: bbox_ts_register_core failed.\n");
        return BBOX_FAILURE;
    }

    // register exceptions
    ret = bbox_ts_register_start_exception();
    BB_CHECK_RET(ret != BBOX_SUCCESS, "ts-proxy: register start exception failed.\n");
    ret = bbox_ts_register_heartbeat_exception();
    BB_CHECK_RET(ret != BBOX_SUCCESS, "ts-proxy: register heartbeat exception failed.\n");

    sema_init(&g_ts_except_info.sem, 0);
    g_ts_main_task = kthread_run(bbox_ts_main_thread, NULL, "bbox_ts_main");
    if (g_ts_main_task == NULL) {
        BB_PRINT_ERR("ts-proxy: create bbox_ts_main thread failed.\n");
        (void)bbox_unregister_exception(TS_START_FAIL_EXCEPTION);
        (void)bbox_unregister_exception(TS_HEART_BEAT_LOST_EXCEPTION);
        (void)bbox_unregister_module(BBOX_TS);
        return BBOX_FAILURE;
    }

    g_proxy_ts_init = true;
    return BBOX_SUCCESS;
}

/*
 * @brief       : exit ts proxy
 * @return      : NA
 */
void bbox_ts_exit(void)
{
    u32 dev;

    if (g_proxy_ts_init == true) {
        KTHREAD_STOP(g_ts_main_task);
        BB_PRINT_INFO("ts-proxy: bbox_ts_main thread exit.\n");
        bbox_ts_unregister_exceptions();
        (void)bbox_unregister_module(BBOX_TS);
        g_proxy_ts_init = false;
    }

    for (dev = 0; dev < DEVICE_MAX_NUM; dev++) {
        BBOX_KFREE(g_ts_vaddr[dev].start_except);
        if (g_ts_vaddr[dev].run_except != NULL) {
            rdr_module_free_vaddr((uintptr_t)g_ts_vaddr[dev].run_except);
            g_ts_vaddr[dev].run_except = NULL;
        }
    }
}

/*
 * @brief       : get ts config
 * @param [in]  : u32 devid           device id
 * @return      : ts config info
 */
bbox_tsconfig *bbox_get_tsconfig(u32 devid)
{
    s32 ret;
    u64 addr = 0;
    bbox_tsconfig *config = NULL;

    BB_CHECK_EXP_ACT((devid >= bbox_get_device_num()), return NULL, "invalid parameter. dev: %u.\n", devid);
    config = &g_ts_paddr_config[devid];
    config->enable_bbox = BBOX_TRUE;

    if (g_ts_vaddr[devid].run_except == NULL) {
        ret = rdr_module_get_vaddr(devid, BBOX_TS, &addr);
        BB_CHECK_EXP_ACT((ret == BBOX_FAILURE), return NULL, "[devid-%u] get ts module vaddr failed.\n", devid);
        g_ts_vaddr[devid].run_except = (void *)(uintptr_t)addr;
    }
    if (g_ts_vaddr[devid].start_except == NULL) {
        // ts start log buffer is not initialized, execute buffer initialization
        ret = bbox_ts_startlog_buff_init(devid);
        if (ret < 0) {
            BB_PRINT_ERR("%s(): devid[%u] get ts start mem addr fail.\n", __func__, devid);
            rdr_module_free_vaddr((uintptr_t)g_ts_vaddr[devid].run_except);
            g_ts_vaddr[devid].run_except = NULL;
            return NULL;
        }
    }

    return config;
}
EXPORT_SYMBOL(bbox_get_tsconfig);
