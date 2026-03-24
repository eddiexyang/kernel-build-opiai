/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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

#include <linux/sizes.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

#include "soc_misc_spec.h"
#include "dms_dev_node.h"
#include "dms_sensor_type.h"
#include "soc_misc_init.h"
#include "dfm_report.h"
#include "l3d_config.h"
#include "l3d_safety.h"
#include "l3d_irq_ras.h"
#include "l3d_arm_ras.h"

struct dfm_struct g_l3d_dfm[SOC_MISC_DEVICE_NUM_MAX];

int soc_misc_l3d_ops_init(struct dms_node *device)
{
    soc_misc_drv_debug("DEBUG (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return 0;
}

void soc_misc_l3d_ops_uninit(struct dms_node *device)
{
    soc_misc_drv_debug("DEBUG (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return;
}

STATIC struct dms_node_operations g_soc_misc_l3d_ops = {
    .init = soc_misc_l3d_ops_init,
    .uninit = soc_misc_l3d_ops_uninit,
    .scan = NULL,
    .fault_diag = NULL,
    .event_notify = NULL,
    .get_link_state = NULL,
    .set_link_state = NULL
};

STATIC void l3d_set_dms_node(struct dms_node *node, u32 dev_id, u32 node_id)
{
    node->node_type = DMS_DEV_TYPE_L3D;
    node->node_id = node_id;
    (void)sprintf_s(node->node_name,  DMS_MAX_DEV_NAME_LEN, "L3D-%d-%d", dev_id, node_id);
    node->capacity = 0x01;
    node->permission = 0x01;
    node->owner_devid = dev_id;
    node->owner_device = NULL;
    node->ops = &g_soc_misc_l3d_ops;
}

int l3d_dms_node_init(void)
{
    static struct dms_node g_l3d_dms_node[SOC_MISC_DEVICE_NUM_MAX][L3D_NODE_NUM];
    struct dms_sensor_object_cfg sensor_cfg[] = L3D_DMS_SENSORS;
    struct dfm_struct *dfm = NULL;
    u32 sensor_num = (u32)ARRAY_SIZE(sensor_cfg);
    u32 dev_id, node_id;
    u32 sensor_idx;
    int i = 0;

    for (dev_id = 0; dev_id < SOC_MISC_DEVICE_NUM_MAX; dev_id++) {
        g_l3d_dfm[dev_id].dev_id = dev_id;
        dfm = &g_l3d_dfm[dev_id];

        if (dfm_struct_init(&g_l3d_dfm[dev_id], L3D_NODE_NUM, sensor_num) != 0) {
            soc_misc_drv_err("init l3d dfm data failed.\n");
            goto _fail;
        }

        for (node_id = 0; node_id < L3D_NODE_NUM; node_id++) {
            l3d_set_dms_node(&g_l3d_dms_node[dev_id][node_id], dev_id, node_id);
            dfm->dev_nodes[node_id].node = &g_l3d_dms_node[dev_id][node_id];
            dfm->dev_nodes[node_id].post_proc = NULL;
            dfm->dev_nodes[node_id].fpdc_notify = NULL;
            dfm->dev_nodes[node_id].get_converage_node = NULL;
            for (sensor_idx = 0; sensor_idx < sensor_num; ++sensor_idx) {
                sensor_cfg[sensor_idx].private_data = SOC_MISC_SENSOR_PRIV_DATA(dev_id,
                    sensor_cfg[sensor_idx].sensor_type, node_id);
                dfm->dev_nodes[node_id].sensors[sensor_idx].dms_sensor = sensor_cfg[sensor_idx];
            }
        }

        if (dfm_register_nodes(dfm) != 0) {
            dfm_struct_final(dfm, L3D_NODE_NUM, sensor_num);
            soc_misc_drv_err("register l3d dms node failed.\n");
            goto _fail;
        }
    }
    return 0;

_fail:
    for (i = (int)dev_id - 1; i >= 0; i--) {
        dfm_unregister_nodes(&g_l3d_dfm[i]);
        dfm_struct_final(&g_l3d_dfm[i], L3D_NODE_NUM, sensor_num);
    }

    return -EFAULT;
}

void l3d_dms_node_uninit(void)
{
    u32 dev_id;

    for (dev_id = 0; dev_id < SOC_MISC_DEVICE_NUM_MAX; dev_id++) {
        dfm_unregister_nodes(&g_l3d_dfm[dev_id]);
        dfm_struct_final(&g_l3d_dfm[dev_id], L3D_NODE_NUM, 0);
    }
}

int l3d_fault_manager_init(void)
{
    if (l3d_dms_node_init() != 0) {
        soc_misc_drv_err("register l3d device node failed.\n");
        return -EFAULT;
    }
#if defined(CFG_SOC_PLATFORM_MDC_V51) || defined(CFG_SOC_PLATFORM_MDC_LITE)
    if (l3d_safety_init() != 0) {
        soc_misc_drv_err("init l3d safety irq register failed.\n");
        goto init_l3d_safety_fail;
    }
#endif
#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (l3d_irq_ras_init() != 0) {
        soc_misc_drv_err("init l3d irq ras register failed.\n");
        goto init_l3d_irq_ras_fail;
    }
#endif

#if defined(CFG_FEATURE_FAULT_MANAGER) && !defined(CFG_SOC_PLATFORM_MDC_LITE)
    if (l3d_arm_ras_init() != 0) {
        soc_misc_drv_err("init l3d arm ras init failed.\n");
        goto init_l3d_arm_ras_fail;
    }
#endif

    return 0;

#if defined(CFG_FEATURE_FAULT_MANAGER) && !defined(CFG_SOC_PLATFORM_MDC_LITE)
init_l3d_arm_ras_fail:
#endif
#ifdef CFG_SOC_PLATFORM_MDC_V51
    l3d_irq_ras_uninit();
init_l3d_irq_ras_fail:
#endif
#if defined(CFG_SOC_PLATFORM_MDC_V51) || defined(CFG_SOC_PLATFORM_MDC_LITE)
    l3d_safety_uninit();
init_l3d_safety_fail:
#endif
    l3d_dms_node_uninit();

    return -EFAULT;
}

void l3d_fault_manager_uninit(void)
{
#if defined(CFG_FEATURE_FAULT_MANAGER) && !defined(CFG_SOC_PLATFORM_MDC_LITE)
    l3d_arm_ras_uninit();
#endif

#ifdef CFG_SOC_PLATFORM_MDC_V51
    l3d_irq_ras_uninit();
#endif
#if defined(CFG_SOC_PLATFORM_MDC_V51) || defined(CFG_SOC_PLATFORM_MDC_LITE)
    l3d_safety_uninit();
#endif

    l3d_dms_node_uninit();
}

int l3d_sensor_scan(u64 private_data, struct dms_sensor_event_data *data)
{
    u32 dev_id = (private_data >> SOC_MISC_OFFSET_32BIT) & SOC_MISC_MASK_32BIT;
    u32 node_id = private_data & 0xFF;
    u32 sensor_type = (private_data >> SOC_MISC_OFFSET_16BIT) & 0xFF;

    if (node_id >= g_l3d_dfm[dev_id].node_num) {
        soc_misc_drv_err("invalid node id. (node_id=%u, priv=0x%llx)\n", node_id, private_data);
        return -EINVAL;
    }

    (void)dfm_scan_events(&g_l3d_dfm[dev_id], node_id, (u8)sensor_type, data);
    return 0;
}
