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


#include <linux/delay.h>
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/pm.h>
#include <linux/pvclock_gtod.h>
#include <linux/slab.h>
#include <linux/hrtimer.h>
#include <linux/securec.h>
#include <asm/uaccess.h>

#ifndef DEVMNG_UT
#include "devdrv_mailbox.h"
#include "devdrv_common.h"
#include "devdrv_driver_pm.h"
#include "devdrv_functional_cqsq_api.h"
#include "devdrv_mailbox.h"
#include "devdrv_manager_common.h"
#include "devdrv_platform.h"
#include "devdrv_pm.h"
#include "devdrv_manager.h"
#include "devdrv_dfm.h"
#include "devdrv_manager_time.h"
#include "devdrv_ipc.h"
#include "tsdrv_status.h"
#include "icm_interface.h"

struct os_heart_beat g_os_heart_beat;

struct devdrv_pm *devdrv_manager_register_pm(int run_stage, int (*suspend)(u32 devid, u32 status), int (*resume)(u32 devid))
{
    struct devdrv_manager_info *d_info = NULL;
    struct devdrv_pm *pm = NULL;
    unsigned long flags;

    if (suspend == NULL || resume == NULL) {
        devdrv_drv_err("callback func is null.\n");
        return NULL;
    }

    pm = kzalloc(sizeof(struct devdrv_pm), GFP_KERNEL | __GFP_ACCOUNT);
    if (pm == NULL) {
        devdrv_drv_err("kmalloc failed.\n");
        return NULL;
    }

    pm->suspend = suspend;
    pm->resume = resume;
    pm->run_stage = run_stage;

    d_info = devdrv_get_manager_info();

    spin_lock_irqsave(&d_info->pm_list_lock, flags);
    list_add(&pm->list, &d_info->pm_list_header);
    spin_unlock_irqrestore(&d_info->pm_list_lock, flags);

    return pm;
}
EXPORT_SYMBOL(devdrv_manager_register_pm);

void devdrv_manager_unregister_pm(struct devdrv_pm *pm)
{
    struct devdrv_manager_info *d_info = NULL;
    unsigned long flags;

    if (pm == NULL) {
        devdrv_drv_err("invalid input argument.\n");
        return;
    }
    if (pm->suspend == NULL || pm->resume == NULL) {
        devdrv_drv_err("callback func is null.\n");
        return;
    }

    d_info = devdrv_get_manager_info();

    spin_lock_irqsave(&d_info->pm_list_lock, flags);
    list_del(&pm->list);
    spin_unlock_irqrestore(&d_info->pm_list_lock, flags);

    kfree(pm);
    pm = NULL;
}
EXPORT_SYMBOL(devdrv_manager_unregister_pm);

void *devdrv_manager_register_msg_pm(int (*suspend)(void), int (*resume)(void))
{
    struct devdrv_manager_info *d_info = NULL;
    struct devdrv_msg_pm *msg_pm = NULL;
    unsigned long flags;

    if (suspend == NULL || resume == NULL) {
        devdrv_drv_err("callback func is null.\n");
        return NULL;
    }

    msg_pm = kzalloc(sizeof(struct devdrv_msg_pm), GFP_KERNEL | __GFP_ACCOUNT);
    if (msg_pm == NULL) {
        devdrv_drv_err("kmalloc failed.\n");
        return NULL;
    }

    msg_pm->suspend = suspend;
    msg_pm->resume = resume;

    d_info = devdrv_get_manager_info();

    spin_lock_irqsave(&d_info->msg_pm_list_lock, flags);
    list_add(&msg_pm->list, &d_info->msg_pm_list_header);
    spin_unlock_irqrestore(&d_info->msg_pm_list_lock, flags);

    return (void *)msg_pm;
}
EXPORT_SYMBOL(devdrv_manager_register_msg_pm);

void devdrv_manager_unregister_msg_pm(void *msg_pm_p)
{
    struct devdrv_manager_info *d_info = NULL;
    struct devdrv_msg_pm *msg_pm = NULL;
    unsigned long flags;

    if (msg_pm_p == NULL) {
        devdrv_drv_err("invalid input argument.\n");
        return;
    }

    msg_pm = (struct devdrv_msg_pm *)msg_pm_p;
    if (msg_pm->suspend == NULL || msg_pm->resume == NULL) {
        devdrv_drv_err("callback func is null.\n");
        return;
    }

    d_info = devdrv_get_manager_info();

    spin_lock_irqsave(&d_info->msg_pm_list_lock, flags);
    list_del(&msg_pm->list);
    spin_unlock_irqrestore(&d_info->msg_pm_list_lock, flags);

    kfree(msg_pm);
    msg_pm = NULL;
}
EXPORT_SYMBOL(devdrv_manager_unregister_msg_pm);

