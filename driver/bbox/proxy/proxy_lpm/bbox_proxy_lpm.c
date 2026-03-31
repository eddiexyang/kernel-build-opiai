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

#include <asm/cacheflush.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/securec.h>
#include <linux/securectype.h>
#include <linux/semaphore.h>
#include <linux/syscalls.h>
#include <linux/vmalloc.h>

#include "../proxy_common/bbox_proxy_common.h"
#include "bbox_platform.h"
#include "common/bbox_print.h"
#include "device/bbox_pub.h"
#include "device/bbox_pub_mini.h"
#include "rdr/rdr_common.h"

#define LPM3_HEARTBEAT_EXCEPTION    0xA619FFFF
#define LPM3_STARTUP_EXCEPTION      0xA819FFFF

STATIC bool g_proxy_lpm_init = false;

struct bbox_lpm_dump_info {
    u8 etype;
    u32 excepid;
    excep_time time;
    bbox_dump_done_ops done;
};

struct bbox_lpm_info {
    u64 ddr_vaddr;
    u32 ddr_len;
    struct semaphore sem;
    struct task_struct *main_thread;
    struct bbox_lpm_dump_info current_info;
};

STATIC struct bbox_lpm_info         g_bbox_lpm_info;
#define g_bbox_lpm_ddr_len          g_bbox_lpm_info.ddr_len
#define g_bbox_lpm_ddr_vaddr        g_bbox_lpm_info.ddr_vaddr
#define g_bbox_lpm_sem              g_bbox_lpm_info.sem
#define g_bbox_lpm_main_thread      g_bbox_lpm_info.main_thread
#define g_current_etype             g_bbox_lpm_info.current_info.etype
#define g_current_excepid           g_bbox_lpm_info.current_info.excepid
#define g_current_time              g_bbox_lpm_info.current_info.time
#define g_current_done              g_bbox_lpm_info.current_info.done

#define SET_DUMP_DONE_INFO(info) do { \
    (info).devid = 0; \
    (info).coreid = BBOX_LPM; \
    (info).excepid = g_current_excepid; \
    (info).etype = g_current_etype; \
    (info).time = g_current_time; \
} while (0)

static void bbox_lpm_info_init(void)
{
    g_bbox_lpm_ddr_len = 0;
    g_bbox_lpm_ddr_vaddr = 0;
    sema_init(&g_bbox_lpm_sem, 0);
    g_bbox_lpm_main_thread = NULL;
    g_current_etype = BBOX_EXCEPTION_REASON_INVALID;
    g_current_excepid = 0;
    g_current_time.tv_sec = 0;
    g_current_time.tv_usec = 0;
    g_current_done = NULL;
}

STATIC void fn_lpm3_dump(const struct bbox_dump_ops_info *info, bbox_dump_done_ops pfn_cb)
{
    BB_CHECK_PTR(info, return, "invalid param, info is NULL.\n");
    BB_CHECK_PTR(pfn_cb, return, "invalid param, pfn_cb is NULL.\n");
    BB_CHECK_EXP_ACT((info->coreid != (u8)BBOX_LPM), return, "invalid param, coreid[0x%hhx].\n", info->coreid);

    g_current_done = pfn_cb;
    g_current_excepid = info->excepid;
    g_current_etype = info->etype;
    g_current_time.tv_sec = info->time.tv_sec;
    g_current_time.tv_usec = info->time.tv_usec;
    up(&g_bbox_lpm_sem);
    return;
}

STATIC void fn_lpm3_reset(const struct bbox_reset_ops_info *info)
{
    BB_CHECK_PTR(info, return, "invalid param, info is NULL.\n");
    return;
}

#define LPM3_NMI_ADDR                0x10c0020b0
#define LPM3_NMI_ADDR_26BIT_MASK     (1 << 26)

/*
 * @brief       : set the value of the nmi register
 * @return      : NA
 */
STATIC void bbox_lpm_set_nmi(void)
{
    char *lpm3_nmi_map_addr = NULL;

    lpm3_nmi_map_addr = (char *)ioremap(LPM3_NMI_ADDR, 0x4);
    if (lpm3_nmi_map_addr != NULL) {
        unsigned long value = readl(lpm3_nmi_map_addr);
        value |= LPM3_NMI_ADDR_26BIT_MASK;
        writel(value, lpm3_nmi_map_addr);
        iounmap((void *)lpm3_nmi_map_addr);
    }
    return;
}

/*
 * @brief       : lpm3 exception dump process
 * @return      : NA
 */
