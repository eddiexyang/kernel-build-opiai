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

#include <linux/types.h>
#include <linux/pci.h>

#include "resource_comm_drv.h"

/*
 * BAR       ATU       TYPE               SIZE     OFFSET        Target
 * BAR0(64M) ATU0      Doorbell           1M       0             0x800000
             ATU6      test mem           2M       0x200000      0x6FE00000
             ATU7      pcie msg queue     8M       0x800000      0x0
             ATU12     IMU BBOX LOG       8MB      0x1000000     0x5f800000
             ATU8      ts msg queue       32M      0x2000000     0x60000000
 * BAR2(16M) ATU1      APB                2M       0x0           0x148200000
 *           ATU2      IEP BAR            5M       0x500000      0x148800000
 *           ATU3      devdrv Doorbell    4M       0xA00000      0xaf400000
 *           ATU4      TS SRAM            128k     0xE00000      0xaf200000
 *           ATU5      IO SRAM            256k     0xE20000      0xA0D22000
 *           x         hwts               64k      0xE60000      0xaf110000
             x         HDR                8k       0xE70000
 * BAR4      ATU9      HBM                1G       0             0x400000000
 */
/* DMA chan distribution table. */
#define DMA_CHAN_LOCAL_TOTAL_NUM            11
#define DMA_CHAN_LOCAL_USED_NUM             11
#define DMA_CHAN_LOCAL_USED_START_INDEX     0
#define DMA_CHAN_REMOTE_USED_NUM            11
#define DMA_CHAN_REMOTE_USED_START_INDEX    11
#define DMA_CHAN_TS_USED_NUM                8
#define DMA_CHAN_TS_USED_START_INDEX        22

#define DEVDRV_RSV_MEM_BASE 0

#define DEVDRV_SHR_PARA_ADDR 0xA0D22400

#define DEVDRV_TS_SRAM_ADDR 0xAF200000
#define DEVDRV_TS_SRAM_SIZE 0x20000

/* ieps: sdi(nvme), dma, platform */
#define AGENTDRV_SUBDEV_NUM 3

void agentdrv_res_pf_dma_chan(struct devdrv_dma_dev *dma_dev);
void agentdrv_dma_res_common(u32 func_id, u32 dma_irq_base, struct devdrv_res_dma_common_info *out);
void agentdrv_res_sdi_addr(u32 dev_id, u32 func_id, struct agent_res_sdi_addr *sdi_addr);
void agentdrv_res_db_queue_id2func_id(u32 db_id, u32 *db_id_in_func);
void agentdrv_res_dma_chan_id2func_id(u32 dma_chan_id, u32 *func_id, u32 *dma_chan_id_in_func);
#endif /* __RESOURCE_DRV_H__ */
