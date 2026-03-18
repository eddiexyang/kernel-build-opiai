/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
* Create: 2023-05-09
*/
#ifdef CFG_FEATURE_FAULT
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/securec.h>

#include "dms_node_type.h"
#include "tsdrv_log.h"
#include "tsdrv_fault_comm.h"

u32 get_devnode_id(DMS_DEVICE_NODE_TYPE node_type)
{
    u32 node_id;

    switch (node_type) {
        case DMS_DEV_TYPE_TS:
            node_id = DEV_NODE_TS;
            break;
        case DMS_DEV_TYPE_HWTS_S_TS:
            node_id = DEV_NODE_HWTS_STARS;
            break;
        case DMS_DEV_TYPE_AIC:
            node_id = DEV_NODE_AIC;
            break;
        case DMS_DEV_TYPE_AIV:
            node_id = DEV_NODE_AIV;
            break;
        case DMS_DEV_TYPE_SDMA:
            node_id = DEV_NODE_SDMAM;
            break;
        case DMS_DEV_TYPE_DSA:
            node_id = DEV_NODE_DSA;
            break;
        case DMS_DEV_TYPE_TSFW:
            node_id = DEV_NODE_TSFW;
            break;
        default:
            node_id = DEV_NODE_ID_MAX;
            break;
    }

    return node_id;
}

u32 get_sensor_node_id(DMS_DEVICE_NODE_TYPE node_type)
{
    u32 sensor_node_id;

    switch (node_type) {
        case DMS_DEV_TYPE_TS:
            sensor_node_id = SENSOR_NODE_TS;
            break;
        case DMS_DEV_TYPE_HWTS_S_TS:
            sensor_node_id = SENSOR_NODE_HWTS_STARS;
            break;
        case DMS_DEV_TYPE_AIC:
            sensor_node_id = SENSOR_NODE_AIC;
            break;
        case DMS_DEV_TYPE_AIV:
            sensor_node_id = SENSOR_NODE_AIV;
            break;
        case DMS_DEV_TYPE_SDMA:
            sensor_node_id = SENSOR_NODE_SDMAM;
            break;
        case DMS_DEV_TYPE_DSA:
            sensor_node_id = SENSOR_NODE_DSA;
            break;
        case DMS_DEV_TYPE_TSFW:
            sensor_node_id = SENSOR_NODE_TSFW;
            break;
        default:
            sensor_node_id = SENSOR_NODE_ID_MAX;
            break;
    }

    return sensor_node_id;
}

struct fault_event *tsdrv_get_fault_event(u32 devid, u32 tsid, u32 node_id, u32 sensor_node)
{
    struct fault_dev *dev = tsdrv_get_fault_dev(devid);
    struct fault_dev_node *dev_node = &dev->dev_node[node_id];
    struct fault_event *sensor_event = NULL;
    u32 i;

    for (i = 0; i < dev_node->sensor_num; i++) {
        sensor_event = &dev_node->sensor_event[i];
        if (sensor_event->sensor_node != sensor_node ||
            sensor_event->tsid != tsid || sensor_event->status != 1) {
            continue;
        }

        TSDRV_PRINT_DEBUG("Found sensor event. (devid=%u; tsid=%u; node_id=%u; node_type=0x%x; sensor_node=%u)\n",
            devid, tsid, node_id, dev_node->node->node_type, sensor_node);
        return sensor_event;
    }

    return NULL;
}

static void tsdrv_set_fault_event_resume(struct fault_event *event, struct ras_error *error)
{
    struct fault_list_node *pos = NULL;
    struct fault_list_node *n = NULL;
    unsigned long flags;

    spin_lock_irqsave(&event->spinlock, flags);
    list_for_each_entry_safe(pos, n, &event->node.list, list) {
        if ((pos->error.sensor_status == error->sensor_status) && (pos->error.event_attr == EVENT_OCCUR_ATTR)) {
            pos->error.event_attr = EVENT_RESUME_ATTR;
            spin_unlock_irqrestore(&event->spinlock, flags);
            TSDRV_PRINT_EVENT("%s %s resume.\n", fault_node_type_to_name((int)error->node_type),
                fault_type_to_description(error->sensor_status));
            return;
        }
    }
    spin_unlock_irqrestore(&event->spinlock, flags);
}

