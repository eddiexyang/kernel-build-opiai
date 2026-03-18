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
#include "tsmng_interface.h"
#include "tsdrv_cpumask.h"
#include "devdrv_platform.h"
#ifdef AOS_LLVM_BUILD
#include "devdrv_manager_comm.h"
#endif
#ifndef TSDRV_UT
#define TSDRV_INFORM_TS_TIMTOUT 300000
#else
#define TSDRV_INFORM_TS_TIMTOUT 10000
#endif

#define INFORM_STATUS_CHECK_INTERVAL_US 5000
#define INFORM_STATUS_CHECK_INTERVAL_US_MAX 5100

#define DEVDRV_TS_WAITING_SUSPEND 0xFFU
#define DEVDRV_TS_FAIL_SUSPEND    0U
#define DEVDRV_TS_READY_SUSPEND   1U

#define TSDRV_WAIT_TS_READY_INTERVAL        (10000U)  /* us */
#define TSDRV_WAIT_TS_READY_INTERVAL_MAX    (10100U)  /* us */
#ifdef CFG_SOC_PLATFORM_ESL_FPGA
#define TSDRV_WAIT_TS_READY_RETRY_TIMES  (20000U)
#else
#define TSDRV_WAIT_TS_READY_RETRY_TIMES   (200U)
#endif

STATIC atomic_t devdrv_ts_ready_suspend[DEVDRV_MAX_DAVINCI_NUM][DEVDRV_MAX_TS_NUM] = {};

void tsdrv_ts_suspend_ready(u32 devid, u32 tsid, u32 ret)
{
    if (tsid >= DEVDRV_MAX_TS_NUM) {
        devdrv_drv_err("invalid tsid=%u.\n", tsid);
        return;
    }

    if (ret != 0) {
        atomic_set(&devdrv_ts_ready_suspend[devid][tsid], DEVDRV_TS_FAIL_SUSPEND);
    } else {
        atomic_set(&devdrv_ts_ready_suspend[devid][tsid], DEVDRV_TS_READY_SUSPEND);
    }
    devdrv_drv_info("receive TS suspend ack ipc. (devid=%u; tsid=%u; ack=%u)\n", devid, tsid, ret);
}

STATIC u32 devdrv_get_ts_suspend_result(u32 devid, u32 tsid)
{
    return atomic_read(&devdrv_ts_ready_suspend[devid][tsid]);
}

STATIC void devdrv_inform_host_manager(u32 devid, enum devdrv_ts_status status)
{
    devdrv_drv_info("dev[%u] change to stauts[%d]\n", devid, (int)status);
}

#ifdef CFG_SOC_PLATFORM_MINIV2
static rproc_id_t devdrv_get_inform_rproc_id(u8 pm_status, u32 tsid)
{
    rproc_id_t ipc_rproc_id;

    if (pm_status == DEVINFO_STATUS_SHUTDOWN) {
        if (tsid == 0) {
            ipc_rproc_id = HISI_RPROC_TSC_TX_RPID2;
        } else {
            ipc_rproc_id = HISI_RPROC_TSV_TX_RPID2;
        }
    } else {
        if (tsid == 0) {
            ipc_rproc_id = HISI_RPROC_TSC_TX_RPID0;
        } else {
            ipc_rproc_id = HISI_RPROC_TSV_TX_RPID0;
        }
    }
    return ipc_rproc_id;
}
#endif

#define TSDRV_PM_EXTEND_MAX 1
struct tsdrv_pm_extend tsdrv_pm_ex[TSDRV_PM_EXTEND_MAX];
int tsdrv_pm_extend_set(u32 module, int (*suspend)(u32 devid), int (*resume)(u32 devid))
{
    if (module >= TSDRV_PM_EXTEND_MAX || suspend == NULL || resume == NULL) {
        devdrv_drv_err("invalid value, module(%u).\n", module);
        return -EINVAL;
    }

    tsdrv_pm_ex[module].suspend = suspend;
    tsdrv_pm_ex[module].resume = resume;
    return 0;
}
EXPORT_SYMBOL(tsdrv_pm_extend_set);

