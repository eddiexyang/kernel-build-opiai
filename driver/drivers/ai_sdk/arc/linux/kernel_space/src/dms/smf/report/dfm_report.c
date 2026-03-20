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

#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/kfifo.h>
#include <linux/delay.h>
#include <linux/time.h>

#include "dfm_dev_register.h"
#include "dms_define.h"

#ifdef AOS_LLVM_BUILD
#  define LOGLEVEL_INFO    6
#  define LOGLEVEL_DEBUG   7
#endif

/**
* used for generates fault IDs.
*/
#define FAULT_EVENT_FROM_DEVICE (0x2U)
#define FAULT_EVENT_CODE_TYPE   (0x0U)
#define FAULT_EVENT_SEVERITY    (0x0U)
#define FAULT_SOURCE_OFFSET     (30U)
#define FAULT_CODE_TYPE_OFFSET  (28U)
#define FAULT_SEVERITY_OFFSET   (25U)
#define FAULT_DEV_TYPE_OFFSET   (17U)
#define FAULT_SENSOR_OFFSET     (9U)

STATIC void dfm_print_event(const dfm_event *event, int level)
{
    if (level == LOGLEVEL_DEBUG) {
        dfm_debug("Receive or find an event. (subsys=0x%x, module_id=%u, section_type=%u, ras_code=0x%llx,"
            " sensor_type=0x%x, err_type=0x%x, desc=%*s)\n",
            event->subsys_id, event->module_id, event->section_type, event->ras_code.err_status,
            event->sensor_type, event->error_type, DMS_MAX_EVENT_DATA_LENGTH, event->describe);
    } else {
        dfm_event_log("Receive or find an event. (subsys=0x%x, module_id=%u, section_type=%u, ras_code=0x%llx,"
            " sensor_type=0x%x, err_type=0x%x, desc=%*s)\n",
            event->subsys_id, event->module_id, event->section_type, event->ras_code.err_status,
            event->sensor_type, event->error_type, DMS_MAX_EVENT_DATA_LENGTH, event->describe);
    }
}

u32 dfm_gen_event_id(u8 dev_node_type, u8 sensor_type, u8 error_type)
{
    u32 fault_id = (FAULT_EVENT_FROM_DEVICE << FAULT_SOURCE_OFFSET);
    fault_id |= (FAULT_EVENT_CODE_TYPE << FAULT_CODE_TYPE_OFFSET);
    fault_id |= (FAULT_EVENT_SEVERITY << FAULT_SEVERITY_OFFSET);
    fault_id |= (dev_node_type << FAULT_DEV_TYPE_OFFSET);
    fault_id |= (sensor_type << FAULT_SENSOR_OFFSET);
    fault_id |= (u32)error_type;
    return fault_id;
}
#ifndef DMS_SHARED_NO_EXPORT
EXPORT_SYMBOL(dfm_gen_event_id);
#endif

dfm_event_node *dfm_create_event_list_node(const dfm_event *event)
{
    dfm_event_node *event_new = NULL;
    int ret;

    event_new = kzalloc(sizeof(dfm_event_node), GFP_KERNEL | __GFP_ACCOUNT);
    if (event_new == NULL) {
        dfm_err("kzalloc failed.\n");
        return NULL;
    }

    ret = memcpy_s(&event_new->event, sizeof(dfm_event), event, sizeof(dfm_event));
    if (ret != 0) {
        kfree(event_new);
        event_new = NULL;
        dfm_err("memcpy_s failed. (ret=%d).\n", ret);
        return NULL;
    }

    return event_new;
}

/**
 *  check whether event has already exist in sensor's event list
 */
int dfm_event_exist(struct dfm_sensor *sensor, const dfm_event *event)
{
    dfm_event_node *event_node = NULL;
    struct list_head *pos = NULL, *n = NULL;

    if (!list_empty_careful(&sensor->event_list)) {
        list_for_each_safe(pos, n, &sensor->event_list) {
            event_node = list_entry(pos, dfm_event_node, node);
            if (event_node->event.error_type == event->error_type) {
                return true;
            }
        }
    }
    return false;
}

int dfm_add_sensor_event(struct dfm_sensor *sensor, const dfm_event *event)
{
    dfm_event_node *event_new = NULL;
    unsigned long irq_flags;

    event_new = dfm_create_event_list_node(event);
    if (event_new == NULL) {
        dfm_err("create event node failed\n");
        return -ENOMEM;
    }

    spin_lock_irqsave(&sensor->sensor_lock, irq_flags);
    if (dfm_event_exist(sensor, event)) {
        spin_unlock_irqrestore(&sensor->sensor_lock, irq_flags);
        dfm_warn("the event has already exists in the event list\n");
        kfree(event_new);
        return -EEXIST;
    }

    list_add(&event_new->node, &sensor->event_list); /* add new event to queue */
    sensor->error_num++;
    spin_unlock_irqrestore(&sensor->sensor_lock, irq_flags);

    dfm_print_event(event, LOGLEVEL_INFO);
    return 0;
}

