/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-12-2
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
#include "tsdrv_subsys_safety_ras.h"

/* Each device type provides only one node_id information,
    which will be extended by using dfm_duplicate_dms_nodes. */
STATIC struct dms_node g_tsdrv_dev_type[] = {
    GEN_DEV_NODE_TABLE_OBJ(DMS_DEV_TYPE_HWTS_S_TS, 0, "HWTS", 2, 0x1, 0, &tsdrv_dev_node_ops), \
    GEN_DEV_NODE_TABLE_OBJ(DMS_DEV_TYPE_TSCPU, 0, "TSCPU", 2, 0x1, 0, &tsdrv_dev_node_ops), \
    GEN_DEV_NODE_TABLE_OBJ(DMS_DEV_TYPE_AIC, 0, "AIC", 10, 0x1, 0, &tsdrv_dev_node_ops), \
    GEN_DEV_NODE_TABLE_OBJ(DMS_DEV_TYPE_SDMA, 0, "SDMA", 16, 0x1, 0, &tsdrv_dev_node_ops), \
    GEN_DEV_NODE_TABLE_OBJ(DMS_DEV_TYPE_AIV, 0, "AIV", 8, 0x1, 0, &tsdrv_dev_node_ops), \
    GEN_DEV_NODE_TABLE_OBJ(DMS_DEV_TYPE_TS, 0, "TS", 2, 0x1, 0, &tsdrv_dev_node_ops ), \
    GEN_DEV_NODE_TABLE_OBJ(DMS_DEV_TYPE_HWTSCPU, 0, "HWTSCPU", 2, 0x1, 0, &tsdrv_dev_node_ops ), \
    GEN_DEV_NODE_TABLE_OBJ(DMS_DEV_TYPE_TSFW, 0, "TSFW", 2, 0x1, 0, &tsdrv_dev_node_ops ), \
};

STATIC struct dms_sensor_object_cfg g_hwts_ras_sensors[] = {
    /* soc_event */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "hwts_ras", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFBF), \
    /* soc_event1 */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_SAFETY_SENSOR, "hwts_safety", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFFF), \
    /* soc_event2 */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_EXTEND_SENSOR, "hwts_extend", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFFF), \

    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_MAX_CUSTOM, "invalid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), \
};

STATIC struct dms_sensor_object_cfg g_tscpu_ras_sensors[] = {
    /* soc_event */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "tscpu_ras", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFBF), \
    /* soc_event1 */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_SAFETY_SENSOR, "tscpu_safety", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFFF), \

    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_HEARTBEAT, "tscpu_heartbeat", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFFF), \

    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_MAX_CUSTOM, "invalid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), \
};

STATIC struct dms_sensor_object_cfg g_aic_ras_sensors[] = {
    /* soc_event */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "aic_ras", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFBF), \
    /* soc_event1 */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_SAFETY_SENSOR, "aic_safety", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFFF), \
    /* soc_event2 */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_EXTEND_SENSOR, "aic_extend", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFFF), \
    /* soc_event7 */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_MODULE_SENSOR2, "aic_module2", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFFF), \

    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_SYSTEM_FW_PROGRESS, "aic_firmware", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFFF), \

    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_MAX_CUSTOM, "invalid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), \
};

STATIC struct dms_sensor_object_cfg g_aiv_ras_sensors[] = {
    /* soc_event */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "aiv_ras", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFBF), \
    /* soc_event1 */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_SAFETY_SENSOR, "aiv_safety", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFFF), \
    /* soc_event2 */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_EXTEND_SENSOR, "aiv_extend", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFFF), \

    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_SYSTEM_FW_PROGRESS, "aiv_firmware", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFFF), \

    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_MAX_CUSTOM, "invalid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), \
};

STATIC struct dms_sensor_object_cfg g_sdma_ras_sensors[] = {
    /* soc_event */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "sdma_ras", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFBF), \

    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_MAX_CUSTOM, "invalid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), \
};

STATIC struct dms_sensor_object_cfg g_ts_ras_sensors[] = {
    /* soc_event */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "ts_ras", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFBF), \
    /* soc_event2 */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_EXTEND_SENSOR, "ts_extend", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFFF), \

    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_SYSTEM_FW_PROGRESS, "ts_firmware", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFFF), \

    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_MAX_CUSTOM, "invalid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), \
};

STATIC struct dms_sensor_object_cfg g_hwts_cpu_ras_sensors[] = {
    /* soc_event1 */ \
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "hwtscpu_ras", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFBF), \

    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_MAX_CUSTOM, "invalid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), \
};

STATIC struct dms_sensor_object_cfg g_tsfw_ras_sensors[] = {

    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_HEARTBEAT, "tsfw_heartbeat", DMS_DISCRETE_SENSOR_CLASS, \
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_IPC_FAULT_EVENT_SCAN, \
        0, 0xFFFF, 0xFFFF), \

    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_MAX_CUSTOM, "invalid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), \
};

enum mdc_dev_node_id get_node_type_id_ipc(DMS_DEVICE_NODE_TYPE node_type)
{
    switch (node_type) {
        case DMS_DEV_TYPE_HWTS_S_TS:
            return MDC_DEV_NODE_HWTS;
        case DMS_DEV_TYPE_TSCPU:
            return MDC_DEV_NODE_TSCPU;
        case DMS_DEV_TYPE_AIC:
            return MDC_DEV_NODE_AIC;
        case DMS_DEV_TYPE_AIV:
            return MDC_DEV_NODE_AIV;
        case DMS_DEV_TYPE_SDMA:
            return MDC_DEV_NODE_SDMA;
        case DMS_DEV_TYPE_TS:
            return MDC_DEV_NODE_TS;
        case DMS_DEV_TYPE_HWTSCPU:
            return MDC_DEV_NODE_HWTSCPU;
        case DMS_DEV_TYPE_TSFW:
            return MDC_DEV_NODE_TSFW;
        default:
            return MDC_DEV_NODE_ID_MAX;
    }
}