static void tsdrv_print_fault_describe(struct ras_error *error)
{
    TSDRV_PRINT_ERR("%s %s.\n", fault_node_type_to_name((int)error->node_type),
        fault_type_to_description(error->sensor_status));
}

static int tsdrv_add_fault_event(struct fault_event *event, struct ras_error *error)
{
    struct fault_list_node *pos = NULL;
    struct fault_list_node *n = NULL;
    struct fault_list_node *fault_new = NULL;
    unsigned long flags;
    int ret;

    fault_new = kzalloc(sizeof(struct fault_list_node), GFP_KERNEL | __GFP_ACCOUNT);
    if (fault_new == NULL) {
        TSDRV_PRINT_ERR("Failed to kzalloc new fault list.\n");
        return -ENOMEM;
    }

    ret = memcpy_s(&(fault_new->error), sizeof(struct ras_error), error, sizeof(struct ras_error));
    if (ret != EOK) {
        kfree(fault_new);
        TSDRV_PRINT_ERR("Memcpy fault_new->error failed. (ret=%d)\n", ret);
        return ret;
    }

    spin_lock_irqsave(&event->spinlock, flags);
    list_for_each_entry_safe(pos, n, &event->node.list, list) {
        if (pos->error.sensor_status == fault_new->error.sensor_status) {
            spin_unlock_irqrestore(&event->spinlock, flags);
            kfree(fault_new);
            TSDRV_PRINT_DEBUG("Fault event had been added, not repeat add.\n");
            return -EEXIST;
        }
    }

    list_add(&fault_new->list, &event->node.list);
    event->error_num++;
    spin_unlock_irqrestore(&event->spinlock, flags);
    tsdrv_print_fault_describe(error);
    TSDRV_PRINT_DEBUG("Fault event add success. (node_type=0x%x; section_type=%u; sensor_status=%u; error_num=%u)\n",
        error->node_type, error->section_type, error->sensor_status, event->error_num);

    return 0;
}

static struct dms_sensor_object_cfg *tsdrv_get_dms_sensor_obj(
    u32 devid, DMS_DEVICE_NODE_TYPE node_type, u32 tsid, u32 sensor_node)
{
    struct dms_sensor_object_cfg *sensor_objs = tsdrv_get_sensor_obj_table(devid);
    u32 node_id = get_devnode_id(node_type);
    u32 i;

    for (i = 0; i < DMS_MAX_NODE_SENSOR_COUNT; i++) {
        if (sensor_objs[i].sensor_type == DMS_SEN_TYPE_MAX_CUSTOM) {
            break;
        }
        if (sensor_objs[i].private_data ==
            SENSOR_PRIV_DATA(devid, tsid, node_type, node_id, sensor_node)) {
            return &sensor_objs[i];
        }
    }
    TSDRV_PRINT_ERR("can't find dms sensor. (devid=%u; tsid=%u; node_id=%u; sensor_node=%u)\n",
        devid, tsid, node_id, sensor_node);
    return NULL;
}

STATIC void tsdrv_notify_sensor_scan_immediately(u32 devid, DMS_DEVICE_NODE_TYPE node_type, u32 tsid, u32 sensor_node)
{
    struct dms_sensor_object_cfg *sensor_obj = NULL;

    sensor_obj = tsdrv_get_dms_sensor_obj(devid, node_type, tsid, sensor_node);
    if (sensor_obj == NULL) {
        TSDRV_PRINT_ERR("find dms sensor object failed. (devid=%u; node_type=%d; tsid=%u; sensor_node=%u)\n",
            devid, (int)node_type, tsid, sensor_node);
        return;
    }

    if (dms_sensor_event_notify(devid, sensor_obj) != 0) {
        TSDRV_PRINT_ERR("notify dms sensor event failed. (devid=%u; node_type=%d; tsid=%u; sensor_node=%u)\n",
            devid, (int)node_type, tsid, sensor_node);
    }
}

