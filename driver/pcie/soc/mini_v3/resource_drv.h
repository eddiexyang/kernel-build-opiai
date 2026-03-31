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
 * BAR        ATU       TYPE               SIZE       OFFSET        Target
 * BAR0(64M)  x         MSI-X Table        4k         0             x
 *            ATU0      rsv mem 0          65532k     0x1000        0x21000000
 * BAR2(128M) ATU1      PCIe dma reg       1536Byte   0x0           0xa0212600
 *            ATU2      ts sybsys SRAM     128k       0x10000       0x500100000
 *            ATU3      tsdrv doorbell     130698k    0x30000       0x508000000
 *            ATU4      STARS reg          16k        0x7fd2800     x
 *            ATU5      L3 SRAM            102k       0x7fd6800     0x90300000
 *            ATU6      soc doorbell       64k        0x7ff0000     0xa0170000
 */

/* DMA chan distribution table. */
#define DMA_CHAN_LOCAL_TOTAL_NUM            6
#define DMA_CHAN_LOCAL_USED_NUM             6
#define DMA_CHAN_LOCAL_USED_START_INDEX     0
#define DMA_CHAN_REMOTE_USED_NUM            6
#define DMA_CHAN_REMOTE_USED_START_INDEX    6
#define DMA_CHAN_TS_USED_NUM                4
#define DMA_CHAN_TS_USED_START_INDEX        12

#define DEVDRV_SYSCTRL_SC_VER 0xC014FFFCU
#define DEVDRV_BOARD_TYPE_MASK 0xFFFF0000U
#define DEVDRV_BOARD_TYPE_OFFSET 16
#define DEVDRV_BOARD_TYPE_ASIC_FPGA 0x0
#define DEVDRV_BOARD_TYPE_ESL 0x2
#define DEVDRV_VERSION_MASK 0xFFFF
#define DEVDRV_VERSION_ASIC 0

#define DEVDRV_SHR_PARA_ADDR_FPGA 0x90018400U
#define DEVDRV_SHR_PARA_ADDR 0x90300400U

#define DEVDRV_TS_SRAM_ADDR 0xAF200000U
#define DEVDRV_TS_SRAM_SIZE 0x20000

/* ieps: dma, platform */
#define AGENTDRV_SUBDEV_NUM 2

void agentdrv_res_pf_dma_chan(struct devdrv_dma_dev *dma_dev);
void agentdrv_dma_res_common(u32 func_id, u32 dma_irq_base, struct devdrv_res_dma_common_info *out);
void agentdrv_res_sdi_addr(u32 dev_id, u32 func_id, struct agent_res_sdi_addr *sdi_addr);
void agentdrv_res_db_queue_id2func_id(u32 db_id, u32 *db_id_in_func);
void agentdrv_res_dma_chan_id2func_id(u32 dma_chan_id, u32 *func_id, u32 *dma_chan_id_in_func);
#endif /* __RESOURCE_DRV_H__ */
