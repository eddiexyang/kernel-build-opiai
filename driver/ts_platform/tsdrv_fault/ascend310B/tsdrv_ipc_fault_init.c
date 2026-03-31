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

#include <linux/kernel.h>
#include <linux/securec.h>
#ifndef AOS_LLVM_BUILD
#include <linux/securectype.h>
#endif
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/notifier.h>
#ifndef AOS_LLVM_BUILD
#include <linux/cper.h>
#endif
#include <linux/slab.h>
#ifndef AOS_LLVM_BUILD
#include <acpi/ghes.h>
#include <acpi/apei.h>
#endif
#include "tsdrv_ipc_fault_init.h"
#include "tsdrv_log.h"
#include "drv_ipc.h"
#include "drv_log.h"
#include "devdrv_manager_comm.h"
#include "devdrv_manager.h"
#include "devdrv_ipc.h"
#include "tsdrv_ipc.h"
#include "dms_sensor_notify.h"
#include "icm_interface.h"
#include "tsmng_interface.h"
#include "tsdrv_fault_init.h"
#include "dfm_dev_register.h"
#include "dfm_report.h"
#include "dms_define.h"
#include "tsdrv_safety_fault.h"
#include "tsdrv_common.h"

#define GEN_DEV_NODE_TABLE_OBJ(_type, _id, _name, _cap, _perm, _devid, _ops) \
    { \
        .node_type = _type, \
        .node_id = _id, \
        .node_name = _name, \
        .capacity = _cap, \
        .permission = _perm, \
        .owner_devid = _devid, \
        .ops = _ops \
    }

/* Each device type provides only one node_id information, which will be extended by using dfm_duplicate_dms_nodes. */
/* Only one STARS replaces HWTS on AS31XM1 with one TS_SUB_SYS. TSFW runs on TSCPU. */
struct dms_node g_tsdrv_dev_type[] = {
    GEN_DEV_NODE_TABLE_OBJ(DMS_DEV_TYPE_HWTS_S_TS, 0, "HWTS", 1, 0x1, 0, &tsdrv_dev_node_ops), \
    GEN_DEV_NODE_TABLE_OBJ(DMS_DEV_TYPE_AIC, 0, "AIC", 1, 0x1, 0, &tsdrv_dev_node_ops), \
    GEN_DEV_NODE_TABLE_OBJ(DMS_DEV_TYPE_SDMA, 0, "SDMA", 8, 0x1, 0, &tsdrv_dev_node_ops), \
    GEN_DEV_NODE_TABLE_OBJ(DMS_DEV_TYPE_TS, 0, "TS", 1, 0x1, 0, &tsdrv_dev_node_ops ), \
    GEN_DEV_NODE_TABLE_OBJ(DMS_DEV_TYPE_TSFW, 0, "TSFW", 1, 0x1, 0, &tsdrv_dev_node_ops ), \
};

struct dms_sensor_object_cfg g_hwts_ras_sensors[] = {
    /* FaultCode=0x80F78003/0x80F78006/0x80F78008/0x80F78009, SensorType="soc_event" */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "hwts_ras", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFBF), \

    /* FaultCode=0x80F78809, SensorType="soc_event2" */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_EXTEND_SENSOR, "hwts_extend", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFFF), \

    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_MAX_CUSTOM, "invalid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), \
};

/* as31xm1 tscpu has no IPC fault, replace IPC fault with RAS fault */
struct dms_sensor_object_cfg g_tscpu_ras_sensors[] = {
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_MAX_CUSTOM, "invalid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), \
};

struct dms_sensor_object_cfg g_aic_ras_sensors[] = {
    /* FaultCode=0x80C98002/0x80C98008/0x80C98009, SensorType="soc_event" */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "aic_ras", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFBF), \

    /* FaultCode=0x80C98809, SensorType="soc_event2" */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_EXTEND_SENSOR, "aic_extend", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFFF), \

    /* FaultCode=0x80C81E01, SensorType="System_Firmware_Error" */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_SYSTEM_FW_PROGRESS, "aic_firmware", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFFF), \

    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_MAX_CUSTOM, "invalid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), \
};

/* as31xm1 without AIV */
struct dms_sensor_object_cfg g_aiv_ras_sensors[] = {
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_MAX_CUSTOM, "invalid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), \
};

struct dms_sensor_object_cfg g_sdma_ras_sensors[] = {
    /* FaultCode=0x80CF8009/0x80CF8008, SensorType="soc_event" */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "sdma_ras", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFBF), \

    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_MAX_CUSTOM, "invalid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), \
};

struct dms_sensor_object_cfg g_ts_ras_sensors[] = {
    /* FaultCode=0x80C78008, SensorType="soc_event" */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "ts_ras", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFBF), \

    /* FaultCode=0x80C78806, SensorType="SOC_Event2" */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_EXTEND_SENSOR, "ts_extend", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFFF), \

    /* FaultCode=0x80C61E00, SensorType="System_Firmware_Error" */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_SYSTEM_FW_PROGRESS, "ts_firmware", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFFF), \

    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_MAX_CUSTOM, "invalid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), \
};

