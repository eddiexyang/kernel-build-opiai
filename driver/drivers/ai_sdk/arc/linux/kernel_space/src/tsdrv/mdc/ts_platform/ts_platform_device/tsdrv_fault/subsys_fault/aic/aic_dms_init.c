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
#include "aic_dms_init.h"
#include "dms_node_type.h"
#include "aic_safety_smmu.h"
#include "aic_safety_disp.h"
#include "subsys_safety.h"
#include "subsys_fault_define.h"
#include "subsys_log.h"
#include "dfm_safety_report.h"
#include "dfm_report.h"
#include "dms_sensor_type.h"
#include "dms_define.h"
#include "drv_ras_common.h"
#ifndef DRVFAULT_UT
#include "ascend_kernel_hal.h"
#include "drv_ipc.h"
#include "tsdrv_ipc.h"
#include "tsdrv_common_fault_platform.h"
int tsdrv_send_fault_info_to_ts(u32 devid, rproc_id_t rproc_id, struct ras_message *info);
#endif

STATIC int aic_dms_ops_init(struct dms_node *device)
{
    subsys_debug("DEBUG (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return 0;
}

STATIC void aic_dms_ops_uninit(struct dms_node *device)
{
    subsys_debug("DEBUG (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return;
}

STATIC struct dms_node_operations g_aic_dms_ops = {
    .init = aic_dms_ops_init,
    .uninit = aic_dms_ops_uninit,
    .scan = NULL,
    .fault_diag = NULL,
    .event_notify = NULL,
    .get_link_state = NULL,
    .set_link_state = NULL
};

STATIC struct dfm_struct g_aic_dms;

#define AIC_DMS_NODE(_idx) { \
    .node_type = DMS_DEV_TYPE_AIC, \
    .node_id = _idx, \
    .node_name = "AIC-" #_idx, \
    .capacity = 0x1, \
    .permission = 0x1, \
    .owner_devid = 0, \
    .owner_device = NULL, \
    .ops = &g_aic_dms_ops, \
}

static struct dms_node g_aic_dms_node[AIC_NODE_NUM] = {
    [0] = AIC_DMS_NODE(0),
    [1] = AIC_DMS_NODE(1),
    [2] = AIC_DMS_NODE(2),
    [3] = AIC_DMS_NODE(3),
    [4] = AIC_DMS_NODE(4),
    [5] = AIC_DMS_NODE(5),
    [6] = AIC_DMS_NODE(6),
    [7] = AIC_DMS_NODE(7),
    [8] = AIC_DMS_NODE(8),
    [9] = AIC_DMS_NODE(9),
};


#define AIC_SUBMODULE_NODE(_sub_node_type, _sub_name, _idx) { \
    .node_type = _sub_node_type,                  \
    .node_id = _idx,                                 \
    .node_name = "AIC-" #_sub_name #_idx,         \
    .capacity = 0x1,                              \
    .permission = 0x1,                            \
    .owner_devid = 0,                             \
    .owner_device = &g_aic_dms_node[_idx],          \
    .ops = &g_aic_dms_ops,                        \
}

#define AIC_ONE_SUB_MODULS_IN_ONE(_idx) \
    [(_idx * AIC_SUB_NODE_NUM_PER)] = AIC_SUBMODULE_NODE(DMS_DEV_TYPE_AIC_DISP, "DISP", _idx), \
    [(_idx * AIC_SUB_NODE_NUM_PER + 1)] = AIC_SUBMODULE_NODE(DMS_DEV_TYPE_AIC_SMMU, "SMMU", _idx)

static struct dms_node g_aic_submodule_nodes[AIC_SUB_NODE_NUM] = {
    AIC_ONE_SUB_MODULS_IN_ONE(0),
    AIC_ONE_SUB_MODULS_IN_ONE(1),
    AIC_ONE_SUB_MODULS_IN_ONE(2),
    AIC_ONE_SUB_MODULS_IN_ONE(3),
    AIC_ONE_SUB_MODULS_IN_ONE(4),
    AIC_ONE_SUB_MODULS_IN_ONE(5),
    AIC_ONE_SUB_MODULS_IN_ONE(6),
    AIC_ONE_SUB_MODULS_IN_ONE(7),
    AIC_ONE_SUB_MODULS_IN_ONE(8),
    AIC_ONE_SUB_MODULS_IN_ONE(9),
};

STATIC const struct ras_fault_converge_item *aic_safety_converge(u32 section_type, u64 ras_code)
{
    subsys_err("safety report converge not supported.\n");
    return NULL;
}

