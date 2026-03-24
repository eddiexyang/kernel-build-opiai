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
#include <linux/securectype.h>
#include <linux/version.h>
#ifndef UT_TEST
#include "kernel_version_adapt.h"
#endif
#include "devdrv_dfm.h"
#include "devdrv_dfm_mini.h"

#ifndef STATIC
#ifdef UT_TEST
#define STATIC
#else
#define STATIC static
#endif
#endif


STATIC void dfm_driver_dump(unsigned int excep_id, unsigned int etype, unsigned int module_id, char *logpath,
    pfn_cb_dump_done fndone);
STATIC void dfm_dvpp_dump(unsigned int excep_id, unsigned int etype, unsigned int module_id, char *logpath,
    pfn_cb_dump_done fndone);

struct dfm_mng_info {
    unsigned int init_success_flag;

    unsigned int driver_log_len;         /* driver模块黑匣子静态内存物理长度 */
    unsigned long long driver_log_paddr; /* driver模块黑匣子静态内存物理地址 */
    unsigned long long driver_log_vaddr; /* driver模块黑匣子静态内存虚拟地址 */

    unsigned int dvpp_log_len;         /* driver模块黑匣子静态内存物理长度 */
    unsigned long long dvpp_log_paddr; /* driver模块黑匣子静态内存物理地址 */
    unsigned long long dvpp_log_vaddr; /* driver模块黑匣子静态内存虚拟地址 */

    spinlock_t spinlock;
    dfm_dump ops_dvpp_dump;                      /* DVPP模块dump接口 */
    dfm_dump ops_drv_dump[DFM_SUBMODULE_ID_END]; /* driver各子模块dump接口 */
    dfm_dump_ex ops_drv_dump_ex[DFM_SUBMODULE_ID_END]; /* driver各子模块dump接口 */
};

STATIC struct dfm_mng_info dfm_info = { 0 };
STATIC struct dfm_exception_info drv_excep_list[] = {
    /* DVPP */
    {DVPP_JPEGD_IRQ_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "dvpp", "JPEGD irq failed"},
    {DVPP_JPEGE_IRQ_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "dvpp", "JPEGE irq failed"},
    {DVPP_VPC_IRQ_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "dvpp", "VPC irq failed"},
    {DVPP_PNGD_IRQ_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "dvpp", "PNGD irq failed"},
    {DVPP_VDEC_IRQ_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "dvpp", "VDEC irq failed"},
    {DVPP_VENC_IRQ_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "dvpp", "VENC irq failed"},

    {DVPP_JPEGD_PROC_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_WARN, RDR_REBOOT_NO, "dvpp", "JPEGD PROC failed"},
    {DVPP_JPEGE_PROC_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_WARN, RDR_REBOOT_NO, "dvpp", "JPEGE PROC failed"},
    {DVPP_VPC_PROC_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_WARN, RDR_REBOOT_NO, "dvpp", "VPC PROC failed"},
    {DVPP_PNGD_PROC_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_WARN, RDR_REBOOT_NO, "dvpp", "PNGD PROC failed"},
    {DVPP_VDEC_PROC_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_WARN, RDR_REBOOT_NO, "dvpp", "VDEC PROC failed"},
    {DVPP_VENC_PROC_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_WARN, RDR_REBOOT_NO, "dvpp", "VENC PROC failed"},
    {DVPP_CMD_LIST_PROC_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_WARN, RDR_REBOOT_NO,
     "dvpp", "CMD_LIST PROC failed"},

    {DVPP_BUS_ERROR, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "dvpp", "DVPP BUS error"},
    /* AGENTDRV */
    {DFM_AGENTDRV_DMA_PROBE_FAIL, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT,
     "driver", "dma probe fail"},
    {DFM_AGENTDRV_SDIO_PROBE_FAIL, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT,
     "driver", "sdio probe fail"},
    {DFM_AGENTDRV_PM_PROBE_FAIL, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT,
     "driver", "pm probe fail"},
    {DFM_AGENTDRV_LINKDOWN_FAIL, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT,
     "driver", "linkdown fail"},

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
    /* HIGMAC */
    {DFM_HIGMAC_E_TX_TIMEOUT, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT,
     "driver", "Higmac transmit time out"},
    {DFM_HIGMAC_E_RX_INCONST, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT,
     "driver", "Higmac receive skb inconsistent"},
    {DFM_HIGMAC_E_TX_INCONST, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT,
     "driver", "Higmac transmit skb inconsistent"},
    {DFM_HIGMAC_E_TX_BD_ERROR, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT,
     "driver", "Higmac revlaim bd has error"},
    /* USB */
    {DFM_AGENTDRV_DMA_FAIL, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT,
     "driver", "dma fail"},

};

