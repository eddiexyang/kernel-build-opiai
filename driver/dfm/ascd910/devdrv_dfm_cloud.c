/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
* Create: 2019-10-15
*/

#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/securec.h>
#include <asm/io.h>
#include <linux/securectype.h>
#include <linux/topology.h>
#include <linux/version.h>
#ifndef UT_TEST
#include "kernel_version_adapt.h"
#endif
#include "devdrv_dfm.h"
#include "devdrv_dfm_cloud.h"
#include "ascend_platform.h"

#ifndef STATIC
#ifdef UT_TEST
#define STATIC
#else
#define STATIC static
#endif
#endif

STATIC void dfm_driver_dump(u32 dev_id, u8 coreid, u32 excep_id, u32 etype);
#ifdef CFG_FEATURE_DFM_WITH_DVPP
STATIC void dfm_dvpp_dump(u32 dev_id, u8 coreid, u32 excep_id, u32 etype);
#endif

#define STUB_SIZE               35
#define DFM_TIME_STAMP_LENGTH   30
#define DFM_TIME_START_YEAR     1900
#define DFM_NUM_OF_MS_PER_SEC   1000
#define DFM_LOG_BUF_HEAD_LENGTH 28
#define DFM_MONTH_COMPENSATION  1

#define DFM_TIME_SYNC_MAX_DELAY 3600 /* time should be sync in 1 hour */

STATIC unsigned int g_dev_num = 0;

struct dfm_mng_info {
    unsigned int init_success_flag;

    unsigned int driver_log_len;
    unsigned long long driver_log_paddr;
    unsigned long long driver_log_vaddr[DFM_MAX_DAVINCI_NUM];

    unsigned int dvpp_log_len;
    unsigned long long dvpp_log_paddr;
    unsigned long long dvpp_log_vaddr[DFM_MAX_DAVINCI_NUM];

    spinlock_t spinlock;
    dfm_dump ops_dvpp_dump;                      /* DVPP模块dump接口 */
    dfm_dump ops_drv_dump[DFM_SUBMODULE_ID_END]; /* driver各子模块dump接口 */
    dfm_dump_ex ops_drv_dump_ex[DFM_SUBMODULE_ID_END]; /* driver各子模块dump接口 */
};

