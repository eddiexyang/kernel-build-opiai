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

#include "devdrv_dfm.h"
#include "devdrv_dfm_miniv2.h"
#include "kernel_version_adapt.h"

#ifndef STATIC
#ifdef UT_TEST
#define STATIC
#else
#define STATIC static
#endif
#endif

#define DFM_CHECK_RET(expr, fmt, ...) do {  \
    if (expr) {                             \
        dfm_err(fmt, ##__VA_ARGS__);        \
    }                                       \
} while (0)

#define DFM_CHECK_EXP_ACT(expr, action, fmt, ...) do {  \
    if (expr) {                                         \
        action;                                         \
    }                                                   \
} while (0)

#define DFM_CHECK_PTR(p, action, fmt, ...) do { \
    if ((p) == NULL) {                          \
        action;                                 \
    }                                           \
} while (0)


#define DFM_PREFIX_INFO_LENGTH      100
#define DFM_TIME_STAMP_LENGTH       50
#define DFM_TIME_START_YEAR         1900
#define DFM_NUM_OF_MS_PER_SEC       1000
#define DFM_LOG_BUF_HEAD_LENGTH     28
#define DFM_MONTH_COMPENSATION      1
#define DFM_TIME_SYNC_MAX_DELAY     3600    /* time should be sync in 1 hour */

/* for miniv2, smp addr offset is 0x8000000000 */
#define DFM_SMP_ADDR_OFFSET     0x8000000000ULL

STATIC const int DFM_TRUE = 1;
STATIC const int DFM_FALSE = 0;

STATIC u32 g_dev_num = 0;

struct dfm_mng_info {
    u32 init_success_flag;

    u64 driver_log_vaddr[DFM_MAX_DAVINCI_NUM];
    u32 driver_log_offset[DFM_MAX_DAVINCI_NUM];
    u32 driver_log_len[DFM_MAX_DAVINCI_NUM];

    u64 dvpp_log_paddr;
    u64 dvpp_log_vaddr[DFM_MAX_DAVINCI_NUM];
    u32 dvpp_log_offset[DFM_MAX_DAVINCI_NUM];
    u32 dvpp_log_len[DFM_MAX_DAVINCI_NUM];

    spinlock_t spinlock;
    dfm_dump ops_dvpp_dump;                      /* DVPP模块dump接口 */
    dfm_dump ops_drv_dump[DFM_SUBMODULE_ID_END]; /* driver各子模块dump接口 */
    dfm_dump_ex ops_drv_dump_ex[DFM_SUBMODULE_ID_END]; /* driver各子模块dump接口 */
};

STATIC struct dfm_mng_info g_dfm_info = {0};
STATIC struct dfm_exception_info g_drv_excep_list[] = {
    /* DVPP */
    { DVPP_JPEGD_IRQ_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "dvpp", "JPEGD irq failed" },
    { DVPP_JPEGE_IRQ_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "dvpp", "JPEGE irq failed" },
    { DVPP_VPC_IRQ_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "dvpp", "VPC irq failed" },
    { DVPP_PNGD_IRQ_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "dvpp", "PNGD irq failed" },
    { DVPP_VDEC_IRQ_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "dvpp", "VDEC irq failed" },
    { DVPP_VENC_IRQ_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "dvpp", "VENC irq failed" },

    { DVPP_JPEGD_PROC_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_WARN, RDR_REBOOT_NO, "dvpp",
      "JPEGD PROC failed" },
    { DVPP_JPEGE_PROC_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_WARN, RDR_REBOOT_NO, "dvpp",
      "JPEGE PROC failed" },
    { DVPP_VPC_PROC_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_WARN, RDR_REBOOT_NO, "dvpp", "VPC PROC failed" },
    { DVPP_PNGD_PROC_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_WARN, RDR_REBOOT_NO, "dvpp", "PNGD PROC failed" },
    { DVPP_VDEC_PROC_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_WARN, RDR_REBOOT_NO, "dvpp", "VDEC PROC failed" },
    { DVPP_VENC_PROC_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_WARN, RDR_REBOOT_NO, "dvpp", "VENC PROC failed" },
    { DVPP_CMD_LIST_PROC_FAIL, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_WARN, RDR_REBOOT_NO, "dvpp",
      "CMD_LIST PROC failed" },

    { DVPP_BUS_ERROR, DFM_MODULE_ID_DVPP, DVPP_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "dvpp", "DVPP BUS error" },
    /* AGENTDRV */
    { DFM_AGENTDRV_DMA_PROBE_FAIL, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "driver",
      "dma probe fail" },
    { DFM_AGENTDRV_SDIO_PROBE_FAIL, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "driver",
      "sdio probe fail" },
    { DFM_AGENTDRV_PM_PROBE_FAIL, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "driver",
      "pm probe fail" },
    { DFM_AGENTDRV_LINKDOWN_FAIL, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "driver",
      "linkdown fail" },

    /* HIGMAC */
    { DFM_HIGMAC_E_TX_TIMEOUT, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "driver",
      "Higmac transmit time out" },
    { DFM_HIGMAC_E_RX_INCONST, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "driver",
      "Higmac receive skb inconsistent" },
    { DFM_HIGMAC_E_TX_INCONST, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "driver",
      "Higmac transmit skb inconsistent" },
    { DFM_HIGMAC_E_TX_BD_ERROR, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "driver",
      "Higmac revlaim bd has error" },

    { DFM_AGENTDRV_DMA_FAIL, DFM_MODULE_ID_DRIVER, DRIVER_S_EXCEPTION, RDR_ERR, RDR_REBOOT_WAIT, "driver", "dma fail" },

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
};

