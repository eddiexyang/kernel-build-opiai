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

#ifndef _DMA_DRV_H_
#define _DMA_DRV_H_

#include "hipciec_ap_dma_reg_reg_offset.h"

#define HISI_IEP_DMA_DEVICE_ID 0xd101

#define DMA_CTRL0_CHAN_EN_OFFSET 0
#define DMA_CTRL0_CHAN_EN_MASK (0x1 << DMA_CTRL0_CHAN_EN_OFFSET)
#define DMA_CTRL0_ERR_ABORT_EN_OFFSET 2
#define DMA_CTRL0_ERR_ABORT_EN_MASK (0x1 << DMA_CTRL0_ERR_ABORT_EN_OFFSET)
#define DMA_CTRL0_PAUSE_OFFSET 4
#define DMA_CTRL0_PAUSE_MASK (0x1 << DMA_CTRL0_PAUSE_OFFSET)
#define DMA_CTRL0_ARB_WAIGHT_OFFSET 8
#define DMA_CTRL0_ARB_WAIGHT_MASK (0xff << DMA_CTRL0_ARB_WAIGHT_OFFSET)
#define DMA_CTRL0_CQSQ_DIR_OFFSET 24
#define DMA_CTRL0_CQSQ_DIR_MASK (0x1 << DMA_CTRL0_CQSQ_DIR_OFFSET)

#define DMA_CTRL1_QUE_RESET_OFFSET 0
#define DMA_CTRL1_QUE_RESET_MASK (0x1 << DMA_CTRL1_QUE_RESET_OFFSET)

#define DMA_CTRL2_PF_NUM_OFFSET 0
#define DMA_CTRL2_PF_NUM_MASK (0x7 << DMA_CTRL2_PF_NUM_OFFSET)

#define DMA_CTRL2_VF_NUM_OFFSET 16
#define DMA_CTRL2_VF_NUM_MASK (0xff << DMA_CTRL2_VF_NUM_OFFSET)

#define DEVDRV_DMA_MSI_MAX_VECTORS 32

#define DEVDRV_DMA_CHAN_NUM 16
#define DMA_DONE_IRQ_NUM 16
#define DMA_DONE_IRQ_BASE 16
#define DMA_ERR_IRQ_NUM 16
#define DMA_ERR_IRQ_BASE 0

#define DEVDRV_DMA_QUEUE_ERR_STS 0x40 /* DMA_QUEUE_INT_STS */
#define DMA_ERR_MASK 0x7ffe

#define HOST_VF_DMA_MASK 0  /* not support sriov */
#define DEV_VF_DMA_MASK  0  /* not support sriov */

#define DEVDRV_DMA_CHANNEL_IDLE_STATE 0x0
#define DEVDRV_DMA_CHANNEL_RUN_STATE 0x1
#define DEVDRV_DMA_CHANNEL_CPL_STATE 0x2
#define DEVDRV_DMA_CHANNEL_PAUSE_STATE 0x3
#define DEVDRV_DMA_CHANNEL_HALT_STATE 0x4
#define DEVDRV_DMA_CHANNEL_ABORT_STATE 0x5

#define DEVDRV_ADDR_MOVE 32

#define DEVDRV_DMA_CQ_HEAD(cq_depth) ((cq_depth)-1)

#include "dma_comm_drv.h"

#define DEVDRV_DMA_CHAN_ADDR_OFFSET 0


#define MAX_SQ_CQ_BUF_LEN 0x100000
#define MAX_SQ_CQ_BUF_PHY_BASE 0x7FF00000

#endif
