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
#define MAX_AGENTFUNC_CNT 2
#define AGENTDRV_1PF_OF_FUNC_TOTAL 2 /* one chip has two die(function) now */

#define AGENTDRV_NVME_LOW_LEVEL_DB_IRQ_NUM 2

#define HISI_IEP_NVME_DEVICE_ID 0xa124

#define PCI_BAR_IO      0
#define PCI_BAR_RSV_MEM 2
#define PCI_BAR_MEM     4

#define DEVDRV_PF_NUM   0
#define DEVDRV_CORE_NUM 0x1
#define DEVDRV_PORT_NUM 0x0

#define DEVDRV_CHIP_ADDR_SPACE_SIZE 0x8000000000ULL
#define DEVDRV_PCIE_RESERVE_MEM_SIZE (9 * 1024 * 1024)

/* Bar offset */
/* BAR0 IO_MEM */
#define DEVDRV_IO_TS_SRAM_OFFSET    0xA0000     /* ts sram */
#define DEVDRV_IO_TS_SRAM_SIZE      0x1000      /* 4K */
#define DEVDRV_IO_TS_DB_OFFSET      0x100000    /* ts doorbell */
#define DEVDRV_IO_TS_DB_SIZE        0x400000    /* 4M */
#define DEVDRV_IO_HWTS_OFFSET       0x500000    /* hwts aic */
#define DEVDRV_IO_HWTS_SIZE         0x100000


/* BAR2 RSV_MEM */
#define DEVDRV_RESERVE_MEM_DB_BASE      0x0     /*  doorbell base */
#define AGENTDRV_DB_IOMAP_SIZE          0x20000 /* include msi table */

/* Device offset */
/* Doorbell */
#define AGENTDRV_DB_MEM_ADDRESS     0x0
#define AGENTDRV_DB_MEM_SIZE        0x1000 /* Doorbell Mem 1M, pcie need 4K */
#define AGENTDRV_DB_STRDE           8
#define AGENTDRV_MSG_QUEUE_ADDRESS  0x100000

/* ************ interrupt defined for normal host ************* */
#define DEVDRV_MSI_X_MAX_VECTORS 256
#define DEVDRV_MSI_X_MIN_VECTORS 128

/* device os load notify use irq vector 0, later 0 alse use to admin msg chan */
#define DEVDRV_LOAD_MSI_X_VECTOR_NUM 0

/* irq used to msg trans, a msg chan need two vector. one for tx finish, the other for rx msg.
   msg chan 0 is used to admin(chan 0) role */
#define DEVDRV_MSG_MSI_X_VECTOR_BASE 0
#define DEVDRV_MSG_MSI_X_VECTOR_NUM 58

/* irq used to dma, a dma chan need 22 vector. one for cq, the other for err.
  host surport 11 dma chan witch is related to enum devdrv_dma_data_type */
#define DEVDRV_DMA_MSI_X_VECTOR_BASE 58
#define DEVDRV_DMA_MSI_X_VECTOR_NUM 22

/* irq used to devmm */
#define DEVDRV_DEVMM_MSI_X_VECTOR_BASE 80
#define DEVDRV_DEVMM_MSI_X_VECTOR_NUM 48

/* msg chan irq section2 */
#define DEVDRV_MSG_MSI_X_VECTOR_2_BASE 128

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
        128,136,144,…,248	    21
        129,137,145,…,121	    22
        130,138,146,…,122	    23
        131,139,147,…,123	    24
        132,140,148,…,124	    25
        133,141,149,…,125	    26
        134,142,150,…,254	    27
        135,143,151,…,255	    28
        */
#define AGENTDRV_NVME_DB_IRQ_BASE 11
#define AGENTDRV_NVME_DB_IRQ_NUM 16

#define AGENTDRV_NVME_DB_IRQ_BASE_1 11
#define AGENTDRV_NVME_DB_IRQ_END_1 18
#define AGENTDRV_NVME_DB_IRQ_BASE_2 21
#define AGENTDRV_NVME_DB_IRQ_END_2 28

#define AGENTDRV_NVME_DB_IRQ_STRDE 8
#define AGENTDRV_QCNT_EACH_IRQ 16
#define AGENTDRV_NVME_DB_QID_STRDE 128

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

#define AGENTDRV_NVME_INT_REQ HiPCIECTRL40V200_HIPCIEC_NVME_GLOBAL_REG_INT_REQ_REG
#define AGENTDRV_NVME_SQ_DB_STS_BASE HiPCIECTRL40V200_HIPCIEC_NVME_GLOBAL_REG_SQDB_INT_STS0_REG
#define AGENTDRV_NVME_CQ_DB_STS_BASE HiPCIECTRL40V200_HIPCIEC_NVME_GLOBAL_REG_CQDB_INT_STS0_REG
#define AGENTDRV_NVME_SQ_DB_INT_MASK_BASE HiPCIECTRL40V200_HIPCIEC_NVME_GLOBAL_REG_SQDB_INT_MASK0_REG
#define AGENTDRV_NVME_CQ_DB_INT_MASK_BASE HiPCIECTRL40V200_HIPCIEC_NVME_GLOBAL_REG_CQDB_INT_MASK0_REG
#define AGENTDRV_NVME_INT_VECTOR_MASK_BASE HiPCIECTRL40V200_HIPCIEC_NVME_GLOBAL_REG_NVME_VECTOR_MASK_0_REG

#define DEVDRV_NVME_CTRL_PF_OFST 0x2000

#endif