int dfm_add_event(struct dfm_struct *dfm, u32 node_id, const dfm_event *event)
{
    struct dfm_node *node;
    struct dfm_sensor *sensor = NULL;

    if (dfm == NULL) {
        dfm_err("invalid dfm object\n");
        return -EINVAL;
    }
    if (event == NULL) {
        dfm_err("invalid event object\n");
        return -EINVAL;
    }

    if (node_id >= dfm->node_num) {
        dfm_err("invalid node id.(node_id=%u, max_node_num=%u)\n", node_id, dfm->node_num);
        return -EINVAL;
    }

    node = &dfm->dev_nodes[node_id];
    sensor = dfm_get_sensor(node, event->sensor_type);
    if (sensor == NULL) {
        dfm_err("can't find out sensor from sensor list.(dev=%.*s, sensor_type=%#x)\n",
            DMS_MAX_DEV_NAME_LEN, node->node->node_name, event->sensor_type);
        return -EFAULT;
    }

    return dfm_add_sensor_event(sensor, event);
}
#ifndef DMS_SHARED_NO_EXPORT
EXPORT_SYMBOL(dfm_add_event);
#endif

STATIC void dfm_remove_event_node(struct dfm_sensor *sensor, dfm_event_node *event_node)
{
    list_del(&event_node->node);
    kfree(event_node);
    sensor->error_num--;
}

int dfm_remove_event(struct dfm_struct *dfm, u32 node_id, const dfm_event *event)
{
    dfm_event_node *event_node = NULL;
    struct dfm_node *node = NULL;
    struct dfm_sensor *sensor = NULL;
    struct list_head *pos = NULL, *n = NULL;
    unsigned long irq_flags;
    bool event_exist = false;

    if (dfm == NULL || event == NULL) {
        dfm_err("invalid input parameters\n");
        return -EINVAL;
    }

    node = dfm_get_node(dfm, node_id);
    if (node == NULL) {
        dfm_err("the dfm node does not exist,"
            "maybe the node id is invalid.(node id=%u)\n", node_id);
        return -EINVAL;
    }
    sensor = dfm_get_sensor(node, event->sensor_type);
    if (sensor == NULL) {
        dfm_err("the dfm sensor does not exist. (sensor type=%u)\n", event->sensor_type);
        return -EINVAL;
    }

    /* remove the event from event list */
    spin_lock_irqsave(&sensor->sensor_lock, irq_flags);
    list_for_each_safe(pos, n, &sensor->event_list) {
        event_node = list_entry(pos, dfm_event_node, node);
        if (event_node->event.error_type == event->error_type) {
            dfm_remove_event_node(sensor, event_node);
            event_exist = true;
        }
    }
    spin_unlock_irqrestore(&sensor->sensor_lock, irq_flags);

    if (!event_exist) {
        return -EEXIST;
    }
    return 0;
}
#ifndef DMS_SHARED_NO_EXPORT
EXPORT_SYMBOL(dfm_remove_event);
#endif

void dfm_clear_event_list(struct dfm_sensor *sensor)
{
    dfm_event_node *event_node = NULL;
    struct list_head *pos = NULL, *n = NULL;
    unsigned long irq_flags;

    if (sensor == NULL) {
        dfm_err("invalid sensor object\n");
        return;
    }

    spin_lock_irqsave(&sensor->sensor_lock, irq_flags);
    /* get every event of the sensor event queue */
    if (list_empty_careful(&sensor->event_list)) {
        spin_unlock_irqrestore(&sensor->sensor_lock, irq_flags);
        return;
    }

    list_for_each_safe(pos, n, &sensor->event_list) {
        event_node = list_entry(pos, dfm_event_node, node);
        dfm_remove_event_node(sensor, event_node);
    }
    spin_unlock_irqrestore(&sensor->sensor_lock, irq_flags);
}
#ifndef DMS_SHARED_NO_EXPORT
EXPORT_SYMBOL(dfm_clear_event_list);
#endif

