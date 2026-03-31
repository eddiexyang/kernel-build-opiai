/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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
#include <linux/kthread.h>
#include <linux/atomic.h>

#include "tsdrv_pm.h"
#include "tsdrv_heartbeat.h"
#include "tsdrv_firmware_load.h"
#include "tsdrv_soc_common.h"
#include "tsdrv_soc_pm.h"
#include "tsdrv_log.h"
#include "devdrv_common.h"
#include "devdrv_manager.h"
#include "drv_ipc.h"
#include "tsdrv_interface.h"
#include "icm_interface.h"
#include "tsdrv_log.h"
#include "tsdrv_firmware_reader.h"
#include "tsdrv_file_check.h"
#include "event_sched_inner.h"
#include "devdrv_platform.h"

#ifndef TSDRV_UT
#ifdef CFG_SOC_PLATFORM_MDC_V11
#define TSDRV_INFORM_TS_TIMTOUT 300
#else
#define TSDRV_INFORM_TS_TIMTOUT 5000
#endif
#else
#define TSDRV_INFORM_TS_TIMTOUT 10
#endif

#define INFORM_STATUS_CHECK_INTERVAL_MS 5

#define DEVDRV_TS_WAITING_SUSPEND 0xFFU
#define DEVDRV_TS_FAIL_SUSPEND    0U
#define DEVDRV_TS_READY_SUSPEND   1U

#define TSDRV_WAIT_TS_READY_INTERVAL        (10U)  /* ms */
#ifdef CFG_SOC_PLATFORM_ESL_FPGA
#define TSDRV_WAIT_TS_READY_RETRY_TIMES  (2000U)
#else
#ifdef CFG_SOC_PLATFORM_MDC_V11
#define TSDRV_WAIT_TS_READY_RETRY_TIMES   (200U)
#else
#define TSDRV_WAIT_TS_READY_RETRY_TIMES   (2000U)
#endif
#endif

STATIC atomic_t devdrv_ts_ready_suspend[DEVDRV_MAX_DAVINCI_NUM][DEVDRV_MAX_TS_NUM] = {};

void tsdrv_ts_suspend_ready(u32 devid, u32 tsid, u32 ret)
{
    if (tsid >= DEVDRV_MAX_TS_NUM) {
        devdrv_drv_err("invalid tsid=%u.\n", tsid);
        return;
    }
#ifndef TSDRV_UT
    if (ret != 0) {
        atomic_set(&devdrv_ts_ready_suspend[devid][tsid], DEVDRV_TS_FAIL_SUSPEND);
    } else {
        atomic_set(&devdrv_ts_ready_suspend[devid][tsid], DEVDRV_TS_READY_SUSPEND);
    }
    devdrv_drv_info("receive TS suspend ack ipc. (devid=%u; tsid=%u; ack=%u)\n", devid, tsid, ret);
#endif
}

STATIC u32 devdrv_get_ts_suspend_result(u32 devid, u32 tsid)
{
    return atomic_read(&devdrv_ts_ready_suspend[devid][tsid]);
}

STATIC void devdrv_inform_host_manager(u32 devid, enum devdrv_ts_status status)
{
    devdrv_drv_info("dev change to stauts. (devid=%u; status=%d)\n", devid, (int)status);
}

STATIC int devdrv_inform_ts(u32 devid, u32 tsid, u8 pm_status)
{
    int ret;
    struct icmdrv_ipc_msg ipc_msg = { 0 };

    if (pm_status == DEVINFO_STATUS_SHUTDOWN) {
#ifndef TSDRV_UT
        ipc_msg.sub_cmd = ICM_SUB_CMD_POWEROFF;
#endif
    } else {
        ipc_msg.sub_cmd = ICM_SUB_CMD_SUSPEND;
    }
    ipc_msg.cmd = ICM_MAIN_CMD_TS_REQ;
    ipc_msg.cmd_dest = OBJ_CMD_TS;
    ipc_msg.cmd_src = OBJ_CMD_CCPU;
    ipc_msg.len = 0;

    devdrv_drv_info("Inform ts change power status. (devid=%u; tsid=%u; status=%u)\n",
        devid, tsid, (u32)pm_status);

    ret = icm_msg_send_async(ICM_FD_BUILD(devid, HISI_RPROC_TX_TS_ACPU0), (u32 *)&ipc_msg, IPCDRV_RPROC_MSG_LENGTH);
    if (ret != 0) {
        devdrv_drv_err("Send icm msg failed. (devid=%u; tsid=%u; ret=%d)\n", devid, tsid, ret);
        return ret;
    }

    return 0;
}

