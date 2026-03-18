/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#ifdef CFG_FEATURE_FAULT
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/securec.h>
#include <linux/securectype.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/notifier.h>
#include <linux/cper.h>
#include <linux/uuid.h>
#include <linux/slab.h>
#include <acpi/ghes.h>
#include <acpi/apei.h>

#include "tsdrv_fault_init.h"
#include "tsdrv_ras.h"
#include "tsdrv_non_ras.h"
#include "tsdrv_common_fault_platform.h"
#include "tsdrv_fault_comm.h"
#include "tsdrv_log.h"
#include "ascend_kernel_hal.h"
#include "drv_ipc.h"
#include "drv_log.h"
#include "devdrv_manager_comm.h"
#include "devdrv_manager.h"
#include "devdrv_ipc.h"
#include "tsdrv_ipc.h"
#include "tsmng_interface.h"
#include "dms_sensor_notify.h"
#include "icm_interface.h"
#include "fpdc_ras_receiver.h"

#ifdef CFG_FEATURE_USE_FAULT_NEW_FRAME_WORK
#include "tsdrv_ipc_fault_init.h"
#include "tsdrv_ipc_fault_rcv.h"
#endif

static struct tsdrv_fault_ctrl fault_ctrl;

static void fault_ctrl_set_num(void)
{
    fault_ctrl.dev_num++;
}

u32 fault_ctrl_get_num(void)
{
    return fault_ctrl.dev_num;
}

struct fault_dev *tsdrv_get_fault_dev(u32 devid)
{
    return &fault_ctrl.fault_dev[devid];
}
EXPORT_SYMBOL_UNRELEASE(tsdrv_get_fault_dev);

static int tsdrv_node_ops_init(struct dms_node *device)
{
    return 0;
}

static void tsdrv_node_ops_uninit(struct dms_node *device)
{
    return;
}

struct dms_node_operations tsdrv_dev_node_ops = {
    .init = tsdrv_node_ops_init,
    .uninit = tsdrv_node_ops_uninit,
    .get_info_list = NULL,
    .get_state = NULL,
    .get_capacity = NULL,
    .set_power_state = NULL,
    .scan = NULL,
    .fault_diag = NULL,
    .event_notify = NULL,
    .get_link_state = NULL,
    .set_link_state = NULL
};

#define DEV_NODE_TABLE_OBJ(_devid) { \
    GEN_DMS_NODE(DMS_DEV_TYPE_HWTS_S_TS, DEV_NODE_HWTS_STARS, "HWTS/STARS", 0x1, 0x1, \
        _devid, &tsdrv_dev_node_ops), \
    GEN_DMS_NODE(DMS_DEV_TYPE_TS, DEV_NODE_TS, "TS", 0x1, 0x1, \
        _devid, &tsdrv_dev_node_ops), \
    GEN_DMS_NODE(DMS_DEV_TYPE_AIC, DEV_NODE_AIC, "AIC", 0x1, 0x1, \
        _devid, &tsdrv_dev_node_ops), \
    GEN_DMS_NODE(DMS_DEV_TYPE_SDMA, DEV_NODE_SDMAM, "SDMA", 0x1, 0x1, \
        _devid, &tsdrv_dev_node_ops), \
    GEN_DMS_NODE(DMS_DEV_TYPE_AIV, DEV_NODE_AIV, "AIV", 0x1, 0x1, \
        _devid, &tsdrv_dev_node_ops), \
    GEN_DMS_NODE(DMS_DEV_TYPE_DSA, DEV_NODE_DSA, "DSA", 0x1, 0x1, \
        _devid, &tsdrv_dev_node_ops), \
    GEN_DMS_NODE(DMS_DEV_TYPE_TSFW, DEV_NODE_TSFW, "TSFW", 0x1, 0x1, \
        _devid, &tsdrv_dev_node_ops), \
}

static struct dms_node tsdrv_dev_node_table[TSDRV_MAX_DAVINCI_NUM][DEV_NODE_ID_MAX] = {
    DEV_NODE_TABLE_OBJ(0), /* dev0 */
    DEV_NODE_TABLE_OBJ(1), /* dev1 */
    DEV_NODE_TABLE_OBJ(2), /* dev2 */
    DEV_NODE_TABLE_OBJ(3), /* dev3 */
};

int fault_get_node_type(u32 devid, u32 node_id)
{
    return tsdrv_dev_node_table[devid][node_id].node_type;
}

#ifndef CFG_FEATURE_USE_FAULT_NEW_FRAME_WORK
static void fault_ctrl_dev_node_init(struct fault_dev_node *dev_node, u32 devid, u32 node_id)
{
    dev_node->node = &tsdrv_dev_node_table[devid][node_id];
    dev_node->sensor_num = 0;
}