STATIC u32 dfm_get_devnum(void)
{
    u32 node;

    if (g_dev_num != 0) {
        /* get device nubmer once */
        return g_dev_num;
    }

    for_each_node_with_cpus(node) {
        g_dev_num++;
    }
    dfm_inf("get devnum for the first time, devnum(%u)\n", g_dev_num);

    return g_dev_num;
}

STATIC void dfm_clear_devnum(void)
{
    g_dev_num = 0;
}

void dfm_reset_bbox(u32 dev_id, u8 core_id, u32 excep_id, u8 etype)
{
    dfm_inf("bbox reset, devid(%u) coreid(%u) excep_id(%u) etype(%u)\n", dev_id, core_id, excep_id, etype);
    return;
}

STATIC void dfm_update_log_offset(u32 dev_id, u32 module_id, u32 buflen)
{
    switch (module_id) {
        case DFM_MODULE_ID_DVPP:
            g_dfm_info.dvpp_log_offset[dev_id] = buflen;
            break;
        case DFM_MODULE_ID_DRIVER:
            g_dfm_info.driver_log_offset[dev_id] = buflen;
            break;
        default:
            dfm_err("Unknown module, devid(%u) module_id(%u)\n", dev_id, module_id);
            break;
    }
}

STATIC inline int dfm_is_time_sync(struct timespec64 *time_sync)
{
    if (time_sync->tv_sec < DFM_TIME_SYNC_MAX_DELAY) {
        return DFM_FALSE;
    }
    return DFM_TRUE;
}

STATIC int dfm_get_timestamp(char *timestamp, u32 size)
{
    struct timespec64 sys_time = {0};
    struct tm tm = {0};
    int ret;

    ktime_get_coarse_real_ts64(&sys_time);
    if (dfm_is_time_sync(&sys_time) == DFM_FALSE) {
        ret = snprintf_s(timestamp, size, size - 1, "system startup time:%ld(s)", (long)sys_time.tv_sec);
    } else {
        time64_to_tm(sys_time.tv_sec, 0, &tm);
        ret = snprintf_s(timestamp, size, size - 1, "%4ld-%02d-%02d %02d:%02d:%02d %03d",
                         tm.tm_year + DFM_TIME_START_YEAR, tm.tm_mon + DFM_MONTH_COMPENSATION, tm.tm_mday, tm.tm_hour,
                         tm.tm_min, tm.tm_sec, (int)(sys_time.tv_nsec / DFM_NUM_OF_MS_PER_SEC));
    }

    if (ret < 0) {
        dfm_err("get timestamp fail, timestamp: (%4ld-%02d-%02d %02d:%02d:%02d %03d)\n",
                tm.tm_year + DFM_TIME_START_YEAR, tm.tm_mon + DFM_MONTH_COMPENSATION, tm.tm_mday, tm.tm_hour, tm.tm_min,
                tm.tm_sec, (int)(sys_time.tv_nsec / DFM_NUM_OF_MS_PER_SEC));
        return DFM_ERROR;
    }

    return DFM_OK;
}

