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
#ifdef CFG_FEATURE_FAULT_FPDC
#include <linux/io.h>
#include "tsdrv_fault_init.h"
#include "tsdrv_common_fault_platform.h"
#include "tsdrv_fault_comm.h"
#include "tsdrv_log.h"
#include "soc_res.h"
#include "dfm_dev_register.h"
#include "dms_sensor_type.h"
#include "dms_sensor.h"
#include "dfm_report.h"
#include "tsdrv_ras.h"
#include "tsdrv_fault_platform.h"
#include "tsdrv_tscpu_fault.h"

STATIC struct dfm_struct tscpu_dms[TSDRV_MAX_DAVINCI_NUM];
struct dfm_struct *tsdrv_get_tscpu_dms(u32 devid)
{
    return &tscpu_dms[devid];
}

struct dms_node g_tscpu_dms_nodes[TSDRV_MAX_DAVINCI_NUM][TSCPU_NUM];
struct dms_node *tsdrv_get_tscpu_dms_nodes(u32 devid, u32 node_id)
{
    return &g_tscpu_dms_nodes[devid][node_id];
}

STATIC struct dms_sensor_object_cfg g_tscpu_sensor_cfg[] = {
    /* FaultCode=0x80FB8008, SensorType="soc_event" */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "tscpu", DMS_DISCRETE_SENSOR_CLASS,
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL,
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, tsdrv_ras_event_scan,
        SENSOR_PRIV_DATA(0, 0, 0, 0, DMS_SEN_TYPE_RAS_SENSOR), 0xFFFF, 0xFFBF),

    /* FaultCode=0x80FA4E00, SensorType="Heartbeat" */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_HEARTBEAT, "ts_heartbeat", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, tsdrv_ras_event_scan, \
        0, 0xFFFF, 0xFFFF), \
};
struct dms_sensor_object_cfg *tsdrv_get_tscpu_sensor_cfg(u32 *sensor_num)
{
    *sensor_num = sizeof(g_tscpu_sensor_cfg) / sizeof(g_tscpu_sensor_cfg[0]);
    return g_tscpu_sensor_cfg;
}

STATIC int tsdrv_get_heartbeat_fault_event(dfm_event *event)
{
    const char *fault_info = "tscpu heartbeat";
    int ret;
    event->subsys_id = DMS_DEV_TYPE_TSCPU;
    event->module_id = DMS_DEV_TYPE_TSCPU;
    event->section_type = RAS_SEC_OTHER;
    ret = strcpy_s((char *)&event->describe[0], DMS_MAX_EVENT_DATA_LENGTH, fault_info);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Copy fail. (ret=%d)\n", ret);
        return ret;
    }
    event->sensor_type = DMS_SEN_TYPE_HEARTBEAT;
    event->error_type = 0;
    return 0;
}

int tsdrv_heartbeat_fault_event_add(u32 devid, u32 tsid)
{
    int ret;
    dfm_event event = {0};
    struct dfm_struct *dfm = NULL;

    if ((devid >= TSDRV_MAX_DAVINCI_NUM) || (tsid >= TSCPU_NUM)) {
        TSDRV_PRINT_ERR("tsdrv_heartbeat_fault_event_add Incorrect input parameters. (devid=%u; tsid=%u)\n",
            devid, tsid);
        return -EINVAL;
    }

    dfm = tsdrv_get_tscpu_dms(devid);
    ret = tsdrv_get_heartbeat_fault_event(&event);
    if (ret != 0) {
        return ret;
    }

    ret = dfm_add_event(dfm, tsid, &event);
    if ((ret != 0) && (ret != -EEXIST)) {
        TSDRV_PRINT_ERR("tsdrv_heartbeat_fault_event_add Process event info failed. (devid=%u; tsid=%u; ret=%d)\n",
            devid, tsid, ret);
        return ret;
    }

    return 0;
}
#endif