static int tsdrv_fault_ctrl_init(u32 devid)
{
    struct fault_dev *dev = NULL;
    u32 node_id;
#ifdef CFG_FEATURE_FAULT_FPDC
    int ret;
#endif

    fault_ctrl_set_num();
    dev = tsdrv_get_fault_dev(devid);
    dev->dev_node = kzalloc(sizeof(struct fault_dev_node) * DEV_NODE_ID_MAX, GFP_KERNEL);
    if (dev->dev_node == NULL) {
        TSDRV_PRINT_ERR("Failed to kzalloc dev_node. (devid=%u)\n", devid);
        return -ENOMEM;
    }
    for (node_id = 0; node_id < DEV_NODE_ID_MAX; node_id++) {
        fault_ctrl_dev_node_init(&dev->dev_node[node_id], devid, node_id);
    }
    dev->devid = devid;
    dev->dev_node_num = DEV_NODE_ID_MAX;
#ifdef CFG_FEATURE_FAULT_FPDC
    ret = fault_remap_l2buf(devid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to remap l2buf. (devid=%u)\n", devid);
        kfree(dev->dev_node);
        dev->dev_node = NULL;
        return ret;
    }
    INIT_DELAYED_WORK(&dev->unmask_irq_work, fault_unmask_ras_irq);
#endif
    return 0;
}

STATIC void tsdrv_fault_ctrl_uninit(u32 devid)
{
    struct fault_dev *fault_dev = NULL;

    fault_dev = tsdrv_get_fault_dev(devid);
    kfree(fault_dev->dev_node);
    fault_dev->dev_node = NULL;
#ifdef CFG_FEATURE_FAULT_FPDC
    (void)cancel_delayed_work_sync(&fault_dev->unmask_irq_work);
    fault_unmap_l2buf(devid);
#endif
}

STATIC int tsdrv_dev_node_register(struct fault_dev *fault_dev, u32 node_id)
{
    u32 sensor_id;
    u32 dev_node_id;
    int ret;
    struct fault_dev_node *dev_node = NULL;
    struct fault_event *event = NULL;
    struct dms_sensor_object_cfg *object_cfg = NULL;

    dev_node = &fault_dev->dev_node[node_id];
    ret = dms_register_dev_node(dev_node->node);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to register device node. (node_id=%u; ret=%d)\n", node_id, ret);
        return ret;
    }
    object_cfg = tsdrv_get_sensor_obj_table(fault_dev->devid);
    for (sensor_id = 0; sensor_id < DMS_MAX_NODE_SENSOR_COUNT; sensor_id++) {
        if (object_cfg[sensor_id].sensor_type == DMS_SEN_TYPE_MAX_CUSTOM) {
            break;
        }

        dev_node_id = get_dev_node_from_priv(object_cfg[sensor_id].private_data);
        if (dev_node_id != node_id) {
            continue;
        }

        event = &dev_node->sensor_event[dev_node->sensor_num];
        INIT_LIST_HEAD(&event->node.list);
        event->tsid = get_ts_id_from_priv(object_cfg[sensor_id].private_data);
        event->sensor_node = get_sensor_node_from_priv(object_cfg[sensor_id].private_data);
        spin_lock_init(&event->spinlock);
        event->status = 1;

        ret = dms_sensor_register(dev_node->node, &object_cfg[sensor_id]);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Failed to register sensor. (node_id=%u; sensor_id=%u; ret=%d)\n",
                node_id, sensor_id, ret);
            goto err_sensor_register;
        }

        dev_node->sensor_num++;

        TSDRV_PRINT_INFO("Register dms sensor. (name=%s; type=%d; priv=0x%llx)\n",
            object_cfg[sensor_id].sensor_name, object_cfg[sensor_id].sensor_type,
            object_cfg[sensor_id].private_data);
    }
    return 0;

err_sensor_register:
    (void)dms_sensor_node_unregister(dev_node->node);
    (void)dms_unregister_dev_node(dev_node->node);
    return ret;
}

STATIC void tsdrv_dev_node_unregister(struct fault_dev *fault_dev, u32 node_id)
{
    (void)dms_sensor_node_unregister(fault_dev->dev_node[node_id].node);
    (void)dms_unregister_dev_node(fault_dev->dev_node[node_id].node);
}

STATIC int tsdrv_dev_node_init(u32 devid)
{
    struct fault_dev *fault_dev = NULL;
    u32 node_id, i;
    int ret;

    fault_dev = tsdrv_get_fault_dev(devid);
    for (node_id = 0; node_id < fault_dev->dev_node_num; node_id++) {
        ret = tsdrv_dev_node_register(fault_dev, node_id);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Failed to register device node. (devid=%u; node_id=%u; ret=%d)\n", devid, node_id, ret);
            goto out;
        }
    }

    return 0;
out:
    for (i = 0; i < node_id; i++) {
        tsdrv_dev_node_unregister(fault_dev, node_id);
    }
    return ret;
}

