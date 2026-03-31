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
* Create: 2023-7-5
*/
#ifndef __TSDRV_TSCPU_FAULT_H__
#define __TSDRV_TSCPU_FAULT_H__

#define RAS_CODE_TSCPU 0xFFFE

struct dfm_struct *tsdrv_get_tscpu_dms(u32 devid);
struct dms_node *tsdrv_get_tscpu_dms_nodes(u32 devid, u32 node_id);
struct dms_sensor_object_cfg *tsdrv_get_tscpu_sensor_cfg(u32 *sensor_num);
int tsdrv_heartbeat_fault_event_add(u32 devid, u32 tsid);

#endif