STATIC int dfm_get_logbuf(u32 dev_id, u32 module_id, char **logbuf, u32 *buflen, u32 *buf_offset)
{
    switch (module_id) {
        case DFM_MODULE_ID_DVPP:
            *logbuf = (char *)(uintptr_t)g_dfm_info.dvpp_log_vaddr[dev_id];
            *buflen = g_dfm_info.dvpp_log_len[dev_id];
            *buf_offset = g_dfm_info.dvpp_log_offset[dev_id];
            break;
        case DFM_MODULE_ID_DRIVER:
            *logbuf = (char *)(uintptr_t)g_dfm_info.driver_log_vaddr[dev_id];
            *buflen = g_dfm_info.driver_log_len[dev_id];
            *buf_offset = g_dfm_info.driver_log_offset[dev_id];
            break;
        default:
            dfm_err("Unknown module, devid(%u) module_id(%u)\n", dev_id, module_id);
            return DFM_ERROR;
    }
    DFM_CHECK_PTR(*logbuf, return DFM_ERROR, "log_buf is NULL, devid(%u) module_id(%u)\n", dev_id, module_id);
    DFM_CHECK_EXP_ACT(*buflen == 0, return DFM_ERROR, "buf len is 0, devid(%u) module_id(%u)\n", dev_id, module_id);
    DFM_CHECK_EXP_ACT(*buf_offset == 0, return DFM_ERROR, "buf offset is 0, devid(%u) module_id(%u)\n", dev_id,
                      module_id);

    return DFM_OK;
}

STATIC int dfm_append_logbuf(u32 dev_id, u32 module_id, const char *src)
{
    char *log_buf = NULL;
    unsigned long flags;
    u32 buf_offset = 0;
    u32 src_len;
    u32 buf_len;
    int ret;

    spin_lock_irqsave(&g_dfm_info.spinlock, flags);
    ret = dfm_get_logbuf(dev_id, module_id, &log_buf, &buf_len, &buf_offset);
    if (ret != DFM_OK) {
        spin_unlock_irqrestore(&g_dfm_info.spinlock, flags);
        dfm_err("get dfm logbuf fail, devid(%u) module_id(%u) ret(%d)\n", dev_id, module_id, ret);
        return DFM_ERROR;
    }
    src_len = strnlen(src, buf_offset);
    if (buf_offset < src_len + 1) {
        spin_unlock_irqrestore(&g_dfm_info.spinlock, flags);
        dfm_err("too small buf, buf_len(%u) src_len(%u), buf_offset(%u)\n", buf_len, src_len, buf_offset);
        return DFM_ERROR;
    }

    ret = strncat_s(log_buf, buf_offset, src, src_len);
    if (ret != EOK) {
        spin_unlock_irqrestore(&g_dfm_info.spinlock, flags);
        dfm_err("sdfm_write_black_box:devid: %u, module: %u %s snprintf_s fail.\n", dev_id, module_id, src);
        return DFM_ERROR;
    }
    buf_offset -= src_len;
    dfm_update_log_offset(dev_id, module_id, buf_offset);
    spin_unlock_irqrestore(&g_dfm_info.spinlock, flags);

    return DFM_OK;
}

STATIC int dfm_write_logbuf(u32 dev_id, u32 module_id, const char *src)
{
    char *log_buf = NULL;
    unsigned long flags;
    u32 buf_offset;
    u32 src_len;
    u32 buf_len;
    int ret;

    spin_lock_irqsave(&g_dfm_info.spinlock, flags);
    ret = dfm_get_logbuf(dev_id, module_id, &log_buf, &buf_len, &buf_offset);
    if (ret != DFM_OK) {
        spin_unlock_irqrestore(&g_dfm_info.spinlock, flags);
        dfm_err("get dfm logbuf fail, devid(%u) module_id(%u) ret(%d)\n", dev_id, module_id, ret);
        return DFM_ERROR;
    }
    src_len = strnlen(src, buf_len);
    if (buf_len < src_len + DFM_LOG_BUF_HEAD_LENGTH + 1) {
        spin_unlock_irqrestore(&g_dfm_info.spinlock, flags);
        dfm_err("len check fail, devid(%u) module(%u) buf_len(%u) src_len(%u)\n", dev_id, module_id, buf_len, src_len);
        return DFM_ERROR;
    }
    ret = snprintf_s(log_buf, buf_len - 1, src_len, "%s", src);
    if (ret < 0) {
        spin_unlock_irqrestore(&g_dfm_info.spinlock, flags);
        dfm_err("write logbuf fail, devid(%u) module_id(%u) ret(%d)\n", dev_id, module_id, ret);
        return DFM_ERROR;
    }
    buf_len -= src_len;

    dfm_update_log_offset(dev_id, module_id, buf_len);
    spin_unlock_irqrestore(&g_dfm_info.spinlock, flags);

    return DFM_OK;
}