STATIC int devdrv_inform_ts(u32 devid, u32 tsid, u8 pm_status)
{
    rproc_msg_t ack_buffer[IPCDRV_RPROC_MSG_LENGTH];
    rproc_msg_t msg[IPCDRV_RPROC_MSG_LENGTH];
    struct ipcdrv_msg_payload *payload = NULL;
    struct ipcdrv_message *ipc_msg = NULL;
    int ret;
#ifdef CFG_SOC_PLATFORM_MINIV2
    rproc_id_t ipc_rproc_id;
#endif

    ipc_msg = (struct ipcdrv_message *)msg;
    ipc_msg->ipc_msg_header.msg_type = MSGTYPE_DRIVER_SEND;
    if (pm_status == DEVINFO_STATUS_SHUTDOWN) {
        ipc_msg->ipc_msg_header.cmd_type = IPCDRV_TS_POWEROFF;
    } else {
        ipc_msg->ipc_msg_header.cmd_type = IPCDRV_TS_SUSPEND;
    }
    ipc_msg->ipc_msg_header.sync_type = IPCDRV_MSG_SYNC;
    ipc_msg->ipc_msg_header.reserved = 0;
    ipc_msg->ipc_msg_header.msg_length = 1;
    ipc_msg->ipc_msg_header.msg_index = 0;
    payload = (struct ipcdrv_msg_payload *)ipc_msg->ipcdrv_payload;
    payload->result = 0;

    ipc_msg = (struct ipcdrv_message *)ack_buffer;
    payload = (struct ipcdrv_msg_payload *)ipc_msg->ipcdrv_payload;
    payload->result = 0;

    devdrv_drv_info("Inform ts change power status. (devid%u; tsid=%u; status=%u)\n",
        devid, tsid, (u32)pm_status);
#ifdef CFG_SOC_PLATFORM_CLOUD
    ret = icm_msg_send_async(ICM_FD_BUILD(devid, HISI_RPROC_TX_TS_MBX17), msg, IPCDRV_RPROC_MSG_LENGTH);
#elif (defined CFG_SOC_PLATFORM_MINIV2)
    ipc_rproc_id = devdrv_get_inform_rproc_id(pm_status, tsid);
    ret = icm_msg_send_async(ICM_FD_BUILD(devid, ipc_rproc_id), msg, IPCDRV_RPROC_MSG_LENGTH);
#elif (defined CFG_SOC_PLATFORM_MINI)
    ret = icm_msg_send_async(ICM_FD_BUILD(devid, HISI_RPROC_TX_TS), msg, IPCDRV_RPROC_MSG_LENGTH);
#else
    ret = rproc_xfer_async(HISI_RPROC_TX_TS, msg, IPCDRV_RPROC_MSG_LENGTH);
#endif
    if (ret != 0) {
#ifndef TSDRV_UT
        devdrv_drv_err("rproc_xfer_async failed ret = %d.\n", ret);
        return ret;
#endif
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
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("wait TS shutdown timeout, (elapsed_time=%lu us).\n", elapsed_time);
#endif
            return -ETIMEDOUT;
        }

        elapsed_time += INFORM_STATUS_CHECK_INTERVAL_US;
        usleep_range(INFORM_STATUS_CHECK_INTERVAL_US, INFORM_STATUS_CHECK_INTERVAL_US_MAX);
    }

    if (devdrv_get_ts_suspend_result(devid, tsid) == DEVDRV_TS_READY_SUSPEND) {
        TSDRV_PRINT_INFO("receive ts suspend ack ipc success. ts:%u ready to suspend.\n", tsid);
        return 0;
    } else {
        TSDRV_PRINT_ERR("receive ts suspend ack ipc success. ts:%u can't turn to suspend.\n", tsid);
        return -EPERM;
    }
}

