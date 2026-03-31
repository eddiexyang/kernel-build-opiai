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

#include <linux/pci.h>

#include "resource_comm_drv.h"

/*
 * BAR       ATU       TYPE               SIZE     OFFSET        Target
 * BAR0(512M)ATU0      pcie msg queue     49M      0             0x34A00000
 *           ATU1      topic sched        768K     0x1B000000    0x36D40000
 * BAR2(512M)DMA_REMAP pcie dma reg       32K      0             0x801010000
 *           ATU2      ts DOORBELL        64M      0x8000        0x688000000
 *           ATU3      stars slv0         64M      0x4008000     0x6a0000000
 *           ATU4      stars sqcq         256M     0x8008000     0x6a8000000
 *           ATU6      LOAD SRAM/SHR PARA 256k     0x18208000    0x402000000
 *           ATU7      ts sysbsys SRAM    128k     0x18308000    0x680200000
 *           ATU8      soc doorbell       128k     0x18408000    0x4001e0000
 *           ATU9      STARS_CNT_NOTIFY   4K       0x18508000    0x680045000
 * BAR4(64G) ATU10     HBM                64G      0             0xC0000000
 */
#ifdef CFG_SOC_PLATFORM_ESL_FPGA
#define DMA_CHAN_LOCAL_TOTAL_NUM            2
#define DMA_CHAN_LOCAL_USED_NUM             2
#define DMA_CHAN_LOCAL_USED_START_INDEX     0
#define DMA_CHAN_REMOTE_USED_NUM            2
#define DMA_CHAN_REMOTE_USED_START_INDEX    2
#define DMA_CHAN_TS_USED_NUM                4
#define DMA_CHAN_TS_USED_START_INDEX        4
#else
/* DMA chan distribution table. */
#define DMA_CHAN_LOCAL_TOTAL_NUM            13
#define DMA_CHAN_LOCAL_USED_NUM             6 /* 13->6 for mem usage optimization */
#define DMA_CHAN_LOCAL_USED_START_INDEX     0
#define DMA_CHAN_REMOTE_USED_NUM            13
#define DMA_CHAN_REMOTE_USED_START_INDEX    25
#define DMA_CHAN_TS_USED_NUM                12 /* in cloud v2, this is used by stars */
#define DMA_CHAN_TS_USED_START_INDEX        13
#endif

#define DEVDRV_RSV_MEM_BASE 0x34A00000ULL
#define DEVDRV_RSV_VF_MEM_BASE 0x34B00000ULL

#define DEVDRV_RSV_VF_MEM_OFFSET (1 * 1024 * 1024)

#define DEVDRV_SHR_PARA_ADDR 0x402000400ULL
#define DEVDRV_VF_SHR_PARA_ADDR_OFFSET 0x800

#define DEVDRV_PEH_NPU_ADDR_START 0x200000000000ULL
#define DEVDRV_UNIFIED_ADDR_MAINBOARD_ID 0x11

/* ieps: sdi(nvme), dma, platform */
#define AGENTDRV_SUBDEV_NUM 3

void agentdrv_res_pf_dma_chan(struct devdrv_dma_dev *dma_dev);
void agentdrv_dma_res_common(u32 func_id, u32 dma_irq_base, struct devdrv_res_dma_common_info *out);
void agentdrv_res_sdi_addr(u32 dev_id, u32 func_id, struct agent_res_sdi_addr *sdi_addr);
void agentdrv_res_db_queue_id2func_id(u32 db_id, u32 *db_id_in_func);
void agentdrv_res_dma_chan_id2func_id(u32 dma_chan_id, u32 *func_id, u32 *dma_chan_id_in_func);

#endif /* __RESOURCE_DRV_H__ */