/* Write Info to the RDR bb */
int dfm_write_black_box(u32 dev_id, u32 module_id, u32 sub_module_id, const char *black_box_info)
{
    char prefix_info[DFM_PREFIX_INFO_LENGTH] = {0};
    char timestamp[DFM_TIME_STAMP_LENGTH] = {0};
    u32 devnum = dfm_get_devnum();
    int ret;

    DFM_CHECK_EXP_ACT(dev_id >= devnum, goto err, "invalid devid, devnum(%u) ", devnum);
    DFM_CHECK_PTR(black_box_info, goto err, "buf is NULL, ");

    ret = (module_id != DFM_MODULE_ID_DRIVER) && (module_id != DFM_MODULE_ID_DVPP);
    DFM_CHECK_EXP_ACT(ret != 0, goto err, "module id check fail, ");

    ret = dfm_get_timestamp(timestamp, DFM_TIME_STAMP_LENGTH);
    DFM_CHECK_EXP_ACT(ret != DFM_OK, goto err, "get timestamp fail, ");

    /* prepare devid, module and timestamp */
    ret = snprintf_s(prefix_info, DFM_PREFIX_INFO_LENGTH, DFM_PREFIX_INFO_LENGTH - 1, "devid:%u, module:%u %s ", dev_id,
                     module_id, timestamp);
    DFM_CHECK_EXP_ACT(ret < 0, goto err, "prefix info prepare fail ");

    /* clear bbox logbuf and write prefix contents to bbox logbuf */
    ret = dfm_write_logbuf(dev_id, module_id, prefix_info);
    DFM_CHECK_EXP_ACT(ret != DFM_OK, goto err, "prefix info write fail ");

    /* append income buf to bbox logbuf */
    ret = dfm_append_logbuf(dev_id, module_id, black_box_info);
    DFM_CHECK_EXP_ACT(ret != DFM_OK, goto err, "write logbuf fail ");

    return DFM_OK;
err:
    dfm_err("devid(%u) module_id(%u) sub_module_id(%u)\n", dev_id, module_id, sub_module_id);
    return DFM_ERROR;
}
EXPORT_SYMBOL(dfm_write_black_box); //lint !e508 !e532

STATIC void dfm_driver_dump(u32 dev_id, u8 coreid, u32 excep_id, u32 etype)
{
    u32 devnum = dfm_get_devnum();
    dfm_dump ops_drv_dump = NULL;
    char *buf = NULL;
    u32 sub_module;
    int ret;

    if (dev_id >= devnum) { //lint !e574
        dfm_err("Unknown devid, dev_id(%u), coreid(%u) devnum(%u).\n", dev_id, coreid, devnum);
        return;
    }

    /* clear bbox logbuf and write timestamp on bbox logbuf head  */
    ret = dfm_write_black_box(dev_id, DFM_MODULE_ID_DRIVER, 0, "");
    if (ret != DFM_OK) {
        return;
    }
    for (sub_module = 0; sub_module < DFM_SUBMODULE_ID_END; sub_module++) {
        ops_drv_dump = g_dfm_info.ops_drv_dump[sub_module];
        if (ops_drv_dump == NULL) {
            continue;
        }
        buf = NULL;
        ops_drv_dump(dev_id, excep_id, etype, coreid, &buf);
        DFM_CHECK_PTR(buf, continue, "buf is NULL, devid(%u) sub_module_id(%u)\n", dev_id, sub_module);

        ret = dfm_append_logbuf(dev_id, DFM_MODULE_ID_DRIVER, buf);
        DFM_CHECK_EXP_ACT(ret != DFM_OK, continue, "append logbuf fail, devid(%u) sub_module_id(%u)\n", dev_id,
                          sub_module);
    }
}

STATIC void dfm_driver_dump_match_bbox(u32 dev_id, u8 coreid, u32 excep_id, u8 etype, const excep_time *time_sync,
                                       u32 arg, bbox_cb_dump_done fndone)
{
    dfm_driver_dump(dev_id, coreid, excep_id, etype);
    if (fndone != NULL) {
        fndone(dev_id, coreid, excep_id, etype, time_sync);
    }
    return;
}
#ifndef CFG_SOC_PLATFORM_MINIV2
STATIC void dfm_dvpp_dump(u32 dev_id, u8 coreid, u32 excep_id, u32 etype)
{
    char *buf = NULL;
    int ret;

    if ((g_dfm_info.ops_dvpp_dump == NULL) || (g_dfm_info.dvpp_log_paddr == 0) ||
        (g_dfm_info.dvpp_log_len[dev_id] == 0)) {
        dfm_err("dev_id(%u) coreid(%u): dfm_dvpp_dump.\n", dev_id, coreid);
        return;
    }
    g_dfm_info.ops_dvpp_dump(dev_id, excep_id, etype, coreid, &buf);

    ret = dfm_write_black_box(dev_id, DFM_MODULE_ID_DVPP, 0, buf);
    if (ret != DFM_OK) {
        dfm_err("dev_id(%u) coreid(%u): dfm_write_black_box erro, module_id(%u).\n", dev_id, coreid,
                DFM_MODULE_ID_DVPP);
    }

    return;
}