STATIC struct dfm_mng_info g_dfm_info = { 0 };
STATIC struct dfm_exception_info g_drv_excep_list[] = {
#ifdef CFG_FEATURE_DFM_WITH_DVPP
    /* DVPP */
    { DVPP_JPEGD_IRQ_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "dvpp", "JPEGD irq failed" },
    { DVPP_JPEGE_IRQ_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "dvpp", "JPEGE irq failed" },
    { DVPP_VPC_IRQ_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "dvpp", "VPC irq failed" },
    { DVPP_PNGD_IRQ_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "dvpp", "PNGD irq failed" },
    { DVPP_VDEC_IRQ_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "dvpp", "VDEC irq failed" },
    { DVPP_JPEGD_PROC_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_NOTICE, RDR_REBOOT_NO, "dvpp",
      "JPEGD PROC failed" },
    { DVPP_JPEGE_PROC_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_NOTICE, RDR_REBOOT_NO, "dvpp",
      "JPEGE PROC failed" },
    { DVPP_VPC_PROC_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_NOTICE, RDR_REBOOT_NO, "dvpp", "VPC PROC failed" },
    { DVPP_PNGD_PROC_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_NOTICE, RDR_REBOOT_NO,
      "dvpp", "PNGD PROC failed" },
    { DVPP_VDEC_PROC_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_NOTICE, RDR_REBOOT_NO,
      "dvpp", "VDEC PROC failed" },
    { DVPP_CMD_LIST_PROC_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_NOTICE, RDR_REBOOT_NO, "dvpp",
      "CMD_LIST PROC failed" },
#endif
    /* AGENTDRV */
    { DFM_AGENTDRV_DMA_PROBE_FAIL, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "driver",
      "dma probe fail" },
    { DFM_AGENTDRV_SDIO_PROBE_FAIL, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "driver",
      "sdio probe fail" },
    { DFM_AGENTDRV_PM_PROBE_FAIL, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "driver",
      "pm probe fail" },

    /* APPMOND */
    { APPMON_IDLE_RECOVERY, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO, "ascend_monitor",
      "ascend_monitor adda restart success" },
    { APPMON_IDLE_HEART_BEAT_LOST, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO,
      "ascend_monitor", "ascend_monitor adda heart beat lost error" },
    { APPMON_IDLE_RAISE_FAIL, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO, "ascend_monitor",
      "ascend_monitor adda raise failed error" },
    { APPMON_SLOGD_RECOVERY, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO, "ascend_monitor",
      "ascend_monitor slogd restart success" },
    { APPMON_SLOGD_HEART_BEAT_LOST, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO,
      "ascend_monitor", "ascend_monitor slogd heart beat lost error" },
    { APPMON_SLOGD_RAISE_FAIL, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO, "ascend_monitor",
      "ascend_monitor slogd raise failed error" },
    { APPMON_SKLOGD_RECOVERY, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO, "ascend_monitor",
      "ascend_monitor sklogd restart success" },
    { APPMON_SKLOGD_HEART_BEAT_LOST, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO,
      "ascend_monitor", "ascend_monitor sklogd heart beat lost error" },
    { APPMON_SKLOGD_RAISE_FAIL, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO, "ascend_monitor",
      "ascend_monitor sklogd raise failed error" },
    { APPMON_DMP_RECOVERY, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO, "ascend_monitor",
      "ascend_monitor dmp restart success" },
    { APPMON_DMP_HEART_BEAT_LOST, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO,
      "ascend_monitor", "ascend_monitor dmp heart beat lost error" },
    { APPMON_DMP_RAISE_FAIL, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO, "ascend_monitor",
      "ascend_monitor dmp raise failed error" },
    { APPMON_TSDAEMON_RECOVERY, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO,
      "ascend_monitor", "ascend_monitor tsdaemon restart success" },
    { APPMON_TSDAEMON_HEART_BEAT_LOST, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO,
      "ascend_monitor", "ascend_monitor tsdaemon heart beat lost error" },
    { APPMON_TSDAEMON_RAISE_FAIL, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO,
      "ascend_monitor", "ascend_monitor tsdaemon raise failed error" },
    { APPMON_DEVMM_RECOVERY, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO, "ascend_monitor",
      "ascend_monitor devmm restart success" },
    { APPMON_DEVMM_HEART_BEAT_LOST, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO,
      "ascend_monitor", "ascend_monitor devmm heart beat lost error" },
    { APPMON_DEVMM_RAISE_FAIL, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO, "ascend_monitor",
      "ascend_monitor devmm raise failed error" },
    { APPMON_LOG_RECOVERY, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO, "ascend_monitor",
      "ascend_monitor log restart success" },
    { APPMON_LOG_HEART_BEAT_LOST, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO,
      "ascend_monitor", "ascend_monitor log heart beat lost error" },
    { APPMON_LOG_RAISE_FAIL, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO, "ascend_monitor",
      "ascend_monitor log raise failed error" },
    { APPMON_HDCD_RECOVERY, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO, "ascend_monitor",
      "ascend_monitor hdcd restart success" },
    { APPMON_HDCD_HEART_BEAT_LOST, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO,
      "ascend_monitor", "ascend_monitor hdcd heart beat lost error" },
    { APPMON_HDCD_RAISE_FAIL, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO, "ascend_monitor",
      "ascend_monitor hdcd raise failed error" },
    { FW_SYNC_FAIL, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_NOTICE, RDR_REBOOT_NO, "FIRMWARE_SYNC",
      "firmware sync abnormal need rollback" },
    { DVPP_JPEGD_PROC_RECOVER_ID, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_NOTICE, RDR_REBOOT_NO, "dvpp",
      "jpegd engines have recovered" },
    { DVPP_JPEGE_PROC_RECOVER_ID, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_NOTICE, RDR_REBOOT_NO, "dvpp",
      "jpege engines have recovered" },
    { DVPP_VPC_PROC_RECOVER_ID, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_NOTICE, RDR_REBOOT_NO, "dvpp",
      "vpc engines have recovered" },
    { DVPP_PNGD_PROC_RECOVER_ID, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_NOTICE, RDR_REBOOT_NO, "dvpp",
      "pngd engines have recovered" },
    { DVPP_VDEC_PROC_RECOVER_ID, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_NOTICE, RDR_REBOOT_NO, "dvpp",
      "vdec engines have recovered" },

    /* Multi ECC ERROR */
    { ECC_EXCEED_16_WARNING, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_NOTICE, RDR_REBOOT_NO, "storage",
      "ddr/hbm multi bits ecc errors exceeds 16" },
    { ECC_REACH_64_WARNING, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO, "storage",
      "total ddr/hbm multi ecc errors reaches 64" },
    { ECC_APPEAR_WARNING, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_NO, "storage",
      "appear new multi bits ecc error" },
};

