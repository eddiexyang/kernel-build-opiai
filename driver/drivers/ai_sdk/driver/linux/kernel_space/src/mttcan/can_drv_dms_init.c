/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Create: 2022-10-29
 */

#include <linux/sizes.h>
#include <linux/slab.h>

#include "can_drv_dms_init.h"
#include "dms_node_type.h"
#include "dms_sensor_type.h"
#include "can_drv_log.h"

#define CAN_NODE_NUM 1
#define CAN_SENSOR_NUM 1
#define CAN_DEVICE_NUM_MAX 15

#define CAN_DEV_NODE_OFFSET 16
#define CAN_DEV_NODE_MASK 0xFF
#define CAN_EVENT_ASSERT_MASK 0XFFFF
#define CAN_EVENT_DEASSERT_MASK 0XFFFD
#define CAN_SENSOR_PRIV_DATA(dev_id, node_id) ((((u64)(dev_id & CAN_DEV_NODE_MASK)) << CAN_DEV_NODE_OFFSET) | \
                                               (node_id & CAN_DEV_NODE_MASK))

#define CAN_DMS_NODE_DEFINE(_id, _ops) { \
    .node_type = DMS_DEV_TYPE_CAN,          \
    .node_id = _id,                         \
    .node_name = "CAN-" #_id,               \
    .capacity = 0x1,                        \
    .permission = 0x1,                      \
    .owner_devid = 0,                       \
    .ops = _ops                             \
}

#if !defined(DFM_DISABLE)
struct dfm_struct g_can_drv_dms[CAN_DEVICE_NUM_MAX];

struct dfm_struct *can_get_drv_dms(u32 dev_id)
{
    if (dev_id >= CAN_DEVICE_NUM_MAX) {
        mttcan_err("invalid id. (dev_id=%d)\n", dev_id);
        return NULL;
    }

    return &g_can_drv_dms[dev_id];
}

STATIC int can_dms_node_ops_init(struct dms_node *device)
{
    mttcan_debug("DEBUG (node_name=%s)\n", device->node_name);
    return 0;
}

STATIC void can_dms_node_ops_uninit(struct dms_node *device)
{
    mttcan_debug("DEBUG (node_name=%s)\n", device->node_name);
    return;
}

STATIC struct dms_node_operations g_can_dms_node_ops = {
    .init = can_dms_node_ops_init,
    .uninit = can_dms_node_ops_uninit,
    .scan = NULL,
    .fault_diag = NULL,
    .event_notify = NULL,
    .get_link_state = NULL,
    .set_link_state = NULL
};

static struct dms_node g_can_dms_node[CAN_DEVICE_NUM_MAX][CAN_NODE_NUM] = {
    {CAN_DMS_NODE_DEFINE(0, &g_can_dms_node_ops)},
    {CAN_DMS_NODE_DEFINE(1, &g_can_dms_node_ops)},
    {CAN_DMS_NODE_DEFINE(2, &g_can_dms_node_ops)},
    {CAN_DMS_NODE_DEFINE(3, &g_can_dms_node_ops)},
    {CAN_DMS_NODE_DEFINE(4, &g_can_dms_node_ops)},
    {CAN_DMS_NODE_DEFINE(5, &g_can_dms_node_ops)},
    {CAN_DMS_NODE_DEFINE(6, &g_can_dms_node_ops)},
    {CAN_DMS_NODE_DEFINE(7, &g_can_dms_node_ops)},
    {CAN_DMS_NODE_DEFINE(8, &g_can_dms_node_ops)},
    {CAN_DMS_NODE_DEFINE(9, &g_can_dms_node_ops)},
    {CAN_DMS_NODE_DEFINE(10, &g_can_dms_node_ops)},
    {CAN_DMS_NODE_DEFINE(11, &g_can_dms_node_ops)},
    {CAN_DMS_NODE_DEFINE(12, &g_can_dms_node_ops)},
    {CAN_DMS_NODE_DEFINE(13, &g_can_dms_node_ops)},
    {CAN_DMS_NODE_DEFINE(14, &g_can_dms_node_ops)}
};
#endif

