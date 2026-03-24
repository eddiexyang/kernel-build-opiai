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
#include "tsdrv_fault_init.h"

int tsdrv_fault_event_scan(u64 private_data, struct dms_sensor_event_data *data);

#define TSDRV_FAULT_EVENT_SCAN_HWTS tsdrv_fault_event_scan
#define TSDRV_FAULT_EVENT_SCAN_TS tsdrv_fault_event_scan
#define TSDRV_FAULT_EVENT_SCAN_TSCPU tsdrv_fault_event_scan
#define TSDRV_FAULT_EVENT_SCAN_AIC tsdrv_fault_event_scan
#define TSDRV_FAULT_EVENT_SCAN_L2BUF tsdrv_fault_event_scan
#define TSDRV_FAULT_EVENT_SCAN_SDMA tsdrv_fault_event_scan
#define TSDRV_FAULT_EVENT_SCAN_DSA tsdrv_fault_event_scan
#define TSDRV_FAULT_EVENT_SCAN_TSFW tsdrv_fault_event_scan

u32 get_devnode_id(DMS_DEVICE_NODE_TYPE node_type);
u32 get_sensor_node_id(DMS_DEVICE_NODE_TYPE node_type);
struct fault_event *tsdrv_get_fault_event(u32 devid, u32 tsid, u32 node_id, u32 sensor_node);
int tsdrv_fault_event_process(u32 devid, DMS_DEVICE_NODE_TYPE node_type, u32 tsid, u32 sensor_node,
    struct ras_error *error);