/*
 * func name: bbox_reset
 * func args:
 *    u32   dev_id              device id
 *    u8    core_id             exception core
 *    u32   excep_id            exception id
 *    u8    etype               exception type
 * return value     null
 */
void dfm_reset_bbox(u32 dev_id, u8 core_id, u32 excep_id, u8 etype)
{
    return;
}

// exception id  excep_id actual not used, we default zero for it
void dfm_driver_dump_match_bbox(u32 dev_id, u8 coreid, u32 excep_id, u8 etype, const excep_time *time, u32 arg,
                                bbox_cb_dump_done fndone)
{
    dfm_driver_dump(dev_id, coreid, excep_id, etype);
    if (fndone != NULL) {
        fndone(dev_id, coreid, excep_id, etype, time);
    }
    return;
}

/* DRV模块dump回调接口 */
STATIC void dfm_driver_dump(u32 dev_id, u8 coreid, u32 excep_id, u32 etype)
{
    u32 i;
    char *buf = NULL;
    u32 buflen = g_dfm_info.driver_log_len;
    u32 loglen;
    u32 first_flag[DFM_MAX_DAVINCI_NUM] = { 0 };
    unsigned long flags;
    int ret;

    if (dev_id >= g_dev_num) {
        dfm_err("dev id is invalid, dev_id = %u, coreid = %u.\n", dev_id, coreid);
        return;
    }

    for (i = 0; i < DFM_SUBMODULE_ID_END; i++) {
        if (((g_dfm_info.ops_drv_dump[i] == NULL) && (g_dfm_info.ops_drv_dump_ex[i] == NULL))
            || (g_dfm_info.driver_log_paddr == 0) || (g_dfm_info.driver_log_len == 0)) {
            continue;
        }

        buf = NULL;

        g_dfm_info.ops_drv_dump[i](dev_id, excep_id, etype, coreid, &buf);

        if (buf == NULL) {
            continue;
        }

        loglen = strlen(buf);
        if ((loglen >= (buflen - 1)) || (loglen == 0)) {
            continue;
        }

        if (first_flag[dev_id] == 0) {
            (void)dfm_write_black_box(dev_id, DFM_MODULE_ID_DRIVER, i, buf);
            first_flag[dev_id] = 1;
        } else {
            spin_lock_irqsave(&g_dfm_info.spinlock, flags);
            ret = strncat_s((void *)(uintptr_t)g_dfm_info.driver_log_vaddr[dev_id], buflen, buf, loglen);
            if (ret) {
                spin_unlock_irqrestore(&g_dfm_info.spinlock, flags);
                dfm_err("dev_id(%u) coreid(%u): strncat_s failed , ret=%d.\n", dev_id, coreid, ret);
                return;
            }
            spin_unlock_irqrestore(&g_dfm_info.spinlock, flags);
        }
        buflen -= loglen;
    }
}

#ifdef CFG_FEATURE_DFM_WITH_DVPP
// exception id excep_id actual not used, we default zero for it
void dfm_dvpp_dump_match_bbox(u32 dev_id, u8 coreid, u32 excep_id, u8 etype, const excep_time *time, u32 arg,
                              bbox_cb_dump_done fndone)
{
    dfm_dvpp_dump(dev_id, excep_id, etype, coreid);
    if (fndone != NULL) {
        fndone(dev_id, coreid, excep_id, etype, time);
    }
    return;
}