STATIC void dfm_dvpp_dump_match_bbox(u32 dev_id, u8 coreid, u32 excep_id, u8 etype, const excep_time *time, u32 arg,
                                     bbox_cb_dump_done fndone)
{
    dfm_dvpp_dump(dev_id, excep_id, etype, coreid);
    if (fndone != NULL) {
        fndone(dev_id, coreid, excep_id, etype, time);
    }
    return;
}
#endif
STATIC int dfm_get_excep_item(bbox_exception_info_s *exc, struct dfm_exception_info *excep, u32 idx)
{
    int ret;

    ret = memcpy_s(exc->e_from_module, RDR_MODULE_NAME_LEN - 1, excep->module_name, strlen(excep->module_name) + 1);
    if (ret != EOK) {
        dfm_err("module name copy fail, ret(%d)\n", ret);
        return DFM_ERROR;
    }
    exc->e_from_module[RDR_MODULE_NAME_LEN - 1] = '\0';

    ret = memcpy_s(exc->e_desc, RDR_EXCEPTIONDESC_MAXLEN - 1, excep->desc, strlen(excep->desc) + 1);
    if (ret != EOK) {
        dfm_err("desc copy fail, ret(%d)\n", ret);
        return DFM_ERROR;
    }
    exc->e_desc[RDR_EXCEPTIONDESC_MAXLEN - 1] = '\0';

    exc->e_excepid = excep->excep_id;
    exc->e_excepid_end = excep->excep_id;
    exc->e_process_priority = (u8)excep->process_priority;
    exc->e_reboot_priority = (u8)excep->reboot_priority;
    exc->e_notify_core_mask = RDR_COREID_MASK(excep->module_id);
    exc->e_reset_core_mask = 0;
    exc->e_from_core = (u8)excep->module_id;
    exc->e_exce_type = excep->exce_type;

    if (excep->module_id == DFM_MODULE_ID_DVPP) {
        exc->e_reentrant = RDR_REENTRANT_ALLOW;
    } else {
        exc->e_reentrant = RDR_REENTRANT_DISALLOW;
    }

    return DFM_OK;
}

/* exception code registered from dfm to rdr */
int dfm_register_exception(struct dfm_exception_info *excep, u32 num)
{
    u32 i, j;
    int ret;

    for (i = 0; i < num; i++) {
        bbox_exception_info_s exc = {0};
        ret = dfm_get_excep_item(&exc, &excep[i], i);
        if (ret != DFM_OK) {
            dfm_err("excep item get fail, index(%u), ret(%d)\n", i, ret);
            goto err_register_except;
        }
        /* return 0 means fail */
        if (bbox_register_exception(&exc) == 0) {
            dfm_err("excep register fail, index(%u)\n", i);
            goto err_register_except;
        }
    }
    return DFM_OK;
err_register_except:
    for (j = 0; j < i; j++) {
        ret = bbox_unregister_exception(excep[j].excep_id);
        if (ret != BBOX_SUCCESS) {
            dfm_err("dfm unregiseter excep fail, index(%u), ret(%d)\n", j, ret);
            /* don't return here, unregister except list as much as we can */
        }
    }
    return DFM_ERROR;
}

int dfm_unregister_exception(struct dfm_exception_info *excep, u32 num)
{
    u32 ret;
    u32 i;

    for (i = 0; i < num; i++) {
        ret = bbox_unregister_exception(excep[i].excep_id);
        if (ret != BBOX_SUCCESS) {
            dfm_err("dfm unregister exception fail, index(%u), ret(%d).\n", i, ret);
            return ret;
        }
    }

    return DFM_OK;
}

STATIC int dfm_dvpp_dump_reigster(struct dfm_module_register *dfm_module_info)
{
    if (dfm_module_info->ops_dump == NULL) {
        dfm_err("dump is NULL\n");
        return DFM_ERROR;
    }
    g_dfm_info.ops_dvpp_dump = dfm_module_info->ops_dump;
    return DFM_OK;
}

STATIC int dfm_dvpp_dump_unregister(void)
{
    g_dfm_info.ops_dvpp_dump = NULL;
    return DFM_OK;
}