STATIC int devdrv_notify_power_mode_change(u32 devid, u32 tsid, u8 pm_status)
{
    unsigned long elapsed_time = 0;
    int ret;
    int old_status;

    old_status = atomic_cmpxchg(&devdrv_ts_ready_suspend[devid][tsid],
        DEVDRV_TS_FAIL_SUSPEND, DEVDRV_TS_WAITING_SUSPEND);
    if (old_status != DEVDRV_TS_READY_SUSPEND) {
        /* send ipc to inform ts to enter low power */
        ret = devdrv_inform_ts(devid, tsid, pm_status);
        if (ret != 0) {
#ifndef TSDRV_UT
            devdrv_drv_err("devdrv_suspend_inform_ts failed, (ret=%d).\n", ret);
            return ret;
#endif
        }
    }

    while (devdrv_get_ts_suspend_result(devid, tsid) == DEVDRV_TS_WAITING_SUSPEND) {
        if (elapsed_time > TSDRV_INFORM_TS_TIMTOUT) {
            TSDRV_PRINT_ERR("wait TS shutdown timeout, (elapsed_time=%lums).\n", elapsed_time);
            return -ETIMEDOUT;
        }

        elapsed_time += INFORM_STATUS_CHECK_INTERVAL_MS;
        tsdrv_sleep(INFORM_STATUS_CHECK_INTERVAL_MS);
    }

    if (devdrv_get_ts_suspend_result(devid, tsid) == DEVDRV_TS_READY_SUSPEND) {
        TSDRV_PRINT_INFO("receive ts suspend ack ipc success. (ts=%u)\n", tsid);
        return 0;
    } else {
        TSDRV_PRINT_ERR("receive ts suspend ack ipc error. (ts=%u)\n", tsid);
        return -EPERM;
    }
}

STATIC int devdrv_wait_ts_suspend_ready(u32 devid, u32 tsid, u8 pm_status)
{
#define NOTIFY_TS_SUSPEND_RETRY_TIMES 3
#define NOTIFY_TS_SUSPEND_RETRY_INTERVAL 100
    int i;
    int ret;
    atomic_set(&devdrv_ts_ready_suspend[devid][tsid], DEVDRV_TS_FAIL_SUSPEND);
    for (i = 0; i < NOTIFY_TS_SUSPEND_RETRY_TIMES; ++i) {
        ret = devdrv_notify_power_mode_change(devid, tsid, pm_status);
        if (ret == 0) {
            break;
        }

        if (i == (NOTIFY_TS_SUSPEND_RETRY_TIMES - 1)) {
            break;
        }
#ifndef TSDRV_UT
        tsdrv_sleep(NOTIFY_TS_SUSPEND_RETRY_INTERVAL);
#endif
    }
    return ret;
}

int tsdrv_pm_suspend(u32 devid)
{
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *info;
    u32 *sc_test_reg = NULL;
    u32 tsid, i;
    int ret;

    info = devdrv_manager_get_devdrv_info(devid);
    if (info == NULL) {
        devdrv_drv_err("devdrv info is null.\n");
        return -EINVAL;
    }

    /* 6. inform host manager ts is going to sleep */
    devdrv_inform_host_manager(devid, TS_SUSPEND);

    for (tsid = 0; tsid < DEVDRV_MAX_TS_NUM; tsid++) {
        devdrv_drv_info("inform ts to suspend.\n");
        TSDRV_BOOTDOT_BY_TSID(TS_PLATFORM_SUSPEND_PM_START, tsid);
        if (devdrv_wait_ts_suspend_ready(devid, tsid, DEVINFO_STATUS_SUSPEND) != 0) {
#ifndef TSDRV_UT
            devdrv_drv_err("send notification to ts failed. (tsid=%u)\n", tsid);
            TSDRV_BOOTDOT_BY_TSID(TS_PLATFORM_SUSPEND_PM_NOT_READY, tsid);
            goto error_inform;
#endif
        }

        /* 9. reset ts's gicr */
        devdrv_drv_info("config ts gicr.\n");
        pdata = info->pdata;
        ret = tsdrv_config_gicr(pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_GIC_BASE_INDEX]
            + 0x200000 + 0x14);
        if (ret != 0) {
#ifndef TSDRV_UT
            devdrv_drv_err("devdrv_config_gicr, fail config TS's gicr. (ret=%d)\n", ret);
            goto error_inform;
#endif
        }

        /* 10. reset ts */
        devdrv_drv_info("reset ts.\n");
        TSDRV_BOOTDOT_BY_TSID(TS_PLATFORM_SUSPEND_RESET_TSCPU, tsid);
        ret = tsdrv_soc_reset_tscpu(info->pdata->ts_pdata[tsid].ts_sysctl_vaddr, tsid);
        if (ret != 0) {
#ifndef TSDRV_UT
            devdrv_drv_err("Suspend tscpu failed, tscpu suspend exit. (ret=%d)\n", ret);
            sc_test_reg = (u32 *)((unsigned long)(uintptr_t)pdata->ts_pdata[tsid].ts_sysctl_vaddr
                + DEVDRV_SC_TESTREG_OFFSET);
            tsdrv_tsfw_boot_reg_print(sc_test_reg, DEVDRV_SC_TESTREG_MAX_NUM);
            writel(DEVDRV_SC_TESTREG_INIT, &sc_test_reg[DEVDRV_SC_TESTREG_TS]);
            goto error_inform;
#endif
        }
    }

    TSDRV_PRINT_INFO("suspend succ.\n");
    return 0;