/* DVPP模块dump回调接口 */
STATIC void dfm_dvpp_dump(u32 dev_id, u8 coreid, u32 excep_id, u32 etype)
{
    char *buf = NULL;
    int ret;

    if ((g_dfm_info.ops_dvpp_dump == NULL) || (g_dfm_info.dvpp_log_paddr == 0) || (g_dfm_info.dvpp_log_len == 0)) {
        dfm_err("dev_id(%u) coreid(%u): dfm_dvpp_dump.\n", dev_id, coreid);
        return;
    }

    g_dfm_info.ops_dvpp_dump(dev_id, excep_id, etype, coreid, &buf);

    ret = dfm_write_black_box(dev_id, DFM_MODULE_ID_DVPP, 0, buf);
    if (ret != DFM_OK) {
        dfm_err("dev_id(%u) coreid(%u): dfm_write_black_box erro, module_ID = %d.\n", dev_id, coreid,
                DFM_MODULE_ID_DVPP);
    }

    return;
}
#endif

/* unregister exception to RDR */
int dfm_unregister_exception(struct dfm_exception_info *excep, unsigned int num)
{
    u32 ex_id = 0;
    u32 i;
    u32 ret;
    for (i = 0; i < num; i++) {
        if ((strlen(excep->module_name) >= (RDR_MODULE_NAME_LEN - 1)) ||
            (strlen(excep->desc) >= (RDR_EXCEPTIONDESC_MAXLEN - 1))) {
            dfm_err("dfm unregister exception fail, index=%u, strlen(excep->module_name)=%lu, "
			    "strlen(excep->desc)=%lu.\n",
                i, strlen(excep->module_name), strlen(excep->desc));
            return DFM_ERROR;
        }
        ex_id = excep[i].excep_id;
        ret = bbox_unregister_exception(ex_id);
        if (ret != DFM_OK) {
            dfm_err("dfm unregister exception fail, index=%u, ret=%u.\n", i, ret);
            return ret;
        }
    }

    return DFM_OK;
}

/* exception code registered from dfm to rdr */
int dfm_register_exception(struct dfm_exception_info *excep, u32 num)
{
    u32 i;
    u32 ret;
    u32 core_id;
    bbox_exception_info_s exc = { 0 };

    for (i = 0; i < num; i++) {
        if ((strlen(excep[i].module_name) >= (RDR_MODULE_NAME_LEN - 1)) ||
            (strlen(excep[i].desc) >= (RDR_EXCEPTIONDESC_MAXLEN - 1))) {
            dfm_err(
                "dfm register exception fail, index=%u, strlen(excep[i].module_name)=%lu, strlen(excep[i].desc)=%lu.\n",
                i, strlen(excep[i].module_name), strlen(excep[i].desc));
            return DFM_ERROR;
        }

        core_id = excep[i].module_id;
        exc.e_excepid = excep[i].excep_id;
        exc.e_excepid_end = excep[i].excep_id;
        exc.e_process_priority = (u8)excep[i].process_priority;
        exc.e_reboot_priority = (u8)excep[i].reboot_priority;
        exc.e_notify_core_mask = RDR_COREID_MASK(core_id);
        exc.e_reset_core_mask = 0;
        exc.e_from_core = (u8)core_id;
        if ((strcmp(excep[i].module_name, "dvpp") == 0) || strcmp(excep[i].module_name, "storage") == 0) {
            exc.e_reentrant = RDR_REENTRANT_ALLOW;
        } else {
            exc.e_reentrant = RDR_REENTRANT_DISALLOW;
        }

        exc.e_exce_type = excep[i].exce_type;

        ret = strncpy_s((char *)exc.e_from_module, RDR_MODULE_NAME_LEN, excep[i].module_name,
                        strlen(excep[i].module_name) + 1);
        if (ret) {
            dfm_err("strncpy_s failed, ret=%u.\n", ret);
            return ret;
        }
        ret = strncpy_s((char *)exc.e_desc, RDR_EXCEPTIONDESC_MAXLEN, excep[i].desc, strlen(excep[i].desc) + 1);
        if (ret) {
            dfm_err("strncpy_s failed, ret=%u.\n", ret);
            return ret;
        }

        ret = bbox_register_exception(&exc);  // return 0 means failed
        if (ret == 0) {
            dfm_err("dfm register exception fail, index=%u, ret=%u.\n", i, ret);
            return DFM_ERROR;
        }
    }

    return DFM_OK;
}