int devdrv_manager_msg_suspend(void)
{
    struct devdrv_manager_info *d_info = NULL;
    struct list_head *pos = NULL, *n = NULL;
    struct list_head *stop = NULL;
    struct devdrv_msg_pm *msg_pm = NULL;
    unsigned long flags;
    int ret;

    devdrv_drv_info("call devdrv_manager_msg_suspend.\n");

    d_info = devdrv_get_manager_info();

    spin_lock_irqsave(&d_info->msg_pm_list_lock, flags);
    list_for_each_safe(pos, n, &d_info->msg_pm_list_header)
    {
        stop = pos;
        msg_pm = list_entry(pos, struct devdrv_msg_pm, list);
        if (msg_pm->suspend != NULL) {
            ret = msg_pm->suspend();
            if (ret) {
                spin_unlock_irqrestore(&d_info->msg_pm_list_lock, flags);
                devdrv_drv_err("one suspend func failed.\n");
                return ret;
            }
        } else {
            spin_unlock_irqrestore(&d_info->msg_pm_list_lock, flags);
            devdrv_drv_err("suspend func not registered.\n");
            return -1;
        }
    }
    spin_unlock_irqrestore(&d_info->msg_pm_list_lock, flags);

    return ret;
}

int devdrv_manager_msg_resume(void)
{
    struct devdrv_manager_info *d_info = NULL;
    struct list_head *pos = NULL, *n = NULL;
    struct list_head *stop = NULL;
    struct devdrv_msg_pm *msg_pm = NULL;
    unsigned long flags;
    int ret;

    devdrv_drv_info("call devdrv_manager_msg_resume.\n");

    d_info = devdrv_get_manager_info();

    spin_lock_irqsave(&d_info->msg_pm_list_lock, flags);
    list_for_each_safe(pos, n, &d_info->msg_pm_list_header)
    {
        stop = pos;
        msg_pm = list_entry(pos, struct devdrv_msg_pm, list);
        if (msg_pm->resume != NULL) {
            ret = msg_pm->resume();
            if (ret) {
                spin_unlock_irqrestore(&d_info->msg_pm_list_lock, flags);
                devdrv_drv_err("one resume func failed.\n");
                return ret;
            }
        } else {
            spin_unlock_irqrestore(&d_info->msg_pm_list_lock, flags);
            devdrv_drv_err("resume func not registered.\n");
            return -1;
        }
    }
    spin_unlock_irqrestore(&d_info->msg_pm_list_lock, flags);

    return ret;
}

STATIC void devdrv_inform_device_manager(struct devdrv_info *info, enum devdrv_ts_status status)
{
    struct devdrv_manager_info *d_info = NULL;
    struct list_head *pos = NULL, *n = NULL;
    struct devdrv_pm *pm = NULL;
    unsigned long flags;

    d_info = devdrv_get_manager_info();

    /* inform all modules related to ts driver that ts can not work */
    spin_lock_irqsave(&d_info->pm_list_lock, flags);
    list_for_each_safe(pos, n, &d_info->pm_list_header)
    {
        pm = list_entry(pos, struct devdrv_pm, list);
        if (pm->suspend != NULL)
            (void)pm->suspend(info->dev_id, TS_DOWN);
    }
    spin_unlock_irqrestore(&d_info->pm_list_lock, flags);

    devdrv_driver_hardware_exception(info);
}

extern void devdrv_manager_inform_device_status(struct devdrv_info *info, enum devdrv_ts_status status);

STATIC int devdrv_manager_suspend_prev(struct devdrv_info *info)
{
    /* 1. stop heart beat */
    if (info->drv_ops->tsdrv_heart_beat_set_work_state != NULL) {
        info->drv_ops->tsdrv_heart_beat_set_work_state(info->dev_id, 0, 0);
    }

    /* 2. set sleep status */
    tsdrv_set_ts_status(info->dev_id, 0, TS_SUSPEND);

    return 0;
}