#ifndef TSDRV_UT
error_inform:
    devdrv_inform_host_manager(devid, TS_FAIL_TO_SUSPEND);
    for (i = tsid; i < DEVDRV_MAX_TS_NUM; i++) {
        tsdrv_set_ts_status(info->dev_id, i, TS_WORK);
        tsdrv_heart_beat_set_work_state(info->dev_id, i, 1);
    }
    return -ECOMM;
#endif
}
EXPORT_SYMBOL(tsdrv_pm_suspend);

int tsdrv_pm_resume(u32 devid)
{
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *info;
    u32 __iomem *sc_testreg = NULL;
    u32 times;
    u32 tsid;
    u32 reg_val;
    int ret = 0;

    info = devdrv_manager_get_devdrv_info(devid);
    if (info == NULL) {
        devdrv_drv_err("devdrv info is null.\n");
        return -EINVAL;
    }

    pdata = (struct devdrv_platform_data *)info->pdata;
    tsdrv_sche_port0_clamp_en();

    for (tsid = 0; tsid < pdata->ts_num; tsid++) {
        if (tsdrv_get_ts_status(devid, tsid) != TS_SUSPEND) {
#ifndef TSDRV_UT
            devdrv_drv_info("ts is not in suspend status. (tsid=%u)\n", tsid);
            continue;
#endif
        }

        TSDRV_BOOTDOT_BY_TSID(TS_PLATFORM_RESUME_DERESET_TSCPU, tsid);

        /* 2. clear status register */
        sc_testreg = (u32 *)((unsigned long)(uintptr_t)pdata->ts_pdata[tsid].ts_sysctl_vaddr
                + DEVDRV_SC_TESTREG_OFFSET);
        writel(DEVDRV_SC_TESTREG_INIT, &sc_testreg[DEVDRV_SC_TESTREG_TS]);

        devdrv_drv_info("set ts boot addr and time.\n");

        info->fw_info.ts_boot_addr = (u64)pdata->ts_pdata[tsid].ts_dma_handle;
        info->fw_info.ts_boot_addr_virt = (u64)((uintptr_t)pdata->ts_pdata[tsid].ts_load_addr);

        /* 3. set ts's boot addr and current time */
        tsdrv_soc_startup(info->dev_id, info->fw_info.ts_boot_addr,
            pdata->ts_pdata[0].ts_sysctl_paddr, info->env_type);
        aicpu_set_current_time(info);

        /* 4. dereset ts */
        devdrv_drv_info("dereset ts.\n");
        ret = tsdrv_soc_dereset_tscpu(pdata->ts_pdata[tsid].ts_sysctl_vaddr);
        if (ret != 0) {
            devdrv_drv_err("devdrv_dereset_tscpu failed, ts is useless.\n");
            return 0;
        }

        /* 5. polling flag status register with timeout */
        times = 0;
        while (1) {
            reg_val = readl(sc_testreg);
            if ((reg_val >= DEVDRV_SC_TESTREG_TS_READY) && (reg_val != 0xFFFFFFFFUL)) {
                tsdrv_tsfw_boot_reg_print(sc_testreg, DEVDRV_SC_TESTREG_MAX_NUM);
                devdrv_drv_debug("TS resume succ, times: %d.\n", times);
                break;
            }

            tsdrv_sleep(TSDRV_WAIT_TS_READY_INTERVAL);
            times++;

            /* the maximum time to load TS is 2000 */
            if (times > TSDRV_WAIT_TS_READY_RETRY_TIMES) {
#ifndef TSDRV_UT
                devdrv_drv_err("resume TS timeout, times: %u.\n", times);
                tsdrv_set_ts_status(info->dev_id, tsid, TS_DOWN);
                tsdrv_tsfw_boot_reg_print(sc_testreg, DEVDRV_SC_TESTREG_MAX_NUM);
                return 0;
#endif
            }
        }
        devdrv_drv_info("set work.\n");

        TSDRV_BOOTDOT_BY_TSID(TS_PLATFORM_RESUME_PM_NOT_READY, tsid);
        /* 6. set ts work */
        tsdrv_set_ts_status(devid, tsid, TS_WORK);
        tsdrv_heart_beat_set_work_state(info->dev_id, tsid, 1);
    }

    /* 8. inform host manager ts is working */
    devdrv_inform_host_manager(devid, TS_WORK);

    return 0;
}
EXPORT_SYMBOL(tsdrv_pm_resume);