/* register callback to dfm */
int dfm_register_module(struct dfm_module_register *dfm_module_info)
{
    u32 id;

    if (dfm_module_info == NULL) {
        dfm_err("dfm_module_info is NULL.\n");
        return DFM_ERROR;
    }

    if (dfm_module_info->module_id == DFM_MODULE_ID_DVPP) {
        g_dfm_info.ops_dvpp_dump = dfm_module_info->ops_dump;
    } else if (dfm_module_info->module_id == DFM_MODULE_ID_DRIVER) {
        if ((dfm_module_info->sub_module_id >= DFM_SUBMODULE_ID_END) ||
            ((dfm_module_info->ops_dump == NULL) && (dfm_module_info->ops_dump_ex == NULL))) {
            dfm_err("dfm register module fail, sub_module_id=%u, (ops_dump == NULL)=%d.\n",
                    dfm_module_info->sub_module_id, (dfm_module_info->ops_dump == NULL));
            return DFM_ERROR;
        }
        id = dfm_module_info->sub_module_id;
        g_dfm_info.ops_drv_dump[id] = dfm_module_info->ops_dump;
        g_dfm_info.ops_drv_dump_ex[id] = dfm_module_info->ops_dump_ex;
    } else {
        dfm_err("dfm register with illegal, module_id: %u\n", dfm_module_info->module_id);
        return DFM_ERROR;
    }

    return DFM_OK;
}

/* unregister from driver and dvpp */
int dfm_unregister_module(u32 module_id, u32 sub_module_id)
{
    if (module_id == DFM_MODULE_ID_DVPP) {
        g_dfm_info.ops_dvpp_dump = NULL;
    } else if (module_id == DFM_MODULE_ID_DRIVER) {
        if (sub_module_id >= DFM_SUBMODULE_ID_END) {
            dfm_err("dfm unregister module fail, "
                    "sub_module_id=%u\n",
                    sub_module_id);
            return DFM_ERROR;
        }
        g_dfm_info.ops_drv_dump[sub_module_id] = NULL;
        g_dfm_info.ops_drv_dump_ex[sub_module_id] = NULL;
    } else {
        dfm_err("dfm unregister module fail, module_id=%u\n", module_id);
        return DFM_ERROR;
    }

    return DFM_OK;
}

/* Write Info to the RDR bb */
int dfm_write_black_box(u32 dev_id, u32 module_id, u32 sub_module_id, const char *black_box_info)
{
    char timestamp[DFM_TIME_STAMP_LENGTH]; /* 2018-06-01 16:47:12 123 */
    char *log_buf = NULL;

    u32 buf_len;
    unsigned long flags;
    struct timespec sys_time;
    struct rtc_time tm = { 0 };
    int tlen;

    if (dev_id >= g_dev_num) {
        dfm_err("device id invalid, dev_id = %u.\n", dev_id);
        return DFM_ERROR;
    }

    if (black_box_info == NULL) {
        dfm_err("dfm write bbox fail\n");
        return DFM_ERROR;
    }
    if ((module_id != DFM_MODULE_ID_DRIVER) && (module_id != DFM_MODULE_ID_DVPP)) {
        dfm_err("dfm write bbox fail, module_id=%u\n", module_id);
        return DFM_ERROR;
    }

    sys_time = current_kernel_time();
    /* time not sync, use relative time */
    if (sys_time.tv_sec < DFM_TIME_SYNC_MAX_DELAY) {
        tlen = snprintf_s(timestamp, sizeof(timestamp), sizeof(timestamp) - 1, "system startup time:%ld(s)",
            (long)sys_time.tv_sec);
    } else {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
        struct timespec64 ts64 = {0};
        ts64.tv_sec = sys_time.tv_sec;
        tm = rtc_ktime_to_tm(timespec64_to_ktime(ts64));
#else
        rtc_time_to_tm(sys_time.tv_sec, &tm);
#endif
        tlen = snprintf_s(timestamp, sizeof(timestamp), sizeof(timestamp) - 1, "%4d-%02d-%02d %02d:%02d:%02d %03d",
                          tm.tm_year + DFM_TIME_START_YEAR, tm.tm_mon + DFM_MONTH_COMPENSATION, tm.tm_mday, tm.tm_hour,
                          tm.tm_min, tm.tm_sec, (int)(sys_time.tv_nsec / DFM_NUM_OF_MS_PER_SEC));
    }

    if (tlen < 0) {
        dfm_err("dev_id(%u): sdfm_write_black_box:%4d-%02d-%02d %02d:%02d:%02d %03d snprintf_s fail.\n", dev_id,
                tm.tm_year + DFM_TIME_START_YEAR, tm.tm_mon + DFM_MONTH_COMPENSATION, tm.tm_mday, tm.tm_hour, tm.tm_min,
                tm.tm_sec, (int)(sys_time.tv_nsec / DFM_NUM_OF_MS_PER_SEC));
        return DFM_ERROR;
    }
    if (module_id == DFM_MODULE_ID_DVPP) {
        buf_len = g_dfm_info.dvpp_log_len;
        log_buf = (char *)(uintptr_t)((uintptr_t)g_dfm_info.dvpp_log_vaddr[dev_id]);
    } else {
        buf_len = g_dfm_info.driver_log_len;
        log_buf = (char *)(uintptr_t)((uintptr_t)g_dfm_info.driver_log_vaddr[dev_id]);
    }
    if ((log_buf == NULL) || (buf_len == 0) ||
        (buf_len < strlen(timestamp) + strlen(black_box_info) + DFM_LOG_BUF_HEAD_LENGTH)) {
        return DFM_ERROR;
    }

    spin_lock_irqsave(&g_dfm_info.spinlock, flags);
    tlen = snprintf_s(log_buf, buf_len, strlen(timestamp) + strlen(black_box_info) + DFM_LOG_BUF_HEAD_LENGTH,
                      "devid: %u, module: %u %s %s", dev_id, module_id, timestamp, black_box_info);
    if (tlen < 0) {
        spin_unlock_irqrestore(&g_dfm_info.spinlock, flags);
        dfm_err("sdfm_write_black_box:devid: %u, module: %u %s %s snprintf_s fail.\n", dev_id, module_id, timestamp,
                black_box_info);
        return DFM_ERROR;
    }
    spin_unlock_irqrestore(&g_dfm_info.spinlock, flags);

    dfm_debug("dev_id(%u), black_box_msg:%s\n", dev_id, log_buf);

    return DFM_OK;
}

