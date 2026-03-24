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

#ifndef __RESOURCE_DRV_H__
#define __RESOURCE_DRV_H__

#include "resource_comm_drv.h"

/*
 * BAR       ATU       TYPE                 SIZE     OFFSET        Target
 * BAR0(8M)  NA        Nvme iep             4K       0             0xA2400000 + nvme
 *           NA        Dma iep              32K      0x4000        0xA2400000 + dma
 *           NA        MSI-X table          8K       0x10000       chip space
 *           ATU0      IO SRAM              512K     0x20000       0xC6F00000/FPGA:0xC6F1E000
 *           ATU1      TS SRAM              4K       0xA0000       0xBA400000
 *           ATU2      devdrv doorbell      4M       0x100000      0xBAC00000
 *           ATU3      hwts AIC             1M       0x500000      0xBA700000
 *           ATU8      HDR                  512K     0x600000      0x31280000
 * BAR2(128M) ATU4      doorbell(1M)/MSG(8M) 9M       0x0           0x0
 *           ATU5      TS queue             64M      0x1000000     0xBB00000
 *           ATU6      kernel/xloder log    10M      0x5000000     0xB100000  kernel 2M(first 4k for test), IMU LOG 8M
 * BAR4      ATU7      DDR(p2p)             0~24G    0             0x95f800000
 */
/* DMA chan distribution table. */
#ifdef CFG_SOC_PLATFORM_ESL_FPGA
#define DMA_CHAN_LOCAL_TOTAL_NUM            2
#define DMA_CHAN_LOCAL_USED_NUM             2
#define DMA_CHAN_LOCAL_USED_START_INDEX     0
#define DMA_CHAN_REMOTE_USED_NUM            2
#define DMA_CHAN_REMOTE_USED_START_INDEX    2
#define DMA_CHAN_TS_USED_NUM                4
#define DMA_CHAN_TS_USED_START_INDEX        4
#else
#define DMA_CHAN_LOCAL_TOTAL_NUM            6
#define DMA_CHAN_LOCAL_USED_NUM             6
#define DMA_CHAN_LOCAL_USED_START_INDEX     0
#define DMA_CHAN_REMOTE_USED_NUM            6
#define DMA_CHAN_REMOTE_USED_START_INDEX    6
#define DMA_CHAN_TS_USED_NUM                4
#define DMA_CHAN_TS_USED_START_INDEX        12

#define DMA_CHAN_DOUBLE_LOCAL_USED_NUM             10
#define DMA_CHAN_DOUBLE_LOCAL_USED_START_INDEX     0
#define DMA_CHAN_DOUBLE_REMOTE_USED_NUM            10
#define DMA_CHAN_DOUBLE_REMOTE_USED_START_INDEX    16
#define DMA_CHAN_DOUBLE_TS_USED_NUM                6
#define DMA_CHAN_DOUBLE_TS_USED_START_INDEX        10
#endif
#define DEVDRV_RM_DOORBELL_QUEUE_PER_PF     128
#define DEVDRV_RM_DOORBELL_QUEUE_TOTL       256
#define DEVDRV_SHR_PARA_ADDR_OFST_PF1       0x1000

#define DEVDRV_CHIP_ADDR_SPACE_SIZE_TEST    0xA00000ULL

#define DEVDRV_CHIP0_PCIE_MEM_BASE_ADDR_HIGH 0x4000000000ULL
#define DEVDRV_CHIP1_PCIE_MEM_BASE_ADDR_HIGH 0xC000000000ULL
#define DEVDRV_CHIP_PCIE_MEM_SIZE_HIGH 0x4000000000ULL
#define DEVDRV_CHIP0_PCIE_MEM_BASE_ADDR_LOW 0xE0000000ULL
#define DEVDRV_CHIP1_PCIE_MEM_BASE_ADDR_LOW 0xE8000000ULL
#define DEVDRV_CHIP_PCIE_MEM_SIZE_LOW 0x8000000ULL

#define DEVDRV_RSV_MEM_BASE 0x100000

#ifdef CFG_SOC_PLATFORM_ESL_FPGA
#define DEVDRV_SHR_PARA_ADDR 0xC6F1E400
#else
#define DEVDRV_SHR_PARA_ADDR 0x100400
#endif
#define DEVDRV_TS_SRAM_ADDR 0xBA400000
#define DEVDRV_TS_SRAM_SIZE 0xA0000

/* ieps: sdi(nvme), dma, platform */
#define AGENTDRV_SUBDEV_NUM 3

void agentdrv_res_pf_dma_chan(struct devdrv_dma_dev *dma_dev);
void agentdrv_dma_res_common(u32 func_id, u32 dma_irq_base, struct devdrv_res_dma_common_info *out);
void agentdrv_res_sdi_addr(u32 dev_id, u32 func_id, struct agent_res_sdi_addr *sdi_addr);
void agentdrv_res_db_queue_id2func_id(u32 db_id, u32 *db_id_in_func);
void agentdrv_res_dma_chan_id2func_id(u32 dma_chan_id, u32 *func_id, u32 *dma_chan_id_in_func);
#endif /* __RESOURCE_DRV_H__ */
