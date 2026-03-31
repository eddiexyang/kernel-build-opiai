/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
 * Description: ts power management
 * Author: huawei
 * Create: 2019-10-15
 */
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/limits.h>

#include "tsdrv_pm.h"
#include "devdrv_manager.h"
#include "tsdrv_log.h"
#include "tsdrv_firmware_load.h"
#include "tsdrv_soc_common.h"
#include "tsdrv_soc_pm.h"
#include "drv_ipc.h"
#include "icm_interface.h"
#include "tsmng_interface.h"

#define INFORM_STATUS_CHECK_INTERVAL_MS  5
#define DEVDRV_PLAT_TYPE_EMU         0x1

#define DEVDRV_TS_FAIL_SUSPEND 0
#define DEVDRV_TS_READY_SUSPEND 1

STATIC u32 devdrv_ts_ready_suspend = DEVDRV_TS_FAIL_SUSPEND;

STATIC void devdrv_ts_suspend_begin(u32 tsid __maybe_unused)
{
    devdrv_ts_ready_suspend = DEVDRV_TS_FAIL_SUSPEND;
}

STATIC int devdrv_get_ts_suspend_result(u32 tsid __maybe_unused, unsigned long timeout)
{
    unsigned long elapsed_time = 0;

    while (devdrv_ts_ready_suspend == DEVDRV_TS_FAIL_SUSPEND) {
        if (time_after(elapsed_time, timeout)) {
#ifndef TSDRV_UT
            devdrv_drv_err("timeout, fail to get TS suspend ready ipc in %lu ms.\n", timeout);
#endif
            return -ETIMEDOUT;
        }

        elapsed_time += 5; // interval time 5 ms
        msleep(5);
    }

    if (devdrv_ts_ready_suspend == DEVDRV_TS_FAIL_SUSPEND) {
#ifndef TSDRV_UT
        devdrv_drv_err("receive ts suspend ack ipc success. can't turn to suspend.\n");
        return -EPERM;
#endif
    } else {
        devdrv_drv_info("receive ts suspend ack ipc success. turn to suspend.\n");
        return (int)devdrv_ts_ready_suspend;
    }
}

void tsdrv_ts_suspend_ready(u32 devid, u32 tsid, u32 ret)
{
    devdrv_drv_info("receive TS %u suspend ack ipc, ack is: %u.\n", tsid, ret);
    if (ret != 0) {
        devdrv_ts_ready_suspend = DEVDRV_TS_FAIL_SUSPEND;
    } else {
        devdrv_ts_ready_suspend = DEVDRV_TS_READY_SUSPEND;
    }
}

STATIC void devdrv_inform_host_manager(u32 devid, enum devdrv_ts_status status)
{
    devdrv_drv_info("dev[%u] change to stauts[%d]\n", devid, (int)status);
}

STATIC int devdrv_suspend_inform_ts(struct devdrv_info *info)
{
    rproc_msg_t ack_buffer[IPCDRV_RPROC_MSG_LENGTH];
    rproc_msg_t msg[IPCDRV_RPROC_MSG_LENGTH];
    struct ipcdrv_msg_payload *payload = NULL;
    struct ipcdrv_message *ipc_msg = NULL;
    int ret;

    ipc_msg = (struct ipcdrv_message *)msg;
    ipc_msg->ipc_msg_header.msg_type = MSGTYPE_DRIVER_SEND;
    ipc_msg->ipc_msg_header.cmd_type = IPCDRV_TS_SUSPEND;
    ipc_msg->ipc_msg_header.sync_type = IPCDRV_MSG_SYNC;
    ipc_msg->ipc_msg_header.reserved = 0;
    ipc_msg->ipc_msg_header.msg_length = 1;
    ipc_msg->ipc_msg_header.msg_index = 0;
    payload = (struct ipcdrv_msg_payload *)ipc_msg->ipcdrv_payload;
    payload->result = 0;

    ipc_msg = (struct ipcdrv_message *)ack_buffer;
    payload = (struct ipcdrv_msg_payload *)ipc_msg->ipcdrv_payload;
    payload->result = 0;

    ret = icm_msg_send_async(ICM_FD_BUILD(info->dev_id, HISI_RPROC_TX_TS), msg, IPCDRV_RPROC_MSG_LENGTH);
    if (ret != 0) {
        devdrv_drv_err("hisi_rproc_xfer_async failed.\n");
        return -1;
    }
    return 0;
}