/* system error report */
void dfm_system_error_report(u32 dev_id, u32 excep_id, u32 arg)
{
    struct timespec sys_time;
    excep_time timestamp;

    if (dev_id >= g_dev_num) {
        dfm_err("device id invalid, dev_id = %u.\n", dev_id);
        return;
    }

    dfm_err("dev_id(%u): enter dfm_system_error_report\n", dev_id);

    sys_time = current_kernel_time();

    timestamp.tv_sec = sys_time.tv_sec;
    timestamp.tv_usec = sys_time.tv_nsec / DFM_NUM_OF_MS_PER_SEC;

    bbox_system_error(dev_id, excep_id, &timestamp, arg);
}

void dfm_system_error_report_ex(unsigned int dev_id, unsigned int excep_id, unsigned int arg)
{
    dfm_system_error_report(dev_id, excep_id, arg);
}
EXPORT_SYMBOL(dfm_system_error_report_ex);

/* DFM module init */
int __init dfm_init(void)
{
    int ret;
    unsigned int i;
    u32 num;
    struct bbox_module_ops ops;
    struct bbox_register_module_result retinfo;
    unsigned int node;

    for_each_node_with_cpus(node)
    {
        g_dev_num++;
    }

    dfm_inf("from os we get devnum = %u.\n", g_dev_num);

    if (g_dev_num > DFM_MAX_DAVINCI_NUM) {
        dfm_err("dfm get dev num error, dev_num = %u.\n", g_dev_num);
        return DFM_ERROR;
    }

    /* register driver to bbox */
    ops.ops_dump = dfm_driver_dump_match_bbox;
    ops.ops_reset = dfm_reset_bbox;
    ret = memset_s(&retinfo, sizeof(struct bbox_register_module_result), 0,
                   sizeof(struct bbox_register_module_result));
    if (ret) {
        dfm_err("memset_s failed. ret=%d\n", ret);
        return DFM_ERROR;
    }

    // initial to zero
    ret = bbox_register_module_ops(DFM_MODULE_ID_DRIVER, &ops, &retinfo);
    if (ret < 0) {
        dfm_err("bbox register driver fail, ret=%d\n", ret);
    }
    g_dfm_info.driver_log_len = retinfo.log_len;
    g_dfm_info.driver_log_paddr = retinfo.log_addr;

    for (i = 0; i < g_dev_num; i++) {
        g_dfm_info.driver_log_vaddr[i] =
            (unsigned long long)(uintptr_t)ioremap_wc((phys_addr_t)(g_dfm_info.driver_log_paddr +
                ASCEND_CHIP_ADDR_OFFSET * i), g_dfm_info.driver_log_len);

        if (g_dfm_info.driver_log_vaddr[i] == 0) {
            dfm_err("driver ioremap error, vaddr[%u] = %pK\n", i, (void *)(uintptr_t)g_dfm_info.driver_log_vaddr[i]);
        }
    }

#ifdef CFG_FEATURE_DFM_WITH_DVPP
    /* register DVPP to bbox */
    ops.ops_dump = dfm_dvpp_dump_match_bbox;
    ops.ops_reset = dfm_reset_bbox;

    ret = memset_s(&retinfo, sizeof(struct bbox_register_module_result), 0,
                   sizeof(struct bbox_register_module_result));
    if (ret) {
        dfm_err("memset_s failed. ret=%d\n", ret);
        ret = bbox_unregister_module_ops(DFM_MODULE_ID_DRIVER);
        if (ret != DFM_OK) {
            dfm_err("bbox driver unregister module fail\n");
        }
        return DFM_ERROR;
    }

    ret = bbox_register_module_ops(DFM_MODULE_ID_DVPP, &ops, &retinfo);
    if (ret < 0) {
        dfm_err("bbox register dvpp fail, ret=%d\n", ret);
    }
    // Attention!!  it turn to physical address in cloud
    g_dfm_info.dvpp_log_len = retinfo.log_len;
    g_dfm_info.dvpp_log_paddr = retinfo.log_addr;

    for (i = 0; i < g_dev_num; i++) {
        g_dfm_info.dvpp_log_vaddr[i] =
            (unsigned long long)(uintptr_t)ioremap_wc((phys_addr_t)(g_dfm_info.dvpp_log_paddr +
                ASCEND_CHIP_ADDR_OFFSET * i), g_dfm_info.dvpp_log_len);

        if (g_dfm_info.dvpp_log_vaddr[i] == 0) {
            dfm_err("dvpp ioremap error, vaddr[%u] = %pK\n", i, (void *)(uintptr_t)g_dfm_info.dvpp_log_vaddr[i]);
        }
    }
#endif

    /* register exception */
    num = sizeof(g_drv_excep_list) / sizeof(g_drv_excep_list[0]);
    (void)dfm_register_exception(g_drv_excep_list, num);

    spin_lock_init(&g_dfm_info.spinlock);

    dfm_inf("dfm init finish\n");
    dfm_inf("dfm driver bbox info(0x%pK %u)\n", (void *)(uintptr_t)g_dfm_info.driver_log_paddr,
            g_dfm_info.driver_log_len);
#ifdef CFG_FEATURE_DFM_WITH_DVPP
    dfm_inf("dfm dvpp bbox info(0x%pK %u)\n", (void *)(uintptr_t)g_dfm_info.dvpp_log_paddr, g_dfm_info.dvpp_log_len);
#endif

    return DFM_OK;
}

