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
* Create: 2022-9-20
*/
#ifndef TSDRV_UT
#include <linux/sizes.h>
#include <linux/slab.h>
#include <linux/errno.h>

#include "dfm_dev_register.h"
#include "fpdc.h"
#include "ts_dms_init.h"
#include "dms_node_type.h"
#include "ts_safety_smmu.h"
#include "ts_safety_disp.h"
#include "ts_safety_sche.h"
#include "subsys_safety.h"
#include "subsys_fault_define.h"
#include "subsys_log.h"
#include "dfm_safety_report.h"
#include "dfm_report.h"
#include "dms_sensor_type.h"
#include "dms_define.h"
#include "drv_ras_common.h"

STATIC int ts_dms_ops_init(struct dms_node *device)
{
    subsys_debug("DEBUG (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return 0;
}

STATIC void ts_dms_ops_uninit(struct dms_node *device)
{
    subsys_debug("DEBUG (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return;
}

STATIC struct dms_node_operations g_ts_dms_ops = {
    .init = ts_dms_ops_init,
    .uninit = ts_dms_ops_uninit,
    .scan = NULL,
    .fault_diag = NULL,
    .event_notify = NULL,
    .get_link_state = NULL,
    .set_link_state = NULL
};

STATIC struct dfm_struct g_ts_dms;

static struct dms_node g_ts_dms_node = {
    .node_type = DMS_DEV_TYPE_TS,
    .node_id = TS_NODE_0,
    .node_name = "TSSUBSYS",
    .capacity = 0x1,
    .permission = 0x1,
    .owner_devid = 0,
    .owner_device = NULL,
    .ops = &g_ts_dms_ops,
};

#define TS_SUBMODULE_NODE(_sub_node_type, _sub_name) { \
    .node_type = _sub_node_type,                  \
    .node_id = 0,                                 \
    .node_name = "TSSUBSYS-" #_sub_name,         \
    .capacity = 0x1,                              \
    .permission = 0x1,                            \
    .owner_devid = 0,                             \
    .owner_device = &g_ts_dms_node,          \
    .ops = &g_ts_dms_ops,                        \
}

#define TS_SUB_DISP_NODE_INDEX  (0)
#define TS_SUB_SCHE_NODE_INDEX  (1)
#define TS_SUB_SMMU_NODE_INDEX  (2)

static struct dms_node g_ts_submodule_nodes[TS_SUB_NODE_NUM] = {
    TS_SUBMODULE_NODE(DMS_DEV_TYPE_TS_DISP, "DISP"),
    TS_SUBMODULE_NODE(DMS_DEV_TYPE_TS_SCHE, "SCHE"),
    TS_SUBMODULE_NODE(DMS_DEV_TYPE_TS_SMMU, "SMMU"),
};

STATIC const struct ras_fault_converge_item *ts_safety_converge(u32 section_type, u64 ras_code)
{
    subsys_err("safety report converge not supported.\n");
    return NULL;
}

STATIC int ts_scan_event(u64 private_data, struct dms_sensor_event_data *data)
{
    u32 node_id;
    u32 sensor_type;

    node_id = ((private_data >> DEV_NODE_OFFSET) & DEV_NODE_MASK);
    sensor_type = ((private_data >> SENSOR_NODE_OFFSET) & SENSOR_NODE_MASK);

    if (node_id >= g_ts_dms.node_num) {
        subsys_err("invalid node id. (node_id=%u, priv=0x%llx)\n", node_id, private_data);
        return -EINVAL;
    }

    (void)dfm_scan_events(&g_ts_dms, node_id, sensor_type, data);
    return 0;
}

STATIC int ts_dms_register_dms_node(u32 dev_id)
{
    struct dms_sensor_object_cfg sensor_cfg[] = {
        SUBSYS_SENOR_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "tssubsys", DMS_DISCRETE_SENSOR_CLASS,
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, DMS_SENSOR_CHECK_INTERVAL_TIME,
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, ts_scan_event,
            SENSOR_PRIV_DATA(0, DMS_SEN_TYPE_RAS_SENSOR), 0xFFFF, 0xFFBF),
        SUBSYS_SENOR_OBJ(DMS_SEN_TYPE_SAFETY_SENSOR, "tssubsys", DMS_DISCRETE_SENSOR_CLASS,
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, DMS_SENSOR_CHECK_INTERVAL_TIME,
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, ts_scan_event,
            SENSOR_PRIV_DATA(0, DMS_SEN_TYPE_SAFETY_SENSOR), 0xFFFF, 0xFFFF),
        SUBSYS_SENOR_OBJ(DMS_SEN_TYPE_CHECK_SENSOR, "tssubsys", DMS_DISCRETE_SENSOR_CLASS,
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, DMS_SENSOR_CHECK_INTERVAL_TIME,
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, ts_scan_event,
            SENSOR_PRIV_DATA(0, DMS_SEN_TYPE_CHECK_SENSOR), 0xFFFF, 0xFFFF),
    };
    u32 sensor_num = (u32)ARRAY_SIZE(sensor_cfg);
    u32 i;
    u32 sensor_idx;
    g_ts_dms.dev_id = dev_id;

    if (dfm_struct_init(&g_ts_dms, TS_SUB_NODE_NUM, sensor_num) != 0) {
        subsys_err("init tssubsys's dfm data failed\n");
        goto _fail;
    }

    for (i = 0; i < TS_SUB_NODE_NUM; ++i) {
        g_ts_dms.dev_nodes[i].node = &g_ts_submodule_nodes[i];
        g_ts_dms.dev_nodes[i].post_proc = NULL;
        g_ts_dms.dev_nodes[i].fpdc_notify = NULL;
        g_ts_dms.dev_nodes[i].get_converage_node = ts_safety_converge;
        for (sensor_idx = 0; sensor_idx < sensor_num; ++sensor_idx) {
            sensor_cfg[sensor_idx].private_data = SENSOR_PRIV_DATA(i, sensor_cfg[sensor_idx].sensor_type);
            g_ts_dms.dev_nodes[i].sensors[sensor_idx].dms_sensor = sensor_cfg[sensor_idx];
        }
    }

    if (dfm_register_nodes(&g_ts_dms) != 0) {
        subsys_err("register ts subsys dms node failed\n");
        goto _fail;
    }
    return 0;

_fail:
    dfm_struct_final(&g_ts_dms, TS_SUB_NODE_NUM, sensor_num);
    return -EFAULT;
}