void dfm_reset(u32 excep_id, u8 etype, u8 core_id)
{
    return;
}

/* excep_id actual not used, we default zero for it */
void dfm_driver_dump_match_rdr(u32 excep_id, u8 etype, u8 coreid, excep_time time, char *logpath,
    pfn_cb_dump_done fndone)
{
    dfm_driver_dump(excep_id, etype, coreid, logpath, fndone);
    if (fndone != NULL) {
        fndone(excep_id, coreid, etype, time);
    }
    return;
}

/* DRV模块dump回调接口 */
STATIC void dfm_driver_dump(u32 excep_id, u32 etype, u32 module_id, char *logpath, pfn_cb_dump_done fndone)
{
    u32 i;
    char *buf = NULL;
    u32 buflen = dfm_info.driver_log_len;
    u32 loglen;
    u32 first_flag = 0;
    unsigned long flags;
    int ret;

    for (i = 0; i < DFM_SUBMODULE_ID_END; i++) {
        if (((dfm_info.ops_drv_dump[i] == NULL) && (dfm_info.ops_drv_dump_ex[i] == NULL))
            || dfm_info.driver_log_vaddr == 0 || dfm_info.driver_log_len == 0) {
            continue;
        }

        /* 调用各模块dump接口 */
        // 在循环里面的用到的公用变量，先初始化为0
        buf = NULL;

        dfm_info.ops_drv_dump[i](excep_id, etype, module_id, &buf);

        if (buf == NULL) {
            continue;
        }

        loglen = strlen(buf);
        if ((loglen >= (buflen - 1)) || (loglen == 0)) {
            continue;
        }

        /* 把buf写入黑匣子静态内存 */
        if (first_flag == 0) {
            /* 第一个打印需要时间戳，调用现有的接口 */
            (void)dfm_write_black_box(DFM_MODULE_ID_DRIVER, i, buf);
            first_flag = 1;
        } else {
            spin_lock_irqsave(&dfm_info.spinlock, flags);
            ret = strncat_s((void *)(uintptr_t)dfm_info.driver_log_vaddr, buflen, buf, loglen);
            if (ret) {
                spin_unlock_irqrestore(&dfm_info.spinlock, flags);
                dfm_err("strncat_s failed , ret=%d.\n", ret);
                return;
            }
            spin_unlock_irqrestore(&dfm_info.spinlock, flags);
        }
        buflen -= loglen;
    }
}
/* excep_id actual not used, we default zero for it */
void dfm_dvpp_dump_match_rdr(u32 module_id, u8 etype, u8 coreid, excep_time time, char *logpath,
    pfn_cb_dump_done fndone)
{
    dfm_dvpp_dump(module_id, etype, coreid, logpath, fndone);
    if (fndone != NULL) {
        fndone(module_id, coreid, etype, time);
    }
    return;
}


/* DVPP模块dump回调接口 */
STATIC void dfm_dvpp_dump(u32 excep_id, u32 etype, u32 module_id, char *logpath, pfn_cb_dump_done fndone)
{
    char *buf = NULL;

    if (dfm_info.ops_dvpp_dump == NULL || dfm_info.dvpp_log_vaddr == 0 || dfm_info.dvpp_log_len == 0) {
        return;
    }

    dfm_info.ops_dvpp_dump(excep_id, etype, module_id, &buf);
    /* 把buf写入黑匣子静态内存 */
    (void)dfm_write_black_box(DFM_MODULE_ID_DVPP, 0, buf);
    return;
}

