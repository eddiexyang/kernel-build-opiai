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

#define SOC_MISC_MAX_NODE_NUM 1
 /**
 * device node id
 */
#define SOC_MISC_NODE_0      0
#define SOC_MISC_DEVICE_NUM_MAX 1

#define DIE_DEVICE_OFFSET   0
#define CHIP_DEVICE_OFFSET  0

/* use interrupts mask  */
#define RAS_INT_BASE_ADDR   0x00C01E0000ULL
#define RAS_INT_L3D_BIT 7U
#define RAS_INT_L3T_BIT 8U
#define RAS_INT_CPUCORE0_BIT 30U
#define RAS_INT_CPUCORE1_BIT 31U
#define RAS_INT_CPUCORE2_BIT 0U // 32 % 32
#define RAS_INT_CPUCORE3_BIT 1U // 33 % 32

#define RAS_INT_TYPE_ENABLE_MASK      0x07
#define RAS_INT_TYPE_OFFSET           0x08
#define RAS_INT_TYPE_NUM_MAX          0x03
#define RAS_INT_TYPE0_ENA_L_OFFSET    0x0008
#define RAS_INT_TYPE0_ENA_H_OFFSET    0x000C
#define RAS_INT_TYPE1_ENA_L_OFFSET    0x0010
#define RAS_INT_TYPE1_ENA_H_OFFSET    0x0014

#define RAS_L3D_NUM_MAX 2
#define RAS_L3D_NUM 1

#define RAS_L3D0_BASE_ADDR RAS_INT_BASE_ADDR
#define RAS_L3D0_UNMASK_OFFSET RAS_INT_TYPE0_ENA_L_OFFSET
#define RAS_L3D0_UNMASK_VAL (1 << RAS_INT_L3D_BIT)

#define RAS_L3D1_BASE_ADDR 0
#define RAS_L3D1_UNMASK_OFFSET 0
#define RAS_L3D1_UNMASK_VAL 0

#define RAS_L3T_NUM_MAX 2
#define RAS_L3T_NUM 1

#define RAS_L3T0_BASE_ADDR RAS_INT_BASE_ADDR
#define RAS_L3T0_UNMASK_OFFSET RAS_INT_TYPE0_ENA_L_OFFSET
#define RAS_L3T0_UNMASK_VAL (1 << RAS_INT_L3T_BIT)

#define RAS_L3T1_BASE_ADDR 0
#define RAS_L3T1_UNMASK_OFFSET 0
#define RAS_L3T1_UNMASK_VAL 0

#define RAS_CPUCORE_MAX_NUM 4U
#define RAS_CPUCORE_NUM 4U

#define RAS_CPUCORE0_BASE_ADDR RAS_INT_BASE_ADDR
#define RAS_CPUCORE0_UNMASK_OFFSET RAS_INT_TYPE0_ENA_L_OFFSET
#define RAS_CPUCORE0_UNMASK_VAL (1 << RAS_INT_CPUCORE0_BIT)

#define RAS_CPUCORE1_BASE_ADDR RAS_INT_BASE_ADDR
#define RAS_CPUCORE1_UNMASK_OFFSET RAS_INT_TYPE0_ENA_L_OFFSET
#define RAS_CPUCORE1_UNMASK_VAL (1 << RAS_INT_CPUCORE1_BIT)

#define RAS_CPUCORE2_BASE_ADDR RAS_INT_BASE_ADDR
#define RAS_CPUCORE2_UNMASK_OFFSET RAS_INT_TYPE0_ENA_H_OFFSET
#define RAS_CPUCORE2_UNMASK_VAL (1 << RAS_INT_CPUCORE2_BIT)

#define RAS_CPUCORE3_BASE_ADDR RAS_INT_BASE_ADDR
#define RAS_CPUCORE3_UNMASK_OFFSET RAS_INT_TYPE0_ENA_H_OFFSET
#define RAS_CPUCORE3_UNMASK_VAL (1 << RAS_INT_CPUCORE3_BIT)

#define RAS_INT_PCIE_0_BIT BIT(19)
#define RAS_INT_PCIE_1_BIT BIT(20)
#define RAS_INT_PCIE_2_BIT BIT(21)
#define RAS_INT_PCIE_3_BIT BIT(22)

#define RAS_PCIE_CE_UNMASK_ADDR (RAS_INT_BASE_ADDR + RAS_INT_TYPE0_ENA_L_OFFSET) // CE
#define RAS_PCIE_NFE_UNMASK_ADDR (RAS_INT_BASE_ADDR + RAS_INT_TYPE1_ENA_L_OFFSET) // NFE
#define RAS_PCIE_UNMASK_VAL (RAS_INT_PCIE_0_BIT | RAS_INT_PCIE_1_BIT | RAS_INT_PCIE_2_BIT | RAS_INT_PCIE_3_BIT)
#define RAS_PCIE_UNMASK_REMAP_SIZE 0x4

#endif // SOC_MISC_SPEC_H