static void tsdrv_free_fault_event(struct fault_event *event)
{
    struct fault_list_node *pos = NULL;
    struct fault_list_node *n = NULL;
    unsigned long flags;

    if (event == NULL) {
        TSDRV_PRINT_ERR("The parameter is invalid.\n");
        return;
    }

    spin_lock_irqsave(&event->spinlock, flags);
    list_for_each_entry_safe(pos, n, &event->node.list, list) {
        list_del(&pos->list);
        kfree(pos);
        pos = NULL;
        event->error_num--;
    }
    spin_unlock_irqrestore(&event->spinlock, flags);
}

static void tsdrv_dev_node_event_destroy(struct fault_dev_node *dev_node)
{
    u32 i;

    for (i = 0; i < dev_node->sensor_num; i++) {
        tsdrv_free_fault_event(&dev_node->sensor_event[i]);
    }
}

STATIC void tsdrv_dev_node_uninit(u32 devid)
{
    struct fault_dev *fault_dev = tsdrv_get_fault_dev(devid);
    u32 node_id;

    for (node_id = 0; node_id < fault_dev->dev_node_num; node_id++) {
        tsdrv_dev_node_unregister(fault_dev, node_id);
        tsdrv_dev_node_event_destroy(&fault_dev->dev_node[node_id]);
    }
}
#endif /* CFG_FEATURE_USE_FAULT_NEW_FRAME_WORK */

int tsdrv_fault_dev_init(u32 devid)
{
    int ret = 0;

#ifdef CFG_FEATURE_USE_FAULT_NEW_FRAME_WORK
    fault_ctrl_set_num();
    tsdrv_register_ipc_fault_event(devid);
#else
    ret = tsdrv_fault_ctrl_init(devid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to initialize the tsdrv fault control structure. (ret=%d)\n", ret);
        return ret;
    }

    ret = tsdrv_dev_node_init(devid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to initialize the tsdrv device node. (ret=%d)\n", ret);
        goto err_dev_node_init;
    }
#endif /* CFG_FEATURE_USE_FAULT_NEW_FRAME_WORK */

#ifdef CFG_FEATURE_FAULT_FPDC
    ret = tsdrv_register_ras_dms_node(devid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to register ras dms node. (ret=%d; devid=%u)\n", ret, devid);
        goto err_register_ras_dms_node;
    }
#endif
    TSDRV_PRINT_INFO("TSDRV fault manager device init successfully. (devid=%u)\n", devid);
    return ret;

#ifdef CFG_FEATURE_FAULT_FPDC
err_register_ras_dms_node:
#ifdef CFG_FEATURE_USE_FAULT_NEW_FRAME_WORK
    tsdrv_unregister_ipc_fault_event(devid);
#else
    tsdrv_dev_node_uninit(devid);
#endif /* CFG_FEATURE_USE_FAULT_NEW_FRAME_WORK */
#endif /* CFG_FEATURE_FAULT_FPDC */

#ifndef CFG_FEATURE_USE_FAULT_NEW_FRAME_WORK
err_dev_node_init:
    tsdrv_fault_ctrl_uninit(devid);
#endif
    return ret;
}

void tsdrv_fault_dev_exit(u32 devid)
{
#ifdef CFG_FEATURE_FAULT_FPDC
    tsdrv_unregister_ras_dms_node(devid);
#endif

#ifdef CFG_FEATURE_USE_FAULT_NEW_FRAME_WORK
    tsdrv_unregister_ipc_fault_event(devid);
#else
    tsdrv_dev_node_uninit(devid);
    tsdrv_fault_ctrl_uninit(devid);
#endif
}

void tsdrv_fault_init(void)
{
#if (defined(CFG_SOC_PLATFORM_MINI) && !defined(CFG_SOC_PLATFORM_MINIV2) && !defined(CFG_SOC_PLATFORM_MINIV3))
    /* not support. */
#else
    u32 cmd_type;
    int ret;

#ifdef CFG_FEATURE_USE_ICM_CHAN
    cmd_type = ICM_MAIN_CMD_TS_NOTIFY;
#else
    cmd_type = IPCDRV_TS_ADV_CMDTYPE_ALARM_MSG_REPORT;
#endif

#ifdef CFG_FEATURE_USE_FAULT_NEW_FRAME_WORK
    ret = tsdrv_ipc_handler_register(cmd_type, tsdrv_ipc_fault_event_handler);
#else
    ret = tsdrv_ipc_handler_register(cmd_type, tsdrv_ts_fault_msg_handle);
#endif
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to register ipc fault handle. (ret=%d)\n", ret);
    }
#endif
}

void tsdrv_fault_exit(void)
{
#if (defined(CFG_SOC_PLATFORM_MINI) && !defined(CFG_SOC_PLATFORM_MINIV2))
    /* not support. */
#else
    u32 cmd_type;

#ifdef CFG_FEATURE_USE_ICM_CHAN
    cmd_type = ICM_MAIN_CMD_TS_NOTIFY;
#else
    cmd_type = IPCDRV_TS_ADV_CMDTYPE_ALARM_MSG_REPORT;
#endif
    tsdrv_ipc_handler_unregister(cmd_type);
#endif
}
#endif /* CFG_FEATURE_FAULT */