STATIC void bbox_lpm_dump(void)
{
    s32 wait_timeout = 15000;
    s32 inter_ms = 500;
    bool set_nmi_flag = false;
    struct exc_module_info_s *info = NULL;
    struct bbox_dump_done_ops_info dump_done_info = {0};

    BB_PRINT_START();
    info = (struct exc_module_info_s *)(uintptr_t)g_bbox_lpm_ddr_vaddr;
    while (1) {
        if (info->cur_info.e_dump_status == (u16)STATUS_INIT) {
            if (set_nmi_flag == false) {
                BB_PRINT_INFO("lpm-proxy: send nmi.\n");
                info->cur_info.e_save_status = STATUS_INIT;
                /* send nmi */
                bbox_lpm_set_nmi();
                set_nmi_flag = true;
            }

            /* timeout opreate */
            if (wait_timeout > 0) {
                msleep((u32)inter_ms);
                wait_timeout -= inter_ms;
                continue;
            }
            break;
        } else if (info->cur_info.e_dump_status == (u16)STATUS_DOING) {
            /* timeout opreate */
            if (wait_timeout > 0) {
                msleep((u32)inter_ms);
                wait_timeout -= inter_ms;
                continue;
            }
            break;
        } else if (info->cur_info.e_dump_status == (u16)STATUS_DONE) {
            break;
        }

        break;
    }

    BB_PRINT_INFO("lpm-proxy: dump over. status: %hu.\n", info->cur_info.e_dump_status);
    info->cur_info.e_save_status = STATUS_DOING;
    if (g_current_done != NULL) {
        SET_DUMP_DONE_INFO(dump_done_info);
        bbox_proxy_module_dump(&dump_done_info, g_current_done);
    }
    info->cur_info.e_save_status = STATUS_INIT;
    info->cur_info.e_dump_status = STATUS_INIT;
    BB_PRINT_INFO("lpm-proxy dump over.\n");
    BB_PRINT_END();
}

/*
 * @brief       : register heartbeat exception of lpm proxy
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_lpm_register_heartbeat_exception(void)
{
    s32 ret;
    u32 reg_result;
    const char *module = "lpm3";
    const char *desc = "lpm3 heartbeat lost";
    struct bbox_exception_info einfo = {0};

    ret = memset_s(&einfo, sizeof(einfo), 0, sizeof(einfo));
    BB_CHECK_MEMSET(ret, return BBOX_FAILURE);

    einfo.e_excepid = LPM3_HEARTBEAT_EXCEPTION;
    einfo.e_excepid_end = einfo.e_excepid;
    einfo.e_process_priority = BBOX_MAJOR;
    einfo.e_reboot_priority = BBOX_REBOOT_WAIT;
    einfo.e_notify_core_mask = bbox_coreid_mask(BBOX_LPM);
    einfo.e_reset_core_mask = bbox_coreid_mask(BBOX_LPM);
    einfo.e_from_core = BBOX_LPM;
    einfo.e_reentrant = BBOX_REENTRANT_DISALLOW;
    einfo.e_exce_type = HEARTBEAT_EXCEPTION;
    ret = memcpy_s(einfo.e_from_module, BBOX_MODULE_NAME_LEN, module, strlen(module));
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);
    ret = memcpy_s(einfo.e_desc, BBOX_EXCEPTIONDESC_MAXLEN, desc, strlen(desc));
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);

    reg_result = bbox_register_exception(&einfo);
    if (!reg_result) {
        BB_PRINT_ERR("lpm-proxy: register exception failed with result[%u]. excepid[%u], excepid_end[%u].\n",
                     reg_result, einfo.e_excepid, einfo.e_excepid_end);
        return BBOX_FAILURE;
    }

    BB_PRINT_INFO("lpm-proxy: bbox lpm heartbeat exception register success.\n");
    return BBOX_SUCCESS;
}

/*
 * @brief       : register startup exception of lpm proxy
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_lpm_register_startup_exception(void)
{
    s32 ret;
    u32 reg_result;
    const char *module = "lpm3";
    const char *desc = "lpm3 startup error";
    struct bbox_exception_info einfo = {0};

    ret = memset_s(&einfo, sizeof(einfo), 0, sizeof(einfo));
    BB_CHECK_MEMSET(ret, return BBOX_FAILURE);

    einfo.e_excepid = LPM3_STARTUP_EXCEPTION;
    einfo.e_excepid_end = einfo.e_excepid;
    einfo.e_process_priority = BBOX_CRITICAL;
    einfo.e_reboot_priority = BBOX_REBOOT_WAIT;
    einfo.e_notify_core_mask = bbox_coreid_mask(BBOX_LPM);
    einfo.e_reset_core_mask = bbox_coreid_mask(BBOX_LPM);
    einfo.e_from_core = BBOX_LPM;
    einfo.e_reentrant = BBOX_REENTRANT_DISALLOW;
    einfo.e_exce_type = STARTUP_EXCEPTION;
    ret = memcpy_s(einfo.e_from_module, BBOX_MODULE_NAME_LEN, module, strlen(module));
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);
    ret = memcpy_s(einfo.e_desc, BBOX_EXCEPTIONDESC_MAXLEN, desc, strlen(desc));
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);

    reg_result = bbox_register_exception(&einfo);
    if (!reg_result) {
        BB_PRINT_ERR("lpm-proxy: register exception failed with result[%u]. excepid[%u], excepid_end[%u].\n",
                     reg_result, einfo.e_excepid, einfo.e_excepid_end);
        return BBOX_FAILURE;
    }

    BB_PRINT_INFO("lpm-proxy: bbox lpm startup exception register success.\n");
    return BBOX_SUCCESS;
}

/*
 * @brief       : get lpm module info
 * @return      : module info
 */
