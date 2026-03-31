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
#define SOC_MISC_MAX_NODE_NUM 2
 /**
 * device node id
 */
#define SOC_MISC_NODE_0      0
#define SOC_MISC_NODE_1      1
#define SOC_MISC_DEVICE_NUM_MAX 2

#define DIE_DEVICE_OFFSET 0x10000000000ULL
#define CHIP_DEVICE_OFFSET 0x80000000000ULL
#define RAS_L3D_UNMASK_OFFSET 0x0014
#define RAS_L3D_UNMASK_VAL 0x1800

#define RAS_L3T_UNMASK_OFFSET 0x0014
#define RAS_L3T_UNMASK_VAL 0x0600

/* use interrupts mask  */
#define RAS_INT_BASE_ADDR   0X000703860000ULL
#define RAS_INT_L3D0_BIT 11U // 43 % 32
#define RAS_INT_L3D1_BIT 12U // 44 % 32
#define RAS_INT_L3T0_BIT 9U  // 41 % 32
#define RAS_INT_L3T1_BIT 10U // 42 % 32

#define RAS_INT_TYPE_ENABLE_MASK      0x07
#define RAS_INT_TYPE_OFFSET           0x08
#define RAS_INT_TYPE_NUM_MAX          0x03
#define RAS_INT_TYPE0_ENA_L_OFFSET    0x0008
#define RAS_INT_TYPE0_ENA_H_OFFSET    0x000C
#define RAS_INT_TYPE1_ENA_L_OFFSET    0x0010
#define RAS_INT_TYPE1_ENA_H_OFFSET    0x0014

#define RAS_L3D_NUM_MAX 2
#define RAS_L3D_NUM 2

#define RAS_L3D0_BASE_ADDR RAS_INT_BASE_ADDR
#define RAS_L3D0_UNMASK_OFFSET RAS_INT_TYPE0_ENA_H_OFFSET
#define RAS_L3D0_UNMASK_VAL (1 << RAS_INT_L3D0_BIT)

#define RAS_L3D1_BASE_ADDR RAS_INT_BASE_ADDR
#define RAS_L3D1_UNMASK_OFFSET RAS_INT_TYPE0_ENA_H_OFFSET
#define RAS_L3D1_UNMASK_VAL (1 << RAS_INT_L3D1_BIT)

#define RAS_L3T_NUM_MAX 2
#define RAS_L3T_NUM 2

#define RAS_L3T0_BASE_ADDR RAS_INT_BASE_ADDR
#define RAS_L3T0_UNMASK_OFFSET RAS_INT_TYPE0_ENA_H_OFFSET
#define RAS_L3T0_UNMASK_VAL (1 << RAS_INT_L3T0_BIT)

#define RAS_L3T1_BASE_ADDR RAS_INT_BASE_ADDR
#define RAS_L3T1_UNMASK_OFFSET RAS_INT_TYPE0_ENA_H_OFFSET
#define RAS_L3T1_UNMASK_VAL (1 << RAS_INT_L3T1_BIT)

#define RAS_INT_SMMU_PCIE_BIT  BIT(15) // 47 % 32
#define RAS_INT_PCIE_LOCAL_BIT BIT(16) // 48 % 32
#define RAS_INT_DISP_PCIE_BIT  BIT(17) // 49 % 32

#define RAS_PCIE_CE_UNMASK_ADDR (RAS_INT_BASE_ADDR + RAS_INT_TYPE0_ENA_H_OFFSET) // CE
#define RAS_PCIE_NFE_UNMASK_ADDR (RAS_INT_BASE_ADDR + RAS_INT_TYPE1_ENA_H_OFFSET) // NFE
#define RAS_PCIE_UNMASK_VAL (RAS_INT_SMMU_PCIE_BIT | RAS_INT_PCIE_LOCAL_BIT | RAS_INT_DISP_PCIE_BIT)
#define RAS_PCIE_UNMASK_REMAP_SIZE 0x4

#endif // SOC_MISC_SPEC_H
