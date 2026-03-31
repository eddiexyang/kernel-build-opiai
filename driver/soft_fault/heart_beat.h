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

#ifndef __HEART_BEAT_H
#define __HEART_BEAT_H

#include <linux/workqueue.h>

#include "drv_type.h"
#include "dms_define.h"
#include "dms_notifier.h"

#define HEART_BEAT_DEATH 3 /* times */
#define HEART_BEAT_INTERVAL_TIME_15 15 /* second */
#define HEART_BEAT_INTERVAL_TIME_10 10 /* second */
#define HEART_BEAT_INTERVAL_CNT 4 /* times */
#define HEART_DEBUG_COUNTT 5 /* times */
#define HEART_BEAT_EXIT   0
#define HEART_BEAT_READY   1

#define OS_HEART_BEAT_INTERVAL 5 /* 5 second */

/* host heat beat */
struct heart_beat {
volatile u32 dev_id;
u32 timer_task_id;    /* get from timer register and used for unregister from timer */
u32 heartbeat_switch;           /* heartbeat enable or disable */
u32 status;            /* used for work/exit flag */
volatile u32 stop;
volatile u32 lost_count;
volatile u64 old_count;
volatile u64 total_lost_count;
volatile u64 total_15s_count;
volatile u64 total_10s_count;
u64 old_time;
volatile int debug_count;
struct work_struct work; /* work for deal with some time consume tasks when heartbeat is lost */
struct workqueue_struct *heart_beat_wq;
};

/* device heat beat */
struct heart_beat_device {
    u32 timer_task_id[DEVICE_NUM_MAX];
    u64 count[DEVICE_NUM_MAX];
    u32 status[DEVICE_NUM_MAX];
};

int devdrv_get_heartbeat_count(u32 devid, u64* count);
int agentdrv_set_heartbeat_count(u32 devid, u64 count);
void enable_heat_beat(u32 dev_id);
void heart_beat_notify(unsigned long mode, u32 dev_id);
void heartbeat_exit(void);
int heartbeat_dev_register(u32 dev_id);
int heart_beat_init(u32 dev_id);
void heart_beat_stop(u32 dev_id);
int heart_beat_uninit(u32 dev_id);
void heart_beat_start(u32 dev_id);

#endif
