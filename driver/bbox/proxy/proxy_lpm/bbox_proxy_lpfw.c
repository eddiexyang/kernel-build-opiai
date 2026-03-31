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

#include "bbox_proxy_lpfw.h"

#include <asm/cacheflush.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/securectype.h>
#include <linux/securec.h>
#include <linux/semaphore.h>
#include <linux/syscalls.h>

#include "../proxy_common/bbox_proxy_common.h"
#include "bbox_platform.h"
#include "common/bbox_print.h"
#include "device/bbox_pub.h"
#include "device/bbox_pub_cloud.h"
#include "rdr/rdr_common.h"
#include "rdr/rdr_module_core.h"

#define LPFW_HEARTBEAT_EXCEPTION    0xA62FFFFF
#define LPFW_STARTUP_EXCEPTION      0xA82FFFFF
#define LPFW_SPI_REG_ADDR           0x206000050UL
#define LPFW_SPI_REG_VALUE          0x5F
#define IMU_DATA_PHY_ADDR           0x5F800000UL
#define IMU_DATA_TOTAL_LEN          0x800000UL // 8M
#define INTERVAL_MS                 500

STATIC bool g_proxy_lpfw_init = false;

struct bbox_lpfw_dump_info {
    u8 etype;
    u32 devid;
    u32 excepid;
    excep_time time;
    bbox_dump_done_ops done;
};

struct bbox_lpfw_info {
    u64 ddr_addr;
    u32 ddr_len;
    struct semaphore sem;
    struct task_struct *main_thread;
    struct bbox_lpfw_dump_info current_info;
};

STATIC struct bbox_lpfw_info         g_bbox_lpfw_info;
#define g_bbox_lpfw_ddr_len          g_bbox_lpfw_info.ddr_len
#define g_bbox_lpfw_ddr_addr         g_bbox_lpfw_info.ddr_addr
#define g_bbox_lpfw_sem              g_bbox_lpfw_info.sem
#define g_bbox_lpfw_main_thread      g_bbox_lpfw_info.main_thread
#define g_current_etype              g_bbox_lpfw_info.current_info.etype
#define g_current_devid              g_bbox_lpfw_info.current_info.devid
#define g_current_excepid            g_bbox_lpfw_info.current_info.excepid
#define g_current_time               g_bbox_lpfw_info.current_info.time
#define g_current_done               g_bbox_lpfw_info.current_info.done

#define SET_DUMP_DONE_INFO(info) do { \
    (info).devid = g_current_devid; \
    (info).coreid = BBOX_LPFW; \
    (info).excepid = g_current_excepid; \
    (info).etype = g_current_etype; \
    (info).time = g_current_time; \
} while (0)

static void bbox_lpfw_info_init(void)
{
    g_bbox_lpfw_ddr_len = 0;
    g_bbox_lpfw_ddr_addr = 0;
    sema_init(&g_bbox_lpfw_sem, 0);
    g_bbox_lpfw_main_thread = NULL;
    g_current_etype = BBOX_EXCEPTION_REASON_INVALID;
    g_current_devid = 0;
    g_current_excepid = 0;
    g_current_time.tv_sec = 0;
    g_current_time.tv_usec = 0;
    g_current_done = NULL;
}