/* DFM exit */
void __exit dfm_exit(void)
{
    unsigned int i;
    int ret;
    int num;

    /* we just unregister dfm --> rdr resources ,other resources who use who unregister */
    num = sizeof(g_drv_excep_list) / sizeof(g_drv_excep_list[0]);

    ret = dfm_unregister_exception(g_drv_excep_list, num);
    if (ret != DFM_OK) {
        dfm_err("bbox exception unregister module fail, ret=%d.\n", ret);
    }

    ret = bbox_unregister_module_ops(DFM_MODULE_ID_DRIVER);
    if (ret != DFM_OK) {
        dfm_err("bbox driver unregister module fail, ret=%d.\n", ret);
    }

#ifdef CFG_FEATURE_DFM_WITH_DVPP
    ret = bbox_unregister_module_ops(DFM_MODULE_ID_DVPP);
    if (ret != DFM_OK) {
        dfm_err("bbox dvpp unregister module fail, ret=%d.\n", ret);
    }
#endif

    for (i = 0; i < g_dev_num; i++) {
        iounmap((void *)(uintptr_t)g_dfm_info.driver_log_vaddr[i]);
        g_dfm_info.driver_log_vaddr[i] = 0;
#ifdef CFG_FEATURE_DFM_WITH_DVPP
        iounmap((void *)(uintptr_t)g_dfm_info.dvpp_log_vaddr[i]);
        g_dfm_info.dvpp_log_vaddr[i] = 0;
#endif
    }

    ret = memset_s(&g_dfm_info, sizeof(g_dfm_info), 0, sizeof(g_dfm_info));
    if (ret) {
        dfm_err("g_dfm_info set to zero failed, ret=%d.\n", ret);
    }

    dfm_inf("dfm unregister module ok\n");
}

