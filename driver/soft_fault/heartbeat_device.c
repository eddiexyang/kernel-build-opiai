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

#include <linux/time.h>

#include "devdrv_common.h"
#include "dms_define.h"
#include "dms_timer.h"
#include "soft_fault_define.h"
#include "heart_beat.h"

#ifdef CFG_ENV_FPGA
#define HEART_BEAT_TIMER_EXPIRE_MS (5000 * 50)
#else
#define HEART_BEAT_TIMER_EXPIRE_MS 5000
#endif
static struct heart_beat_device g_os_heart_beat;

int heart_beat_event(u64 dev_id)
{
    int ret;
    static struct timeval last_time_record;
    struct timeval current_time;
    unsigned long interval;

    g_os_heart_beat.count[dev_id]++;

    ret = agentdrv_set_heartbeat_count(dev_id, g_os_heart_beat.count[dev_id]);
    if (ret) {
        soft_drv_err("Set heart beat count failed! (dev_id=%llu) \n", dev_id);
    }

    do_gettimeofday(&current_time);
    interval = current_time.tv_sec - last_time_record.tv_sec;
    if (last_time_record.tv_sec != 0 && interval > HEART_BEAT_INTERVAL_TIME_15) {
        soft_drv_warn("Unexpect interval of count. (last_time=%ld.%ld; curr_time=%ld.%ld; count=%llu)\n",
            last_time_record.tv_sec, last_time_record.tv_usec, current_time.tv_sec, current_time.tv_usec,
            g_os_heart_beat.count[dev_id]);
    }
    last_time_record = current_time;

    return DRV_ERROR_NONE;
}

int heart_beat_init(u32 dev_id)
{
    int ret;
    u32 timer_task_id = 0;
    struct dms_timer_task heart_beat_task = {0};

    g_os_heart_beat.count[dev_id] = 0;
    heart_beat_task.expire_ms = HEART_BEAT_TIMER_EXPIRE_MS;
    heart_beat_task.count_ms = 0;
    heart_beat_task.user_data = dev_id;
    heart_beat_task.handler_mode = TIMER_IRQ;
    heart_beat_task.exec_task = heart_beat_event;

    ret = dms_timer_task_register(&heart_beat_task, &timer_task_id);
    if (ret) {
        soft_drv_err("Dms timer task register failed. (dev_id=%u; ret=%d) \n", dev_id, ret);
        return ret;
    }

    g_os_heart_beat.timer_task_id[dev_id] = timer_task_id;
    g_os_heart_beat.status[dev_id] = HEART_BEAT_READY;

    soft_drv_info("Device heartbeat init success. (dev_id=%u)\n", dev_id);
    return DRV_ERROR_NONE;
}

int heart_beat_uninit(u32 dev_id)
{
    int ret;

    if (g_os_heart_beat.status[dev_id] == HEART_BEAT_READY) {
        g_os_heart_beat.status[dev_id] = HEART_BEAT_EXIT;
        ret = dms_timer_task_unregister(g_os_heart_beat.timer_task_id[dev_id]);
        if (ret) {
            soft_drv_err("Dms timer task unregister failed. (dev_id=%u; ret=%d) \n", dev_id, ret);
            return ret;
        }
        soft_drv_info("Device heartbeat exit success. (dev_id=%u)\n.", dev_id);
    }

    return DRV_ERROR_NONE;
}

void heartbeat_exit()
{
    int i;

    for (i = 0; i < DEVICE_NUM_MAX; i++) {
        (void)heart_beat_uninit(i);
    }

    return;
}