int devdrv_manager_suspend(struct devdrv_info *info)
{
    struct devdrv_manager_info *d_info = NULL;
    struct list_head *pos = NULL, *n = NULL;
    struct list_head *stop = NULL;
    struct devdrv_pm *pm = NULL;
    unsigned long flags;
    int ret;

    devdrv_drv_debug("call devdrv_manager_suspend.\n");

    d_info = devdrv_get_manager_info();

    /* 1. stop heart beat */
    /* 2. check whether ts has completed all task */
    /* 3. set sleep status */
    /* 4. flush mailbox */
    if (devdrv_manager_suspend_prev(info)) {
        devdrv_drv_err("devdrv_manager_suspend_prev fail, can not suspend.\n");
        return -1;
    }

    /* 5. all module's suspend callback func */
    spin_lock_irqsave(&d_info->pm_list_lock, flags);
    list_for_each_safe(pos, n, &d_info->pm_list_header)
    {
        stop = pos;
        pm = list_entry(pos, struct devdrv_pm, list);
        if (pm->suspend != NULL) {
            ret = pm->suspend(info->dev_id, TS_SUSPEND);
            if (ret) {
                spin_unlock_irqrestore(&d_info->pm_list_lock, flags);
                devdrv_drv_err("one suspend func failed.\n");
                goto error;
            }
        }
    }
    spin_unlock_irqrestore(&d_info->pm_list_lock, flags);
    devdrv_drv_info("suspend succ.\n");
    return 0;

error:
    spin_lock_irqsave(&d_info->pm_list_lock, flags);
    list_for_each_safe(pos, n, &d_info->pm_list_header)
    {
        pm = list_entry(pos, struct devdrv_pm, list);
        if (stop == pos)
            break;
        if (pm->resume != NULL) {
            (void)pm->resume(info->dev_id);
        }
    }
    spin_unlock_irqrestore(&d_info->pm_list_lock, flags);

    tsdrv_set_ts_status(info->dev_id, 0, TS_WORK);
    if (info->drv_ops->tsdrv_heart_beat_set_work_state != NULL) {
        info->drv_ops->tsdrv_heart_beat_set_work_state(info->dev_id, 0, 1);
    }

    devdrv_drv_err("suspend fail.\n");

    return -1;
}

int devdrv_manager_resume(struct devdrv_info *info)
{
    struct devdrv_manager_info *d_info = NULL;
    struct list_head *pos = NULL, *n = NULL;
    struct devdrv_pm *pm = NULL;
    unsigned long flags;

    devdrv_drv_debug("call devdrv_manager_resume.\n");
    d_info = devdrv_get_manager_info();

    /* resume all registered resume callback func */
    spin_lock_irqsave(&d_info->pm_list_lock, flags);
    list_for_each_safe(pos, n, &d_info->pm_list_header)
    {
        pm = list_entry(pos, struct devdrv_pm, list);
        if (pm->resume != NULL) {
            (void)pm->resume(info->dev_id);
        }
    }
    spin_unlock_irqrestore(&d_info->pm_list_lock, flags);

    devdrv_drv_info("resume succ.\n");
    return 0;
}

EXPORT_SYMBOL(devdrv_manager_suspend);
EXPORT_SYMBOL(devdrv_manager_resume);

int devdrv_inform_lpm3_upper_ddr_freq(u32 devid)
{
    struct devdrv_ipc_lpm3 ipc_msg = { 0 };
    struct devdrv_manager_info *manager_info = NULL;
    struct devdrv_info *dev_info = NULL;
    int ret;

    manager_info = devdrv_get_manager_info();
    if (manager_info == NULL) {
        devdrv_drv_err("device does not exist.\n");
        return -1;
    }

    if (devid >= MAX_CHIP_NUM) {
        devdrv_drv_err("device id %d is invalid.\n", devid);
        return -1;
    }

    dev_info = manager_info->dev_info[devid];
    if (dev_info == NULL) {
        devdrv_drv_err("device does not exist.\n");
        return -1;
    }


    ipc_msg.cmd_type1 = LPM3_DDR_FREQ;
    ipc_msg.cmd_type0 = LPM3_NOTIFY_CMD;
    ipc_msg.target_id = LPM3_PSCI_ID;
    ipc_msg.source_id = DEVICE_CCPU_ID;
    ipc_msg.cmd_para0 = LPM3_UPPER_FREQ;
    ret = icm_msg_send_async(ICM_FD_BUILD(0, HISI_RPROC_TX_LPM3), (rproc_msg_t *)&ipc_msg, 2);

    if (ret) {
        devdrv_drv_err("ipc message send failed\n");
        return -1;
    }

    devdrv_drv_debug("inform lpm3 set freq success, status = %d\n", ipc_msg.cmd_para0);

    return 0;
}

