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
* Create: 2023-05-29
*/

#ifndef TSDRV_L2BUFF_SAFETY_H
#define TSDRV_L2BUFF_SAFETY_H

#include "fpdc.h"
#include "dms_sensor.h"

struct tsdrv_ras_node_info {
    int node_type;
    int sub_node_type;
    int node_num;
};

#define L2BUFF_MODULE_ID 41
#define DISP_MODULE_ID   7

int tsdrv_ras_event_scan(u64 private_data, struct dms_sensor_event_data *data);

/* change to dfm template */
int tsdrv_register_ras_dms_node(u32 devid);
void tsdrv_unregister_ras_dms_node(u32 devid);

#endif
