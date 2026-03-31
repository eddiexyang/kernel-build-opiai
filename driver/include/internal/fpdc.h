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

#ifndef FPDC_INCLUDE_FPDC_H
#define FPDC_INCLUDE_FPDC_H

#include <linux/uuid.h>
#include "dms_node_type.h"

typedef enum {
    FPDC_SRC_RAS,
    FPDC_SRC_SAFETY
} FPDC_SRC_TYPE;

enum ras_sec_type {
    RAS_SEC_GENERIC = 0x00,
    RAS_SEC_IA = 0x01,
    RAS_SEC_IPF = 0x02,
    RAS_SEC_ARM = 0x03,
    RAS_SEC_MEM = 0x04,
    RAS_SEC_PCIE = 0x05,
    RAS_SEC_OEM = 0x06,
    RAS_SEC_EMMC = 0x07,
    RAS_SEC_OTHER
};

#ifdef DEFINE_HNS_LLT
typedef struct {
    __u8 b[16];
} guid_t;
#endif

struct notify_data {
    FPDC_SRC_TYPE src_type;
    const guid_t *section_type;
    DMS_DEVICE_NODE_TYPE node_type;
    unsigned int chip_id;
    unsigned long long sub_id; /* optional */
    const void *origin_data;
    unsigned int data_len;
    unsigned int arm_error_idx;
};

#pragma pack(1)
struct vendor_specific_error_info {
    unsigned int err_status;
    unsigned long long err_addr;
    unsigned char oem_valid_flag;
    unsigned char oem_socket_id;
    unsigned char oem_die_id;
    unsigned char oem_sub_module;
};
#pragma pack()

/* callback function define */
typedef void (*FAULT_NOTIFY_FUNC)(const struct notify_data *pdata);

/* register callback function when node_type fault occurs */
int fpdc_register_fault_notifier(DMS_DEVICE_NODE_TYPE node_type, FAULT_NOTIFY_FUNC notify_func);
int fpdc_unregister_fault_notifier(DMS_DEVICE_NODE_TYPE node_type);


#endif