int tsdrv_fault_event_process(u32 devid, DMS_DEVICE_NODE_TYPE node_type, u32 tsid, u32 sensor_node,
    struct ras_error *error)
{
    u32 node_id = get_devnode_id(node_type);
    struct fault_event *event = NULL;
    int ret;

    event = tsdrv_get_fault_event(devid, tsid, node_id, sensor_node);
    if (event == NULL) {
        TSDRV_PRINT_ERR("Event info is NULL. (devid=%u; tsid=%u; node_id=%u; sensor_node=%u)\n",
            devid, tsid, node_id, sensor_node);
        return -ENOMEM;
    }

    if (error->event_attr == EVENT_RESUME_ATTR) {
        tsdrv_set_fault_event_resume(event, error);
        return 0;
    }

    ret = tsdrv_add_fault_event(event, error);
    if (ret == 0) {
        tsdrv_notify_sensor_scan_immediately(devid, node_type, tsid, sensor_node);
    } else if (ret != -EEXIST) {
        TSDRV_PRINT_ERR("Fault event added failed. (devid=%u; tsid=%u; node_id=%u; sensor_node=%u; event_attr=%u)\n",
            devid, tsid, node_id, sensor_node, (u32)error->event_attr);
        return ret;
    } else {
        /* do nothing */
    }

    return 0;
}

static bool tsdrv_fault_event_is_need_clear(struct ras_error *error) /* Further confirmation is required */
{
    if ((error->event_attr == EVENT_ONE_TIME_ATTR) || (error->event_attr == EVENT_RESUME_ATTR)) {
        return true;
    }
    if (error->sensor_status == RAS_ERROR_TYPE_SBECCOverThold) {
        return true;
    }

    return false;
}

int tsdrv_fault_event_scan(u64 private_data, struct dms_sensor_event_data *data)
{
    struct fault_event *event = NULL;
    struct fault_list_node *pos = NULL;
    struct fault_list_node *n = NULL;
    unsigned long flags;
    u32 devid = get_dev_id_from_priv(private_data);
    u32 tsid = get_ts_id_from_priv(private_data);
    u32 node_id = get_dev_node_from_priv(private_data);
    u32 sensor_node = get_sensor_node_from_priv(private_data);
    if ((data == NULL) || (devid >= fault_ctrl_get_num()) ||
        (node_id >= DEV_NODE_ID_MAX) || (sensor_node >= SENSOR_NODE_ID_MAX)) {
        TSDRV_PRINT_ERR("The parameter is invalid. (devid=%u; node_id=%u; sensor_node=%u)\n",
            devid, node_id, sensor_node);
        return -EINVAL;
    }

    event = tsdrv_get_fault_event(devid, tsid, node_id, sensor_node);
    if (event == NULL) {
        TSDRV_PRINT_ERR("Get fault event failed. (devid=%u; node_id=%u; sensor_node=%u)\n",
            devid, node_id, sensor_node);
        return -EINVAL;
    }

    data->event_count = 0;
    spin_lock_irqsave(&event->spinlock, flags);
    /* get every event of the sensor event list */
    list_for_each_entry_safe(pos, n, &event->node.list, list) {
        data->sensor_data[data->event_count].current_value = pos->error.sensor_status;

        if (tsdrv_fault_event_is_need_clear(&pos->error)) {
            list_del(&pos->list);
            kfree(pos);
            pos = NULL;
            event->error_num--;
        }

        data->event_count++;
        if (data->event_count == DMS_MAX_SENSOR_EVENT_COUNT) {
            goto out;
        }
    }
    spin_unlock_irqrestore(&event->spinlock, flags);
    return 0;

out:
    spin_unlock_irqrestore(&event->spinlock, flags);
    TSDRV_PRINT_DEBUG("The maximum number of sensor events had been reached.\n");
    return 0;
}

#endif