/*
 * @brief       : register heartbeat exception of lpfw proxy
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_lpfw_register_heartbeat_exception(void)
{
    s32 ret;
    u32 reg_result;
    const char *module = "imu";
    const char *desc = "imu heartbeat lost";
    struct bbox_exception_info einfo = {0};

    ret = memset_s(&einfo, sizeof(einfo), 0, sizeof(einfo));
    BB_CHECK_MEMSET(ret, return BBOX_FAILURE);

    einfo.e_excepid = LPFW_HEARTBEAT_EXCEPTION;
    einfo.e_excepid_end = einfo.e_excepid;
    einfo.e_process_priority = BBOX_MAJOR;
    einfo.e_reboot_priority = BBOX_REBOOT_WAIT;
    einfo.e_notify_core_mask = bbox_coreid_mask(BBOX_LPFW);
    einfo.e_reset_core_mask = bbox_coreid_mask(BBOX_LPFW);
    einfo.e_from_core = BBOX_LPFW;
    einfo.e_reentrant = (u32)BBOX_REENTRANT_DISALLOW;
    einfo.e_exce_type = HEARTBEAT_EXCEPTION;
    ret = memcpy_s(einfo.e_from_module, BBOX_MODULE_NAME_LEN, module, strlen(module));
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);
    ret = memcpy_s(einfo.e_desc, BBOX_EXCEPTIONDESC_MAXLEN, desc, strlen(desc));
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);

    reg_result = bbox_register_exception(&einfo);
    if (!reg_result) {
        BB_PRINT_ERR("lpfw-proxy: bbox_register_exception failed with result[%u]. excepid[%u], excepid_end[%u].\n",
                     reg_result, einfo.e_excepid, einfo.e_excepid_end);
        return BBOX_FAILURE;
    }

    BB_PRINT_INFO("lpfw-proxy: bbox lpfw heartbeat exception register success.\n");
    return BBOX_SUCCESS;
}

/*
 * @brief       : register startup exception of lpfw proxy
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_lpfw_register_startup_exception(void)
{
    s32 ret;
    u32 reg_result;
    const char *module = "imu";
    const char *desc = "imu startup error";
    struct bbox_exception_info einfo = {0};

    ret = memset_s(&einfo, sizeof(einfo), 0, sizeof(einfo));
    BB_CHECK_MEMSET(ret, return BBOX_FAILURE);

    einfo.e_excepid = LPFW_STARTUP_EXCEPTION;
    einfo.e_excepid_end = einfo.e_excepid;
    einfo.e_process_priority = BBOX_CRITICAL;
    einfo.e_reboot_priority = BBOX_REBOOT_WAIT;
    einfo.e_notify_core_mask = bbox_coreid_mask(BBOX_LPFW);
    einfo.e_reset_core_mask = bbox_coreid_mask(BBOX_LPFW);
    einfo.e_from_core = BBOX_LPFW;
    einfo.e_reentrant = (u32)BBOX_REENTRANT_DISALLOW;
    einfo.e_exce_type = STARTUP_EXCEPTION;
    ret = memcpy_s(einfo.e_from_module, BBOX_MODULE_NAME_LEN, module, strlen(module));
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);
    ret = memcpy_s(einfo.e_desc, BBOX_EXCEPTIONDESC_MAXLEN, desc, strlen(desc));
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);

    reg_result = bbox_register_exception(&einfo);
    if (!reg_result) {
        BB_PRINT_ERR("lpfw-proxy: register exception failed with result[%u]. excepid[%u], excepid_end[%u].\n",
                     reg_result, einfo.e_excepid, einfo.e_excepid_end);
        return BBOX_FAILURE;
    }

    BB_PRINT_INFO("lpfw-proxy: bbox lpfw startup exception register success.\n");
    return BBOX_SUCCESS;
}

/*
 * @brief       : get lpfw module info
 * @return      : module info
 */
static struct exc_module_info_s *bbox_lpfw_get_module_info(void)
{
    struct exc_module_info_s *info = NULL;
    void *buffer = rdr_map(0, (phys_addr_t)g_bbox_lpfw_ddr_addr, g_bbox_lpfw_ddr_len);
    BB_CHECK_PTR(buffer, return NULL, "lpfw-proxy: map lpfw ddr address failed.\n");

    info = (struct exc_module_info_s *)buffer;
    if ((info->magic != MODULE_MAGIC) || (info->e_excep_valid != MODULE_VALID)) {
        rdr_unmap(buffer);
        return NULL;
    }

    return info;
}