/* as31xm1 without HWTSCPU：HWTS(RISC-V) */
struct dms_sensor_object_cfg g_hwts_cpu_ras_sensors[] = {
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_MAX_CUSTOM, "invalid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), \
};

struct dms_sensor_object_cfg g_tsfw_ras_sensors[] = {
    /* FaultCode=0x812E4E00, SensorType="Heartbeat" */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_HEARTBEAT, "tsfw_heartbeat", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFFF), \

    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_MAX_CUSTOM, "invalid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), \
};

enum ts_dev_node_id get_node_type_id_ipc(DMS_DEVICE_NODE_TYPE node_type)
{
    switch (node_type) {
        case DMS_DEV_TYPE_HWTS_S_TS:
            return TS_DEV_NODE_HWTS;
        case DMS_DEV_TYPE_TSCPU:
            return TS_DEV_NODE_TSCPU;
        case DMS_DEV_TYPE_AIC:
            return TS_DEV_NODE_AIC;
        case DMS_DEV_TYPE_AIV:
            return TS_DEV_NODE_AIV;
        case DMS_DEV_TYPE_SDMA:
            return TS_DEV_NODE_SDMA;
        case DMS_DEV_TYPE_TS:
            return TS_DEV_NODE_TS;
        case DMS_DEV_TYPE_HWTSCPU:
            return TS_DEV_NODE_HWTSCPU;
        case DMS_DEV_TYPE_TSFW:
            return TS_DEV_NODE_TSFW;
        default:
            return TS_DEV_NODE_ID_MAX;
    }
}

struct dfm_struct *tsdrv_get_dfm_struct(enum ts_dev_node_id node_type_id)
{
    static struct dfm_struct g_tsdrv_dfm_structs[TS_DEV_NODE_ID_MAX];

    if (node_type_id >= TS_DEV_NODE_ID_MAX) {
        TSDRV_PRINT_ERR("Invalid node type. (node_type_id=%d)\n", node_type_id);
        return NULL;
    }