/* In init and uninit, you can use this function to stub */
int dfm_stub_print(u32 dev_id, u32 module_id, u32 type)
{
    char timestamp[DFM_TIME_STAMP_LENGTH]; /* 2018-06-01 16:47:12 123 */
    char *log_buf = NULL;

    u32 buf_len;
    unsigned long flags;
    int tlen;
    struct timespec64 curr_time = {0};

    if (dev_id >= g_dev_num) {
        dfm_err("device id invalid, dev_id = %u.\n", dev_id);
        return DFM_ERROR;
    }

    if ((module_id != DFM_MODULE_ID_DRIVER) && (module_id != DFM_MODULE_ID_DVPP)) {
        dfm_err("dev_id(%u): dfm write bbox fail, module_id=%u\n", dev_id, module_id);
        return DFM_ERROR;
    }

    ktime_get_raw_ts64(&curr_time);
    tlen = snprintf_s(timestamp, sizeof(timestamp), sizeof(timestamp) - 1, "%ld.%06d",
                      (unsigned long)curr_time.tv_sec, (curr_time.tv_nsec / 1000));
    if (tlen < 0) {
        dfm_err("dev_id(%u): current time %lu.%06ld\n", dev_id, (unsigned long)curr_time.tv_sec,
                (curr_time.tv_nsec / 1000));
        return DFM_ERROR;
    }
    if (module_id == DFM_MODULE_ID_DVPP) {
        buf_len = g_dfm_info.dvpp_log_len;
        log_buf = (char *)(uintptr_t)g_dfm_info.dvpp_log_vaddr[dev_id];
    } else {
        buf_len = g_dfm_info.driver_log_len;
        log_buf = (char *)(uintptr_t)g_dfm_info.driver_log_vaddr[dev_id];
    }
    if ((log_buf == NULL) || (buf_len == 0) || (buf_len < strlen(timestamp) + STUB_SIZE)) {
        return DFM_ERROR;
    }

    spin_lock_irqsave(&g_dfm_info.spinlock, flags);
    if (type == 0) {
        tlen = snprintf_s(log_buf, buf_len, strlen(timestamp) + STUB_SIZE,
                          "Init devid[%u], module: %u %s", dev_id, module_id, timestamp);
        if (tlen < 0) {
            spin_unlock_irqrestore(&g_dfm_info.spinlock, flags);
            dfm_err("dfm_stub_print:Init devid[%u], module: %u %s snprintf_s fail.\n", dev_id, module_id, timestamp);
            return DFM_ERROR;
        }
    } else if (type == 1) {
        tlen = snprintf_s(log_buf, buf_len, strlen(timestamp) + STUB_SIZE,
                          "Uninit devid[%u], module: %u %s", dev_id, module_id, timestamp);
        if (tlen < 0) {
            spin_unlock_irqrestore(&g_dfm_info.spinlock, flags);
            dfm_err("dfm_stub_print:Uninit devid[%u], module: %u %s snprintf_s fail.\n", dev_id, module_id, timestamp);
            return DFM_ERROR;
        }
    } else {
        spin_unlock_irqrestore(&g_dfm_info.spinlock, flags);
        dfm_err("dev_id(%u): type invalid, type = %u\n", dev_id, type);
        return DFM_ERROR;
    }
    spin_unlock_irqrestore(&g_dfm_info.spinlock, flags);

    return DFM_OK;
}

EXPORT_SYMBOL(dfm_stub_print); //lint !e508 !e532
EXPORT_SYMBOL(dfm_register_module);
EXPORT_SYMBOL(dfm_unregister_module);
EXPORT_SYMBOL(dfm_write_black_box);
EXPORT_SYMBOL(dfm_system_error_report);

module_init(dfm_init);
module_exit(dfm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("DAVINCI dfm driver");