static struct exc_module_info_s *bbox_lpm_get_module_info(void)
{
    struct exc_module_info_s *info = (struct exc_module_info_s *)(uintptr_t)g_bbox_lpm_ddr_vaddr;
    BB_CHECK_PTR(info, return NULL, "invalid param, info is NULL.\n");

    if ((info->magic != MODULE_MAGIC) || (info->e_excep_valid != MODULE_VALID)) {
        return NULL;
    }
    return info;
}

/*
 * @brief       : register lpm proxy exception
 * @param [in]  : struct exc_description_s  *e_desc       exception description
 * @param [in]  : u8 *e_from_module                       exception from module
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_lpm_register_exception(const struct exc_description_s *e_desc, const u8 *e_from_module)
{
    s32 ret;
    u32 reg_result;
    struct bbox_exception_info einfo = {0};

    ret = memset_s(&einfo, sizeof(einfo), 0, sizeof(einfo));
    BB_CHECK_MEMSET(ret, return BBOX_FAILURE);

    einfo.e_excepid = e_desc->e_excepid;
    einfo.e_excepid_end = einfo.e_excepid;
    einfo.e_process_priority = e_desc->e_process_level;
    einfo.e_reboot_priority = e_desc->e_reboot_priority;
    einfo.e_notify_core_mask = bbox_coreid_mask(BBOX_LPM);
    einfo.e_reset_core_mask = bbox_coreid_mask(BBOX_LPM);
    einfo.e_from_core = BBOX_LPM;
    einfo.e_reentrant = e_desc->e_reentrant;
    einfo.e_exce_type = e_desc->e_excep_type;
    ret = memcpy_s(einfo.e_from_module, BBOX_MODULE_NAME_LEN, e_from_module, BBOX_MODULE_NAME_LEN);
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);
    ret = memcpy_s(einfo.e_desc, BBOX_EXCEPTIONDESC_MAXLEN, e_desc->e_desc, BBOX_EXCEPTIONDESC_MAXLEN);
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);

    reg_result = bbox_register_exception(&einfo);
    if (reg_result == 0) {
        BB_PRINT_ERR("lpm-proxy: register exception failed with result[%u]. excepid[%u], excepid_end[%u].",
                     reg_result, einfo.e_excepid, einfo.e_excepid_end);
        return BBOX_FAILURE;
    }
    return BBOX_SUCCESS;
}

/*
 * @brief       : register lpm proxy exceptions
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_lpm_register_exceptions(void)
{
    s32 i;
    struct exc_module_info_s *info = NULL;

    info = bbox_lpm_get_module_info();
    if (info == NULL) {
        return BBOX_FAILURE;
    }

    // exception register
    BB_PRINT_INFO("lpm-proxy: register exception num : %hu.\n", info->e_excep_num);
    for (i = 0; i < BBOX_MIN(info->e_excep_num, MODULE_EXCEPTION_REGISTER_MAXNUM); i++) {
        s32 ret;
        ret = bbox_lpm_register_exception(&info->e_description[i], info->e_from_module);
        if (ret != BBOX_SUCCESS) {
            BB_PRINT_ERR("lpm-proxy : register exception [%u] failed.\n", info->e_description[i].e_excepid);
        }
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : lpm proxy main thread
 * @param [in]  : void *arg         thread arg
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_lpm_main_thread(void *arg)
{
    struct bbox_dump_done_ops_info dump_done_info = {0};
    bool register_done = false;

    UNUSED(arg);
    BB_PRINT_INFO("lpm-proxy: bbox lpm exception main thread enter.\n");
    while (!kthread_should_stop()) {
        if (!register_done && bbox_lpm_register_exceptions() == BBOX_SUCCESS) {
            register_done = true;
        }
        if (down_timeout(&g_bbox_lpm_sem, (bbox_long_t)msecs_to_jiffies(WAIT_MS_LONG))) {
            continue;
        }

        if (g_current_excepid != LPM3_STARTUP_EXCEPTION) {
            bbox_lpm_dump();
        } else if (g_current_done != NULL) {
            SET_DUMP_DONE_INFO(dump_done_info);
            bbox_proxy_module_dump(&dump_done_info, g_current_done);
        }
    }

    BB_PRINT_INFO("lpm-proxy: bbox lpm exception main thread exit.\n");
    return 0;
}

/*
 * @brief       : unegister lpm proxy exceptions
 * @return      : <0 failure; =0 success
 */