int tsdrv_pm_suspend(u32 devid)
{
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *info;
    unsigned long timeout;
    u32 tsid = 0;
    int ret;

    info = devdrv_manager_get_devdrv_info(devid);
    if (info == NULL) {
        devdrv_drv_err("devdrv info is null.\n");
        return -EINVAL;
    }

    /* 6. inform host manager ts is going to sleep */
    devdrv_inform_host_manager(devid, TS_SUSPEND);

    devdrv_drv_info("inform ts to suspend.\n");
    devdrv_ts_suspend_begin(tsid);

    /* 7. send ipc to inform ts to enter low power */
    ret = devdrv_suspend_inform_ts(info);
    if (ret != 0) {
        devdrv_drv_err("devdrv_suspend_inform_ts failed.\n");
        goto error_inform;
    }
    /* 8. wait for ts suspend down */
    if (info->env_type == DEVDRV_PLAT_TYPE_EMU) {
        timeout = 500;  // 500 ms for emu env
    } else {
        timeout = 5000; // 5000 ms for other env
    }
    ret = devdrv_get_ts_suspend_result(tsid, timeout);
    if (ret != DEVDRV_TS_READY_SUSPEND) {
        devdrv_drv_err("receive ts suspend ack ipc, but result is not 0.\n");
        goto error_inform;
    }
    devdrv_drv_debug("config ts gicr.\n");

    pdata = info->pdata;
    /* 9. reset ts's gicr */
    ret = tsdrv_config_gicr(pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_GIC_BASE_INDEX] + 0x300000 + 0x14);
    if (ret != 0) {
        devdrv_drv_err("devdrv_config_gicr, fail config TS's gicr.\n");
        goto error_inform;
    }
    devdrv_drv_debug("reset ts.\n");

    /* 10. reset ts */
    ret = tsdrv_soc_reset_tscpu(pdata->ts_pdata[tsid].ts_sysctl_vaddr, tsid);
    if (ret != 0) {
        devdrv_drv_err("devdrv_reset_tscpu failed, ts device suspend exit.\n");
        goto error_inform;
    }

    devdrv_drv_debug("disable ts disp.\n");

    /* 11. reset_prot_disable */
    tsdrv_soc_reset_prot_disable(pdata->ts_pdata[tsid].ts_sysctl_vaddr);

    devdrv_drv_info("suspend succ.\n");
    return 0;
error_inform:
    devdrv_inform_host_manager(devid, TS_FAIL_TO_SUSPEND);

    /* start heart beat */
    tsdrv_set_ts_status(info->dev_id, 0, TS_WORK);
    if (info->drv_ops->tsdrv_heart_beat_set_work_state != NULL) {
        info->drv_ops->tsdrv_heart_beat_set_work_state(info->dev_id, 0, 1);
    }
    return -ECOMM;
}
EXPORT_SYMBOL(tsdrv_pm_suspend);

int tsdrv_pm_resume(u32 devid)
{
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *info;
    u32 __iomem *sc_testreg = NULL;
    u64 before, after;
    u64 timeout;
    u32 times;
    u32 reg_val;
    int ret;

    info = devdrv_manager_get_devdrv_info(devid);
    if (info == NULL) {
        devdrv_drv_err("devdrv info is null.\n");
        return -EINVAL;
    }
    pdata = (struct devdrv_platform_data *)info->pdata;

    if (!tsdrv_is_ts_sleep(devid, 0)) {
        devdrv_drv_info("dev(%u) ts 0 is not in suspend status. current status %u\n",
            devid, (unsigned int)tsdrv_get_ts_status(devid, 0));
        return 0;
    }
    /* 1. enable ts's daw */
    tsdrv_enable_ts_disp(pdata->platform_info.disp_base);

    /* 2. clear status register */
    sc_testreg = (u32 *)((unsigned long)(uintptr_t)pdata->ts_pdata[0].ts_sysctl_vaddr + DEVDRV_SC_TESTREG_OFFSET);
    writel(DEVDRV_SC_TESTREG_INIT, &sc_testreg[DEVDRV_SC_TESTREG_TS]);

    devdrv_drv_debug("set ts boot addr and time.\n");

    /* 3. set ts's boot addr and current time */
    tsdrv_soc_startup(pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_TS_SUBSYSCTL_INDEX],
        info->fw_info.ts_boot_addr);
    aicpu_set_current_time(info);

    devdrv_drv_debug("dereset ts.\n");

    /* 4. dereset ts */
    ret = tsdrv_soc_dereset_tscpu(pdata->ts_pdata[0].ts_sysctl_vaddr);
    if (ret != 0) {
        devdrv_drv_err("devdrv_dereset_tscpu failed, ts is useless.\n");
        return 0;
    }

    /* 5. polling flag status register with timeout */
    times = 0;
    before = get_jiffies_64();
    timeout = before + msecs_to_jiffies(3500);

    while (1) {
        reg_val = readl(sc_testreg);
        if ((reg_val >= DEVDRV_SC_TESTREG_TS_READY) && (reg_val != 0xFFFFFFFFUL)) {
            devdrv_drv_debug("TS resume succ, times: %u.\n", times);
            break;
        }

        after = get_jiffies_64();
        times = jiffies_to_msecs((u32)((after >= before) ? (after - before) : (U64_MAX - before + after)));

        if (time_after_eq64(after, timeout)) {
            devdrv_drv_err("resume TS timeout, times: %u, reg_val=%u.\n", times, reg_val);
            tsdrv_set_ts_status(info->dev_id, 0, TS_DOWN);
            return 0;
        }
    }

    /* 6. set ts work and start heart beat */
    tsdrv_set_ts_status(info->dev_id, 0, TS_WORK);
    if (info->drv_ops->tsdrv_heart_beat_set_work_state != NULL) {
        info->drv_ops->tsdrv_heart_beat_set_work_state(info->dev_id, 0, 1);
    }

    /* 8. inform host manager ts is working */
    devdrv_inform_host_manager(devid, TS_WORK);
    return 0;
}
EXPORT_SYMBOL(tsdrv_pm_resume);

void tsdrv_pm_shutdown(struct devdrv_info *dev_info)
{
    devdrv_drv_info("device %u is going to shutdown\n", dev_info->dev_id);
}

