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
 * Create: 2023-05-12
 */
#include <linux/types.h>
#include "fpdc.h"
#include "dms_define.h"
#include "dfm_dev_register.h"
#include "dms_sensor_type.h"
#include "soc_misc_dms_def.h"
#include "soc_misc_init.h"
#include "drv_ras_common.h"
#include "dfm_report.h"
#include "soc_misc_fault.h"
#include "soc_misc_err_info.h"
#include "soc_misc_hccs.h"

#define SOC_MISC_HCCS_MAX_NODE_NUM    8
#define SOC_MISC_HCCS_MAX_CHIP_NUM    2

#define SENSOR_TYPE_OFFSET       0
#define SENSOR_TYPE_MASK         0xFF
#define DEV_NODE_OFFSET          16
#define DEV_NODE_MASK            0xFFFF
#define SOC_MISC_DEVID_OFFSET    32
#define SOC_MISC_DEVID_MASK      0xFFFFFFFF

#define HPCS_INT_CFG_ERR_MASK        0xBE6
#define HDLC_INT_CFG_ERR_MASK        0xEF0
#define HDLC_SBECC_OT_ERR_MASK       0xC001

struct hccs_event_tpye {
    unsigned int fault_mask;  /* masks for the same fault type in registers */
    int fault_handle;         /* fault type after covergence */
};

enum hccs_ras_code {
    HCCS_INT_CFG_ERR = 0,
    HCCS_SBECC_OT_ERR = 1,
};

#define SENSOR_PRIV_DATA(_dev_id, _node_id, _sensor_type)               \
    (((u64)(_node_id & DEV_NODE_MASK) << DEV_NODE_OFFSET) |             \
    ((u64)(_sensor_type & SENSOR_TYPE_MASK) << SENSOR_TYPE_OFFSET) |    \
    ((u64)(_dev_id & SOC_MISC_DEVID_MASK) << SOC_MISC_DEVID_OFFSET))

#define SOC_MISC_HCCS_DMS_NODE_DEFINE(_dev_id, _node_id, _ops) {   \
    .node_type = DMS_DEV_TYPE_HCCS,             \
    .node_id = _node_id,                                  \
    .node_name = "SOC-HCCS-" #_node_id,                   \
    .capacity = 0x1,                                 \
    .permission = 0x1,                               \
    .owner_devid = _dev_id,                              \
    .ops = _ops                                      \
}

#define HCCS_RAS_COVERAGE(hccs_ras_code, msg, ras_err) {          \
    .subsys_id = DMS_DEV_TYPE_HCCS,                               \
    .module_id = SOC_MISC_SENSOR_HCCS,                            \
    .section_type = RAS_SEC_ARM,                                  \
    .ras_code.int_status = hccs_ras_code,                         \
    .describe = msg,                                              \
    .sensor_type = DMS_SEN_TYPE_RAS_SENSOR,                       \
    .error_type = ras_err,                                        \
}

#define HCCS_REGISTER_FAULT_DEFINED(_mask, _handle) {   \
    .fault_mask = _mask,                                \
    .fault_handle = _handle,                            \
}

STATIC int soc_misc_hccs_ops_init(struct dms_node *device)
{
    soc_misc_drv_debug("soc_misc_hccs_ops_init success.\n");
    return 0;
}

STATIC void soc_misc_hccs_ops_uninit(struct dms_node *device)
{
    soc_misc_drv_debug("soc_misc_hccs_ops_uninit success.\n");
    return;
}

static struct dfm_struct g_soc_misc_hccs_dms[SOC_MISC_HCCS_MAX_CHIP_NUM];

static struct dms_node_operations g_soc_misc_hccs_ops = {
    .init = soc_misc_hccs_ops_init,
    .uninit = soc_misc_hccs_ops_uninit,
    .scan = NULL,
    .fault_diag = NULL,
    .event_notify = NULL,
    .get_link_state = NULL,
    .set_link_state = NULL
};

static struct dms_node g_soc_misc_hccs_dms_nodes[] = {
    SOC_MISC_HCCS_DMS_NODE_DEFINE(0, 0, &g_soc_misc_hccs_ops),
    SOC_MISC_HCCS_DMS_NODE_DEFINE(0, 1, &g_soc_misc_hccs_ops),
    SOC_MISC_HCCS_DMS_NODE_DEFINE(0, 2, &g_soc_misc_hccs_ops),
    SOC_MISC_HCCS_DMS_NODE_DEFINE(0, 3, &g_soc_misc_hccs_ops),
    SOC_MISC_HCCS_DMS_NODE_DEFINE(0, 4, &g_soc_misc_hccs_ops),
    SOC_MISC_HCCS_DMS_NODE_DEFINE(0, 5, &g_soc_misc_hccs_ops),
    SOC_MISC_HCCS_DMS_NODE_DEFINE(0, 6, &g_soc_misc_hccs_ops),
    SOC_MISC_HCCS_DMS_NODE_DEFINE(0, 7, &g_soc_misc_hccs_ops),
    SOC_MISC_HCCS_DMS_NODE_DEFINE(1, 0, &g_soc_misc_hccs_ops),
    SOC_MISC_HCCS_DMS_NODE_DEFINE(1, 1, &g_soc_misc_hccs_ops),
    SOC_MISC_HCCS_DMS_NODE_DEFINE(1, 2, &g_soc_misc_hccs_ops),
    SOC_MISC_HCCS_DMS_NODE_DEFINE(1, 3, &g_soc_misc_hccs_ops),
    SOC_MISC_HCCS_DMS_NODE_DEFINE(1, 4, &g_soc_misc_hccs_ops),
    SOC_MISC_HCCS_DMS_NODE_DEFINE(1, 5, &g_soc_misc_hccs_ops),
    SOC_MISC_HCCS_DMS_NODE_DEFINE(1, 6, &g_soc_misc_hccs_ops),
    SOC_MISC_HCCS_DMS_NODE_DEFINE(1, 7, &g_soc_misc_hccs_ops),
};

