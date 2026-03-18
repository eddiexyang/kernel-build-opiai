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
#ifndef TSDRV_COMMON_FAULT_PLATFORM_H
#define TSDRV_COMMON_FAULT_PLATFORM_H
#include <linux/workqueue.h>

#pragma pack(1)
struct ras_message {
    u64 ras_err_code;
    u8 node_type;
    u8 node_id;
    u8 sub_node_type;
    u8 sub_node_id;
    u8 sensor_type;
    u8 sensor_status;
    u8 event_attr;
    u8 resv[7]; /* reserved 7 bytes */
};
#pragma pack()
#ifdef CFG_FEATURE_FAULT_FPDC
void fault_unmask_l2buf_irq(struct work_struct *work);
int fault_remap_l2buf(u32 devid);
void fault_unmap_l2buf(u32 devid);
#endif

#endif
