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

#include <linux/workqueue.h>
#include <linux/moduleparam.h>
#include "devdrv_manager_common.h"
#include "devdrv_pm.h"
#include "dms_define.h"
#include "dms_common.h"
#include "dms_timer.h"
#include "dms_sensor_type.h"
#include "soft_fault_define.h"
#include "heart_beat.h"
#include "soft_fault_define.h"
#include "davinci_api.h"
#include "davinci_interface.h"

#define HEART_BEAT_EN    1
#ifdef CFG_ENV_FPGA
#define HEART_BEAT_TIMER_EXPIRE_MS (6000 * 50)
#else
#define HEART_BEAT_TIMER_EXPIRE_MS 6000
#endif
static int g_heart_beat_switch = HEART_BEAT_EN;
module_param(g_heart_beat_switch, int, S_IRUGO);

void devdrv_hb_broken_stop_msg_send(u32 devid);

static struct heart_beat g_heart_beat[DEVICE_NUM_MAX] = {{0}};

void heart_beat_stop(u32 dev_id)
{
    g_heart_beat[dev_id].stop = 1;
    g_heart_beat[dev_id].lost_count = 0;
    g_heart_beat[dev_id].old_count = 0;
    g_heart_beat[dev_id].total_lost_count = 0;
    g_heart_beat[dev_id].total_15s_count = 0;
    g_heart_beat[dev_id].total_10s_count = 0;
    g_heart_beat[dev_id].debug_count = 0;
    (void)ascend_intf_report_device_status(dev_id, DAVINCI_INTF_DEVICE_STATUS_HEARTBIT_LOST);
}

void heart_beat_start(u32 dev_id)
{
    g_heart_beat[dev_id].dev_id = dev_id;
    g_heart_beat[dev_id].stop = 0;
    g_heart_beat[dev_id].lost_count = 0;
    g_heart_beat[dev_id].old_count = 0;
    g_heart_beat[dev_id].total_lost_count = 0;
    g_heart_beat[dev_id].total_15s_count = 0;
    g_heart_beat[dev_id].total_10s_count = 0;
    g_heart_beat[dev_id].debug_count = 0;
    g_heart_beat[dev_id].heartbeat_switch = g_heart_beat_switch;
    g_heart_beat[dev_id].old_time = ktime_get_raw_ns();

    (void)ascend_intf_report_device_status(dev_id,
        DAVINCI_INTF_DEVICE_CLEAR_STATUS | DAVINCI_INTF_DEVICE_STATUS_HEARTBIT_LOST);
}

STATIC struct heart_beat *get_heart_beat_info(u32 dev_id)
{
    return &g_heart_beat[dev_id];
}

STATIC void host_manager_device_exception(u32 dev_id)
{
    unsigned long flags;
    struct devdrv_manager_info *d_info = NULL;
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    struct devdrv_pm *pm = NULL;

    d_info = devdrv_get_manager_info();
    if (d_info == NULL) {
        soft_drv_err("d_info is NULL. (dev_id=%u) \n", dev_id);
        return;
    }

    spin_lock_irqsave(&d_info->pm_list_lock, flags);
    if (!list_empty_careful(&d_info->pm_list_header)) {
        list_for_each_safe(pos, n, &d_info->pm_list_header)
        {
            pm = list_entry(pos, struct devdrv_pm, list);
            if (pm->suspend != NULL) {
                (void)pm->suspend(dev_id, TS_DOWN);
            }
        }
    }
    spin_unlock_irqrestore(&d_info->pm_list_lock, flags);
}

STATIC void heart_beat_broken_work(struct work_struct *work)
{
    int ret;
    int tsid = 0;
    u32 dev_id;
    int data_len;
    struct soft_fault heartbeat_fault;
    struct heart_beat *heartbeat_info = NULL;
    char *fault_info = "device heartbeat lost";

    heartbeat_info = container_of(work, struct heart_beat, work);
    dev_id = heartbeat_info->dev_id;

    data_len = strlen(fault_info) + 1;
    ret = memcpy_s(heartbeat_fault.data, DMS_MAX_EVENT_DATA_LENGTH, fault_info, data_len);
    if (ret != 0) {
        soft_drv_err("Memcpy failed! (device=%u) \n", dev_id);
    }

    heartbeat_fault.data_len = data_len;
    heartbeat_fault.dev_id = dev_id;
    heartbeat_fault.sensor_type = DMS_SEN_TYPE_HEARTBEAT;
    heartbeat_fault.user_id = SF_SENSOR_DAVINCI;
    heartbeat_fault.node_id = SF_USER_NODE_DAVINCI;
    heartbeat_fault.sub_id = SF_SUB_ID0;
    heartbeat_fault.err_type = HEARTBEAT_ERROR_TYPE_HEARTBEAT_LOST;

    ret = soft_fault_event_handler(&heartbeat_fault);
    if (ret != 0) {
        soft_drv_err("Soft fault report failed! (device=%u) \n", dev_id);
    }

    devdrv_hb_broken_stop_msg_send(dev_id);
    tsdrv_set_ts_status(dev_id, tsid, TS_DOWN);
    host_manager_device_exception(dev_id);
}

