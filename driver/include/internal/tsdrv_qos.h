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
* Create: 2022-8-29
*/

#ifndef TSDRV_QOS_H
#define TSDRV_QOS_H

#include "ascend_kernel_hal.h"

#define TS_QOS_CFG_MPAM 0U
#define TS_QOS_CFG_ALLOW 1U
#define TS_QOS_CFG_OTSD 2U

#define TS_QOS_OP_GET 0U
#define TS_QOS_OP_SET 1U

#define TS_QOS_MAX_ALLOW_LEVEL 0xFFFFU

#pragma pack(push)
#pragma pack (1)
typedef struct ts_qos_mpam_cfg {
    u64 bitmap; /* master bitmap for core/subsys/channel */
    u32 mpam_id; /* mpam id */
    u32 qos; /* qos */
    u32 pmg;
    char type; /* master type */
    unsigned char replace_en : 1; /* if enable, use reg value to config mpam_id */
    unsigned char vf_en : 1; /* if enable and smmu enable, use smmu find page table get mpam_id */
    char err : 6;
} ts_qos_mpam_cfg_t;

typedef struct ts_qos_allow_cfg {
    uint32_t mode;
    uint32_t ctrl;
    uint16_t lvl[MAX_QOS_ALLOW_LEVEL];
} ts_qos_allow_cfg_t;

typedef struct ts_qos_otsd_cfg {
    uint32_t mode; // 0: disable otsd limit, 1: read & write merge, 2: read & write not merge
    uint32_t lvl[MAX_OTSD_LEVEL];
} ts_qos_otsd_cfg_t;

typedef struct ts_qos_cfg {
    uint8_t master_type; // aic, aiv, sdma
    uint8_t op_type; // operation type, 0: get, 1: set
    uint8_t qos_cfg_type; // 0: mpam, 1: qos_allow, 2: otsd
    uint8_t status; // resp error code to drv
    union {
        ts_qos_allow_cfg_t allow_cfg;
        ts_qos_otsd_cfg_t otsd_cfg;
    };
} ts_qos_cfg_t;
#pragma pack(pop)

enum tsdrv_qos_prop_type {
    TSDRV_SET_QOS_CFG,
    TSDRV_GET_QOS_CFG,
    TSDRV_QOS_CFG_MAX
};

int tsdrv_set_qos_cfg(int devid, const struct qos_master_config_type *cfg);
int tsdrv_get_qos_cfg(int devid, struct qos_master_config_type *cfg);
int tsdrv_set_otsd_cfg(int devid, const struct qos_otsd_config_type *cfg);
int tsdrv_get_otsd_cfg(int devid, struct qos_otsd_config_type *cfg);
int tsdrv_set_allow_cfg(int devid, const struct qos_allow_config_type *cfg);
int tsdrv_get_allow_cfg(int devid, struct qos_allow_config_type *cfg);

#endif