STATIC int devdrv_wait_ts_suspend_ready(u32 devid, u32 tsid, u8 pm_status)
{
#define NOTIFY_TS_SUSPEND_RETRY_TIMES 3
#define NOTIFY_TS_SUSPEND_RETRY_INTERVAL 100000
#define NOTIFY_TS_SUSPEND_RETRY_INTERVAL_MAX 100100
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
        usleep_range(NOTIFY_TS_SUSPEND_RETRY_INTERVAL, NOTIFY_TS_SUSPEND_RETRY_INTERVAL_MAX);
    }
    return ret;
}
#ifdef AOS_LLVM_BUILD
STATIC void tsdrv_pm_suspend_prev(u32 devid, u32 ts_num)
{
    u32 tsid;
    for (tsid = 0; tsid < ts_num; tsid++) {
        tsdrv_heart_beat_set_work_state(devid, tsid, 0);
        tsdrv_set_ts_status(devid, tsid, TS_SUSPEND);
    }
}
#endif
int tsdrv_pm_suspend(u32 devid)
{
#if !defined(CFG_SOC_PLATFORM_MINIV2) && !defined(CFG_SOC_PLATFORM_MINIV3)
    struct devdrv_platform_data *pdata = NULL;
    u64 gicr_base_addr;
#endif
    struct devdrv_info *info;
    u32 tsid, i;
    int ret;
#ifndef AOS_LLVM_BUILD
    info = devdrv_manager_get_devdrv_info(devid);
#else
    info = dms_get_devinfo(devid);
#endif
    if (info == NULL) {
        devdrv_drv_err("devdrv info is null.\n");
        return -EINVAL;
    }
#ifdef AOS_LLVM_BUILD
    tsdrv_pm_suspend_prev(devid, info->pdata->ts_num);
#endif
    /* 6. inform host manager ts is going to sleep */
    devdrv_inform_host_manager(devid, TS_SUSPEND);

    for (tsid = 0; tsid < DEVDRV_MAX_TS_NUM; tsid++) {
        devdrv_drv_info("inform ts to suspend.\n");
        TSDRV_BOOTDOT_BY_TSID(TS_PLATFORM_SUSPEND_PM_START, tsid);
        if (devdrv_wait_ts_suspend_ready(devid, tsid, DEVINFO_STATUS_SUSPEND)) {
            devdrv_drv_err("send notification to ts[%u] failed.\n", tsid);
            TSDRV_BOOTDOT_BY_TSID(TS_PLATFORM_SUSPEND_PM_NOT_READY, tsid);
            goto error_inform;
        }
        devdrv_drv_debug("config ts gicr.\n");

#if !defined(CFG_SOC_PLATFORM_MINIV2) && !defined(CFG_SOC_PLATFORM_MINIV3)
        pdata = info->pdata;
        gicr_base_addr = (uintptr_t)pdata->platform_info.gicv3_base + 0x300000 + 0x14;
        /* 9. reset ts's gicr */
        ret = tsdrv_soc_config_gicr(gicr_base_addr);
        if (ret != 0) {
            devdrv_drv_err("devdrv_config_gicr, fail config TS's gicr ret = %d.\n", ret);
            goto error_inform;
        }
#endif
        devdrv_drv_debug("reset ts.\n");

        /* 10. reset ts */
        TSDRV_BOOTDOT_BY_TSID(TS_PLATFORM_SUSPEND_RESET_TSCPU, tsid);
        ret = tsdrv_soc_reset_tscpu(info->pdata->ts_pdata[tsid].ts_sysctl_vaddr, tsid);
        if (ret != 0) {
            devdrv_drv_err("devdrv_reset_tscpu failed, ts device suspend exit ret = %d.\n", ret);
            goto error_inform;
        }
    }

    devdrv_drv_debug("disable ts disp.\n");

#if !defined(CFG_SOC_PLATFORM_MINIV2) && !defined(CFG_SOC_PLATFORM_MINIV3)
    /* 11. reset_prot_disable */
    tsdrv_soc_reset_prot_disable(info->pdata->ts_pdata[0].ts_sysctl_vaddr);

#endif

    TSDRV_PRINT_INFO("suspend succ.\n");
    return 0;

error_inform:
    devdrv_inform_host_manager(devid, TS_FAIL_TO_SUSPEND);
    for (i = tsid; i < DEVDRV_MAX_TS_NUM; i++) {
        tsdrv_set_ts_status(info->dev_id, i, TS_WORK);
        tsdrv_heart_beat_set_work_state(info->dev_id, i, 1);
    }
    return -ECOMM;
}
EXPORT_SYMBOL(tsdrv_pm_suspend);

