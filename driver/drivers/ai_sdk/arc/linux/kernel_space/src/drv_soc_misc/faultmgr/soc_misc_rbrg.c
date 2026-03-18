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
 * Create: 2023-01-03
 */

#include "soc_misc_rbrg.h"
#include "dms_define.h"
#include "dfm_dev_register.h"
#include "dms_sensor_type.h"
#include "soc_misc_dms_def.h"
#include "soc_misc_init.h"
#include "drv_ras_common.h"
#include "dfm_report.h"
#include "fpdc.h"

 /* RBRG RAS IERR TYPE */
enum rbrg_ras_ierr {
    RBRG_PARITY_ERROR = 0,
};

/*
 * src id, from EMU_SUBS sheet, intterrupt number
 */
#define AO_RBRG_INT_SAFETY_BIT_ID 6
#define AO_RBRG_ERR_SAFETY_BIT_ID 22
#define SOC_MISC_RBRG_NODE_NUM    1
#define SOC_MISC_RBRG_EVENT_SEVERITY  3

// match to bios
#define RBRG_INT_SAFETY_REG_INDEX 1  // reg_name:INT_RING_SRC 0x8800
#define RBRG_ERR_SAFETY_REG_INDEX 0  // reg_name:INT_MESH_SRC 0x0800

#define SOC_ERR_SECTOR_SAFETY (0xFFU)

#define RBRG_SAFETY_COVERGE_NODE(ierr, msg, ras_err) { \
    .subsys_id = DMS_DEV_TYPE_RBRG,                    \
    .module_id = SOC_MISC_SENSOR_RBRG,                 \
    .section_type = SOC_ERR_SECTOR_SAFETY,             \
    .ras_code.int_status = ierr,                       \
    .describe = msg,                                   \
    .sensor_type = DMS_SEN_TYPE_RAS_SENSOR,            \
    .error_type = ras_err,                             \
}

static struct ras_fault_converge_item g_soc_misc_rbrg_converge_table[] = {
    RBRG_SAFETY_COVERGE_NODE(RBRG_PARITY_ERROR, "RBRG parity error", RAS_ERROR_TYPE_PARITY),
};

static struct dfm_struct g_soc_misc_rbrg_dms;

STATIC int soc_misc_rbrg_ops_init(struct dms_node *device)
{
    soc_misc_drv_debug("soc_misc_rbrg_ops_init success.\n");
    return 0;
}

STATIC void soc_misc_rbrg_ops_uninit(struct dms_node *device)
{
    soc_misc_drv_debug("soc_misc_rbrg_ops_uninit success.\n");
    return;
}

static struct dms_node_operations g_soc_misc_rbrg_ops = {
    .init = soc_misc_rbrg_ops_init,
    .uninit = soc_misc_rbrg_ops_uninit,
    .scan = NULL,
    .fault_diag = NULL,
    .event_notify = NULL,
    .get_link_state = NULL,
    .set_link_state = NULL
};

#define SOC_MISC_RBRG_DMS_NODE_DEFINE(_id, _ops) {   \
    .node_type = DMS_DEV_TYPE_RBRG,                  \
    .node_id = _id,                                  \
    .node_name = "SOC-RBRG-" #_id,                   \
    .capacity = 0x1,                                 \
    .permission = 0x1,                               \
    .owner_devid = 0,                                \
    .ops = _ops                                      \
}

static struct dms_node g_soc_misc_rbrg_dms_nodes[SOC_MISC_RBRG_NODE_NUM] = {
    SOC_MISC_RBRG_DMS_NODE_DEFINE(0, &g_soc_misc_rbrg_ops),
};

#define DEV_NODE_OFFSET 16
#define DEV_NODE_MASK 0xFFFF
#define SENSOR_TYPE_OFFSET (0)
#define SENSOR_TYPE_MASK   (0xFF)

#define SENSOR_PRIV_DATA(dev_node, sensor_type) \
    (((u64)(dev_node & DEV_NODE_MASK) << DEV_NODE_OFFSET) | \
    ((u64)(sensor_type & SENSOR_TYPE_MASK) << SENSOR_TYPE_OFFSET))


STATIC int soc_scan_rbrg_event(u64 private_data, struct dms_sensor_event_data *data)
{
    u32 node_id;
    u32 sensor_type;

    node_id = ((private_data >> DEV_NODE_OFFSET) & DEV_NODE_MASK);
    sensor_type = ((private_data >> SENSOR_TYPE_OFFSET) & SENSOR_TYPE_MASK);

    if (node_id >= g_soc_misc_rbrg_dms.node_num) {
        soc_misc_drv_err("invalid node id(%u).\n", node_id);
        return -EINVAL;
    }
    (void)dfm_scan_events(&g_soc_misc_rbrg_dms, node_id, sensor_type, data);
    return 0;
}

void rbrg_report_fault_to_sils(hisi_common_error_info *hisi_err)
{
    uint32_t event_num = 1;
    struct safety_event fault_list = { 0 };
    static u8 g_event_serial = 0;

    fault_list.emu_id = AO_SUB_EMU_ID;
    fault_list.src_id = 1;
    fault_list.bit_id = AO_RBRG_ERR_SAFETY_BIT_ID; // now chip only report Safety_err, not report Safety_int
    fault_list.node_type = DMS_DEV_TYPE_RBRG;
    fault_list.sensor_type = DMS_SEN_TYPE_RAS_SENSOR; // SOC_Event
    fault_list.event_type = RAS_ERROR_TYPE_PARITY; // parity error
    fault_list.node_id = 0;
    fault_list.sub_node_type = 0; // rbrg not has sub, so fill it by node_type
    fault_list.sub_node_id = 0;
    fault_list.event_severity = SOC_MISC_RBRG_EVENT_SEVERITY; // fault rank is emergency
    fault_list.event_assertion = DMS_EVENT_TYPE_OCCUR;
    fault_list.event_serial_num = g_event_serial++;

    drvfault_send_safety_info_to_sils(event_num, &fault_list);
}

