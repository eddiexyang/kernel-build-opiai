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

#pragma pack(push)
#pragma pack (1)
typedef struct ts_qos_mpam_cfg {
    u64 bitmap; /* master bitmap for core/subsys/channel in para */
    u32 mpam_id; /* mpam id */
    u32 qos; /* qos */
    u32 pmg;
    u8 type; /* master type in para */
    u8 replace_en : 1; /* if enable, use reg value to config mpam_id */
    u8 vf_en : 1; /* if enable and smmu enable, use smmu find page table get mpam_id */
    u8 err : 6;
} ts_qos_mpam_cfg_t;
#pragma pack(pop)

int tsdrv_qos_node_register(void);
void tsdrv_qos_node_unregister(void);
#endif