STATIC int ts_add_dfm_event(u32 devid, u32 node_id, const dfm_event *event)
{
    int ret;
    u32 node_idx;

    switch (event->module_id) {
        case HISI_MODULE_DISP:
            node_idx = TS_SUB_DISP_NODE_INDEX;
            break;
        case HISI_MODULE_SCH:
            node_idx = TS_SUB_SCHE_NODE_INDEX;
            break;
        case HISI_MODULE_SMMU:
            node_idx = TS_SUB_SMMU_NODE_INDEX;
            break;
        default:
            subsys_err("invalid submodule id.(subsys_id=%u,module id=%u)\n", event->subsys_id, event->module_id);
            return -EINVAL;
    }

    subsys_debug("report safety event.(subsys_id=%u,module id=%u, node id=%u)\n",
        event->subsys_id, event->module_id, node_id);
    ret = dfm_add_event(&g_ts_dms, node_idx, event);
    if (ret == 0) {
        subsys_notify_sensor_scan(&g_ts_dms, node_idx, event->sensor_type);
    }

    return ret;
}

STATIC subsys_safety_cfg_t g_ts_safety_cfg[] = {
    SUBSYS_SAFETY_CFG_OBJ(
        HISI_SUBSYS_TSSUB, HISI_MODULE_DISP, DMS_DEV_TYPE_TS_DISP, TS_NODE_0, DMS_DEV_TYPE_TS_DISP, 0,
        ts_add_dfm_event, ts_disp_init_safety_cfg, subsys_report_safety_fault),
    SUBSYS_SAFETY_CFG_OBJ(
        HISI_SUBSYS_TSSUB, HISI_MODULE_SCH, DMS_DEV_TYPE_TS_SCHE, TS_NODE_0, DMS_DEV_TYPE_TS_SCHE, 0,
        ts_add_dfm_event, ts_sche_init_safety_cfg, subsys_report_safety_fault),
    SUBSYS_SAFETY_CFG_OBJ(
        HISI_SUBSYS_TSSUB, HISI_MODULE_SMMU, DMS_DEV_TYPE_TS_SMMU, TS_NODE_0, DMS_DEV_TYPE_TS_SMMU, 0,
        ts_add_dfm_event, ts_smmu_init_safety_cfg, subsys_report_safety_fault),
};

STATIC int ts_safety_init(void)
{
    int cfg_num = ARRAY_SIZE(g_ts_safety_cfg);
    int ret;

    ret = subsys_safety_init(DMS_DEV_TYPE_TS, g_ts_safety_cfg, cfg_num);
    if (ret != 0) {
        subsys_err("init subsys safety fault manager failed. (ret=%d)\n", ret);
        return ret;
    }

    subsys_info("init TS fault safety manager success.\n");
    return 0;
}

STATIC void ts_safety_uninit(void)
{
    int ret;

    ret = subsys_safety_uninit(DMS_DEV_TYPE_TS);
    if (ret != 0) {
        subsys_err("register fault notifier failed.(ret=%d)\n", ret);
    } else {
        subsys_info("register fault notifier success.\n");
    }
}

int ts_safety_manager_init(u32 devid)
{
    if (ts_dms_register_dms_node(devid) != 0) {
        subsys_err("register tssubsys device node failed. devid(%u)\n", devid);
        return -EFAULT;
    }

    if (ts_safety_init() != 0) {
        subsys_err("init TS safety manager feature failed.\n");
        return -EFAULT;
    }

    return 0;
}

void ts_safety_manager_uninit(u32 devid)
{
    ts_safety_uninit();
    dfm_unregister_nodes(&g_ts_dms);
    dfm_struct_final(&g_ts_dms, TS_SUB_NODE_NUM, 0);
}
#else
void ts_safety_manager_uninit(int devid)
{
    return;
}
#endif