int can_scan_event(u64 private_data, struct dms_sensor_event_data *data)
{
#if !defined(DFM_DISABLE)
    u32 dev_id, node_id;

    dev_id = ((private_data >> CAN_DEV_NODE_OFFSET) & CAN_DEV_NODE_MASK);
    node_id = (u32)(private_data & CAN_DEV_NODE_MASK);

    if (dev_id >= CAN_DEVICE_NUM_MAX || node_id >= g_can_drv_dms[dev_id].node_num) {
        mttcan_err("invalid id. (dev_id=%d, node_id=%u, priv=0x%llx)\n", dev_id, node_id, private_data);
        return -EINVAL;
    }

    (void)dfm_scan_events(&g_can_drv_dms[dev_id], node_id, DMS_SEN_TYPE_BUS_SENSOR, data);
#endif
    return 0;
}

void can_sensor_init(int can_id, struct dms_sensor_object_cfg *sensor)
{
    sensor->sensor_type = DMS_SEN_TYPE_BUS_SENSOR; // SOC Event3
    strcpy_s(sensor->sensor_name, DMS_SENSOR_DESCRIPT_LENGTH, "CAN Sensor");
    sensor->sensor_class = DMS_DISCRETE_SENSOR_CLASS;
    sensor->sensor_class_cfg.discrete_sensor.attribute = DMS_SENSOR_ATTRIB_THRES_NONE;
    sensor->sensor_class_cfg.discrete_sensor.debounce_time = 0;
    sensor->scan_module = DMS_SERSOR_SCAN_PERIOD;
    sensor->scan_interval = DMS_SENSOR_CHECK_INTERVAL_TIME;
    sensor->proc_flag = DMS_SENSOR_PROC_ENABLE_FLAG;
    sensor->enable_flag = DMS_SENSOR_ENABLE_FALG;
    sensor->pf_scan_func = can_scan_event;
    sensor->assert_event_mask = CAN_EVENT_ASSERT_MASK;
    sensor->deassert_event_mask = CAN_EVENT_DEASSERT_MASK;
}

int mttcan_register_dms_node(u32 dev_id)
{
#if !defined(DFM_DISABLE)
    int i, j;
    struct dms_sensor_object_cfg sensor_cfg;
    struct dfm_struct *can_dms = NULL;

    if (dev_id >= CAN_DEVICE_NUM_MAX) {
        mttcan_err("invalid dev id. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }

    can_dms = &g_can_drv_dms[dev_id];
    can_dms->dev_id = dev_id;
    if (dfm_struct_init(can_dms, CAN_NODE_NUM, CAN_SENSOR_NUM) != 0) {
        mttcan_err("init can's dfm data failed\n");
        goto _fail;
    }

    can_sensor_init(dev_id, &sensor_cfg);
    for (i = 0; i < CAN_NODE_NUM; ++i) {
        can_dms->dev_nodes[i].node = &g_can_dms_node[dev_id][i];
        can_dms->dev_nodes[i].post_proc = NULL;
        can_dms->dev_nodes[i].fpdc_notify = NULL;
        can_dms->dev_nodes[i].get_converage_node = NULL;

        sensor_cfg.private_data = CAN_SENSOR_PRIV_DATA(dev_id, i);
        for (j = 0; j < CAN_SENSOR_NUM; ++j) {
            can_dms->dev_nodes[i].sensors[j].dms_sensor = sensor_cfg;
        }
    }

    if (dfm_register_nodes(can_dms) != 0) {
        mttcan_err("register can dms node failed\n");
        goto _fail;
    }
    return 0;

_fail:
    dfm_struct_final(can_dms, CAN_NODE_NUM, CAN_SENSOR_NUM);
    return -EFAULT;
#else
    return 0;
#endif
}

void can_unregister_dms(u32 devid)
{
#if !defined(DFM_DISABLE)
    dfm_unregister_nodes(&g_can_drv_dms[devid]);
    dfm_struct_final(&g_can_drv_dms[devid], CAN_NODE_NUM, CAN_SENSOR_NUM);
#endif
}
