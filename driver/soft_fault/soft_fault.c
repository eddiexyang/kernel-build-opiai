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

#include <asm/io.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/workqueue.h>
#include <linux/idr.h>
#include <linux/list.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/smp.h>

#include "devdrv_interface.h"
#include "devdrv_common.h"
#include "dms_template.h"
#include "urd_feature.h"
#include "dms_cmd_def.h"
#include "urd_acc_ctrl.h"
#include "dms_notifier.h"
#include "dms_define.h"
#include "heart_beat.h"
#include "soft_fault_define.h"
#include "urd_notifier.h"
#include "urd_init.h"
#if (!defined CFG_HOST_ENV) && (defined CFG_FEATURE_OS_INIT_EVENT)
#include "os_reset.h"
#endif

#define EVENT_EXIST 1
#ifdef CFG_HOST_ENV
STATIC const struct pci_device_id soft_fault_driver_tbl[] = {
    { PCI_VDEVICE(HUAWEI, 0xd100), 0 },
    { PCI_VDEVICE(HUAWEI, 0xd105), 0 },
    { PCI_VDEVICE(HUAWEI, PCI_DEVICE_CLOUD), 0 },
    { PCI_VDEVICE(HUAWEI, 0xd801), 0 },
    { PCI_VDEVICE(HUAWEI, 0xd500), 0 },
    { PCI_VDEVICE(HUAWEI, 0xd501), 0 },
    { PCI_VDEVICE(HUAWEI, 0xd802), 0 },
    { PCI_VDEVICE(HUAWEI, 0xd803), 0 },
    { PCI_VDEVICE(HUAWEI, 0xd804), 0 },
    { PCI_VDEVICE(HUAWEI, 0xd805), 0 },
    { DEVDRV_DIVERSITY_PCIE_VENDOR_ID, 0xd500, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
    {}
};
MODULE_DEVICE_TABLE(pci, soft_fault_driver_tbl);
#endif

STATIC struct dms_node_operations g_soft_ops = {
    .init = soft_ops_init,
    .uninit = soft_ops_exit,
    .get_info_list = soft_get_dev_info_list,
    .get_state = soft_get_dev_state,
    .get_capacity = soft_get_dev_capacity,
    .set_power_state = soft_set_dev_power_state,
    .scan = NULL,
    .fault_diag = NULL,
    .event_notify = NULL,
    .get_link_state = NULL,
    .set_link_state = NULL
};

STATIC struct drv_soft_ctrl g_soft_ctrl;

STATIC struct soft_fault_recover g_fault_recover_table[] = {
    {DMS_SEN_TYPE_HEARTBEAT, HEARTBEAT_ERROR_TYPE_HEARTBEAT_LOST, HEARTBEAT_ERROR_TYPE_HEARTBEAT_RECOVER},
    {DMS_SEN_TYPE_GENERAL_SOFTWARE_FAULT, GENERAL_SOFTWARE_FAULT_MEMORY_OVER_LIMIT,
     GENERAL_SOFTWARE_FAULT_MEMORY_OVER_LIMIT_RECOVER},
    {DMS_SEN_TYPE_GENERAL_SOFTWARE_FAULT, GENERAL_SOFTWARE_FAULT_PROCESS_START_FAILED_OR_EXIT,
     GENERAL_SOFTWARE_FAULT_PROCESS_START_FAILED_OR_EXIT_RECOVER},
    {DMS_SEN_TYPE_GENERAL_SOFTWARE_FAULT, OS_INIT, OS_INIT_DONE}
};

STATIC unsigned int g_fault_event_record_table[] = {
    HEARTBEAT_ERROR_TYPE_HEARTBEAT_LOST,
    GENERAL_SOFTWARE_FAULT_MEMORY_OVER_LIMIT,
    GENERAL_SOFTWARE_FAULT_PROCESS_START_FAILED_OR_EXIT,
    HAL_GENERAL_SOFTWARE_FAULT_NORMAL_RESOURCE_RECYCLE_FAILED,
    HAL_GENERAL_SOFTWARE_FAULT_CRITICAL_RESOURCE_RECYCLE_FAILED
};

bool g_module_is_removing = false;

STATIC void soft_kfree(struct soft_error_list **ptr)
{
    if (*ptr != NULL) {
        kfree(*ptr);
        *ptr = NULL;
    }
}

struct dms_node_operations *soft_get_ops(void)
{
    return &g_soft_ops;
}

struct drv_soft_ctrl *soft_get_ctrl(void)
{
    return &g_soft_ctrl;
}

int soft_get_dev_capacity(struct dms_node *device, unsigned long long *capacity)
{
    if ((device == NULL) || (capacity == NULL)) {
        soft_drv_err("invalid para. (device=%d; capacity=%d)\n", (device == NULL), (capacity == NULL));
        return -EINVAL;
    }

    *capacity = device->capacity;

    return 0;
}

int soft_get_dev_info_list(struct dms_node *device, struct dms_dev_data_attr *info_list)
{
    return 0;
}

int soft_get_dev_state(struct dms_node *device, unsigned int *state)
{
    if ((device == NULL) || (state == NULL)) {
        soft_drv_err("invalid para, (device=%d; state=%d)\n", (device == NULL), (state == NULL));
        return -EINVAL;
    }

    *state = device->state;

    return 0;
}

int soft_set_dev_power_state(struct dms_node *device, DSMI_POWER_STATE power_state)
{
    return 0;
}

int soft_ops_init(struct dms_node *device)
{
    return 0;
}

void soft_ops_exit(struct dms_node *device)
{
    /* in module exit process, no need to release repeatly, otherwise, dead lock will caused */
    if (g_module_is_removing) {
        return;
    }

    /* if the original process dead, the release here is needed */
    if ((device->pid != current->tgid) && (DMS_EVENT_OBJ_TYPE(device->node_type) == DMS_EVENT_OBJ_USER)) {
        soft_client_release(device->pid);
    }

    return;
}

STATIC int soft_add_fault_event(struct soft_error_list *error_new, struct soft_event *event_queue)
{
    struct soft_error_list *pos = NULL;
    struct soft_error_list *n = NULL;

    if ((event_queue == NULL) || (error_new == NULL)) {
        soft_drv_err("invalid para. event_queue or error_new is NULL\n");
        return -EINVAL;
    }

    mutex_lock(&event_queue->mutex);
    list_for_each_entry_safe(pos, n, &event_queue->error_list.list, list) {
        if ((pos->error.sensor_type == error_new->error.sensor_type) &&
            (pos->error.err_type == error_new->error.err_type)) {
            mutex_unlock(&event_queue->mutex);
            return EVENT_EXIST; /* It means found fault event already in event list, don't need add */
        }
    }
    list_add(&error_new->list, &event_queue->error_list.list); /* add new event to queue */
    event_queue->error_num++;
    mutex_unlock(&event_queue->mutex);

    return 0;
}

STATIC int soft_fault_recover(struct soft_event *event_queue, struct soft_fault *event_new)
{
    int i, len;
    struct soft_error_list *pos = NULL;
    struct soft_error_list *n = NULL;
    struct soft_fault *event = NULL;

    len = sizeof(g_fault_recover_table) / sizeof(struct soft_fault_recover);

    for (i = 0; i < len; i++) {
        if ((event_new->sensor_type == g_fault_recover_table[i].sensor_type) &&
            (event_new->err_type == g_fault_recover_table[i].recover_val)) {
            msleep(SF_SENSOR_SCAN_TIME * 2); /* sleep 2 scan time to wait error report */
            mutex_lock(&event_queue->mutex);
            list_for_each_entry_safe(pos, n, &event_queue->error_list.list, list) {
                event = &pos->error;
                if (g_fault_recover_table[i].occur_val == event->err_type) {
                    list_del(&pos->list);
                    kfree(pos);
                    pos = NULL;
                    event_queue->error_num--;
                }
            }
            mutex_unlock(&event_queue->mutex);
            return 1;
        }
    }

    return 0;
}

STATIC int soft_clear_fault_event_check(struct soft_fault *event)
{
    int i, len;
    unsigned int *p_table = NULL;

    if (event == NULL) {
        soft_drv_err("Invalid para, event is NULL\n");
        return -EINVAL;
    }

    p_table = g_fault_event_record_table;
    len = sizeof(g_fault_event_record_table) / sizeof(unsigned int);

    for (i = 0; i < len; i++) {
        if (event->err_type == p_table[i]) {
            return 1; /* 1 means found fault event type in record table, don't need clear */
        }
    }

    return 0;
}

STATIC struct soft_event *soft_event_queue_get(struct soft_dev_client *client, unsigned int node_id, unsigned int idx)
{
    struct soft_dev *pos = NULL;
    struct soft_dev *n = NULL;
    struct soft_event *event_queue = NULL;

    list_for_each_entry_safe(pos, n, &client->head, list) {
        if (pos->node_id == node_id) {
            event_queue = &pos->sensor_event_queue[idx];
            break;
        }
    }

    return event_queue;
}

int soft_fault_event_scan(unsigned long long private_data, struct dms_sensor_event_data *data)
{
    int ret;
    unsigned int dev_id, user_id, node_id, idx;
    struct soft_dev_client *client = NULL;
    struct soft_error_list *pos = NULL;
    struct soft_error_list *n = NULL;
    struct soft_event *event_queue = NULL;
    struct soft_fault *event = NULL;
    struct drv_soft_ctrl *soft_ctrl = soft_get_ctrl();

    dev_id = (private_data >> SF_OFFSET_32BIT) & SF_MASK_32BIT;
    user_id = (private_data & SF_MASK_32BIT) >> SF_OFFSET_16BIT;
    node_id = (private_data & SF_MASK_16BIT) >> SF_OFFSET_8BIT;
    idx = private_data & SF_MASK_8BIT;

    if ((data == NULL) || (dev_id >= DEV_ID_MAX) || (user_id >= SF_USER_MAX) ||
        (node_id >= SF_USER_NODE_MAX) || (idx >= SF_SUB_ID_MAX)) {
        soft_drv_err("invalid para. (dev_id=%u; s_id=%u; node_id=%u; idx=%u)\n", dev_id, user_id, node_id, idx);
        return -EINVAL;
    }

    client = soft_ctrl->s_dev_t[dev_id][user_id];
    event_queue = soft_event_queue_get(client, node_id, idx);
    if (event_queue == NULL) {
        soft_drv_err("get event_queue failed. (dev_id=%u; node_id=%u; idx=%u)\n", dev_id, node_id, idx);
        return -EINVAL;
    }

    data->event_count = 0;
    mutex_lock(&event_queue->mutex);
    /* get every event of the sensor event queue */
    list_for_each_entry_safe(pos, n, &event_queue->error_list.list, list) {
        event = &pos->error;
        data->sensor_data[data->event_count].current_value = event->err_type;
        data->sensor_data[data->event_count].data_size = event->data_len;
        ret = memcpy_s(data->sensor_data[data->event_count].event_data, DMS_MAX_EVENT_DATA_LENGTH,
            event->data, event->data_len);
        if (ret != 0) {
            soft_drv_warn("memcpy_s failed, continue. (ret=%d)\n", ret);
            continue;
        }

        if (soft_clear_fault_event_check(event) == 0) {
            list_del(&pos->list);
            kfree(pos);
            pos = NULL;
            event_queue->error_num--;
        }
        data->event_count++;
        if (data->event_count == DMS_MAX_SENSOR_EVENT_COUNT) {
            break;
        }
    }
    mutex_unlock(&event_queue->mutex);

    return 0;
}

int soft_fault_event_handler(struct soft_fault *event)
{
    int ret;
    struct soft_dev_client *client = NULL;
    struct soft_event *event_queue = NULL;
    struct soft_error_list *error_new = NULL;
    struct drv_soft_ctrl *soft_ctrl = soft_get_ctrl();

    if (event == NULL) {
        soft_drv_err("invalid para, event is NULL.\n");
        return -EINVAL;
    }

    if ((event->dev_id >= DEV_ID_MAX) || (event->user_id >= SF_USER_MAX) ||
        (event->node_id >= SF_USER_NODE_MAX) || (event->sub_id >= SF_SUB_ID_MAX)) {
        soft_drv_err("invalid para. (dev_id=%u; user_id=%u; node_id=%u; idx=%u)\n",
            event->dev_id, event->user_id, event->node_id, event->sub_id);
        return -EINVAL;
    }

    client = soft_ctrl->s_dev_t[event->dev_id][event->user_id];
    event_queue = soft_event_queue_get(client, event->node_id, event->sub_id);
    if (event_queue == NULL) {
        soft_drv_err("get event_queue failed. (dev_id=%u; node_id=%u; idx=%u)\n", event->dev_id,
            event->node_id, event->sub_id);
        return -EINVAL;
    }

    ret = soft_fault_recover(event_queue, event);
    if (ret == 1) {
        soft_drv_event("fault recover. (dev_id=%u; user_id=%u; sensor_type=0x%x; err_type=0x%x)\n",
            event->dev_id, event->user_id, event->sensor_type, event->err_type);
        return 0;
    }

    error_new = kzalloc(sizeof(struct soft_error_list), GFP_KERNEL | __GFP_ACCOUNT);
    if (error_new == NULL) {
        soft_drv_err("new_event kzalloc failed.\n");
        return -ENOMEM;
    }

    ret = memcpy_s(&error_new->error, sizeof(struct soft_fault), event, sizeof(struct soft_fault));
    if (ret != 0) {
        soft_drv_err("new_event memcpy_s failed. (ret=%d)\n", ret);
        goto out;
    }

    ret = soft_add_fault_event(error_new, event_queue);
    if (ret != 0) {
        soft_kfree(&error_new);
        return ret;
    }

    return ret;
out:
    soft_kfree(&error_new);
    soft_drv_err("fault event handler error. (dev_id=%u; user_id=%u; err_type=0x%x)\n",
        event->dev_id, event->user_id, event->err_type);
    return ret;
}

void soft_fault_event_free(struct soft_event *event_queue)
{
    struct soft_error_list *pos = NULL;
    struct soft_error_list *n = NULL;

    if (event_queue == NULL) {
        soft_drv_err("Invalid para, event_queue is NULL\n");
        return;
    }

    mutex_lock(&event_queue->mutex);
    list_for_each_entry_safe(pos, n, &event_queue->error_list.list, list) {
        list_del(&pos->list);
        kfree(pos);
        pos = NULL;
        event_queue->error_num--;
    }
    mutex_unlock(&event_queue->mutex);
}

int soft_register_one_node(struct soft_dev *s_dev)
{
    int ret;
    unsigned int i;

    if (s_dev == NULL) {
        soft_drv_err("Invalid para, h_dev is NULL.\n");
        return -EINVAL;
    }

    if (s_dev->registered == 1) {
        soft_drv_warn("soft_dev is registered. (dev_id=%u; registered=%u)\n", s_dev->dev_id, s_dev->registered);
        return 0;
    }

    ret = dms_register_dev_node(&s_dev->dev_node);
    if (ret != 0) {
        soft_drv_err("soft_fault register dev_node failed. (ret=%d)\n", ret);
        return ret;
    }

    for (i = 0; i < s_dev->sensor_obj_num; i++) {
        ret = dms_sensor_register(&s_dev->dev_node, &s_dev->sensor_obj_table[i]);
        if (ret != 0) {
            soft_drv_err("soft_fault register sensor failed. (s_idx=%u; ret=%d)\n", i, ret);
            goto out;
        }
        s_dev->sensor_obj_registered[i] = 1;
    }

    s_dev->registered = 1;
    return 0;
out:
    if (i > 0) {
        (void)dms_sensor_node_unregister(&s_dev->dev_node);
    }
    (void)dms_unregister_dev_node(&s_dev->dev_node);

    return ret;
}

void soft_unregister_one_node(struct soft_dev *s_dev)
{
    int ret;

    if (s_dev == NULL) {
        soft_drv_err("Invalid para, h_dev is NULL.\n");
        return;
    }

    if (s_dev->registered == 0) {
        return;
    }

    ret = dms_sensor_node_unregister(&s_dev->dev_node);
    if (ret != 0) {
        soft_drv_warn("soft_fault unregister sensor node failed. (ret=%d)\n", ret);
    }

    ret = dms_unregister_dev_node(&s_dev->dev_node);
    if (ret != 0) {
        soft_drv_warn("soft_fault unregister dev node failed. (ret=%d)\n", ret);
    }

    s_dev->registered = 0;
    return;
}

void soft_free_one_node(struct soft_dev_client *client, unsigned int node_type)
{
    unsigned int i;
    struct soft_dev *pos = NULL;
    struct soft_dev *n = NULL;

    if (client == NULL) {
        soft_drv_warn("invalid para, client is NULL.\n");
        return;
    }

    mutex_lock(&client->mutex);
    list_for_each_entry_safe(pos, n, &client->head, list) {
        if ((pos->dev_node.node_type == node_type) && (pos->registered == 1)) {
            soft_unregister_one_node(pos);
            for (i = 0; i < SF_SUB_ID_MAX; i++) {
                if (pos->sensor_obj_registered[i] == 1) {
                    soft_fault_event_free(&pos->sensor_event_queue[i]);
                }
            }
            soft_one_dev_exit(pos);
            list_del(&pos->list);
            kfree(pos);
            pos = NULL;
            client->node_num--;
            break;
        }
    }

    if (client->node_num == 0) {
        client->pid = -1;
        client->user_id = 0;
        client->registered = 0;
    }

    mutex_unlock(&client->mutex);
    return;
}

int soft_device_up(u32 udevid)
{
    int ret = 0;
#ifdef CFG_FEATURE_HEART_BEAT
#ifdef CFG_HOST_ENV /* host side */
    ret = heartbeat_dev_register(udevid);
    if (ret != 0) {
        soft_drv_err("heartbeat dev register failed. (dev_id=%u; ret=%d) \n", udevid, ret);
        return ret;
    }
#endif

    ret = heart_beat_init(udevid);
    if (ret != 0) {
        soft_drv_err("Heartbeat init failed. (device id=%u)\n", udevid);
    }
#endif

    return ret;
}

int soft_device_down(u32 udevid)
{
#ifdef CFG_FEATURE_HEART_BEAT
#ifdef CFG_HOST_ENV /* host side */
    (void)heart_beat_stop(udevid);
#endif

#ifndef CFG_HOST_ENV /* device side */
    (void)heart_beat_uninit(udevid);
#endif
#endif
    return 0;
}

int soft_device_suspend(void *data)
{
#if (defined CFG_FEATURE_HEART_BEAT) && (defined CFG_HOST_ENV) /* host side */
    struct devdrv_info *dev = NULL;
    dev = (struct devdrv_info *)data;
    (void)heart_beat_stop(dev->dev_id);
#endif

    return 0;
}

int soft_device_resume(void *data)
{
#if (defined CFG_FEATURE_HEART_BEAT) && (defined CFG_HOST_ENV) /* host side */
    struct devdrv_info *dev = NULL;
    dev = (struct devdrv_info *)data;
    (void)heart_beat_start(dev->dev_id);
#endif

    return 0;
}


STATIC int soft_h2d_event(void *data)
{
#if (!defined CFG_HOST_ENV) && (defined CFG_FEATURE_OS_INIT_EVENT) /* device side */
    int ret;
    u32 dev_id;
    dev_id = *(u32 *)data;
    ret = os_device_notifier_func(dev_id);
    if (ret != 0) {
        soft_drv_err("Soc dev notify failed. (dev_id=%u; ret=%d) \n", dev_id, ret);
        return ret;
    }
#endif

    return 0;
}

STATIC int soft_notifier(struct notifier_block *nb, unsigned long mode, void *data)
{
    int ret = 0;

    if (data == NULL) {
        soft_drv_err("Data is null, invalid parameter. \n");
        return -EINVAL;
    }
    switch (mode) {
        case DMS_DEVICE_RESUME:
            ret = soft_device_resume(data);
            break;
        case DMS_DEVICE_SUSPEND:
            ret = soft_device_suspend(data);
            break;
        case DMS_H2D_EVENT:
            ret = soft_h2d_event(data);
            break;
        default:
            break;
    }

    return 0;
}

STATIC struct notifier_block g_soft_notifier = {
    .notifier_call = soft_notifier,
};

STATIC int soft_fault_release(void *data)
{
    struct urd_file_private_stru *urd_priv = NULL;
    pid_t owner_pid;

    urd_priv = (struct urd_file_private_stru *)data;
    if (urd_priv == NULL) {
        soft_drv_err("invalid notifier data.\n");
        return -EINVAL;
    }

    owner_pid = urd_priv->owner_pid;
    dms_sensor_release(owner_pid);
    dev_node_release(owner_pid);
    if (dms_event_is_converge()) {
        dms_event_cb_release(owner_pid);
    }

    return 0;
}

STATIC int soft_urd_notifier(struct notifier_block *nb, unsigned long mode, void *data)
{
    int ret = -EINVAL;

    switch (mode) {
        case URD_NOTIFIER_RELEASE:
            ret = soft_fault_release(data);
            break;
        default:
            soft_drv_err("invalid notifier mode.(mode=0x%lx)\n", mode);
            break;
    }

    return ret;
}

STATIC struct notifier_block g_soft_urd_notifier = {
    .notifier_call = soft_urd_notifier,
};

void soft_one_dev_init(struct soft_dev *s_dev)
{
    int i;

    if (s_dev == NULL) {
        return;
    }

    s_dev->registered = 0;
    s_dev->sensor_obj_num = 0;
    mutex_init(&s_dev->mutex);

    for (i = 0; i < SF_SUB_ID_MAX; i++) {
        mutex_init(&s_dev->sensor_event_queue[i].mutex);
        INIT_LIST_HEAD(&s_dev->sensor_event_queue[i].error_list.list);
    }

    return;
}

void soft_one_dev_exit(struct soft_dev *s_dev)
{
    int i;

    if (s_dev == NULL) {
        return;
    }

    for (i = 0; i < SF_SUB_ID_MAX; i++) {
        mutex_destroy(&s_dev->sensor_event_queue[i].mutex);
    }

    mutex_destroy(&s_dev->mutex);
    s_dev->registered = 0;
    s_dev->sensor_obj_num = 0;

    return;
}

STATIC int soft_ctrl_init(void)
{
    int i, j;
    struct drv_soft_ctrl *soft_ctrl = soft_get_ctrl();

    for (i = 0; i < DEV_ID_MAX; i++) {
        soft_ctrl->user_num[i] = 1;
        mutex_init(&soft_ctrl->mutex[i]);
        for (j = 0; j < SF_USER_MAX; j++) {
            soft_ctrl->s_dev_t[i][j] = kzalloc(sizeof(struct soft_dev_client), GFP_KERNEL);
            if (soft_ctrl->s_dev_t[i][j] == NULL) {
                soft_drv_err("Kzalloc s_dev_t failed. \n");
                return -ENOMEM;
            }

            soft_ctrl->s_dev_t[i][j]->pid = -1;
            soft_ctrl->s_dev_t[i][j]->registered = 0;
            soft_ctrl->s_dev_t[i][j]->node_num = 0;
            mutex_init(&soft_ctrl->s_dev_t[i][j]->mutex);
            INIT_LIST_HEAD(&soft_ctrl->s_dev_t[i][j]->head);
        }
    }

    return 0;
}

STATIC void soft_ctrl_exit(void)
{
    int i, j;
    struct drv_soft_ctrl *soft_ctrl = soft_get_ctrl();

    for (i = 0; i < DEV_ID_MAX; i++) {
        for (j = 0; j < SF_USER_MAX; j++) {
            if (soft_ctrl->s_dev_t[i][j] != NULL) {
                mutex_destroy(&soft_ctrl->s_dev_t[i][j]->mutex);
                soft_ctrl->s_dev_t[i][j]->node_num = 0;
                soft_ctrl->s_dev_t[i][j]->registered = 0;
                soft_ctrl->s_dev_t[i][j]->pid = -1;
                kfree(soft_ctrl->s_dev_t[i][j]);
                soft_ctrl->s_dev_t[i][j] = NULL;
            }
        }
        mutex_destroy(&soft_ctrl->mutex[i]);
        soft_ctrl->user_num[i] = 0;
    }

    return;
}

STATIC int soft_uda_notifier_func(u32 udevid, enum uda_notified_action action)
{
    int ret = 0;

    if (udevid >= (u32)DEV_ID_MAX) {
        soft_drv_warn("Device id invalid. (udev_id=%u).\n", udevid);
        return -EINVAL;
    }

    if (action == UDA_INIT) {
        ret = soft_device_up(udevid);
    } else if (action == UDA_UNINIT) {
        ret = soft_device_down(udevid);
    }

    soft_drv_info("notifier action. (udevid=%u; action=%d; ret=%d)\n", udevid, action, ret);
    return ret;
}

STATIC int __init soft_init(void)
{
    int ret;
    struct uda_dev_type type = {0};

    CALL_INIT_MODULE(soft_fault);
    ret = soft_ctrl_init();
    if (ret != 0) {
        soft_drv_err("Soft ctrl init failed. (ret=%d)\n", ret);
        goto SOFT_CTRL_FAIL;
    }

    /* Register callback to the URD. When a process exits, the DMS node,
     * sensor, and aggregated fault data related to the process need
     * to be released. */
    ret = urd_register_notifier(&g_soft_urd_notifier);
    if (ret != 0) {
        soft_drv_err("register urd notifier failed. (ret=%d)\n", ret);
        goto SOFT_CTRL_FAIL;
    }

    ret = dms_register_notifier(&g_soft_notifier);
    if (ret != 0) {
        soft_drv_err("register dms notifier failed. (ret=%d)\n", ret);
        goto DMS_REGISTERF_FAIL;
    }

#ifdef CFG_HOST_ENV
    uda_davinci_near_real_entity_type_pack(&type);
    ret = uda_notifier_register(SOFT_FAULT_NOTIFIER, &type, UDA_PRI1, soft_uda_notifier_func);
    if (ret) {
        soft_drv_err("Host register near real entity type failed. (ret=%d)\n", ret);
        goto UDA_REGISTER_FAIL;
    }
#else
    uda_davinci_local_real_entity_type_pack(&type);
    ret = uda_real_virtual_notifier_register(SOFT_FAULT_NOTIFIER, &type, UDA_PRI1, soft_uda_notifier_func);
    if (ret) {
        soft_drv_err("Device register local real virtual type failed. (ret=%d)\n", ret);
        goto UDA_REGISTER_FAIL;
    }
#endif


    soft_drv_info("soft event driver init success.\n");
    return 0;

UDA_REGISTER_FAIL:
    (void)dms_unregister_notifier(&g_soft_notifier);
DMS_REGISTERF_FAIL:
    (void)urd_unregister_notifier(&g_soft_urd_notifier);
SOFT_CTRL_FAIL:
    soft_ctrl_exit();
    CALL_EXIT_MODULE(soft_fault);
    return ret;
}
module_init(soft_init);

STATIC void __exit soft_exit(void)
{
    struct uda_dev_type type = {0};

#ifdef CFG_HOST_ENV
    uda_davinci_near_real_entity_type_pack(&type);
    (void)uda_notifier_unregister(SOFT_FAULT_NOTIFIER, &type);
#else
    uda_davinci_local_real_entity_type_pack(&type);
    (void)uda_real_virtual_notifier_unregister(SOFT_FAULT_NOTIFIER, &type);
#endif

    (void)dms_unregister_notifier(&g_soft_notifier);
#ifdef CFG_FEATURE_HEART_BEAT
    heartbeat_exit();
#endif
#if (!defined CFG_HOST_ENV) && (defined CFG_FEATURE_OS_INIT_EVENT) /* device side */
    os_dev_unregister();
#endif
    (void)urd_unregister_notifier(&g_soft_urd_notifier);
    g_module_is_removing = true;
    soft_dev_exit();
    g_module_is_removing = false;
    soft_ctrl_exit();
    CALL_EXIT_MODULE(soft_fault);
    soft_drv_info("soft event driver exit success.\n");
}
module_exit(soft_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("DAVINCI driver");
