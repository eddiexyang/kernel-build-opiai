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

#ifndef DEVDRV_PM_H
#define DEVDRV_PM_H

#include <linux/hrtimer.h>
#include <linux/list.h>
#include <linux/workqueue.h>

#include "tsdrv_status.h"
#include "devdrv_common.h"

#define DEVDRV_HEART_BEAT_DEATH 3 /* times */
#define DEVDRV_HEART_BEAT_INTERVAL_TIME_15 15 /* second */
#define DEVDRV_HEART_BEAT_INTERVAL_TIME_10 10 /* second */
#define DEVDRV_HEART_BEAT_INTERVAL_CNT 4 /* times */
#define DEVDRV_HEART_DEBUG_COUNTT 5 /* times */

#define DEVDRV_WAKELOCK_TIMEOUT_SECOND 10 /* second */

struct devdrv_aicore_info {
    u32 dev_id;
    u32 inited_flag;
    const void *exception_info;
    struct hrtimer hrtimer;
    ktime_t kt;
    struct work_struct work;
    struct workqueue_struct *aicore_info_wq;
};

struct devdrv_pm {
    int (*suspend)(u32 devid, u32 status);
    int (*resume)(u32 devid);
    int run_stage; /* 0-platform call, 1-sleep call */
    struct list_head list;
};

struct devdrv_pm *devdrv_manager_register_pm(int (*suspend)(u32 devid, u32 status), int (*resume)(u32 devid));
void devdrv_manager_unregister_pm(struct devdrv_pm *pm);
int devdrv_host_manager_device_suspend(struct devdrv_info *info);
int devdrv_host_manager_device_resume(struct devdrv_info *info);
void devdrv_host_manager_device_exception(struct devdrv_info *info);
int devdrv_refresh_aicore_info_init(u32 dev_id);
void devdrv_refresh_aicore_info_exit(u32 dev_id);

#endif