STATIC void heartbeat_resume(u32 devid)
{
    int ret;
    int data_len;
    struct soft_fault heartbeat_fault;
    const char *fault_info = "device heartbeat resume";

    data_len = strlen(fault_info) + 1;
    ret = memcpy_s(heartbeat_fault.data, DMS_MAX_EVENT_DATA_LENGTH, fault_info, data_len);
    if (ret != 0) {
        soft_drv_err("Memcpy failed! (device=%u) \n", devid);
        return;
    }

    heartbeat_fault.data_len = data_len;
    heartbeat_fault.dev_id = devid;
    heartbeat_fault.sensor_type = DMS_SEN_TYPE_HEARTBEAT;
    heartbeat_fault.user_id = SF_SENSOR_DAVINCI;
    heartbeat_fault.node_id = SF_USER_NODE_DAVINCI;
    heartbeat_fault.sub_id = SF_SUB_ID0;
    heartbeat_fault.err_type = HEARTBEAT_ERROR_TYPE_HEARTBEAT_RECOVER;

    ret = soft_fault_event_handler(&heartbeat_fault);
    if (ret != 0) {
        soft_drv_warn("Report heartbeat recover failed! (device=%u) \n", devid);
    }
}


/* The aim was to record the interval_time between heartbeats, to judge the problem of stuck */
STATIC void print_debug_info(struct heart_beat *heartbeat_info, u64 new_time,
    int lost_count, u64 interval_count)
{
    time64_t interval_time;

    interval_time = (new_time - heartbeat_info->old_time) / NSEC_PER_SEC;
    if (unlikely(interval_time > HEART_BEAT_INTERVAL_TIME_15)) {
        heartbeat_info->debug_count = HEART_DEBUG_COUNTT;
        heartbeat_info->total_15s_count++;
        soft_drv_warn("1. Interval_time > 15s. (Dev_id=%u; interval_time=%lld) \n",
                      heartbeat_info->dev_id, interval_time);
    } else if (unlikely(interval_time > HEART_BEAT_INTERVAL_TIME_10)) {
        heartbeat_info->debug_count = HEART_DEBUG_COUNTT;
        heartbeat_info->total_10s_count++;
        soft_drv_warn("2. Interval_time > 10s. (Dev_id=%u; interval_time=%lld) \n",
                      heartbeat_info->dev_id, interval_time);
    }

    if (unlikely(lost_count > 0)) {
        soft_drv_warn("3. The heartbeat info: (device=%d; lost count=%d; "
            "total_lost_count=%llu; interval_time=%lld; total_10s_count=%llu; total_15s_count=%llu.\n",
            heartbeat_info->dev_id, lost_count, heartbeat_info->total_lost_count, interval_time,
            heartbeat_info->total_10s_count, heartbeat_info->total_15s_count);
    }

    if (unlikely(interval_count >= HEART_BEAT_INTERVAL_CNT)) {
        heartbeat_info->debug_count = HEART_DEBUG_COUNTT;
        soft_drv_warn("4. Interval_count > 4. (Dev_id=%u) \n", heartbeat_info->dev_id);
    }

    /* When the system is abnormal, continue to print the hearbeat_count for 5 times. */
    if (unlikely(heartbeat_info->debug_count > 0)) {
        heartbeat_info->debug_count--;
        soft_drv_warn(
            "5.  The heartbeat debug info:  (dev_id=%u; lost_count=%d; "
            "total_lost_count=%llu; interval_time=%lld; total_10s_count=%llu; total_15s_count=%llu).\n",
            heartbeat_info->dev_id, lost_count, heartbeat_info->total_lost_count, interval_time,
            heartbeat_info->total_10s_count, heartbeat_info->total_15s_count);
    }
}