int tsdrv_pm_resume(u32 devid)
{
#ifndef TSDRV_UT
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *info;
    u32 __iomem *sc_testreg = NULL;
    u32 times;
    u32 tsid;
    u32 reg_val;
#if !defined(CFG_SOC_PLATFORM_MINIV2) && !defined(CFG_SOC_PLATFORM_MINIV3)
    int ret;
#endif
#ifndef AOS_LLVM_BUILD
    info = devdrv_manager_get_devdrv_info(devid);
#else
    info = dms_get_devinfo(devid);
#endif
    if (info == NULL) {
        devdrv_drv_err("devdrv info is null.\n");
        return -EINVAL;
    }

    pdata = (struct devdrv_platform_data *)info->pdata;

#if !defined(CFG_SOC_PLATFORM_MINIV2) && !defined(CFG_SOC_PLATFORM_MINIV3)
    devdrv_drv_debug("enable ts disp.\n");

    /* 1. enable ts's daw */
    tsdrv_enable_ts_disp(pdata->platform_info.disp_base);
#endif
    for (tsid = 0; tsid < pdata->ts_num; tsid++) {
        if (tsdrv_get_ts_status(devid, tsid) != TS_SUSPEND) {
            devdrv_drv_info("ts(%u) is not in suspend status.\n", tsid);
            continue;
        }

        /* 2. clear status register */
        sc_testreg = (u32 *)((unsigned long)(uintptr_t)pdata->ts_pdata[tsid].ts_sysctl_vaddr
                + DEVDRV_SC_TESTREG_OFFSET);
        writel(DEVDRV_SC_TESTREG_INIT, &sc_testreg[DEVDRV_SC_TESTREG_TS]);

        devdrv_drv_info("set ts boot addr and time.\n");

        info->fw_info.ts_boot_addr = (u64)pdata->ts_pdata[tsid].ts_dma_handle;
        info->fw_info.ts_boot_addr_virt = (u64)((uintptr_t)pdata->ts_pdata[tsid].ts_load_addr);
        /* 3. set ts's boot addr and current time */
#if !defined(CFG_SOC_PLATFORM_MINIV2) && !defined(CFG_SOC_PLATFORM_MINIV3)
        tsdrv_soc_startup(info->dev_id, info->fw_info.ts_boot_addr,
            pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_TS_SUBSYSCTL_INDEX], info->env_type);
        aicpu_set_current_time(info);
#elif defined(CFG_SOC_PLATFORM_MINIV3)
        tsdrv_soc_startup(info->dev_id, info->fw_info.ts_boot_addr,
            pdata->ts_pdata[0].ts_sysctl_paddr, info->env_type);
        aicpu_set_current_time(info);
#else
        tsdrv_soc_startup(tsid, info->dev_id, info->fw_info.ts_boot_addr);
#endif

#if !defined(CFG_SOC_PLATFORM_MINIV2) && !defined(CFG_SOC_PLATFORM_MINIV3)
        devdrv_drv_info("dereset ts.\n");
        /* 4. dereset ts */
        ret = tsdrv_soc_dereset_tscpu(pdata->ts_pdata[tsid].ts_sysctl_vaddr);
        if (ret != 0) {
            devdrv_drv_err("devdrv_dereset_tscpu failed, ts is useless.\n");
            return 0;
        }
#endif
        /* 5. polling flag status register with timeout */
        times = 0;
        while (1) {
            reg_val = readl(sc_testreg);
            if ((reg_val >= DEVDRV_SC_TESTREG_TS_READY) && (reg_val != 0xFFFFFFFFUL)) {
                devdrv_drv_debug("TS resume succ, times: %u.\n", times);
                break;
            }

            usleep_range(TSDRV_WAIT_TS_READY_INTERVAL, TSDRV_WAIT_TS_READY_INTERVAL_MAX);
            times++;

            /* the maximum time to load TS is 200 */
            if (times > TSDRV_WAIT_TS_READY_RETRY_TIMES) {
                devdrv_drv_err("resume TS timeout, times: %u.\n", times);
                tsdrv_set_ts_status(info->dev_id, tsid, TS_DOWN);
                TSDRV_BOOTDOT_BY_TSID(TS_PLATFORM_RESUME_PM_NOT_READY, tsid);
                return 0;
            }
        }
        devdrv_drv_debug("set work.\n");
        /* 6. set ts work */
        tsdrv_set_ts_status(devid, tsid, TS_WORK);
        tsdrv_heart_beat_set_work_state(info->dev_id, tsid, 1);
    }

