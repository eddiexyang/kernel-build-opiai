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

#ifndef SOC_MISC_SPEC_H
#define SOC_MISC_SPEC_H

/* maximum number of device nodes in one device */
#define SOC_MISC_MAX_NODE_NUM 4
 /**
 * device node id
 */
#define SOC_MISC_NODE_0      0
#define SOC_MISC_NODE_1      1
#define SOC_MISC_DEVICE_NUM_MAX 2

#define DIE_DEVICE_OFFSET 0x0ULL
#define RAS_L3D0_BASE_ADDR 0X81130000U
#define RAS_L3D1_BASE_ADDR 0X81140000U
#define RAS_L3D2_BASE_ADDR 0X81150000U
#define RAS_L3D3_BASE_ADDR 0X81160000U
#define RAS_L3D_UNMASK_OFFSET 0x2008
#define RAS_L3D_UNMASK_VAL 0x415U
#define RAS_L3D_NUM 4U

#define RAS_L3T0_BASE_ADDR 0X81170000U
#define RAS_L3T1_BASE_ADDR 0X81180000U
#define RAS_L3T2_BASE_ADDR 0X81190000U
#define RAS_L3T3_BASE_ADDR 0X811A0000U
#define RAS_L3T_UNMASK_OFFSET 0x2008
#define RAS_L3T_UNMASK_VAL 0x415U
#define RAS_L3T_NUM 4U

#endif // SOC_MISC_SPEC_H
