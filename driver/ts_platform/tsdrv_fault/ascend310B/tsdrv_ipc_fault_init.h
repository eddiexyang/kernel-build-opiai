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
* Create: 2023-06-26
*/

#ifndef TSDRV_IPC_FAULT_INIT_H
#define TSDRV_IPC_FAULT_INIT_H
#include <linux/types.h>
#include <linux/uuid.h>
#include <linux/workqueue.h>
#include "tsdrv_kernel_common.h"
#include "dms_dev_node.h"
#include "dms_sensor.h"
#include "dms_sensor_type.h"
#include "dms_define.h"
/* The new enum value should match TS_DEV_NODE_ID_MAX. */
enum ts_dev_node_id {
    TS_DEV_NODE_HWTS = 0,
    TS_DEV_NODE_TSCPU,
    TS_DEV_NODE_AIC,
    TS_DEV_NODE_SDMA,
    TS_DEV_NODE_AIV,
    TS_DEV_NODE_TS,
    TS_DEV_NODE_HWTSCPU,
    TS_DEV_NODE_TSFW,
    TS_DEV_NODE_ID_MAX
};

int tsdrv_register_ipc_fault_event(u32 devid);
void tsdrv_unregister_ipc_fault_event(u32 devid);
int tsdrv_ipc_fault_event_scan(u64 private_data, struct dms_sensor_event_data *data);
struct dfm_struct *tsdrv_get_dfm_struct(enum ts_dev_node_id node_type_id);
enum ts_dev_node_id get_node_type_id_ipc(DMS_DEVICE_NODE_TYPE node_type);
#define TSDRV_IPC_FAULT_EVENT_SCAN tsdrv_ipc_fault_event_scan

#endif