static struct ras_fault_converge_item g_soc_misc_hccs_converge_table[] = {
    HCCS_RAS_COVERAGE(HCCS_INT_CFG_ERR, "internal config error", RAS_ERROR_TYPE_IN_CFG_ERR),
    HCCS_RAS_COVERAGE(HCCS_SBECC_OT_ERR, "single bit ecc error overthold", RAS_ERROR_TYPE_SBECCOverThold),
};

static struct hccs_event_tpye g_hpcs_fault_table[] = {
    HCCS_REGISTER_FAULT_DEFINED(HPCS_INT_CFG_ERR_MASK, HCCS_INT_CFG_ERR),
};

static struct hccs_event_tpye g_hdlc_fault_table[] = {
    HCCS_REGISTER_FAULT_DEFINED(HDLC_SBECC_OT_ERR_MASK, HCCS_SBECC_OT_ERR),
    HCCS_REGISTER_FAULT_DEFINED(HDLC_INT_CFG_ERR_MASK, HCCS_INT_CFG_ERR),
};

STATIC int soc_scan_hccs_event(u64 private_data, struct dms_sensor_event_data *data)
{
    u32 dev_id;
    u32 node_id;
    u32 sensor_type;

    dev_id = (private_data >> SOC_MISC_DEVID_OFFSET) & SOC_MISC_DEVID_MASK;
    node_id = ((private_data >> DEV_NODE_OFFSET) & DEV_NODE_MASK);
    sensor_type = ((private_data >> SENSOR_TYPE_OFFSET) & SENSOR_TYPE_MASK);
    if (node_id >= g_soc_misc_hccs_dms[dev_id].node_num) {
        soc_misc_drv_err("Invalid node id(device id=%u, node id=%u).\n", dev_id, node_id);
        return -EINVAL;
    }
    (void)dfm_scan_events(&g_soc_misc_hccs_dms[dev_id], node_id, sensor_type, data);
    return 0;
}

STATIC int soc_misc_hccs_check_notify_data(const struct notify_data *pdata)
{
    if ((pdata == NULL) || (pdata->origin_data == NULL)) {
        soc_misc_drv_err("notify data is NULL\n");
        return -EINVAL;
    }

    if ((pdata->src_type != FPDC_SRC_RAS) || (pdata->node_type != DMS_DEV_TYPE_HCCS)) {
        soc_misc_drv_err("Invalid paramters. (src_type=%u, node_type=%u)\n", pdata->src_type, pdata->node_type);
        return -EINVAL;
    }
    if (pdata->data_len < sizeof(hisi_common_error_info)) {
        soc_misc_drv_err("Ras data is not too short. (len=%u)\n", pdata->data_len);
        return -EINVAL;
    }
    return 0;
}

STATIC const struct ras_fault_converge_item *soc_misc_get_hccs_converge_item(const struct ras_error *error_info)
{
    u32 num = sizeof(g_soc_misc_hccs_converge_table) / sizeof(struct ras_fault_converge_item);
    return soc_misc_parse_table_handle(error_info, g_soc_misc_hccs_converge_table, num);
}

STATIC int get_hpcs_ras_code_from_table(u32 register_data)
{
    int i;
    int num = sizeof(g_hpcs_fault_table) / sizeof(struct hccs_event_tpye);
    for (i = 0; i < num; i++) {
        if ((register_data & g_hpcs_fault_table[i].fault_mask) != 0) {
            return g_hpcs_fault_table[i].fault_handle;
        }
    }
    soc_misc_drv_err("Undefined fault type.\n");
    return -EINVAL;
}

STATIC int get_hdlc_ras_code_from_table(u32 register_data)
{
    int i;
    int num = sizeof(g_hdlc_fault_table) / sizeof(struct hccs_event_tpye);
    for (i = 0; i < num; i++) {
        if ((register_data & g_hdlc_fault_table[i].fault_mask) != 0) {
            return g_hdlc_fault_table[i].fault_handle;
        }
    }
    soc_misc_drv_err("Undefined fault type.\n");
    return -EINVAL;
}