/* unregister exception to RDR */
int dfm_unregister_exception(struct dfm_exception_info *excep, unsigned int num)
{
    u32 ex_id = 0;
    u32 i;
    u32 ret;

    if (excep == NULL) {
        dfm_err("dfm unregister exception fail, excep = NULL\n");
        return DFM_ERROR;
    }
    for (i = 0; i < num; i++) {
        if ((strlen(excep->module_name) >= RDR_MODULE_NAME_LEN) || (strlen(excep->desc) >= RDR_EXCEPTIONDESC_MAXLEN)) {
            dfm_err("dfm unregister exception fail, index=%d\n", i);
            return DFM_ERROR;
        }
        ex_id = excep[i].excep_id;
        ret = rdr_unregister_exception(ex_id);
        if (ret != DFM_OK) {
            dfm_err("dfm unregister exception fail, index=%d\n", i);
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
    struct rdr_exception_info_s exc = { 0 };

    if (excep == NULL) {
        dfm_err("dfm register exception fail, excep = NULL\n");
        return DFM_ERROR;
    }
    for (i = 0; i < num; i++) {
        if ((strlen(excep[i].module_name) >= RDR_MODULE_NAME_LEN) ||
            (strlen(excep[i].desc) >= RDR_EXCEPTIONDESC_MAXLEN)) {
            dfm_err("dfm register exception fail, index=%d\n", i);
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
        exc.e_reentrant = RDR_REENTRANT_DISALLOW;
        exc.e_exce_type = excep[i].exce_type;

        ret = strncpy_s((char *)exc.e_from_module, RDR_MODULE_NAME_LEN, excep[i].module_name,
            strlen(excep[i].module_name) + 1);
        if (ret) {
            dfm_err("strncpy_s failed,ret=%d.\n", ret);
            return ret;
        }
        ret = strncpy_s((char *)exc.e_desc, RDR_EXCEPTIONDESC_MAXLEN, excep[i].desc, strlen(excep[i].desc) + 1);
        if (ret) {
            dfm_err("strncpy_s failed,ret=%d.\n", ret);
            return ret;
        }

        ret = rdr_register_exception(&exc); /* return 0 means failed */
        if (ret == 0) {
            dfm_err("dfm register exception fail, index=%d\n", i);
            return ret;
        }
    }

    return DFM_OK;
}

/* register callback to dfm */
int dfm_register_module(struct dfm_module_register *dfm_module_info)
{
    u32 id;

    if (dfm_module_info == NULL) {
        dfm_err("dfm register module fail, dfm_module_info = NULL\n");
        return DFM_ERROR;
    }
    if (dfm_module_info->module_id == DFM_MODULE_ID_DVPP) {
        dfm_info.ops_dvpp_dump = dfm_module_info->ops_dump;
    } else if (dfm_module_info->module_id == DFM_MODULE_ID_DRIVER) {
        if ((dfm_module_info->sub_module_id >= DFM_SUBMODULE_ID_END) ||
            ((dfm_module_info->ops_dump == NULL) && (dfm_module_info->ops_dump_ex == NULL))) {
            dfm_err("dfm register module fail\n");
            return DFM_ERROR;
        }
        id = dfm_module_info->sub_module_id;
        dfm_info.ops_drv_dump[id] = dfm_module_info->ops_dump;
        dfm_info.ops_drv_dump_ex[id] = dfm_module_info->ops_dump_ex;
    } else {
        dfm_err("dfm register with illegal, module_id: %d\n", dfm_module_info->module_id);
        return DFM_ERROR;
    }

    return DFM_OK;
}


/* unregister from driver and dvpp */
int dfm_unregister_module(u32 module_id, u32 sub_module_id)
{
    if (module_id == DFM_MODULE_ID_DVPP) {
        dfm_info.ops_dvpp_dump = NULL;
    } else if (module_id == DFM_MODULE_ID_DRIVER) {
        if (sub_module_id >= DFM_SUBMODULE_ID_END) {
            dfm_err("dfm unregister module fail, "
                "sub_module_id=%d\n",
                sub_module_id);
            return DFM_ERROR;
        }
        dfm_info.ops_drv_dump[sub_module_id] = NULL;
        dfm_info.ops_drv_dump_ex[sub_module_id] = NULL;
    } else {
        dfm_err("dfm unregister module fail, module_id=%d\n", module_id);
        return DFM_ERROR;
    }

    return DFM_OK;
}

/* Write Info to the RDR bb */
int dfm_write_black_box(u32 module_id, u32 sub_module_id, const char *black_box_info)
{
    char timestamp[30]; /* 2018-06-01 16:47:12 123 */
    char *log_buf = NULL;
    u32 buf_len;
    unsigned long flags;
    struct timespec sys_time;
    struct rtc_time tm = { 0 };
    int tlen;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
    struct timespec64 ts64 = {0};
#endif

    if (black_box_info == NULL) {
        dfm_err("dfm write bbox fail\n");
        return DFM_ERROR;
    }
    if ((module_id != DFM_MODULE_ID_DRIVER) && (module_id != DFM_MODULE_ID_DVPP)) {
        dfm_err("dfm write bbox fail, module_id=%d\n", module_id);
        return DFM_ERROR;
    }

    /* time tag */
    sys_time = current_kernel_time();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
    ts64.tv_sec = sys_time.tv_sec;
    tm = rtc_ktime_to_tm(timespec64_to_ktime(ts64));
#else
    rtc_time_to_tm(sys_time.tv_sec, &tm);
#endif
    tlen = snprintf_s(timestamp, sizeof(timestamp), sizeof(timestamp) - 1, "%4d-%02d-%02d %02d:%02d:%02d %03d",
        tm.tm_year + 1900, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, (int)(sys_time.tv_nsec / 1000));
    if (tlen < 0) {
        dfm_err("dfm_write_black_box:%4d-%02d-%02d %02d:%02d:%02d %03d snprintf_s fail.\n", tm.tm_year + 1900,
            tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, (int)(sys_time.tv_nsec / 1000));
        return DFM_ERROR;
    }
    /* 把black_box_info写到静态内存 */
    if (module_id == DFM_MODULE_ID_DVPP) {
        buf_len = dfm_info.dvpp_log_len;
        log_buf = (char *)(uintptr_t)dfm_info.dvpp_log_vaddr;
    } else {
        buf_len = dfm_info.driver_log_len;
        log_buf = (char *)(uintptr_t)dfm_info.driver_log_vaddr;
    }
    if (log_buf == NULL || buf_len == 0 || (buf_len < strlen(timestamp) + strlen(black_box_info) + 2)) {
        return DFM_ERROR;
    }
    spin_lock_irqsave(&dfm_info.spinlock, flags);
    tlen = snprintf_s(log_buf, buf_len, buf_len - 1, "%s %s", timestamp, black_box_info);
    if (tlen < 0) {
        spin_unlock_irqrestore(&dfm_info.spinlock, flags);
        dfm_err("dfm_write_black_box:%s %s snprintf_s fail.\n", timestamp, black_box_info);
        return DFM_ERROR;
    }

    spin_unlock_irqrestore(&dfm_info.spinlock, flags);

    return DFM_OK;
}


/* system error report */
void dfm_system_error_report(u32 excep_id, u32 arg)
{
    struct timespec sys_time;
    excep_time timestamp;

    sys_time = current_kernel_time();

    timestamp.tv_sec = sys_time.tv_sec;
    timestamp.tv_usec = sys_time.tv_nsec / 1000;

    mntn_system_error(excep_id, timestamp, arg);
}

void dfm_system_error_report_ex(unsigned int dev_id, unsigned int excep_id, unsigned int arg)
{
    dfm_system_error_report(excep_id, arg);
}
EXPORT_SYMBOL(dfm_system_error_report_ex);

/* DFM module init */
int __init dfm_init(void)
{
    int ret;
    u32 num;
    struct rdr_module_ops ops;
    struct rdr_register_module_result retinfo;

    /* register driver to RDR */
    ops.ops_dump = dfm_driver_dump_match_rdr;
    ops.ops_reset = dfm_reset;
    ret = memset_s(&retinfo, sizeof(struct rdr_register_module_result), 0, sizeof(struct rdr_register_module_result));
    if (ret) {
        dfm_err("memset_s failed. ret=%d\n", ret);
        return DFM_ERROR;
    }
    // initial to zero
    ret = rdr_register_module_ops(DFM_MODULE_ID_DRIVER, &ops, &retinfo);
    if (ret < 0) {
        dfm_err("rdr register driver fail, ret=%d\n", ret);
    }
    dfm_info.driver_log_len = retinfo.log_len;
    dfm_info.driver_log_vaddr = retinfo.log_vaddr;

    /* register DVPP to RDR */
    ops.ops_dump = dfm_dvpp_dump_match_rdr;
    ops.ops_reset = dfm_reset;

    ret = memset_s(&retinfo, sizeof(struct rdr_register_module_result), 0, sizeof(struct rdr_register_module_result));
    if (ret) {
        dfm_err("memset_s failed. ret=%d\n", ret);
        ret = rdr_unregister_module_ops(DFM_MODULE_ID_DRIVER);
        if (ret != DFM_OK) {
            dfm_err("rdr driver unregister module fail\n");
        }
        return DFM_ERROR;
    }

    ret = rdr_register_module_ops(DFM_MODULE_ID_DVPP, &ops, &retinfo);
    if (ret < 0) {
        dfm_err("rdr register dvpp fail, ret=%d\n", ret);
    }
    dfm_info.dvpp_log_len = retinfo.log_len;
    dfm_info.dvpp_log_vaddr = retinfo.log_vaddr;
    /* register exception */
    num = sizeof(drv_excep_list) / sizeof(drv_excep_list[0]);
    ret = dfm_register_exception(drv_excep_list, num);
    if (ret != DFM_OK) {
        dfm_err("dfm register exception fail, ret=%d\n", ret);
        return DFM_ERROR;
    }

    spin_lock_init(&dfm_info.spinlock);

    dfm_inf("dfm init finish\n");

    return DFM_OK;
}

/* DFM exit */
void __exit dfm_exit(void)
{
    int ret;
    int num;

    /* we just unregister dfm --> rdr resources ,other resources who use who unregister */
    num = sizeof(drv_excep_list) / sizeof(drv_excep_list[0]);
    ret = dfm_unregister_exception(drv_excep_list, num);
    if (ret != DFM_OK) {
        dfm_err("rdr exception unregister module fail\n");
    }

    ret = rdr_unregister_module_ops(DFM_MODULE_ID_DRIVER);
    if (ret != DFM_OK) {
        dfm_err("rdr driver unregister module fail\n");
    }

    ret = rdr_unregister_module_ops(DFM_MODULE_ID_DVPP);
    if (ret != DFM_OK) {
        dfm_err("rdr dvpp unregister module fail\n");
    }
    ret = memset_s(&dfm_info, sizeof(dfm_info), 0, sizeof(dfm_info));
    if (ret) {
        dfm_err("dfm_info set to zero failed,ret=%d.\n", ret);
    }

    dfm_inf("dfm unregister module ok\n");
}


EXPORT_SYMBOL(dfm_register_module); //lint !e508 !e532
EXPORT_SYMBOL(dfm_unregister_module);
EXPORT_SYMBOL(dfm_write_black_box);
EXPORT_SYMBOL(dfm_system_error_report);

module_init(dfm_init);
module_exit(dfm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("DAVINCI dfm driver");