STATIC int heart_beat_judge(u32 dev_id)
{
    int ret;
    u64 old_count;
    int lost_count;
    u64 new_count = 0;
    u64 interval_count;
    struct heart_beat *heartbeat_info = NULL;
    struct devdrv_manager_info *manager_info = NULL;
    u64 new_time;

    new_time = ktime_get_raw_ns();

    manager_info = devdrv_get_manager_info();
    heartbeat_info = get_heart_beat_info(dev_id);
    old_count = heartbeat_info->old_count;
    lost_count = heartbeat_info->lost_count;

    /* device enter sleep state, stop heart beat check */
    if (heartbeat_info->stop) {
        return DRV_ERROR_NONE;
    }

    ret = devdrv_get_heartbeat_count(dev_id, &new_count);
    if (ret) {
        soft_drv_err("Get the heartbeat_count failed! (device=%u) \n", heartbeat_info->dev_id);
        return ret;
    }

    /* heartbeat count check */
    interval_count = new_count - old_count;
    if (interval_count == 0) {
        lost_count++;
        heartbeat_info->total_lost_count++;
    } else {
        lost_count = 0;
    }

    print_debug_info(heartbeat_info, new_time, lost_count, interval_count);

    if (unlikely(lost_count >= HEART_BEAT_DEATH)) {
        soft_drv_err(
            "Heartbeat lost! (device=%u; old_count=%llu; new_count=%llu; lost_count=%d; "
            "total_lost_count=%llu; total_10s_count=%llu; total_15s_count %llu).\n",
            heartbeat_info->dev_id, old_count, new_count, lost_count, heartbeat_info->total_lost_count,
            heartbeat_info->total_10s_count, heartbeat_info->total_15s_count);
        manager_info->device_status[dev_id] = DRV_STATUS_COMMUNICATION_LOST;
        heart_beat_stop(heartbeat_info->dev_id);
        queue_work(heartbeat_info->heart_beat_wq, &heartbeat_info->work);
    } else {
        heartbeat_info->old_count = new_count;
        heartbeat_info->lost_count = lost_count;
        heartbeat_info->old_time = new_time;
    }

    return DRV_ERROR_NONE;
}

STATIC int heart_beat_event(u64 dev_id)
{
    int ret;

    /* check device heartbeat */
    ret = heart_beat_judge(dev_id);
    if (ret == -ENODEV) {
        soft_drv_err("The device does no exist! (device=%llu) \n", dev_id);
        return ret;
    }

    return DRV_ERROR_NONE;
}

int heart_beat_init(u32 dev_id)
{
    int ret;
    u32 timer_task_id = 0;
    struct heart_beat *heartbeat_info = NULL;
    struct dms_timer_task heart_beat_task = {0};

    if (dev_id >= DEVICE_NUM_MAX) {
        soft_drv_err("Invalid dev_id.(dev_id=%u) \n", dev_id);
        return -EINVAL;
    }

    if (g_heart_beat_switch != HEART_BEAT_EN) {
        soft_drv_warn("Heat beat is not enabled. (dev_id=%u) \n", dev_id);
        heart_beat_stop(dev_id);
        return DRV_ERROR_NONE;
    }

    heart_beat_start(dev_id);

    heartbeat_info = get_heart_beat_info(dev_id);
    if (heartbeat_info->status == HEART_BEAT_READY) { /* when device hot reset, start up again */
        soft_drv_info("Heartbeat already initialized, just start it. (dev_id=%u)\n", dev_id);
        heartbeat_resume(dev_id); /* clear heartbeat lost error after hot reset */
        return DRV_ERROR_NONE;
    }
    if (heartbeat_info->heart_beat_wq == NULL) {
        heartbeat_info->heart_beat_wq = create_singlethread_workqueue("heart_beat_work");
    }
    INIT_WORK(&heartbeat_info->work, heart_beat_broken_work);

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

    heartbeat_info->timer_task_id = timer_task_id;
    heartbeat_info->status = HEART_BEAT_READY;

    soft_drv_info("Heartbeat init success. (dev_id=%u)\n", dev_id);
    return DRV_ERROR_NONE;
}

int heart_beat_uninit(u32 dev_id)
{
    int ret;

    if (g_heart_beat[dev_id].status == HEART_BEAT_READY) {
        g_heart_beat[dev_id].status = HEART_BEAT_EXIT;
        ret = dms_timer_task_unregister(g_heart_beat[dev_id].timer_task_id);
        if (ret) {
            soft_drv_err("Dms timer task unregister failed. (dev_id=%u; ret=%d) \n", dev_id, ret);
            return ret;
        }
        if (g_heart_beat[dev_id].heart_beat_wq != NULL) {
            flush_workqueue(g_heart_beat[dev_id].heart_beat_wq);
            destroy_workqueue(g_heart_beat[dev_id].heart_beat_wq);
            g_heart_beat[dev_id].heart_beat_wq = NULL;
        }
        soft_drv_info("Heart beat event unregister from timer. (Dev_id=%u) \n", dev_id);
    }

    return DRV_ERROR_NONE;
}