int devdrv_inform_lpm3_lower_ddr_freq(u32 devid)
{
    struct devdrv_ipc_lpm3 ipc_msg = { 0 };
    struct devdrv_manager_info *manager_info = NULL;
    struct devdrv_info *dev_info = NULL;
    int ret;

    manager_info = devdrv_get_manager_info();
    if (manager_info == NULL) {
        devdrv_drv_err("device does not exist.\n");
        return -1;
    }

    if (devid >= MAX_CHIP_NUM) {
        devdrv_drv_err("device id %d is invalid.\n", devid);
        return -1;
    }

    dev_info = manager_info->dev_info[devid];
    if (dev_info == NULL) {
        devdrv_drv_err("device does not exist.\n");
        return -1;
    }

    ipc_msg.cmd_type1 = LPM3_DDR_FREQ;
    ipc_msg.cmd_type0 = LPM3_NOTIFY_CMD;
    ipc_msg.target_id = LPM3_PSCI_ID;
    ipc_msg.source_id = DEVICE_CCPU_ID;
    ipc_msg.cmd_para0 = LPM3_LOWER_FREQ;
    ret = icm_msg_send_async(ICM_FD_BUILD(0, HISI_RPROC_TX_LPM3), (rproc_msg_t *)&ipc_msg, 2);

    if (ret) {
        devdrv_drv_err("ipc message send failed\n");
        return -1;
    }

    devdrv_drv_debug("inform lpm3 set freq success, status = %d\n", ipc_msg.cmd_para0);

    return 0;
}

void devdrv_inform_lpm3_change_aicore_freq(int gpio_value, int flag)
{
    struct devdrv_ipc_lpm3 ipc_msg = { 0 };
    int ret;

    devdrv_drv_warn("inform lpm3 change aicore freq ,"
        " gpio_value=%d, flag=%d\n",
        gpio_value, flag);

    ipc_msg.cmd_type1 = LPM3_AICORE_FREQ;
    ipc_msg.cmd_type0 = LPM3_NOTIFY_CMD;
    ipc_msg.target_id = LPM3_EDP;
    ipc_msg.source_id = DEVICE_CCPU_ID;

    if (gpio_value) {
        ipc_msg.cmd_para0 = LPM3_UPPER_FREQ;
    } else {
        ipc_msg.cmd_para0 = LPM3_LOWER_FREQ;
    }

    ipc_msg.cmd_para1 = flag;

    ret = icm_msg_send_async(ICM_FD_BUILD(0, HISI_RPROC_TX_LPM3), (rproc_msg_t *)&ipc_msg, IPCDRV_RPROC_MSG_LENGTH);

    if (ret) {
        devdrv_drv_err("ipc message send failed, ret=%d, "
            "status = %d, gpio_value=%d, flag=%d\n",
            ret, ipc_msg.cmd_para0, gpio_value, flag);
        return;
    }

    devdrv_drv_warn("inform lpm3 set freq success, "
        "status = %d, gpio_value=%d, flag=%d\n",
        ipc_msg.cmd_para0, gpio_value, flag);
}

void devdrv_ts_exception_task(unsigned long data)
{
    enum devdrv_ts_status status;
    struct devdrv_info *info = NULL;

    info = (struct devdrv_info *)((uintptr_t)data);
    status = tsdrv_get_ts_status(info->dev_id, 0);
    if (status != TS_SUSPEND) {
        status = TS_DOWN;
    }

    devdrv_drv_err("begin to inform ts[%d] status: %d.\n", info->dev_id, status);
    devdrv_inform_device_manager(info, status);
    devdrv_manager_inform_device_status(info, status);
}
EXPORT_SYMBOL(devdrv_ts_exception_task);

/*
 * if ts's sram, doorbell or other register are accessed when ts is sleepping or off line,
 * a hardware irq will sent by hardware. The access will finish successfully,
 * but a excepthion irq will sent by hardware to inform a abnormal access.
 */
irqreturn_t devdrv_nfe_handler(int irq, void *data)
{
#define DEVDRV_DISP_TS_DAW_INDEX 7
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *info = NULL;
    unsigned long flags;
    u32 __iomem *clear = NULL;
    u32 __iomem *daw = NULL;
    u32 value;

    local_irq_save(flags);

    info = (struct devdrv_info *)data;
    pdata = (struct devdrv_platform_data *)info->pdata;

    /* clear interrupt status */
    clear =
        (u32 __iomem *)((uintptr_t)((unsigned long)(uintptr_t)pdata->platform_info.disp_base + DEVDRV_DISP_INTCLR0));
    writel_relaxed(1, clear);

    daw = (u32 __iomem *)((uintptr_t)((unsigned long)(uintptr_t)pdata->platform_info.disp_base + DEVDRV_DISP_DAW_EN));
    value = readl_relaxed(daw);
    if ((value & (0x01UL << DEVDRV_DISP_TS_DAW_INDEX))) {
        devdrv_drv_info("TS's daw is enabled, device manager ignored this irq.\n");
        local_irq_restore(flags);
        return IRQ_HANDLED;
    }

    tsdrv_set_ts_status(info->dev_id, 0, TS_SUSPEND);
    tasklet_schedule(&info->nfe_task);

    local_irq_restore(flags);
    return IRQ_HANDLED;
}


