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

#ifndef PLATFORM_CHIP_H
#define PLATFORM_CHIP_H

/* not used */
#define DMS_TS_IPC_CHAN_ID  -1
#define DMS_LP_IPC_CHAN_ID  -1

#define CORE_NUM_PER_CHIP 8
/* sharemem baseaddr */
#define SHAREMEM_BASE_ADDR 0

/* aicore num */
#define DMS_AI_CORE_NUM 10

/* aivector num */
#define DMS_AI_VECTOR_NUM 8

/* L3T register */
/* L3T number */
#define L3T_TOTAL_NUM 0x4

/* L3T register base addr */
#define L3_TAG0_REG_BASE 0x000081170000
#define L3_TAG1_REG_BASE 0x000081180000
#define L3_TAG2_REG_BASE 0x000081190000
#define L3_TAG3_REG_BASE 0x0000811A0000
#define L3T_CHIP_REG_BASE_OFFSET 0x8000000000
/* adapt for other chips */
#define L3T_DIE_REG_BASE_OFFSET  0
/* chip die offset */
#define ASCEND_CHIP_ADDR_OFFSET 0x8000000000

/* bbox ddr dump address */
#define ASCEND_PLATFORM_MEMDUMP_ADDR 0x2F900000
#define ASCEND_PLATFORM_MEMDUMP_SIZE 0x01E00000

/* Bbox export register feature */
#define PCIE_DDR_READ_REG_BASE 0x5F300000
#define PCIE_DDR_READ_REG_SIZE 0x500000

/* HCCS profiling */
#define HCCS_NUM 0

/* sys_ctrl address */
#ifdef CFG_SOC_MDC_V51_LITE
#define SYSCTL_REG_BASE_ADDR      0x80000000U
#define DRV_BOARD_ID_REG_OFFSET   0xF0A0

/* slot id address */
#define DRV_SLOT_ID_REG_OFFSET    0xE08C
#else
#define SYSCTL_REG_BASE_ADDR      0x80000000U
#define DRV_BOARD_ID_REG_OFFSET   0xF07C
#endif

#define SYSCTL_REG_SIZE           0x10000

#define SC_PAD_INFO_BASE 0x8000E000

#endif