#ifdef CFG_SOC_PLATFORM_MDC_V51
    /* 7. hwts need config mb addr */
    devdrv_drv_info("config hwts addr, devid(%u)\n", devid);
    if (tsdrv_pm_ex[0].resume != NULL) {
        if (tsdrv_pm_ex[0].resume(devid)) {
            return 0;
        }
    }
#endif

    /* 8. inform host manager ts is working */
    devdrv_inform_host_manager(devid, TS_WORK);
#endif
    return 0;
}
EXPORT_SYMBOL(tsdrv_pm_resume);

struct tsdrv_shutdown_thread {
    struct devdrv_info *info;
    struct task_struct *task;
    u32 tsid;
    bool task_finished;
};

#ifdef CFG_SOC_PLATFORM_MINIV2
STATIC int tsdrv_shutdown_thread_handler(void *data)
{
#ifndef TSDRV_UT
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
        msleep(1);
    } while (!kthread_should_stop());

    return ret;
#endif
}

STATIC void tsdrv_wait_all_shutdown_task_finished(u32 task_num,
    struct tsdrv_shutdown_thread *shutdown_info)
{
#ifndef TSDRV_UT
    u32 finished_num = 0;
    u32 tsid;
    int ret;

    while (finished_num < task_num) {
        msleep(1);
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
#endif
}
#endif

void tsdrv_pm_shutdown(struct devdrv_info *info)
{
#ifndef TSDRV_UT

#ifdef CFG_SOC_PLATFORM_MINIV2
    struct tsdrv_shutdown_thread shutdown_thread[DEVDRV_MAX_TS_NUM];
    u32 task_cnt = 0;
    u32 tsid;
    int ret = 0;

    for (tsid = 0; tsid < info->pdata->ts_num; tsid++) {
        shutdown_thread[tsid].info = info;
        shutdown_thread[tsid].task_finished = false;
        shutdown_thread[tsid].tsid = tsid;
        shutdown_thread[tsid].task = NULL;

        shutdown_thread[tsid].task = kthread_create(tsdrv_shutdown_thread_handler,
            &shutdown_thread[tsid], "kthread-ts%u-shutdown", tsid);
        if (IS_ERR_OR_NULL(shutdown_thread[tsid].task)) {
            ret = PTR_ERR(shutdown_thread[tsid].task);
            devdrv_drv_err("Unable to run kthread err %d\n", ret);
            continue;
        }

        tsdrv_bind_cpu(shutdown_thread[tsid].task);
        (void)wake_up_process(shutdown_thread[tsid].task);
        task_cnt++;
    }
    tsdrv_wait_all_shutdown_task_finished(task_cnt, shutdown_thread);
#endif

    devdrv_drv_info("Tsdrv platform device shutdown finish. (devid=%u)\n", info->dev_id);
#endif
}