STATIC int dfm_driver_dump_register(struct dfm_module_register *dfm_module_info)
{
    if ((dfm_module_info->ops_dump == NULL) && (dfm_module_info->ops_dump_ex == NULL)) {
        dfm_err("dump is NULL\n");
        return DFM_ERROR;
    }
    if (dfm_module_info->sub_module_id >= DFM_SUBMODULE_ID_END) {
        dfm_err("Unknown sub module, sub_modul_id(%u)\n", dfm_module_info->sub_module_id);
        return DFM_ERROR;
    }
    g_dfm_info.ops_drv_dump[dfm_module_info->sub_module_id] = dfm_module_info->ops_dump;
    g_dfm_info.ops_drv_dump_ex[dfm_module_info->sub_module_id] = dfm_module_info->ops_dump_ex;
    return DFM_OK;
}

STATIC int dfm_drvier_dump_unregister(u32 sub_module_id)
{
    if (sub_module_id >= DFM_SUBMODULE_ID_END) {
        dfm_err("Unknown sub module, sub_module_id(%u)\n", sub_module_id);
        return DFM_ERROR;
    }
    g_dfm_info.ops_drv_dump[sub_module_id] = NULL;
    g_dfm_info.ops_drv_dump_ex[sub_module_id] = NULL;
    return DFM_OK;
}

/* register callback to dfm */
int dfm_register_module(struct dfm_module_register *dfm_module_info)
{
    int ret;

    if (dfm_module_info == NULL) {
        dfm_err("dfm_module_info is NULL.\n");
        return DFM_ERROR;
    }
    switch (dfm_module_info->module_id) {
        case DFM_MODULE_ID_DRIVER:
            ret = dfm_driver_dump_register(dfm_module_info);
            break;
        case DFM_MODULE_ID_DVPP:
            ret = dfm_dvpp_dump_reigster(dfm_module_info);
            break;
        default:
            ret = DFM_ERROR;
            dfm_err("module_id(%u)\n", dfm_module_info->module_id);
            break;
    }

    return ret;
}
EXPORT_SYMBOL(dfm_register_module); //lint !e508

int dfm_unregister_module(u32 module_id, u32 sub_module_id)
{
    int ret;

    switch (module_id) {
        case DFM_MODULE_ID_DVPP:
            ret = dfm_dvpp_dump_unregister();
            break;
        case DFM_MODULE_ID_DRIVER:
            ret = dfm_drvier_dump_unregister(sub_module_id);
            break;
        default:
            dfm_err("Unknown module, module_id(%u)\n", module_id);
            ret = DFM_ERROR;
            break;
    }
    return ret;
}
EXPORT_SYMBOL(dfm_unregister_module); //lint !e508

/* system error report */
void dfm_system_error_report(u32 dev_id, u32 excep_id, u32 arg)
{
    struct timespec64 sys_time = {0};
    excep_time timestamp = {0};

    if (dev_id >= dfm_get_devnum()) { //lint !e574
        dfm_err("device id invalid, devid(%u) devnum(%u)\n", dev_id, dfm_get_devnum());
        return;
    }
    ktime_get_coarse_real_ts64(&sys_time);
    timestamp.tv_sec = sys_time.tv_sec;
    timestamp.tv_usec = sys_time.tv_nsec / DFM_NUM_OF_MS_PER_SEC;

    bbox_system_error(dev_id, excep_id, &timestamp, arg);
}
EXPORT_SYMBOL(dfm_system_error_report); //lint !e508

void dfm_system_error_report_ex(unsigned int dev_id, unsigned int excep_id, unsigned int arg)
{
    dfm_system_error_report(dev_id, excep_id, arg);
}
EXPORT_SYMBOL(dfm_system_error_report_ex); //lint !e508

STATIC int dfm_exceplist_register(void)
{
    u32 num;
    int ret;

    num = sizeof(g_drv_excep_list) / sizeof(g_drv_excep_list[0]);
    ret = dfm_register_exception(g_drv_excep_list, num);
    if (ret != DFM_OK) {
        dfm_err("dfm register exception fail, ret(%d)\n", ret);
        return DFM_ERROR;
    }
    return DFM_OK;
}

STATIC void dfm_exceplist_unregister(void)
{
    int num;
    int ret;

    num = sizeof(g_drv_excep_list) / sizeof(g_drv_excep_list[0]);
    ret = dfm_unregister_exception(g_drv_excep_list, num);
    if (ret != DFM_OK) {
        /* unregister fail, we have nothing to do with this */
        dfm_err("bbox exception unregister module fail, ret=%d.\n", ret);
    }
}

