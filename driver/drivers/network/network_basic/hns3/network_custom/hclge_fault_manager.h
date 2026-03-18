/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Create: 2023-3-2
 */
#ifndef _HCLGE_FAULT_MANAGER_H
#define _HCLGE_FAULT_MANAGER_H

#include "dms_sensor.h"
#include "dms_sensor_type.h"
#include "dms_define.h"
#include "dms_dev_node.h"
#include "dfm_dev_register.h"
#include "hclge_main.h"
#include "hclge_err.h"

#define HCLGE_DMS_SENSOR_SCAN_INTERVAL  200
#define HCLGE_ASSERT_EVENT_MASK         0xFFFF
#define HCLGE_DEASSERT_SOC_EVENT_MASK       0xFFBF
#define HCLGE_DEASSERT_SOC_SAFETY_MASK       0xFFFF

#define DEV_ID_OFFSET                   32
#define DEV_ID_MASK                     0xFFFF
#define DEV_NODE_OFFSET                 16
#define DEV_NODE_MASK                   0xFFFF
#define SENSOR_TYPE_OFFSET              0
#define SENSOR_TYPE_MASK                0xFFFF

#define DISP_ERR_STATUSL 4
#define IERR_MASK 0xFF00

#define TCM_MERR_MASK 0xA02
#define CMDQ_MEM_MERR_MASK 0x8000
#define RD_POISON_MASK 0x1
#define TQP_INT_STS_0_MERR_MASK 0xF0
#define MSIX_MERR_MASK 0xA
#define TQP_INT_STS_1_MERR_MASK 0xFC0

enum hclge_dev_node {
    HCLGE_NODE_ROCE = 0,
    HCLGE_NODE_NIC = 1,
    HCLGE_NODE_NIC_DISP = 2,
    HCLGE_NODE_NIC_PORT = 3,
    HCLGE_NODE_NUM
};

enum hclge_dev_sensor {
    HCLGE_SENSOR_RAS_OR_COMMON = 0,
    HCLGE_SENSOR_NUM
};

enum hclge_fault_bbox_nic {
    NIC_IMP_GLOBAL_RESET = 0,
    NIC_IMP_NMI_INTER,
    NIC_IMP_HARD_FAULT,
    NIC_IMP_MEM_FAULT,
    NIC_IMP_BUS_FAULT,
    NIC_IMP_USAGE_FAULT,
    NIC_IMP_SVCALL_FAULT,
    NIC_IMP_DEBUG_HANDLE,
    NIC_IMP_PENDSV,
    NIC_IMP_UNHANDLE_FAULT
};

/* NIC_DISP */
enum disp_ras_value {
    DISP_DATA_RAM_MBECC_VALUE = 1,
    DISP_CMD_RAM_MBECC_VALUE,
    DISP_NCDIR_MBECC_VALUE,
    DISP_DATA_RAM_SBECC_VALUE,
    DISP_CMD_RAM_SBECC_VALUE,
    DISP_NCDIR_SBECC_VALUE,
    DISP_ERR_RSP_PORT_0_VALUE,
    DISP_ERR_RSP_PORT_1_VALUE,
    DISP_ERR_RSP_PORT_2_VALUE,
    DISP_ERR_RSP_PORT_3_VALUE,
    DISP_ERR_RSP_PORT_4_VALUE,
    DISP_ERR_RSP_PORT_5_VALUE,
    DISP_ERR_RSP_PORT_6_VALUE,
    DISP_ERR_RSP_PORT_7_VALUE,
    DISP_ERR_REQ_VALUE,
    DISP_ERR_WD_VALUE = 0x15
};

enum bios_comm_location {
    LOC_IMP_TCM_ECC_INT_SRC,
    LOC_CMDQ_MEM_ECC_INT_SRC,
    LOC_IMP_RD_POISON_INT_SRC,
    LOC_TQP_INT_ECC_INT_SRC_0,
    LOC_MSIX_ECC_INT_SRC,
    LOC_TQP_INT_ECC_INT_SRC_1_0,
    LOC_TQP_INT_ECC_INT_SRC_1_1,
    LOC_TQP_INT_ECC_INT_SRC_1_2
};

struct hclge_fpdc_ras_error {
    u8 device_id;
    u8 module_id;
    u8 sub_module_id;
    u32 register_array_size;
    const u32 *register_array;
    u8 ras_type;
    u32 ras_code;
};

typedef struct {
    u32 ras_reg_offset;
    union {
        u32 ras_reg_value;
        u32 ras_reg_mask;
    };
    u8 ras_type;
    u8 ras_code; /* RAS converge type */
} comm_ras_msg;

struct hclge_fault_info {
    u32 excep_id;
    dfm_event event;
};

#define SENSOR_PRIV_DATA(dev_id, dev_node, sensor_type) \
    (((u64)(dev_id & DEV_ID_MASK) << DEV_ID_OFFSET) | \
     ((u64)(dev_node & DEV_NODE_MASK) << DEV_NODE_OFFSET) | \
     ((u64)(sensor_type & SENSOR_TYPE_MASK) << SENSOR_TYPE_OFFSET))

#define RAS_EVENT_ERROR_TYPE_OFFSET            8
#define RAS_EVENT_EXCEP_ID(err_code, err_type) ((u32)(err_code) | ((u32)(err_type) << RAS_EVENT_ERROR_TYPE_OFFSET))

#define HCLGE_RAS_FAULT_EVENT(sub_id, mod_id, err_code, desc, err_type) { \
    .excep_id = RAS_EVENT_EXCEP_ID(err_code, err_type), \
    .event = DMS_FAULT_CONVERGE_CONFIG(sub_id, mod_id, RAS_SEC_ARM, err_code, DMS_SEN_TYPE_RAS_SENSOR, err_type, desc) \
}

#define HCLGE_DISP_FAULT_EVENT(sub_id, mod_id, err_code, desc, err_type) { \
    .excep_id = RAS_EVENT_EXCEP_ID(err_code, err_type), \
    .event = DMS_FAULT_CONVERGE_CONFIG(sub_id, mod_id, RAS_SEC_ARM, err_code, DMS_SEN_TYPE_RAS_SENSOR, err_type, desc) \
}

#define HCLGE_BBOX_FAULT_EVENT(err_code, desc) \
    DMS_FAULT_CONVERGE_CONFIG(HISI_SUBSYS_MAX, HISI_MODULE_NIC, RAS_SEC_ARM, err_code, DMS_SEN_TYPE_RAS_SENSOR, \
                              RAS_ERROR_TYPE_ERROR, desc)

#define HCLGE_SAFETY_SENSOR_FAULT_EVENT(sub_id, mod_id, err_code, desc, err_type) { \
    .excep_id = RAS_EVENT_EXCEP_ID(err_code, err_type), \
    .event = DMS_FAULT_CONVERGE_CONFIG(sub_id, mod_id, RAS_SEC_ARM, err_code, DMS_SEN_TYPE_SAFETY_SENSOR, err_type, \
                                       desc) \
}

int hclge_fault_mgr_init(struct hclge_dev *hdev);
void hclge_fault_mgr_unint(void);
void hclge_fault_report_excep(u32 dev_id, u32 excep_id);
void hclge_fault_report_recovery_excep(u32 dev_id, u32 excep_id);
void hclge_link_down_fault_report(struct hclge_dev *hdev, bool flag);
#endif