struct dfm_struct *tsdrv_get_dfm_struct(enum mdc_dev_node_id node_type_id)
{
    static struct dfm_struct g_tsdrv_dfm_structs[MDC_DEV_NODE_ID_MAX];

    if (node_type_id >= MDC_DEV_NODE_ID_MAX) {
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

STATIC u32 tsdrv_get_sensor_num(enum mdc_dev_node_id node_type_id)
{
    switch (node_type_id) {
        case MDC_DEV_NODE_HWTS:
            return (ARRAY_SIZE(g_hwts_ras_sensors) - 1);
        case MDC_DEV_NODE_TSCPU:
            return (ARRAY_SIZE(g_tscpu_ras_sensors) - 1);
        case MDC_DEV_NODE_AIC:
            return (ARRAY_SIZE(g_aic_ras_sensors) - 1);
        case MDC_DEV_NODE_SDMA:
            return (ARRAY_SIZE(g_sdma_ras_sensors) - 1);
        case MDC_DEV_NODE_AIV:
            return (ARRAY_SIZE(g_aiv_ras_sensors) - 1);
        case MDC_DEV_NODE_TS:
            return (ARRAY_SIZE(g_ts_ras_sensors) - 1);
        case MDC_DEV_NODE_HWTSCPU:
            return (ARRAY_SIZE(g_hwts_cpu_ras_sensors) - 1);
        case MDC_DEV_NODE_TSFW:
            return (ARRAY_SIZE(g_tsfw_ras_sensors) - 1);
        default:
            TSDRV_PRINT_ERR("Invalid node type. (node_type_id=%d)\n", node_type_id);
            return 0;
    }
}

STATIC struct dms_sensor_object_cfg *tsdrv_get_sensor_obj(enum mdc_dev_node_id node_type_id)
{
    switch (node_type_id) {
        case MDC_DEV_NODE_HWTS:
            return g_hwts_ras_sensors;
        case MDC_DEV_NODE_TSCPU:
            return g_tscpu_ras_sensors;
        case MDC_DEV_NODE_AIC:
            return g_aic_ras_sensors;
        case MDC_DEV_NODE_SDMA:
            return g_sdma_ras_sensors;
        case MDC_DEV_NODE_AIV:
            return g_aiv_ras_sensors;
        case MDC_DEV_NODE_TS:
            return g_ts_ras_sensors;
        case MDC_DEV_NODE_HWTSCPU:
            return g_hwts_cpu_ras_sensors;
        case MDC_DEV_NODE_TSFW:
            return g_tsfw_ras_sensors;
        default:
            TSDRV_PRINT_ERR("Invalid node type. (node_type_id=%d)\n", node_type_id);
            return NULL;
    }
}
struct tsdrv_safety_proc {
    dfm_get_event_coverage converge;
};

STATIC struct tsdrv_safety_proc g_tsdrv_safety_converge_func[MDC_DEV_NODE_ID_MAX] = {
    {.converge = NULL}, {.converge = NULL},
    {.converge = ts_aic_safety_converge}, {.converge = NULL},
    {.converge = ts_aiv_safety_converge}, {.converge = NULL},
    {.converge = NULL}, {.converge = NULL},
};

STATIC void tsdrv_ipc_node_para_init(struct dfm_struct *dfm, struct dms_sensor_object_cfg *sensors,
    enum mdc_dev_node_id node_type_id, u32 devid)
{
    u32 node_idx, sensor_idx;
    u32 sensor_num = tsdrv_get_sensor_num(node_type_id);
    bool is_ts_subsys = (node_type_id == MDC_DEV_NODE_TSCPU) ? true : false;
    for (node_idx = 0; node_idx < dfm->node_num; ++node_idx) {
        dfm->dev_nodes[node_idx].post_proc = NULL;
        dfm->dev_nodes[node_idx].fpdc_notify = NULL;
        if (tsdrv_get_chip_type() == CHIP_TYPE_MDC_BS9SX1A) {
            dfm->dev_nodes[node_idx].get_converage_node = g_tsdrv_safety_converge_func[node_type_id].converge;
            if (is_ts_subsys) {
                dfm->dev_nodes[node_idx].coverage_version = EVENT_COVERAGE_V2;
                dfm->dev_nodes[node_idx].get_converage_node2 = ts_subsys_safety_ras_converge;
                dfm->dev_nodes[node_idx].fpdc_notify = tsdrv_subsys_ras_fpdc_handler;
            }
        } else {
#ifndef TSDRV_UT
            dfm->dev_nodes[node_idx].get_converage_node = NULL;
#endif
        }

        for (sensor_idx = 0; sensor_idx < sensor_num; ++sensor_idx) {
            sensors[sensor_idx].private_data =
                SENSOR_PRIV_DATA(devid, node_idx, node_type_id, sensors[sensor_idx].sensor_type);
            dfm->dev_nodes[node_idx].sensors[sensor_idx].dms_sensor = sensors[sensor_idx];
        }
    }
    return;
}

int tsdrv_register_ipc_fault_event(u32 devid)
{
    u32 i, j, sensor_num;
    enum mdc_dev_node_id node_type_id;
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

        tsdrv_ipc_node_para_init(dfm, sensors, node_type_id, devid);

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
    enum mdc_dev_node_id node_type_id;
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