/*
 * @brief       : register lpfw proxy exception
 * @param [in]  : struct exc_description_s  *e_desc       exception description
 * @param [in]  : u8 *e_from_module                       exception from module
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_lpfw_register_exception(const struct exc_description_s *e_desc, const u8 *e_from_module)
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
    einfo.e_notify_core_mask = bbox_coreid_mask(BBOX_LPFW);
    einfo.e_reset_core_mask = bbox_coreid_mask(BBOX_LPFW);
    einfo.e_from_core = BBOX_LPFW;
    einfo.e_reentrant = e_desc->e_reentrant;
    einfo.e_exce_type = e_desc->e_excep_type;
    ret = memcpy_s(einfo.e_from_module, BBOX_MODULE_NAME_LEN, e_from_module, BBOX_MODULE_NAME_LEN);
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);
    ret = memcpy_s(einfo.e_desc, BBOX_EXCEPTIONDESC_MAXLEN, e_desc->e_desc, BBOX_EXCEPTIONDESC_MAXLEN);
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);

    reg_result = bbox_register_exception(&einfo);
    if (!reg_result) {
        BB_PRINT_ERR("lpfw-proxy: register exception failed with result[%u]. excepid[%u], excepid_end[%u].",
                     reg_result, einfo.e_excepid, einfo.e_excepid_end);
        return BBOX_FAILURE;
    }
    return BBOX_SUCCESS;
}

/*
 * @brief       : register lpfw proxy exceptions
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_lpfw_register_exceptions(void)
{
    s32 i;
    struct exc_module_info_s *info = NULL;

    info = bbox_lpfw_get_module_info();
    if (info == NULL) {
        return BBOX_FAILURE;
    }

    // exception register
    BB_PRINT_INFO("lpfw-proxy: register exception num : %hu.\n", info->e_excep_num);
    for (i = 0; i < BBOX_MIN(info->e_excep_num, MODULE_EXCEPTION_REGISTER_MAXNUM); i++) {
        s32 ret;
        ret = bbox_lpfw_register_exception(&info->e_description[i], info->e_from_module);
        if (ret != BBOX_SUCCESS) {
            BB_PRINT_ERR("lpfw-proxy : register exception [%u] failed.\n", info->e_description[i].e_excepid);
        }
    }

    rdr_unmap(info);
    return BBOX_SUCCESS;
}

/*
 * @brief       : unregister lpfw proxy exceptions
 * @return      : <0 failure; =0 success
 */
STATIC void bbox_lpfw_unregister_exceptions(void)
{
    s32 i;
    struct exc_module_info_s *info = NULL;

    (void)bbox_unregister_exception(LPFW_HEARTBEAT_EXCEPTION);
    (void)bbox_unregister_exception(LPFW_STARTUP_EXCEPTION);

    info = bbox_lpfw_get_module_info();
    BB_CHECK_PTR(info, return, "lpfw-proxy: get lpfw module info failed.\n");

    for (i = 0; i < BBOX_MIN(info->e_excep_num, MODULE_EXCEPTION_REGISTER_MAXNUM); i++) {
        (void)bbox_unregister_exception(info->e_description[i].e_excepid);
    }

    rdr_unmap(info);

    return;
}

/*
 * @brief       : set the value of the spi register
 * @param [in]  : u32 devid                 device id
 * @return      : NA
 */
STATIC void bbox_lpfw_set_spi(u32 devid)
{
    u64 paddr;
    char *lpfw_spi_map_addr = NULL;

    paddr = bbox_get_device_addr(devid, LPFW_SPI_REG_ADDR);
    lpfw_spi_map_addr = (char *)ioremap((phys_addr_t)paddr, 0x4);
    if (lpfw_spi_map_addr != NULL) {
        writel(LPFW_SPI_REG_VALUE, lpfw_spi_map_addr);
        iounmap((void *)lpfw_spi_map_addr);
        lpfw_spi_map_addr = NULL;
    }

    return;
}

/*
 * @brief       : dump imu log when lpfw exception happens
 * @param [in]  : dump_done_info: dump done info for log dump
 * @return      : NA
 */