STATIC int dfm_driver_register(u32 devnum)
{
    void __iomem *vaddr[DFM_MAX_DAVINCI_NUM] = {NULL};
    struct bbox_register_module_result retinfo = {0};
    struct bbox_module_ops ops = {0};
    phys_addr_t paddr;
    size_t len;
    u32 i, j;
    int ret;

    ops.ops_dump = dfm_driver_dump_match_bbox;
    ops.ops_reset = dfm_reset_bbox;
    ret = bbox_register_module_ops(DFM_MODULE_ID_DRIVER, &ops, &retinfo);
    if (ret != BBOX_SUCCESS) {
        dfm_err("bbox register driver fail, ret(%d)\n", ret);
        return DFM_ERROR;
    }
    paddr = (phys_addr_t)retinfo.log_addr;
    len = (size_t)retinfo.log_len;
    DFM_CHECK_EXP_ACT((paddr == 0) || (len == 0), return DFM_ERROR, "bbox static mem check fail, len(%lu)\n", len);

    for (i = 0; i < devnum; i++) {
        vaddr[i] = ioremap_wc(paddr + (DFM_SMP_ADDR_OFFSET * i), len);
        if (vaddr[i] == NULL) {
            dfm_err("remap fail, devid(%u)\n", i);
            goto err_remap_fail;
        }
        g_dfm_info.driver_log_vaddr[i] = (u64)(uintptr_t)vaddr[i];
        g_dfm_info.driver_log_len[i] = len;
        g_dfm_info.driver_log_offset[i] = g_dfm_info.driver_log_len[i];
    }

    return DFM_OK;
err_remap_fail:
    for (j = 0; j < i; j++) {
        iounmap(vaddr[j]);
        vaddr[j] = NULL;
        g_dfm_info.driver_log_vaddr[j] = 0;
        g_dfm_info.driver_log_len[j] = 0;
        g_dfm_info.driver_log_offset[j] = 0;
    }
    paddr = 0;
    (void)bbox_unregister_module_ops(DFM_MODULE_ID_DRIVER);
    return DFM_ERROR;
}

STATIC void dfm_driver_unregister(u32 devnum)
{
    u32 i;

    for (i = 0; i < devnum; i++) {
        void __iomem *vaddr = (void __iomem *)(uintptr_t)g_dfm_info.driver_log_vaddr[i];
        if (vaddr != NULL) {
            iounmap(vaddr);
            g_dfm_info.driver_log_vaddr[i] = 0;
            g_dfm_info.driver_log_offset[i]  = 0;
            g_dfm_info.driver_log_len[i] = 0;
        }
    }
    (void)bbox_unregister_module_ops(DFM_MODULE_ID_DRIVER);
}
#ifndef CFG_SOC_PLATFORM_MINIV2
STATIC int dfm_dvpp_register(u32 devnum)
{
    void __iomem *vaddr[DFM_MAX_DAVINCI_NUM] = {NULL};
    struct bbox_register_module_result retinfo = {0};
    struct bbox_module_ops ops = {0};
    phys_addr_t paddr;
    size_t len;
    u32 i, j;
    int ret;

    ops.ops_dump = dfm_dvpp_dump_match_bbox;
    ops.ops_reset = dfm_reset_bbox;
    ret = bbox_register_module_ops(DFM_MODULE_ID_DVPP, &ops, &retinfo);
    if (ret != BBOX_SUCCESS) {
        dfm_err("bbox register dvpp fail, ret(%d)\n", ret);
        return DFM_ERROR;
    }
    paddr = (phys_addr_t)retinfo.log_addr;
    len = (size_t)retinfo.log_len;
    for (i = 0; i < devnum; i++) {
        vaddr[i] = ioremap_wc(paddr + DFM_SMP_ADDR_OFFSET * i, len);
        if (vaddr[i] == NULL) {
            dfm_err("remap fail, devid(%u)\n", i);
            goto err_remap_fail;
        }
        g_dfm_info.dvpp_log_vaddr[i] = (u64)(uintptr_t)vaddr[i];
        g_dfm_info.dvpp_log_len[i] = (u32)retinfo.log_len / devnum;
        g_dfm_info.dvpp_log_offset[i] = g_dfm_info.dvpp_log_len[i];
    }
    g_dfm_info.dvpp_log_paddr = (u64)paddr;

    return DFM_OK;
err_remap_fail:
    for (j = 0; j < i; j++) {
        iounmap(vaddr[j]);
        vaddr[j] = NULL;
        g_dfm_info.dvpp_log_vaddr[j] = 0;
        g_dfm_info.dvpp_log_len[j] = 0;
        g_dfm_info.dvpp_log_offset[j] = 0;
    }
    paddr = 0;
    (void)bbox_unregister_module_ops(DFM_MODULE_ID_DVPP);
    return DFM_ERROR;
}
#endif
STATIC void dfm_dvpp_unregister(u32 devnum)
{
    u32 i;

    for (i = 0; i < devnum; i++) {
        void __iomem *vaddr = (void __iomem *)(uintptr_t)g_dfm_info.dvpp_log_vaddr[i];
        if (vaddr != NULL) {
            iounmap(vaddr);
            g_dfm_info.dvpp_log_vaddr[i] = 0;
            g_dfm_info.dvpp_log_len[0] = 0;
            g_dfm_info.dvpp_log_offset[0] = 0;
        }
    }
    g_dfm_info.dvpp_log_paddr = 0;
    (void)bbox_unregister_module_ops(DFM_MODULE_ID_DVPP);
}

