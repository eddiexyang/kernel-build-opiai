/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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

#ifndef _NVME_DRV_H_
#define _NVME_DRV_H_

#include "hipciec_nvme_global_reg_reg_offset.h"

#include "hipciec_nvme_pf_local_ctrl_reg_reg_offset.h"


#define AGENTDRV_CORE_NUM 1
#define AGENTDRV_PORT_NUM 0

#define MAX_AGENTDEV_CNT 4  /* ctrl common */
#define MAX_AGENTFUNC_CNT 1
#define AGENTDRV_1PF_OF_FUNC_TOTAL 1 /* one chip has one die(function) now */

#define HISI_IEP_NVME_DEVICE_ID 0xd102
#define DEVDRV_PCIE_RESERVE_MEM_SIZE (9 * 1024 * 1024)

#define PCI_BAR_RSV_MEM 0
#define PCI_BAR_IO 2
#define PCI_BAR_MEM 4

#define DEVDRV_PF_NUM 0
#define DEVDRV_CORE_NUM 0x1
#define DEVDRV_PORT_NUM 0x0

#define DEVDRV_CHIP_ADDR_SPACE_SIZE 0

#define DEVDRV_IO_TS_DB_OFFSET 0x600000
#define DEVDRV_IO_TS_DB_SIZE 0x400000
#define DEVDRV_IO_TS_SRAM_OFFSET 0xA00000
#define DEVDRV_IO_TS_SRAM_SIZE 0x20000
/* defined for compile */
#define DEVDRV_IO_HWTS_OFFSET 0
#define DEVDRV_IO_HWTS_SIZE 0

#define AGENTDRV_DB_MEM_ADDRESS 0x800000
#define AGENTDRV_DB_MEM_SIZE    0x1000
#define AGENTDRV_DB_STRDE       8
#define AGENTDRV_NVME_DB_IRQ_STRDE 8
#define AGENTDRV_NVME_LOW_LEVEL_DB_IRQ_NUM 2

/* SQ/CQ Doorbell Register Doorbell Interrupt to Local CPU
        Doorbell Number	        MSI Vector Number
        0,8,16,…,120	        11
        1,9,17,…,121	        12
        2,10,18,…,122	        13
        3,11,19,…,123	        14
        4,12,20,…,124	        15
        5,13,21,…,125	        16
        6,14,22,…,126	        17
        7,15,23,…,127	        18
        */
#define AGENTDRV_NVME_DB_IRQ_BASE 11
#define AGENTDRV_NVME_DB_IRQ_NUM 8
#define AGENTDRV_QCNT_EACH_IRQ 16
#define AGENTDRV_TOTALE_DB_IRQ_NUM 128

/* soc doorbell reg info */
#define AGENTDRV_SOC_DB_IRQ_ADDR 0x4001e0000
#define AGENTDRV_SOC_DB_IRQ_SIZE 0x1000
#define AGENTDRV_SOC_DB_RAISE_INIT_ADDR 0x80100c000
#define AGENTDRV_SOC_DB_RAISE_INIT_SIZE 0x1000
#define AGENTDRV_SOC_DB_IRQ_NUM AGENTDRV_NVME_DB_IRQ_NUM
#define AGENTDRV_VF_SOC_DB_IRQ_NUM AGENTDRV_NVME_DB_IRQ_NUM

#define AGENTDRV_MAX_IO_MSG_CHAN 80

#define AGENTDRV_MSG_CHAN_USED_DB_START 0
#define AGENTDRV_MSG_CHAN_USED_DB_END AGENTDRV_MAX_IO_MSG_CHAN

#define AGENTDRV_P2P_MSG_USED_DB_START 88
#define AGENTDRV_P2P_MSG_USED_DB_END 96
#define AGENTDRV_P2P_MSG_USED_DB_NUM (AGENTDRV_P2P_MSG_USED_DB_END - AGENTDRV_P2P_MSG_USED_DB_START)

#define AGENTDRV_GENERAL_INTR_USED_DB_START 96
#define AGENTDRV_GENERAL_INTR_USED_DB_END 127
#define AGENTDRV_GENERAL_INTR_USED_DB_NUM (AGENTDRV_GENERAL_INTR_USED_DB_END - AGENTDRV_GENERAL_INTR_USED_DB_START)

#define AGENTDRV_TEST_USED_DB_ID 127
#define AGENTDRV_TEST_USED_INTR_NUM 2

#define AGENTDRV_NVME_INT_REQ HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_INT_REQ_REG
#define AGENTDRV_NVME_SQ_DB_STS_BASE HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_WR_SQ_DB_INT_STS0_REG
#define AGENTDRV_NVME_CQ_DB_STS_BASE HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_WR_CQ_DB_INT_STS0_REG
#define AGENTDRV_NVME_SQ_DB_INT_MASK_BASE HiPCIECTRL40V200_HIPCIEC_NVME_GLOBAL_REG_PF0_DB_INT_MASK0_REG
#define AGENTDRV_NVME_CQ_DB_INT_MASK_BASE HiPCIECTRL40V200_HIPCIEC_NVME_PF_LOCAL_CTRL_REG_WR_CQ_DB_INT_MASK0_REG

#define DEVDRV_NVME_CTRL_PF_OFST 0x2000

#endif