STATIC void bbox_lpfw_dump_imu_log(const struct bbox_dump_done_ops_info *dump_done_info)
{
    char *imu_map_addr = NULL;

    imu_map_addr = (char *)bbox_ioremap(IMU_DATA_PHY_ADDR, IMU_DATA_TOTAL_LEN);
    BB_CHECK_PTR(imu_map_addr, return, "map imu log address failed.\n");

    (void)rdr_module_log_dump(dump_done_info, g_current_excepid, imu_map_addr, IMU_DATA_TOTAL_LEN);
    bbox_iounmap(imu_map_addr);
    imu_map_addr = NULL;
}

/*
 * @brief       : lpfw exception dump process
 * @return      : <0 failure; ==0 success
 */
STATIC s32 bbox_lpfw_dump_process(void)
{
    s32 wait_timeout = 30000;
    bool set_spi_flag = false;
    struct bbox_dump_done_ops_info dump_done_info = {0};
    void *buffer = rdr_map(g_current_devid, (phys_addr_t)g_bbox_lpfw_ddr_addr, g_bbox_lpfw_ddr_len);
    struct exc_module_info_s *info = (struct exc_module_info_s *)buffer;
    BB_CHECK_PTR(buffer, return BBOX_FAILURE, "lpfw-proxy: map lpfw address failed, device-%u.\n", g_current_devid);

    while (1) {
        if (info->cur_info.e_dump_status == (u16)STATUS_INIT) {
            if (set_spi_flag == false) {
                BB_PRINT_INFO("lpfw-proxy: send spi.\n");
                info->cur_info.e_save_status = (u16)STATUS_INIT;
                /* send spi */
                bbox_lpfw_set_spi(g_current_devid);
                set_spi_flag = true;
            }

            /* 超时机制 */
            if (wait_timeout > 0) {
                msleep(INTERVAL_MS);
                wait_timeout -= INTERVAL_MS;
                continue;
            }
        } else if (info->cur_info.e_dump_status == (u16)STATUS_DOING) {
            /* 超时机制 */
            if (wait_timeout > 0) {
                msleep(INTERVAL_MS);
                wait_timeout -= INTERVAL_MS;
                continue;
            }
        } else if (info->cur_info.e_dump_status == (u16)STATUS_DONE) {
            break;
        }
        break;
    }

    BB_PRINT_INFO("lpfw-proxy: dump over. status: %hu\n", info->cur_info.e_dump_status);
    info->cur_info.e_save_status = (u16)STATUS_DOING;
    if (g_current_done != NULL) {
        SET_DUMP_DONE_INFO(dump_done_info);
        bbox_proxy_module_dump(&dump_done_info, g_current_done);
        bbox_lpfw_dump_imu_log(&dump_done_info);
    }
    info->cur_info.e_save_status = STATUS_INIT;
    info->cur_info.e_dump_status = STATUS_INIT;
    rdr_unmap(buffer);
    return BBOX_SUCCESS;
}

/*
 * @brief       : lpfw proxy main thread
 * @param [in]  : void *arg         thread arg
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_lpfw_main_thread(void *arg)
{
    s32 ret;
    bool register_done = false;

    UNUSED(arg);
    BB_PRINT_INFO("lpfw-proxy: bbox lpfw exception main thread enter.\n");
    while (!kthread_should_stop()) {
        if (!register_done && bbox_lpfw_register_exceptions() == BBOX_SUCCESS) {
            register_done = true;
        }
        if (down_timeout(&g_bbox_lpfw_sem, (bbox_long_t)msecs_to_jiffies(WAIT_MS_LONG))) {
            continue;
        }

        ret = bbox_lpfw_dump_process();
        BB_CHECK_RET(ret != BBOX_SUCCESS, "lpfw-proxy: lpfw dump process failed");
    }
    BB_PRINT_INFO("lpfw-proxy: bbox lpfw exception main thread exit.\n");
    return BBOX_SUCCESS;
}

/*
 * @brief       : dump lpfw proxy exception
 * @param [in]  : struct bbox_dump_ops_info *info           dump info
 * @param [in]  : bbox_dump_done_ops  fndone                dump done callback function
 * @return      : NA
 */