int dfm_stub_print(u32 dev_id, u32 module_id, u32 type)
{
    int tlen;
    u32 buf_len;
    unsigned long flags;
    char *log_buf = NULL;
    char *dot_info = NULL;
    struct timespec64 curr_time = {0};

    if (dev_id >= g_dev_num) {
        dfm_err("device id invalid, dev_id = %d.\n", dev_id);
        return DFM_ERROR;
    }
    ktime_get_raw_ts64(&curr_time);
    buf_len = g_dfm_info.driver_log_len[dev_id];
    log_buf = (char *)(uintptr_t)g_dfm_info.driver_log_vaddr[dev_id];

    if ((log_buf == NULL) || (buf_len == 0)) {
        dfm_err("buf is invalid, buf len = %d\n", buf_len);
        return DFM_ERROR;
    }
    dot_info = (type == 0 ? "Init" : "UnInit");
    spin_lock_irqsave(&g_dfm_info.spinlock, flags);
    tlen = snprintf_s(log_buf, buf_len, buf_len - 1,
                      "%s devid[%u], module: %u, time = %ld.%06d",
                      dot_info, dev_id, module_id, curr_time.tv_sec, (curr_time.tv_nsec / TIME_NS_TO_US));
    if (tlen < 0) {
        spin_unlock_irqrestore(&g_dfm_info.spinlock, flags);
        dfm_err("%s devid[%u], module: %u snprintf_s fail.\n", dot_info, dev_id, module_id);
        return DFM_ERROR;
    }
    spin_unlock_irqrestore(&g_dfm_info.spinlock, flags);
    return DFM_OK;
}

EXPORT_SYMBOL(dfm_stub_print); //lint !e508

STATIC void dfm_info_init(void)
{
    spin_lock_init(&g_dfm_info.spinlock);
}

STATIC void dfm_info_clear(void)
{
    int ret;

    ret = memset_s(&g_dfm_info, sizeof(g_dfm_info), 0, sizeof(g_dfm_info));
    if (ret != EOK) {
        /* we can do nothing with this */
        dfm_err("dfm_info clear fail, ret(%d)\n", ret);
    }
}

int __init dfm_init(void)
{
    u32 devnum;
    int ret;

    devnum = dfm_get_devnum();
    if (devnum > DFM_MAX_DAVINCI_NUM || devnum == 0) {
        dfm_err("get devnum fail, devnum(%u)\n", devnum);
        return DFM_ERROR;
    }
    ret = dfm_driver_register(devnum);
    if (ret != DFM_OK) {
        goto err_drv_register;
    }
#ifndef CFG_SOC_PLATFORM_MINIV2
    ret = dfm_dvpp_register(devnum);
    if (ret != DFM_OK) {
        goto err_dvpp_register;
    }
#endif
    ret = dfm_exceplist_register();
    if (ret != DFM_OK) {
        goto err_exceplist_register;
    }
    dfm_info_init();

    return DFM_OK;
err_exceplist_register:
    dfm_dvpp_unregister(devnum);
#ifndef CFG_SOC_PLATFORM_MINIV2
err_dvpp_register:
    dfm_driver_unregister(devnum);
#endif
err_drv_register:
    dfm_clear_devnum();
    return DFM_ERROR;
}

void __exit dfm_exit(void)
{
    u32 devnum;

    devnum = dfm_get_devnum();
    if (devnum > DFM_MAX_DAVINCI_NUM || devnum == 0) {
        dfm_err("get devnum fail, devnum(%u)\n", devnum);
        return;
    }
    dfm_exceplist_unregister();
    dfm_driver_unregister(devnum);
    dfm_dvpp_unregister(devnum);

    dfm_info_clear();
    dfm_clear_devnum();
}

module_init(dfm_init);
module_exit(dfm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("DAVINCI dfm driver");