    return &g_tsdrv_dfm_structs[node_type_id];
}

int tsdrv_ipc_fault_event_scan(u64 private_data, struct dms_sensor_event_data *data)
{
    int ret;
    struct dfm_struct *dfm = NULL;
    u32 node_id = get_node_id_from_priv(private_data);
    u32 node_type_id = get_dev_node_from_priv(private_data);
    u32 sensor_type = get_sensor_node_from_priv(private_data);

    if (data == NULL) {
        TSDRV_PRINT_ERR("The parameter is invalid, data is NULL.\n");
        return -EINVAL;
    }

    dfm = tsdrv_get_dfm_struct(node_type_id);
    if (dfm == NULL) {
        return -EINVAL;
    }

    ret = dfm_scan_events(dfm, node_id, (u8)sensor_type, data);
    return ret;
}

u32 tsdrv_get_sensor_num(enum ts_dev_node_id node_type_id)
{
    switch (node_type_id) {
        case TS_DEV_NODE_HWTS:
            return (ARRAY_SIZE(g_hwts_ras_sensors) - 1);
        case TS_DEV_NODE_TSCPU:
            return (ARRAY_SIZE(g_tscpu_ras_sensors) - 1);
        case TS_DEV_NODE_AIC:
            return (ARRAY_SIZE(g_aic_ras_sensors) - 1);
        case TS_DEV_NODE_SDMA:
            return (ARRAY_SIZE(g_sdma_ras_sensors) - 1);
        case TS_DEV_NODE_AIV:
            return (ARRAY_SIZE(g_aiv_ras_sensors) - 1);
        case TS_DEV_NODE_TS:
            return (ARRAY_SIZE(g_ts_ras_sensors) - 1);
        case TS_DEV_NODE_HWTSCPU:
            return (ARRAY_SIZE(g_hwts_cpu_ras_sensors) - 1);
        case TS_DEV_NODE_TSFW:
            return (ARRAY_SIZE(g_tsfw_ras_sensors) - 1);
        default:
            TSDRV_PRINT_ERR("Invalid node type. (node_type_id=%d)\n", node_type_id);
            return 0;
    }
}

struct dms_sensor_object_cfg *tsdrv_get_sensor_obj(enum ts_dev_node_id node_type_id)
{
    switch (node_type_id) {
        case TS_DEV_NODE_HWTS:
            return g_hwts_ras_sensors;
        case TS_DEV_NODE_TSCPU:
            return g_tscpu_ras_sensors;
        case TS_DEV_NODE_AIC:
            return g_aic_ras_sensors;
        case TS_DEV_NODE_SDMA:
            return g_sdma_ras_sensors;
        case TS_DEV_NODE_AIV:
            return g_aiv_ras_sensors;
        case TS_DEV_NODE_TS:
            return g_ts_ras_sensors;
        case TS_DEV_NODE_HWTSCPU:
            return g_hwts_cpu_ras_sensors;
        case TS_DEV_NODE_TSFW:
            return g_tsfw_ras_sensors;
        default:
            TSDRV_PRINT_ERR("Invalid node type. (node_type_id=%d)\n", node_type_id);
            return NULL;
    }
}

// Initialize the DFM device nodes on the IPC channel
void tsdrv_ipc_node_para_init(struct dfm_struct *dfm, struct dms_sensor_object_cfg *sensors,
    enum ts_dev_node_id node_type_id, u32 devid, struct dms_node *dev_type)
{
    u32 node_idx, sensor_idx;
    u32 sensor_num = tsdrv_get_sensor_num(node_type_id);
    for (node_idx = 0; node_idx < dfm->node_num; ++node_idx) {
        dfm->dev_nodes[node_idx].node_type = dev_type->node_type;
        dfm->dev_nodes[node_idx].post_proc = NULL;
        dfm->dev_nodes[node_idx].fpdc_notify = NULL;
#ifndef TSDRV_UT
        dfm->dev_nodes[node_idx].get_converage_node = NULL;
#endif

        // Initialize all sensor objects of the device node, including attributes and fault scanning functions.
        for (sensor_idx = 0; sensor_idx < sensor_num; ++sensor_idx) {
            sensors[sensor_idx].private_data =
                SENSOR_PRIV_DATA(devid, node_idx, dev_type->node_type, node_type_id, sensors[sensor_idx].sensor_type);
            dfm->dev_nodes[node_idx].sensors[sensor_idx].dms_sensor = sensors[sensor_idx];
        }
    }
    return;
}

int tsdrv_register_ipc_fault_event(u32 devid)
{
    u32 i, j, sensor_num;
    enum ts_dev_node_id node_type_id;
    struct dfm_struct *dfm = NULL;
    struct dms_node *dev_type = NULL;
    struct dms_sensor_object_cfg *sensors = NULL;
    u32 node_type_num = ARRAY_SIZE(g_tsdrv_dev_type);
    if (devid >= fault_ctrl_get_num()) {
        TSDRV_PRINT_ERR("Invalid devid. (devid=%d)\n", devid);
        return -EINVAL;
    }

    for (i = 0; i < node_type_num; ++i) {
        dev_type = &g_tsdrv_dev_type[i];
        node_type_id = get_node_type_id_ipc(dev_type->node_type);
        sensor_num = tsdrv_get_sensor_num(node_type_id);
        dfm = tsdrv_get_dfm_struct(node_type_id);
        sensors = tsdrv_get_sensor_obj(node_type_id);
        if (dfm == NULL || sensors == NULL) {
            goto _fail;
        }

        if (dfm_struct_init(dfm, dev_type->capacity, sensor_num) != 0) {
#ifndef TSDRV_UT
            dfm_struct_final(dfm, dev_type->capacity, sensor_num);
#endif
            goto _fail;
        }

        if (dfm_duplicate_dms_nodes(dfm, dev_type) != 0) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("Duplicate tsdrv's dms node failed.\n");
            dfm_struct_final(dfm, dev_type->capacity, sensor_num);
#endif
            goto _fail;
        }

        tsdrv_ipc_node_para_init(dfm, sensors, node_type_id, devid, dev_type);

        if (dfm_register_nodes(dfm) != 0) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("Register tsdrv's dms node failed.\n");
            dfm_free_dms_nodes(dfm);
            dfm_struct_final(dfm, dev_type->capacity, sensor_num);
#endif
            goto _fail;
        }
    }
    return 0;

_fail:
#ifndef TSDRV_UT
    for (j = 0; j < i; ++j) {
        node_type_id = get_node_type_id_ipc(g_tsdrv_dev_type[j].node_type);
        dfm = tsdrv_get_dfm_struct(node_type_id);
        dfm_unregister_nodes(dfm);
        dfm_free_dms_nodes(dfm);
        dfm_struct_final(dfm, dev_type->capacity, sensor_num);
    }
#endif
    return -EFAULT;
}

void tsdrv_unregister_ipc_fault_event(u32 devid)
{
    u32 i, sensor_num;
    enum ts_dev_node_id node_type_id;
    struct dfm_struct *dfm = NULL;
    struct dms_node *dev_type = NULL;
    u32 node_type_num = ARRAY_SIZE(g_tsdrv_dev_type);

    for (i = 0; i < node_type_num; ++i) {
        dev_type = &g_tsdrv_dev_type[i];
        node_type_id = get_node_type_id_ipc(dev_type->node_type);
        sensor_num = tsdrv_get_sensor_num(node_type_id);
        dfm = tsdrv_get_dfm_struct(node_type_id);
        if (dfm == NULL) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("Tsdrv dfm unregister error(node_type_id=%u; fault_info_index=%u).\n",
                (u32)node_type_id, i);
            break;
#endif
        }
        dfm_unregister_nodes(dfm);
        dfm_free_dms_nodes(dfm);
        dfm_struct_final(dfm, dev_type->capacity, sensor_num);
    }
    return;
}