STATIC int aic_scan_event(u64 private_data, struct dms_sensor_event_data *data)
{
    u32 node_id;
    u32 sensor_type;

    node_id = ((private_data >> DEV_NODE_OFFSET) & DEV_NODE_MASK);
    sensor_type = ((private_data >> SENSOR_NODE_OFFSET) & SENSOR_NODE_MASK);

    if (node_id >= g_aic_dms.node_num) {
        subsys_err("invalid node id. (node_id=%u, priv=0x%llx)\n", node_id, private_data);
        return -EINVAL;
    }

    (void)dfm_scan_events(&g_aic_dms, node_id, sensor_type, data);
    return 0;
}

STATIC int aic_dms_register_dms_node(u32 dev_id)
{
    struct dms_sensor_object_cfg sensor_cfg[] = {
        SUBSYS_SENOR_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "aicsubsys", DMS_DISCRETE_SENSOR_CLASS,
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, DMS_SENSOR_CHECK_INTERVAL_TIME,
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, aic_scan_event,
            SENSOR_PRIV_DATA(0, DMS_SEN_TYPE_RAS_SENSOR), 0xFFFF, 0xFFBF),
        SUBSYS_SENOR_OBJ(DMS_SEN_TYPE_SAFETY_SENSOR, "aicsubsys", DMS_DISCRETE_SENSOR_CLASS,
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, DMS_SENSOR_CHECK_INTERVAL_TIME,
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, aic_scan_event,
            SENSOR_PRIV_DATA(0, DMS_SEN_TYPE_SAFETY_SENSOR), 0xFFFF, 0xFFFF),
        SUBSYS_SENOR_OBJ(DMS_SEN_TYPE_CHECK_SENSOR, "aicsubsys", DMS_DISCRETE_SENSOR_CLASS,
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, DMS_SENSOR_CHECK_INTERVAL_TIME,
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, aic_scan_event,
            SENSOR_PRIV_DATA(0, DMS_SEN_TYPE_CHECK_SENSOR), 0xFFFF, 0xFFFF),
    };
    u32 sensor_num = (u32)ARRAY_SIZE(sensor_cfg);
    u32 i;
    u32 sensor_idx;
    g_aic_dms.dev_id = dev_id;

    if (dfm_struct_init(&g_aic_dms, AIC_SUB_NODE_NUM, sensor_num) != 0) {
        subsys_err("init aicsubsys's dfm data failed\n");
        goto _fail;
    }

    for (i = 0; i < AIC_SUB_NODE_NUM; ++i) {
        g_aic_dms.dev_nodes[i].node = &g_aic_submodule_nodes[i];
        g_aic_dms.dev_nodes[i].post_proc = NULL;
        g_aic_dms.dev_nodes[i].fpdc_notify = NULL;
        g_aic_dms.dev_nodes[i].get_converage_node = aic_safety_converge;
        for (sensor_idx = 0; sensor_idx < sensor_num; ++sensor_idx) {
            sensor_cfg[sensor_idx].private_data = SENSOR_PRIV_DATA(i, sensor_cfg[sensor_idx].sensor_type);
            g_aic_dms.dev_nodes[i].sensors[sensor_idx].dms_sensor = sensor_cfg[sensor_idx];
        }
    }

    if (dfm_register_nodes(&g_aic_dms) != 0) {
        subsys_err("register aic subsys dms node failed\n");
        goto _fail;
    }
    return 0;

_fail:
    dfm_struct_final(&g_aic_dms, AIC_SUB_NODE_NUM, sensor_num);
    return -EFAULT;
}

STATIC int aic_add_dfm_event(u32 devid, u32 node_id, const dfm_event *event)
{
    int ret, ret_send;
    u32 node_idx, sub_node_type;
#ifndef DRVFAULT_UT
    struct ras_message info = {};
#endif

    switch (event->module_id) {
        case HISI_MODULE_SMMU:
            node_idx = AIC_SUB_NODE_NUM_PER * node_id + AIC_SUB_SMMU_NODE_INDEX;
            sub_node_type = DMS_DEV_TYPE_AIC_SMMU;
            break;
        case HISI_MODULE_DISP:
            node_idx = AIC_SUB_NODE_NUM_PER * node_id + AIC_SUB_DISP_NODE_INDEX;
            sub_node_type = DMS_DEV_TYPE_AIC_DISP;
            break;
        default:
            subsys_err("invalid submodule id.(subsys_id=%u,module id=%u)\n", event->subsys_id, event->module_id);
            return -EINVAL;
    }

    subsys_debug("report safety event.(subsys_id=%u,module id=%u, node id=%u)\n",
        event->subsys_id, event->module_id, node_id);
    ret = dfm_add_event(&g_aic_dms, node_idx, event);
    if (ret == 0) {
        subsys_notify_sensor_scan(&g_aic_dms, node_idx, event->sensor_type);
    }
#ifndef DRVFAULT_UT
    info.sub_node_type = sub_node_type;
    info.node_id = node_id;
    info.node_type = DMS_DEV_TYPE_AIC;
    info.sub_node_id = 0;
    info.sensor_type = event->sensor_type;
    info.sensor_status = event->error_type;

    ret_send = tsdrv_send_fault_info_to_ts(0, HISI_RPROC_TSC_TX_RPID1, &info);
    subsys_info("send aic ipc mesg."
        "(node_id=%u,node_type=%u, sub_node_id=%u, sub_node_type=%u, sensor_type=%u, sensor_status=%u, ret=%d)\n",
        info.node_id, info.node_type, info.sub_node_id, info.sub_node_type, info.sensor_type,
        info.sensor_status, ret_send);
#endif

    return ret;
}

