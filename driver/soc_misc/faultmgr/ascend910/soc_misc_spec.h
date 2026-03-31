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
#define SOC_MISC_MAX_NODE_NUM 1

 /**
 * device node id
 */
#define SOC_MISC_NODE_0      0
#define SOC_MISC_NODE_1      1
#define SOC_MISC_NODE_2      2
#define SOC_MISC_NODE_3      3
#define SOC_MISC_DEVICE_NUM_MAX 4

#define DIE_DEVICE_OFFSET 0x0ULL
#define RAS_L3D0_BASE_ADDR 0X89050000
#define RAS_L3D1_BASE_ADDR 0X8B050000
#define RAS_L3D2_BASE_ADDR 0X0 /* For compatibility with the normalized code */
#define RAS_L3D3_BASE_ADDR 0X0 /* For compatibility with the normalized code */

#define RAS_L3D_UNMASK_OFFSET 0x2008
#define RAS_L3D_UNMASK_VAL 0x51DU
#define RAS_L3D0 0x00U
#define RAS_L3D1 0x01U
#define RAS_L3D_NUM 2U

#define RAS_L3T0_BASE_ADDR 0X89060000
#define RAS_L3T1_BASE_ADDR 0X8B060000
#define RAS_L3T2_BASE_ADDR 0X89070000
#define RAS_L3T3_BASE_ADDR 0X8B070000
#define RAS_L3T_UNMASK_OFFSET 0x2008
#define RAS_L3T_UNMASK_VAL 0x51DU
#define RAS_L3T0 0x00U
#define RAS_L3T1 0x01U
#define RAS_L3T2 0x02U
#define RAS_L3T3 0x03U
#define RAS_L3T_NUM 4U

#endif // SOC_MISC_SPEC_H