int soc_misc_rbrg_check_notify_data(const struct notify_data *pdata)
{
    if ((pdata == NULL) || (pdata->origin_data == NULL)) {
        soc_misc_drv_err("notify data is NULL\n");
        return -EINVAL;
    }

    if (pdata->node_type != DMS_DEV_TYPE_RBRG) {
        soc_misc_drv_err("Invalid paramters: node_type(%u)\n", pdata->node_type);
        return -EINVAL;
    }
    if (pdata->data_len != sizeof(hisi_common_error_info)) {
        soc_misc_drv_err("ras data len(%u) is invalid.\n", pdata->data_len);
        return -EINVAL;
    }
    return 0;
}

void soc_misc_rbrg_notify_sensor_scan(struct dfm_struct *dfm, u32 node_idx, u8 sensor_type)
{
    struct dfm_node *dev_node = &dfm->dev_nodes[node_idx];
    u32 i;
    for (i = 0; i < dev_node->sensor_num; ++i) {
        if (dev_node->sensors[i].dms_sensor.sensor_type == sensor_type) {
            /* notify the sensor to scan event immediately */
            if (dms_sensor_event_notify(0, &dev_node->sensors[i].dms_sensor) != 0) {
                soc_misc_drv_warn("send notify to dms sensor.\n");
            }
        }
    }
}

void soc_misc_rbrg_fault_handler(const struct notify_data *pdata)
{
    u32 node_id = 0;
    int ret = 0;
    hisi_common_error_info *hisi_err;

    if (soc_misc_rbrg_check_notify_data(pdata)) {
        return;
    }
    hisi_err = (hisi_common_error_info *)pdata->origin_data;

    // print rbrg.status.reg info
    soc_misc_drv_event("RBRG.INT_RING_SRC.reg.info = 0x%x .\n",
        hisi_err->register_array[RBRG_INT_SAFETY_REG_INDEX]);
    soc_misc_drv_event("RBRG.INT_MESH_SRC.reg.info = 0x%x .\n",
        hisi_err->register_array[RBRG_ERR_SAFETY_REG_INDEX]);

    ret = dfm_add_event(&g_soc_misc_rbrg_dms, node_id, &g_soc_misc_rbrg_converge_table[0]);
    if (ret != 0) {
        if (ret != -EEXIST) {
            soc_misc_drv_err("dfm_add_event failed.  ret = %d .\n", ret);
            return;
        }
    } else {
        soc_misc_rbrg_notify_sensor_scan(&g_soc_misc_rbrg_dms, node_id,
            g_soc_misc_rbrg_converge_table[0].sensor_type);
    }
    rbrg_report_fault_to_sils(hisi_err);
    return;
}

int soc_misc_rbrg_register(u32 devid)
{
    struct dms_sensor_object_cfg sensor_cfg[] = {
        SOC_MISC_SENOR_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "soc_rbrg", DMS_DISCRETE_SENSOR_CLASS,
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, DMS_SENSOR_CHECK_INTERVAL_TIME,
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, soc_scan_rbrg_event,
            SENSOR_PRIV_DATA(0, DMS_SEN_TYPE_RAS_SENSOR), 0xFFFF, 0xFFFF),
    };
    u32 sensor_num = (u32)ARRAY_SIZE(sensor_cfg);
    int i;
    u32 sensor_idx;
    g_soc_misc_rbrg_dms.dev_id = devid;

    if (dfm_struct_init(&g_soc_misc_rbrg_dms, SOC_MISC_RBRG_NODE_NUM, sensor_num) != 0) {
        soc_misc_drv_err("init soc_misc_rbrg dfm data failed.\n");
        goto _fail;
    }

    for (i = 0; i < (int)SOC_MISC_RBRG_NODE_NUM; ++i) {
        g_soc_misc_rbrg_dms.dev_nodes[i].node = &g_soc_misc_rbrg_dms_nodes[i];
        g_soc_misc_rbrg_dms.dev_nodes[i].post_proc = NULL;
        g_soc_misc_rbrg_dms.dev_nodes[i].fpdc_notify = soc_misc_rbrg_fault_handler;
        g_soc_misc_rbrg_dms.dev_nodes[i].get_converage_node = NULL;
        for (sensor_idx = 0; sensor_idx < sensor_num; ++sensor_idx) {
            sensor_cfg[sensor_idx].private_data = SENSOR_PRIV_DATA(i, sensor_cfg[sensor_idx].sensor_type);
            g_soc_misc_rbrg_dms.dev_nodes[i].sensors[sensor_idx].dms_sensor = sensor_cfg[sensor_idx];
        }
    }

    if (dfm_register_nodes(&g_soc_misc_rbrg_dms) != 0) {
        soc_misc_drv_err("register soc_misc_rbrg dms node failed.\n");
        goto _fail;
    }
    return 0;

_fail:
    dfm_struct_final(&g_soc_misc_rbrg_dms, SOC_MISC_RBRG_NODE_NUM, 1);
    return -EFAULT;
}

void soc_misc_rbrg_unregister(void)
{
    dfm_unregister_nodes(&g_soc_misc_rbrg_dms);
    dfm_struct_final(&g_soc_misc_rbrg_dms, SOC_MISC_RBRG_NODE_NUM, 0);
}
