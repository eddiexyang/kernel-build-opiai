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

/* ****************************** mini v1 ***************************************
 * BAR       ATU       TYPE               SIZE     OFFSET        Target
 * BAR0      ATU0      Doorbell           4k       0             0x800000
 * BAR2      ATU1      APB                2M       0x0           0x110200000
 *           ATU2      IEP BAR            3M       0x300000      0x110800000
 *           ATU3      devdrv Doorbell    4M       0x600000      0x1f0400000
 *           ATU4      TS SRAM            128k     0xA00000      0x1f0200000
 *           ATU5      IO SRAM            256k     0xA20000      0x112000000
 * BAR4      ATU9      DDR                2G       0             0x0
 *           ATU10     DDR                2G       0x80000000    0x880000000
 *           ATU11     DDR                4G       0x100000000   0x900000000
 */
/* DMA chan distribution table. */
#define DMA_CHAN_LOCAL_TOTAL_NUM          6
#define DMA_CHAN_LOCAL_USED_NUM           6
#define DMA_CHAN_LOCAL_USED_START_INDEX   0
#define DMA_CHAN_REMOTE_USED_NUM          6
#define DMA_CHAN_REMOTE_USED_START_INDEX  6
#define DMA_CHAN_TS_USED_NUM              4
#define DMA_CHAN_TS_USED_START_INDEX      12

#define DEVDRV_RSV_MEM_BASE  0

#define DEVDRV_SHR_PARA_ADDR 0x400

/* ieps: sdi(nvme), dma, platform */
#define AGENTDRV_SUBDEV_NUM 3

void agentdrv_res_pf_dma_chan(struct devdrv_dma_dev *dma_dev);
void agentdrv_dma_res_common(u32 func_id, u32 dma_irq_base, struct devdrv_res_dma_common_info *out);
void agentdrv_res_sdi_addr(u32 dev_id, u32 func_id, struct agent_res_sdi_addr *sdi_addr);
void agentdrv_res_db_queue_id2func_id(u32 db_id, u32 *db_id_in_func);
void agentdrv_res_dma_chan_id2func_id(u32 dma_chan_id, u32 *func_id, u32 *dma_chan_id_in_func);
#endif /* __RESOURCE_DRV_H__ */
