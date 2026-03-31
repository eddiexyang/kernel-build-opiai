/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: RAS
 * Author: huawei
 * Create: 2022-12-15
 */

#ifndef __HCLGE_PLF_ERR_H
#define __HCLGE_PLF_ERR_H

#include <linux/device.h>
#include "drv_ras_common.h"
#include "dms_node_type.h"
#include "dms_sensor_type.h"
#include "dms_interface.h"
#include "dms_sensor.h"
#include "dfm_dev_register.h"

enum hclge_plf_dev_node {
    HCLGE_PLF_NODE_NIC0 = 0,
    HCLGE_PLF_NODE_NIC1 = 1,
    HCLGE_PLF_NODE_NIC2 = 2,
    HCLGE_PLF_NODE_NIC3 = 3,
    HClGE_PLF_NODE_ID_MAX
};

enum hclge_plf_dev_sensor {
    HCLGE_PLF_SENSOR_RAS    = 0,
    HCLGE_PLF_SENSOR_NUM
};

#define DMS_SENSOR_SCAN_INTERVAL 100

#define DEV_NODE_OFFSET 16
#define DEV_NODE_MASK 0xFFFF
#define SENSOR_TYPE_OFFSET 0
#define SENSOR_TYPE_MASK 0xFF
#define HCLGE_PLF_DEVCICE_ID_REG_MAGIC_NUM 0x5A5A
#define HCLGE_PLF_NODE_NUM HClGE_PLF_NODE_ID_MAX


#define HCLGE_PLF_NODE_DEFINE(type, id, node_ops) {  \
    .node_type = (type),                             \
    .node_id = (id),                                 \
    .node_name = "NIC",                              \
    .owner_devid = 0,                                \
    .ops = (node_ops)                                \
}

#define SENSOR_PRIV_DATA(dev_node, sensor_type)                      \
    (((u64)((dev_node) & DEV_NODE_MASK) << DEV_NODE_OFFSET) |        \
    ((u64)((sensor_type) & SENSOR_TYPE_MASK) << SENSOR_TYPE_OFFSET))

enum hclge_plf_err_int_type {
    HCLGE_ERR_INT_ABN     = 0,
    HCLGE_ERR_INT_RAS_CE  = 1,
    HCLGE_ERR_INT_RAS_NFE = 2,
    HCLGE_ERR_INT_RAS_FE  = 3
};

enum hclge_plf_mod_name_list {
    MODULE_IGU_EGU      = 0,    // include XXVGE_COMMON
    MODULE_PPP          = 1,
    MODULE_SSU          = 2,
    MODULE_PPU          = 3,    // include RCB,TPU,RPU
    MODULE_NONE         = 4
};

enum hclge_plf_err_type_list {
    NONE_ERROR        = 0,
    FIFO_ERROR        = 1,
    MEM_MBECC_ERROR   = 2,
    ABNORMAL_ERROR    = 3,
    BUFFER_ERROR      = 4,
    MEM_SBECC_ERROR   = 5,
    TX_TIMEOUT_ERROR  = 6,
    LINK_CHANGE_ERROR = 7,
    REG_READ_WRITE_ERROR = 8
};

enum hclge_plf_fault_type_list {
    HCLGE_PLF_TX_TIMEOUT_ERROR  = 0,
    HCLGE_PLF_LINK_CHANGE_ERROR = 1,
    HCLGE_PLF_REG_READ_WRITE_ERROR = 2
};

struct hclge_plf_hw_error {
    u32 int_msk;
    const char *msg;
    enum hclge_plf_err_type_list type_id;
    enum hnae3_reset_type reset_level;
};

struct hclge_plf_module_id {
    enum hclge_plf_mod_name_list module_id;
    const char *msg;
};

struct hclge_plf_hw_type_id {
    enum hclge_plf_err_type_list type_id;
    const char *msg;
};

struct hclge_plf_ras_error {
    u8 sub_module_id;
    u8 sensor_id;
    u8 type_id;
    u32 register_array_size;
    const u32 *register_array;
    u32 ras_code;
};

struct hclge_plf_fault_info {
    u32 excep_id;
    dfm_event event;
};

#define HCLGE_PLF_EVENT_ERROR_TYPE_OFFSET      8
#define HCLGE_PLF_EVENT_EXCEP_ID(err_code, err_type) ((u32)(err_code) | \
    ((u32)(err_type) << HCLGE_PLF_EVENT_ERROR_TYPE_OFFSET))

#define HCLGE_PLF_RAS_FAULT_EVENT(sub_id, mod_id, err_code, desc, err_type) { \
    .excep_id = HCLGE_PLF_EVENT_EXCEP_ID(err_code, err_code), \
    .event = DMS_FAULT_CONVERGE_CONFIG(sub_id, mod_id, RAS_SEC_ARM, err_code, DMS_SEN_TYPE_RAS_SENSOR, err_type, desc) \
}

#define HCLGE_PLF_SAFETY_SENSOR_FAULT_EVENT(sub_id, mod_id, err_code, desc, err_type) { \
    .excep_id = HCLGE_PLF_EVENT_EXCEP_ID(err_code, err_type), \
    .event = DMS_FAULT_CONVERGE_CONFIG(sub_id, mod_id, RAS_SEC_ARM, err_code, DMS_SEN_TYPE_SAFETY_SENSOR, err_type, \
                                       desc) \
}

#define HCLGE_PLF_EXTEND_SENSOR_FAULT_EVENT(sub_id, mod_id, err_code, desc, err_type) { \
    .excep_id = HCLGE_PLF_EVENT_EXCEP_ID(err_code, err_type), \
    .event = DMS_FAULT_CONVERGE_CONFIG(sub_id, mod_id, RAS_SEC_ARM, err_code, DMS_SEN_TYPE_EXTEND_SENSOR, err_type, \
                                       desc) \
}

#define HCLGE_PLF_SOC_SAFETY_CONFIG_ERR    0x4
#define HCLGE_PLF_REG_READ_WRITE_ERR       0x6

int hclge_plf_register_dms_node(void);
void hclge_plf_unregister_dms_node(void);
void hclge_plf_ras_int_config(struct hclge_plf_dev *hdev);

#endif // __HCLGE_PLF_ERR_H