#ifdef CFG_SOC_PLATFORM_MDC_V11
struct tsdrv_shutdown_thread {
    struct devdrv_info *info;
    struct task_struct *task;
    u32 tsid;
    bool task_finished;
};
STATIC int tsdrv_shutdown_thread_handler(void *data)
{
    struct tsdrv_shutdown_thread *shutdown_info = (struct tsdrv_shutdown_thread *)data;
    struct devdrv_info *info = NULL;
    u32 tsid;
    int ret;

    if (shutdown_info == NULL) {
        devdrv_drv_err("invalid parameters.\n");
        return -EINVAL;
    }

    info = shutdown_info->info;
    if ((info == NULL) || (info->pdata == NULL)) {
        devdrv_drv_err("invalid parameters.\n");
        ret = -EINVAL;
        goto stop_thread;
    }

    tsid = shutdown_info->tsid;
    ret = devdrv_wait_ts_suspend_ready(info->dev_id, tsid, DEVINFO_STATUS_SHUTDOWN);
    if (ret != 0) {
        devdrv_drv_err("send notice to ts[%u] failed. ret=%d\n", tsid, ret);
    }

    /* The TS needs to be reset even if the TS fails to be notified or times out. */
    ret = tsdrv_soc_reset_tscpu(info->pdata->ts_pdata[tsid].ts_sysctl_vaddr, tsid);
    if (ret != 0) {
        devdrv_drv_err("reset ts[%u] failed, ret = %d.\n", tsid, ret);
    }

stop_thread:
    shutdown_info->task_finished = true;
    do {
        devdrv_drv_info("waitting for ts[%u] shutdown thread exit.\n", shutdown_info->tsid);
        tsdrv_sleep(1);
    } while (!kthread_should_stop());

    return ret;
}

STATIC void tsdrv_wait_all_shutdown_task_finished(u32 task_num,
    struct tsdrv_shutdown_thread *shutdown_info)
{
    u32 finished_num = 0;
    u32 tsid;
    int ret;

    while (finished_num < task_num) {
        tsdrv_sleep(1);
        for (tsid = 0; tsid < task_num; tsid++) {
            if (IS_ERR_OR_NULL(shutdown_info[tsid].task)) {
                continue;
            }

            if (shutdown_info[tsid].task_finished == true) {
                ret = kthread_stop(shutdown_info[tsid].task);
                devdrv_drv_err("ts %u thread exit, ret = %d\n", tsid, ret);
                ++finished_num;
                shutdown_info[tsid].task = NULL;
            }
        }
    }
    devdrv_drv_info("waitting for ts shutdown thread stop.");
}

void tsdrv_pm_shutdown(struct devdrv_info *info)
{
    struct tsdrv_shutdown_thread shutdown_thread[DEVDRV_MAX_TS_NUM];
    u32 task_cnt = 0;
    u32 tsid;
    int ret = 0;

    for (tsid = 0; tsid < info->pdata->ts_num; tsid++) {
        shutdown_thread[tsid].info = info;
        shutdown_thread[tsid].task_finished = false;
        shutdown_thread[tsid].tsid = tsid;
        shutdown_thread[tsid].task = NULL;

        shutdown_thread[tsid].task = kthread_run(tsdrv_shutdown_thread_handler,
            &shutdown_thread[tsid], "kthread-ts%u-shutdown", tsid);
        if (IS_ERR_OR_NULL(shutdown_thread[tsid].task)) {
            ret = PTR_ERR(shutdown_thread[tsid].task);
            devdrv_drv_err("Unable to run kthread err %d\n", ret);
            continue;
        }
        task_cnt++;
    }
    tsdrv_wait_all_shutdown_task_finished(task_cnt, shutdown_thread);
    devdrv_drv_info("Tsdrv platform device shutdown finish. (devid=%u)\n", info->dev_id);
}

#else
#ifndef TSDRV_UT
void tsdrv_pm_shutdown(struct devdrv_info *info)
{
    devdrv_drv_info("Tsdrv platform device shutdown finish. (devid=%u)\n", info->dev_id);
}
#endif
#endif