#define AIC_SAFETY_ONE_NODE(_idx, disp_init_safety_func, smmu_init_safety_func) \
    SUBSYS_SAFETY_CFG_OBJ( \
        HISI_SUBSYS_AICORESUB, HISI_MODULE_DISP, DMS_DEV_TYPE_AIC, _idx, DMS_DEV_TYPE_AIC_DISP, 0, \
        aic_add_dfm_event, disp_init_safety_func, subsys_report_safety_fault), \
    SUBSYS_SAFETY_CFG_OBJ( \
        HISI_SUBSYS_AICORESUB, HISI_MODULE_SMMU, DMS_DEV_TYPE_AIC, _idx, DMS_DEV_TYPE_AIC_SMMU, 0, \
        aic_add_dfm_event, smmu_init_safety_func, subsys_report_safety_fault)

STATIC subsys_safety_cfg_t g_aic_safety_cfg[] = {
    AIC_SAFETY_ONE_NODE(0, aic_disp_init_safety_cfg0, aic_smmu_init_safety_cfg0),
    AIC_SAFETY_ONE_NODE(1, aic_disp_init_safety_cfg1, aic_smmu_init_safety_cfg1),
    AIC_SAFETY_ONE_NODE(2, aic_disp_init_safety_cfg2, aic_smmu_init_safety_cfg2),
    AIC_SAFETY_ONE_NODE(3, aic_disp_init_safety_cfg3, aic_smmu_init_safety_cfg3),
    AIC_SAFETY_ONE_NODE(4, aic_disp_init_safety_cfg4, aic_smmu_init_safety_cfg4),
    AIC_SAFETY_ONE_NODE(5, aic_disp_init_safety_cfg5, aic_smmu_init_safety_cfg5),
    AIC_SAFETY_ONE_NODE(6, aic_disp_init_safety_cfg6, aic_smmu_init_safety_cfg6),
    AIC_SAFETY_ONE_NODE(7, aic_disp_init_safety_cfg7, aic_smmu_init_safety_cfg7),
    AIC_SAFETY_ONE_NODE(8, aic_disp_init_safety_cfg8, aic_smmu_init_safety_cfg8),
    AIC_SAFETY_ONE_NODE(9, aic_disp_init_safety_cfg9, aic_smmu_init_safety_cfg9),
};

STATIC int aic_safety_init(void)
{
    int cfg_num = ARRAY_SIZE(g_aic_safety_cfg);
    int ret;

    ret = subsys_safety_init(DMS_DEV_TYPE_AIC, g_aic_safety_cfg, cfg_num);
    if (ret != 0) {
        subsys_err("init subsys safety fault manager failed.(ret=%d)\n", ret);
        return ret;
    }

    subsys_info("init AIC fault safety manager success.\n");
    return 0;
}

STATIC void aic_safety_uninit(void)
{
    int ret;

    ret = subsys_safety_uninit(DMS_DEV_TYPE_AIC);
    if (ret != 0) {
        subsys_err("register fault notifier failed.(ret=%d)\n", ret);
    } else {
        subsys_info("register fault notifier success.\n");
    }
}

int aic_safety_manager_init(u32 devid)
{
    if (aic_dms_register_dms_node(devid) != 0) {
        subsys_err("register aicsubsys device node failed. devid(%u)\n", devid);
        return -EFAULT;
    }

    if (aic_safety_init() != 0) {
        subsys_err("init AIC safety manager feature failed.\n");
        return -EFAULT;
    }

    return 0;
}

void aic_safety_manager_uninit(u32 devid)
{
    aic_safety_uninit();
    dfm_unregister_nodes(&g_aic_dms);
    dfm_struct_final(&g_aic_dms, AIC_SUB_NODE_NUM, 0);
}
#else
void aic_safety_manager_uninit(int devid)
{
    return 0;
}
#endif