STATIC int get_hccs_ras_code(hisi_common_error_info* hisi_err)
{
    int  ras_code;
    if (hisi_err->module_id == HISI_MODULE_PCS) {
        ras_code = get_hpcs_ras_code_from_table(hisi_err->register_array[0]);
    } else if (hisi_err->module_id == HISI_MODULE_HLLC) {
        ras_code = get_hdlc_ras_code_from_table(hisi_err->register_array[0]);
    }
    return ras_code;
}

/* register in fpdc */
void soc_misc_hccs_fault_handler(const struct notify_data *pdata)
{
    u32 node_id;
    int ras_code;
    int ret = 0;
    hisi_common_error_info *hisi_err = NULL;
    struct ras_error error_info = {0};
    const dfm_event *event =  NULL;

    if (soc_misc_hccs_check_notify_data(pdata)) {
        soc_misc_drv_err("Invalid fault notify data.\n");
        return;
    }
    hisi_err = (hisi_common_error_info *)pdata->origin_data;

    ras_code = get_hccs_ras_code(hisi_err);
    if (ras_code < 0) {
        soc_misc_drv_err("Invalid notify data. (device id=%u)\n", pdata->chip_id);
        return;
    }
    error_info.device_id = pdata->chip_id;
    node_id = hisi_err->device_or_core_id;
    error_info.sensor_id = SOC_MISC_SENSOR_HCCS;
    error_info.module_id = SOC_MISC_SENSOR_HCCS;
    error_info.ras_code = ras_code;
    error_info.ras_err_severity = hisi_err->error_severity;
    error_info.sec_type = RAS_SEC_ARM;
    event = soc_misc_get_hccs_converge_item(&error_info);
    ret = dfm_add_event(&g_soc_misc_hccs_dms[error_info.device_id], node_id, event);
    if (ret != 0) {
        if (ret != -EEXIST) {
            soc_misc_drv_err("dfm_add_event failed. (device id=%u, ret=%d)\n", error_info.device_id, ret);
            return;
        }
    }

    return;
}

int soc_misc_hccs_register(u32 dev_id)
{
    struct dms_sensor_object_cfg sensor_cfg[] = {
        SOC_MISC_SENOR_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "soc_hccs", DMS_DISCRETE_SENSOR_CLASS,
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, DMS_SENSOR_CHECK_INTERVAL_TIME,
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, soc_scan_hccs_event,
            SENSOR_PRIV_DATA(dev_id, 0, DMS_SEN_TYPE_RAS_SENSOR), 0xFFFF, 0xFFBF),
    };

    u32 sensor_num = (u32)ARRAY_SIZE(sensor_cfg);
    int i;
    u32 sensor_idx;
    u32 initial_idx = dev_id * SOC_MISC_HCCS_MAX_NODE_NUM;

    if (dev_id >= SOC_MISC_HCCS_MAX_CHIP_NUM) {
        soc_misc_drv_err("Invalid device id. (device id=%u)\n", dev_id);
        return -EINVAL;
    }

    g_soc_misc_hccs_dms[dev_id].dev_id = dev_id;
    if (dfm_struct_init(&g_soc_misc_hccs_dms[dev_id], SOC_MISC_HCCS_MAX_NODE_NUM, sensor_num) != 0) {
        soc_misc_drv_err("Init soc_misc_hccs dfm data failed. (device id=%u)\n", dev_id);
        goto _fail;
    }

    for (i = 0; i < (int)SOC_MISC_HCCS_MAX_NODE_NUM; ++i) {
        g_soc_misc_hccs_dms[dev_id].dev_nodes[i].node = &g_soc_misc_hccs_dms_nodes[initial_idx + i];
        g_soc_misc_hccs_dms[dev_id].dev_nodes[i].post_proc = NULL;
        g_soc_misc_hccs_dms[dev_id].dev_nodes[i].fpdc_notify = soc_misc_hccs_fault_handler;
        g_soc_misc_hccs_dms[dev_id].dev_nodes[i].get_converage_node = NULL;
        for (sensor_idx = 0; sensor_idx < sensor_num; ++sensor_idx) {
            sensor_cfg[sensor_idx].private_data = SENSOR_PRIV_DATA(dev_id, i, sensor_cfg[sensor_idx].sensor_type);
            g_soc_misc_hccs_dms[dev_id].dev_nodes[i].sensors[sensor_idx].dms_sensor = sensor_cfg[sensor_idx];
        }
    }

    if (dfm_register_nodes(&g_soc_misc_hccs_dms[dev_id]) != 0) {
        soc_misc_drv_err("Register soc_misc_hccs dms node failed. (device id=%u)\n", dev_id);
        goto _fail;
    }

    return 0;

_fail:
    dfm_struct_final(&g_soc_misc_hccs_dms[dev_id], SOC_MISC_HCCS_MAX_NODE_NUM, 1);
    return -EFAULT;
}

void soc_misc_hccs_unregister(void)
{
    int i;

    for (i = 0; i < SOC_MISC_HCCS_MAX_CHIP_NUM; i++) {
        if (g_soc_misc_hccs_dms[i].node_num > 0) {
            dfm_unregister_nodes(&g_soc_misc_hccs_dms[i]);
            dfm_struct_final(&g_soc_misc_hccs_dms[i], SOC_MISC_HCCS_MAX_NODE_NUM, 1);
        }
    }
}