STATIC void devdrv_heart_beat_ai_down(struct devdrv_info *info, const void *data)
{
    const struct devdrv_heart_beat_cq *cq = NULL;
    u32 core_bitmap;
    u32 core_count;
    u32 cpu_bitmap;
    u32 cpu_count;
    u32 i;

    cq = (struct devdrv_heart_beat_cq *)data;
    cpu_bitmap = 0;
    cpu_count = 0;
    core_bitmap = 0;
    core_count = 0;

    if (cq->aicpu_heart_beat_exception) {
        for (i = 0; i < info->ai_cpu_core_num; i++) {
            if (cq->aicpu_heart_beat_exception & (0x01U << i)) {
                cpu_bitmap |= (0x01U << i);
                if (!(info->inuse.ai_cpu_error_bitmap & (0x01U << i))) {
                    devdrv_drv_err("receive TS message ai cpu: %d heart beat exception.\n", i);
                }
            } else
                cpu_count++;
        }
    }
    if (cq->aicore_bitmap) {
        for (i = 0; i < info->ai_core_num; i++) {
            if (cq->aicore_bitmap & (0x01U << i)) {
                core_bitmap |= (0x01U << i);
                if (!(info->inuse.ai_core_error_bitmap & (0x01U << i))) {
                    devdrv_drv_err("receive TS message ai core: %d exception.\n", i);
                }
            } else
                core_count++;
        }
    }

    if (cq->syspcie_sysdma_status & 0xFFFF) {
        devdrv_drv_err("ts sysdma is broken.\n");
        info->ai_subsys_ip_broken_map |= (0x01U << DEVDRV_AI_SUBSYS_SDMA_WORKING_STATUS_OFFSET);
    }
    if ((cq->syspcie_sysdma_status >> 16) & 0xFFFF) {
        devdrv_drv_err("ts syspcie is broken.\n");
        info->ai_subsys_ip_broken_map |= (0x01U << DEVDRV_AI_SUBSYS_SPCIE_WORKING_STATUS_OFFSET);
    }

    info->inuse.ai_cpu_num = cpu_count;
    info->inuse.ai_cpu_error_bitmap = cpu_bitmap;
    info->inuse.ai_core_num = core_count;
    info->inuse.ai_core_error_bitmap = core_bitmap;
}

void tsdrv_heart_beat_ai_down(u32 devid, u32 tsid, const void *data)
{
    struct devdrv_info *info = NULL;

    info = devdrv_manager_get_devdrv_info(devid);
    if (info == NULL) {
        devdrv_drv_err("dev_info[%u] is NULL.\n", devid);
        return;
    }
    devdrv_heart_beat_ai_down(info, data);
}
EXPORT_SYMBOL(tsdrv_heart_beat_ai_down);

STATIC void devdrv_fresh_error_code(u32 devid)
{
    struct devdrv_info *dev_info = NULL;

    dev_info = devdrv_manager_get_devdrv_info(devid);
    if (dev_info == NULL) {
        devdrv_drv_err("device[%d] dev_info is NULL!.\n", devid);
        return;
    }

    if (dev_info->heart_beat_wq != NULL) {
        queue_work(dev_info->heart_beat_wq, &dev_info->work);
    }
}

STATIC enum hrtimer_restart devdrv_refresh_error_code_event(struct hrtimer *t)
{
    devdrv_fresh_error_code(0);
    hrtimer_forward_now(&g_os_heart_beat.timer, ktime_set(OS_HEART_BEAT_INTERVAL, 0));
    return HRTIMER_RESTART;
}

void devdrv_refresh_error_code_init(void)
{
    hrtimer_init(&g_os_heart_beat.timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    g_os_heart_beat.timer.function = devdrv_refresh_error_code_event;
    hrtimer_start(&g_os_heart_beat.timer, ktime_set(OS_HEART_BEAT_INTERVAL, 0), HRTIMER_MODE_REL);
}

void devdrv_refresh_error_code_exit(void)
{
    hrtimer_cancel(&g_os_heart_beat.timer);
}
#else
void devdrv_ts_suspend_ready(u32 ret, u32 devid)
{
}
#endif
