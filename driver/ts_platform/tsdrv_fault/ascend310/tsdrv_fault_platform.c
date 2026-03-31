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

#include "tsdrv_fault_init.h"
#include "tsdrv_fault_platform.h"
#include "tsdrv_log.h"
#include "tsdrv_tscpu_fault.h"
#include "tsdrv_fault_comm.h"

#define SENSOR_OBJ_TABLE(_devid) \
    { \
        /* Invalid sensor obj */ \
        GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_MAX_CUSTOM, "invalid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), \
    }

static struct dms_sensor_object_cfg g_sensor_obj_table[TSDRV_MAX_DAVINCI_NUM][DMS_MAX_NODE_SENSOR_COUNT] = {
    SENSOR_OBJ_TABLE(0x0ULL), /* dev0 */
};

struct dms_sensor_object_cfg *tsdrv_get_sensor_obj_table(u32 devid)
{
    return g_sensor_obj_table[devid];
}

int tsdrv_heartbeat_fault_event_add(u32 devid, u32 tsid)
{
    return 0;
}