STATIC void bbox_lpfw_dump(const struct bbox_dump_ops_info *info, bbox_dump_done_ops fndone)
{
    BB_CHECK_PTR(info, return, "invalid param, info is NULL.\n");
    BB_CHECK_PTR(fndone, return, "invalid param, fndone is NULL.\n");
    BB_CHECK_EXP_ACT((info->coreid != (u8)BBOX_LPFW), return, "invalid param, coreid[0x%hhx].\n", info->coreid);

    g_current_devid = info->devid;
    g_current_excepid = info->excepid;
    g_current_done = fndone;
    g_current_etype = info->etype;
    g_current_time.tv_sec = info->time.tv_sec;
    g_current_time.tv_usec = info->time.tv_usec;
    up(&g_bbox_lpfw_sem);
    return;
}

/*
 * @brief       : init lpfw proxy
 * @return      : <0 failure; =0 success
 */
s32 bbox_lpfw_init(void)
{
    s32 ret;
    struct bbox_module_result info = {0, 0};
    struct bbox_module_info s_module_ops = {BBOX_LPFW, NULL, NULL};

    if (g_proxy_lpfw_init) {
        BB_PRINT_INFO("lpfw proxy is already initialized");
        return BBOX_SUCCESS;
    }

    bbox_lpfw_info_init();

    /* module register */
    s_module_ops.ops_dump = bbox_lpfw_dump;
    s_module_ops.ops_reset = NULL;
    ret = bbox_register_module(&s_module_ops, &info);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("lpfw-proxy: bbox_register_module failed! return %d\n", ret);
        return BBOX_FAILURE;
    }

    /* addr set */
    if ((info.log_addr == 0) || (info.log_len == 0)) {
        BB_PRINT_ERR("lpfw-proxy: get lpfw addr failed!");
        (void)bbox_unregister_module(s_module_ops.coreid);
        return BBOX_FAILURE;
    }
    g_bbox_lpfw_ddr_len = info.log_len;
    g_bbox_lpfw_ddr_addr = info.log_addr;

    // register exceptions
    ret = bbox_lpfw_register_heartbeat_exception();
    BB_CHECK_RET(ret != BBOX_SUCCESS, "lpfw-proxy: register heartbeat exception failed");
    ret = bbox_lpfw_register_startup_exception();
    BB_CHECK_RET(ret != BBOX_SUCCESS, "lpfw-proxy: register startup exception failed");

    /* main thread */
    g_bbox_lpfw_main_thread = kthread_run(bbox_lpfw_main_thread, NULL, "bbox_lpfw_main");
    if (g_bbox_lpfw_main_thread == NULL) {
        BB_PRINT_ERR("lpfw-proxy: create thread bbox_lpfw_main_thread failed.\n");
        (void)bbox_unregister_exception(LPFW_HEARTBEAT_EXCEPTION);
        (void)bbox_unregister_exception(LPFW_STARTUP_EXCEPTION);
        (void)bbox_unregister_module(s_module_ops.coreid);
        return BBOX_FAILURE;
    }

    g_proxy_lpfw_init = true;
    return BBOX_SUCCESS;
}

/*
 * @brief       : exit lpfw proxy
 * @return      : NA
 */
void bbox_lpfw_exit(void)
{
    BB_PRINT_INFO("bbox lpfw start exit.\n");

    if (g_proxy_lpfw_init) {
        KTHREAD_STOP(g_bbox_lpfw_main_thread);
        bbox_lpfw_unregister_exceptions();
        (void)bbox_unregister_module(BBOX_LPFW);
        bbox_lpfw_info_init();
        g_proxy_lpfw_init = false;
    }

    BB_PRINT_INFO("bbox lpfw end exit.\n");
    return;
}