STATIC int heartbeat_dev_node_config(unsigned int user_id, struct soft_dev *s_dev)
{
    int ret;
    int pid = current->tgid;
    u32 len = DMS_SENSOR_DESCRIPT_LENGTH;
    struct dms_node_operations *soft_ops = soft_get_ops();
    struct dms_node s_node = SOFT_NODE_DEF(DMS_DEV_TYPE_BASE_SERVCIE, "davinci", s_dev->dev_id,
        s_dev->node_id, soft_ops);
    struct dms_sensor_object_cfg s_cfg = SOFT_SENSOR_DEF(DMS_SEN_TYPE_HEARTBEAT, "dev0_heartbeat", 0UL,
        user_id, SF_SUB_ID0, AST_MASK, DST_MASK, SF_SENSOR_SCAN_TIME, soft_fault_event_scan, pid);

    ret = snprintf_s(s_cfg.sensor_name, len, len - 1, "dev%u_heartbeat", s_dev->dev_id);
    if (ret <= 0) {
        soft_drv_err("snprintf_s failed. (dev_id=%u; ret=%d)\n", s_dev->dev_id, ret);
        return ret;
    }
    s_cfg.private_data = ((unsigned long)s_dev->dev_id << SF_OFFSET_32BIT) | (user_id << SF_OFFSET_16BIT) |
        (s_dev->node_id << SF_OFFSET_8BIT) | SF_SUB_ID0;

    /* for kernel space, not used the pid, set default value -1 */
    s_cfg.pid = -1;
    s_node.pid = -1;
    s_dev->sensor_obj_num++;
    s_dev->sensor_obj_table[SF_SUB_ID0] = s_cfg;
    s_dev->dev_node = s_node;

    return 0;
}

int heartbeat_dev_register(u32 dev_id)
{
    int ret;
    unsigned int user_id = SF_SENSOR_DAVINCI;
    struct soft_dev_client *client = NULL;
    struct soft_dev *s_dev = NULL;
    struct drv_soft_ctrl *soft_ctrl = soft_get_ctrl();

    mutex_lock(&soft_ctrl->mutex[dev_id]);

    client = soft_ctrl->s_dev_t[dev_id][user_id];
    if (client->registered == 1) {
        soft_drv_warn("heartbeat is registered, return. (dev_id=%u; registered=%u)\n", dev_id, client->registered);
        mutex_unlock(&soft_ctrl->mutex[dev_id]);
        return 0;
    }

    s_dev =  kzalloc(sizeof(struct soft_dev), GFP_KERNEL | __GFP_ACCOUNT);
    if (s_dev == NULL) {
        soft_drv_err("kzalloc soft_dev failed.\n");
        mutex_unlock(&soft_ctrl->mutex[dev_id]);
        return -ENOMEM;
    }

    soft_one_dev_init(s_dev);
    s_dev->dev_id = dev_id;
    s_dev->node_id = SF_USER_NODE_DAVINCI;

    ret = heartbeat_dev_node_config(user_id, s_dev);
    if (ret != 0) {
        soft_drv_err("soft_fault register one node failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        goto ERROR;
    }

    ret = soft_register_one_node(s_dev);
    if (ret != 0) {
        soft_drv_err("soft_fault register one node failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        goto ERROR;
    }

    list_add(&s_dev->list, &client->head);
    client->user_id = user_id;
    client->registered = 1;
    client->node_num++;
    soft_ctrl->user_num[dev_id]++;
    mutex_unlock(&soft_ctrl->mutex[dev_id]);

    return 0;
ERROR:
    kfree(s_dev);
    s_dev = NULL;
    mutex_unlock(&soft_ctrl->mutex[dev_id]);
    return ret;
}

STATIC void heartbeat_dev_unregister(void)
{
    int i;
    struct soft_dev_client *client = NULL;
    struct drv_soft_ctrl *soft_ctrl = soft_get_ctrl();

    for (i = 0; i < DEV_ID_MAX; i++) {
        mutex_lock(&soft_ctrl->mutex[i]);
        client = soft_ctrl->s_dev_t[i][SF_SENSOR_DAVINCI];
        soft_free_one_node(client, DMS_DEV_TYPE_BASE_SERVCIE);
        soft_ctrl->user_num[i]--;
        mutex_unlock(&soft_ctrl->mutex[i]);
    }

    return;
}

void heartbeat_exit()
{
    int i;

    for (i = 0; i < DEVICE_NUM_MAX; i++) {
        (void)heart_beat_uninit(i);
    }

    heartbeat_dev_unregister();

    return;
}