STATIC void bbox_lpm_unregister_exceptions(void)
{
    s32 i;
    struct exc_module_info_s *info = NULL;

    (void)bbox_unregister_exception(LPM3_HEARTBEAT_EXCEPTION);
    (void)bbox_unregister_exception(LPM3_STARTUP_EXCEPTION);

    info = bbox_lpm_get_module_info();
    BB_CHECK_PTR(info, return, "lpm-proxy: get lpm3 module info failed.\n");

    for (i = 0; i < BBOX_MIN(info->e_excep_num, MODULE_EXCEPTION_REGISTER_MAXNUM); i++) {
        (void)bbox_unregister_exception(info->e_description[i].e_excepid);
    }

    return;
}

/*
 * @brief       : init lpm proxy
 * @return      : <0 failure; =0 success
 */
s32 bbox_lpm_init(void)
{
    s32 ret;
    struct bbox_module_result info = {0, 0};
    struct bbox_module_info s_module_ops = {BBOX_LPM, NULL, NULL};

    if (g_proxy_lpm_init == true) {
        BB_PRINT_INFO("lpm proxy is already initialized");
        return BBOX_SUCCESS;
    }

    // init global data
    bbox_lpm_info_init();

    // module register
    s_module_ops.ops_dump = fn_lpm3_dump;
    s_module_ops.ops_reset = fn_lpm3_reset;
    ret = bbox_register_module(&s_module_ops, &info);
    if (ret != 0) {
        BB_PRINT_ERR("lpm-proxy: bbox_register_module failed! return %d\n", ret);
        return BBOX_FAILURE;
    }

    // addr set
    g_bbox_lpm_ddr_len = info.log_len;
    g_bbox_lpm_ddr_vaddr = (u64)(uintptr_t)rdr_map(0, (phys_addr_t)info.log_addr, info.log_len);
    if ((g_bbox_lpm_ddr_vaddr == 0) || (g_bbox_lpm_ddr_len == 0)) {
        BB_PRINT_ERR("lpm-proxy: get lpm addr failed!");
        (void)bbox_unregister_module(s_module_ops.coreid);
        return BBOX_FAILURE;
    }

    // register exceptions
    ret = bbox_lpm_register_heartbeat_exception();
    BB_CHECK_RET(ret != BBOX_SUCCESS, "lpm-proxy: register heartbeat exception failed.\n");
    ret = bbox_lpm_register_startup_exception();
    BB_CHECK_RET(ret != BBOX_SUCCESS, "lpm-proxy: register startup exception failed.\n");

    // main thread
    g_bbox_lpm_main_thread = kthread_run(bbox_lpm_main_thread, NULL, "bbox_lpm__main");
    if (g_bbox_lpm_main_thread == NULL) {
        BB_PRINT_ERR("lpm-proxy: create thread bbox_lpm__main_thread failed.\n");
        (void)bbox_unregister_exception(LPM3_HEARTBEAT_EXCEPTION);
        (void)bbox_unregister_exception(LPM3_STARTUP_EXCEPTION);
        rdr_unmap((void *)(uintptr_t)g_bbox_lpm_ddr_vaddr);
        g_bbox_lpm_ddr_vaddr = 0;
        (void)bbox_unregister_module(s_module_ops.coreid);
        return BBOX_FAILURE;
    }

    g_proxy_lpm_init = true;
    return BBOX_SUCCESS;
}

/*
 * @brief       : exit lpm proxy
 * @return      : NA
 */
void bbox_lpm_exit(void)
{
    BB_PRINT_INFO("bbox lpm start exit.\n");
    if (g_proxy_lpm_init == true) {
        KTHREAD_STOP(g_bbox_lpm_main_thread);
        BB_PRINT_INFO("bbox lpm main thread exit.\n");
        bbox_lpm_unregister_exceptions();
        (void)bbox_unregister_module(BBOX_LPM);
        rdr_unmap((void *)(uintptr_t)g_bbox_lpm_ddr_vaddr);
        bbox_lpm_info_init();
        g_proxy_lpm_init = false;
    }
    BB_PRINT_INFO("bbox lpm end exit.\n");
    return;
}