STATIC void dfm_clear_dev_events(struct dfm_struct *dfm)
{
    u32 i, j;

    for (i = 0; i < dfm->node_num; ++i) {
        struct dfm_node *node = &dfm->dev_nodes[i];
        for (j = 0; j < node->sensor_num; ++j) {
            dfm_clear_event_list(&node->sensors[j]);
        }
    }
}

void dfm_clear_all_dev_events(void)
{
    struct dfm_struct **dfms = NULL;
    u32 dfm_num;
    u32 i;

    dfms = dfm_get_struct_array(&dfm_num);
    if (dfms == NULL) {
        dfm_err("get struct array failed. only support in debug version\n");
        return;
    }

    for (i = 0; i < dfm_num; ++i) {
        dfm_clear_dev_events(dfms[i]);
    }
}
#ifndef DMS_SHARED_NO_EXPORT
EXPORT_SYMBOL(dfm_clear_all_dev_events);
#endif

bool dfm_is_onetime_event(const struct dfm_sensor *sensor, unsigned int status_bit)
{
    if (((sensor->dms_sensor.assert_event_mask & (1U << status_bit)) != 0) &&
        (sensor->dms_sensor.deassert_event_mask & (1U << status_bit)) == 0) {
        return true;
    }

    return false;
}

int dfm_add_event_to_scan_result(struct dfm_sensor *sensor,
    dfm_event_node *event_node, struct dms_sensor_event_data *events)
{
    struct dms_sensor_event_data_item *sensor_event = NULL;
    int ret;

    sensor_event = &(events->sensor_data[events->event_count]);

    sensor_event->current_value = event_node->event.error_type;
    sensor_event->data_size = DMS_MAX_EVENT_DATA_LENGTH;
    ret = memcpy_s(sensor_event->event_data, DMS_MAX_EVENT_DATA_LENGTH,
        event_node->event.describe, sizeof(event_node->event.describe));
    if (ret != 0) {
        dfm_warn("memcpy_s failed. (*ret=%d)\n", ret);
        return ret;
    }

    events->event_count++;

    return 0;
}

void dfm_scan_sensor_events(struct dfm_sensor *sensor, struct dms_sensor_event_data *events)
{
    dfm_event_node *event_node = NULL;
    struct list_head *pos = NULL, *n = NULL;
    unsigned long irq_flags;

    if ((sensor == NULL) || (events == NULL)) {
        dfm_err("invalid input parameters\n");
        return;
    }

    spin_lock_irqsave(&sensor->sensor_lock, irq_flags);
    events->event_count = 0;
    /* get every event of the sensor event queue */
    if (list_empty_careful(&sensor->event_list)) {
        spin_unlock_irqrestore(&sensor->sensor_lock, irq_flags);
        return;
    }

    list_for_each_safe(pos, n, &sensor->event_list) {
        event_node = list_entry(pos, dfm_event_node, node);
        dfm_print_event(&event_node->event, LOGLEVEL_DEBUG);

        if (dfm_add_event_to_scan_result(sensor, event_node, events) != 0) {
            dfm_err("add event to scan result failed\n");
            continue;
        }

        if (dfm_is_onetime_event(sensor, event_node->event.error_type)) {
            dfm_remove_event_node(sensor, event_node);
        }

        if (events->event_count == DMS_MAX_SENSOR_EVENT_COUNT) {
            dfm_warn("event queue full. (error_num=%u)\n", sensor->error_num);
            break;
        }
    }
    spin_unlock_irqrestore(&sensor->sensor_lock, irq_flags);
}
#ifndef DMS_SHARED_NO_EXPORT
EXPORT_SYMBOL(dfm_scan_sensor_events);
#endif

int dfm_scan_events(struct dfm_struct *dfm, u32 node_id, u8 sensor_type, struct dms_sensor_event_data *data)
{
    struct dfm_node *node;
    struct dfm_sensor *sensor = NULL;

    if ((dfm == NULL) || (data == NULL)) {
        return -EINVAL;
    }

    if (node_id >= dfm->node_num) {
        dfm_err("invalid node id.(node_id=%u, max_node_num=%u)\n", node_id, dfm->node_num);
        return -EINVAL;
    }

    node = &dfm->dev_nodes[node_id];
    sensor = dfm_get_sensor(node, sensor_type);
    if (sensor == NULL) {
        dfm_err("can't find out sensor from sensor list.(dev=%.*s, sensor_type=%#x)\n",
            DMS_MAX_DEV_NAME_LEN, node->node->node_name, sensor_type);
        return -EFAULT;
    }

    dfm_scan_sensor_events(sensor, data);
    return 0;
}
#ifndef DMS_SHARED_NO_EXPORT
EXPORT_SYMBOL(dfm_scan_events);
#endif
