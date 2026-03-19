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

#include <linux/version.h>
#include <linux/kthread.h>
#include <linux/timer.h>
#include "dms_kernel_version_adapt.h"
#include "dms_define.h"
#include "dms_interface.h"
#include "dms_sensor_notify.h"
#include "dms_sensor.h"
#include "kernel_version_adapt.h"
#ifndef CFG_HOST_ENV
#include "timer_affinity.h"
#endif
#ifdef CFG_SOC_PLATFORM_MDC_V51
#include "kthread_affinity.h"
#endif

int dms_sensor_scan_task(void *arg)
{
    int i;
    int ret = 0;
    unsigned long timeout, default_timeout;
    struct timeval time_notify_start, time_notify_end;
    struct dms_dev_ctrl_block *dev_ctrl = NULL;
    struct dms_system_ctrl_block *sys_cb = NULL;
    sys_cb = dms_get_sys_ctrl_cb();
    default_timeout = msecs_to_jiffies(DMS_SENSOR_CHECK_TIMER_LEN);
    timeout = default_timeout;

    dms_debug("dms sensor scan task start\n");
    do_gettimeofday(&time_notify_start);

    while (!kthread_should_stop()) {
        /* Waiting for synchronization semaphore lock */
        ret = wait_event_interruptible_timeout(sys_cb->sensor_scan_wait,
                                               sys_cb->sensor_scan_task_state == true, timeout);
        if (ret == 0) {
        /* wait event timeout */
        ret = -ETIMEDOUT;
        } else if (ret == -ERESTARTSYS) {
            /* wait event is interrupted */
            dms_err("wait event interrupted\n");
            ret = -ERESTARTSYS;
        } else if (ret > 0) {
            /* wait event is awakened */
            ret = 0;
        }
        do_gettimeofday(&time_notify_end);
        dms_sensor_notify_event_proc(DMS_SERSOR_SCAN_NOTIFY);
        /* if exceeded scan time, need to go on scan all */
        if (msecs_to_jiffies(dms_get_time_change_ms(time_notify_start, time_notify_end)) < default_timeout) {
            timeout = default_timeout - msecs_to_jiffies(dms_get_time_change_ms(time_notify_start, time_notify_end));
            sys_cb->sensor_scan_task_state = false;
            continue;
        }
        for (i = 0; i < DEVICE_NUM_MAX; i++) {
            dev_ctrl = dms_get_dev_cb(i);
            if (dev_ctrl != NULL) {
                dms_sensor_scan_proc(&dev_ctrl->dev_sensor_cb);
            }
        }
        sys_cb->sensor_scan_task_state = false;
        timeout = default_timeout;
        dms_sensor_notify_count_clear();
        do_gettimeofday(&time_notify_start);
    }

    dms_debug("dms sensor scan task exit\n");
    return ret;
}

static unsigned int dms_sensor_task_init(void)
{
    struct dms_system_ctrl_block *sys_cb = NULL;
    sys_cb = dms_get_sys_ctrl_cb();

    init_waitqueue_head(&sys_cb->sensor_scan_wait);
    sys_cb->sensor_scan_task_state = false;

    /* Start the sensor scan task */
    sys_cb->sensor_scan_task = kthread_create(dms_sensor_scan_task, NULL, "dms_sensor_scan_task");
    if (IS_ERR(sys_cb->sensor_scan_task)) {
        dms_err("sensor scan start failed.\n");
        return DRV_ERROR_INNER_ERR;
    }
#ifdef CFG_SOC_PLATFORM_MDC_V51
    kthread_bind_to_ctrl_cpu(sys_cb->sensor_scan_task);
#endif
    (void)wake_up_process(sys_cb->sensor_scan_task);
    dms_info("create sensor scan task success\n");
    return DRV_ERROR_NONE;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
static void dms_release_sensor_scan_task_sem(unsigned long time)
#else
static void dms_release_sensor_scan_task_sem(struct timer_list *t)
#endif
{
    struct dms_system_ctrl_block *sys_cb = NULL;
    sys_cb = dms_get_sys_ctrl_cb();
    sys_cb->sensor_scan_task_state = true;
    wake_up(&sys_cb->sensor_scan_wait);
    return;
}
static unsigned int dms_init_sensor_timer(void)
{
    struct dms_system_ctrl_block *sys_cb = NULL;
    sys_cb = dms_get_sys_ctrl_cb();
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
    init_timer(&sys_cb->dms_sensor_check_timer);
    sys_cb->dms_sensor_check_timer.function = dms_release_sensor_scan_task_sem;
#else
    timer_setup(&sys_cb->dms_sensor_check_timer, dms_release_sensor_scan_task_sem, 0);
#endif
    sys_cb->dms_sensor_check_timer.expires = jiffies + msecs_to_jiffies(DMS_SENSOR_CHECK_TIMER_LEN);
#ifndef CFG_HOST_ENV
    add_timer_affinity(&sys_cb->dms_sensor_check_timer);
#else
    add_timer(&sys_cb->dms_sensor_check_timer);
#endif
    dms_info("create sensor scan task timer success\n");
    return 0;
}

static void dms_start_sensor_scan(void)
{
    dms_init_sensor_timer();
}

static void dms_clean_sensor_timer(void)
{
    struct dms_system_ctrl_block *sys_cb = NULL;
    sys_cb = dms_get_sys_ctrl_cb();
    timer_shutdown_sync(&sys_cb->dms_sensor_check_timer);
}

static void dms_stop_sensor_scan(void)
{
    struct dms_system_ctrl_block *sys_cb = NULL;
    dms_info("stop sensor task scan \n");
    sys_cb = dms_get_sys_ctrl_cb();
    sys_cb->sensor_scan_task_state = true;
    wake_up(&sys_cb->sensor_scan_wait);
    dms_clean_sensor_timer();
}

static unsigned int dms_sensor_task_exit(void)
{
    struct dms_system_ctrl_block *sys_cb = NULL;
    dms_info("sensor task exit \n");
    sys_cb = dms_get_sys_ctrl_cb();
    if (!IS_ERR(sys_cb->sensor_scan_task)) {
        kthread_stop(sys_cb->sensor_scan_task);
    }
    sys_cb->sensor_scan_task = NULL;
    return 0;
}

void dms_sensor_init(void)
{
    if (dms_sen_init_sensor() != DRV_ERROR_NONE) {
        dms_err("dms_sensor_init.\n");
    }
    dms_mgnt_clockid_init();
    dms_sensor_notify_init();
    dms_sensor_task_init();
    dms_start_sensor_scan();
}

unsigned int dms_sensor_exit(void)
{
    dms_stop_sensor_scan();
    dms_sensor_task_exit();
    dms_sensor_notify_exit();
    return 0;
}
