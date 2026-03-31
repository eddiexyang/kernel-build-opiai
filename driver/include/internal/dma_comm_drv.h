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

#ifndef _DMA_COMM_DRV_H_
#define _DMA_COMM_DRV_H_

#ifdef CONFIG_DEBUG_BUGVERBOSE
#undef CONFIG_DEBUG_BUGVERBOSE
#endif

#include <linux/sched.h>
#include <linux/types.h>
#include <asm/io.h>
#include "drv_log.h"

#define DEVDRV_DMA_TIMEOUT 1000000 /* 1s */
#define DEVDRV_DMA_QUEUE_NOT_WORK 0x1

#define DMA_FSM_QUE_STS_OFFSET 0
#define DMA_FSM_QUE_STS_MASK (0xf << DMA_FSM_QUE_STS_OFFSET)
#define DMA_FSM_QUE_WORK_OFFSET 4
#define DMA_FSM_QUE_WORK_MASK (0x1 << DMA_FSM_QUE_WORK_OFFSET)

#define DMA_SQ_HEAD_OFFSET 0
#define DMA_SQ_HEAD_MASK (0xffff << DMA_SQ_HEAD_OFFSET)

#define DMA_CQ_TAIL_OFFSET 0
#define DMA_CQ_TAIL_MASK (0xffff << DMA_CQ_TAIL_OFFSET)

#define DEVDRV_DMA_QUEUE_SQ_BASE_L 0x0
#define DEVDRV_DMA_QUEUE_SQ_BASE_H 0x4
#define DEVDRV_DMA_QUEUE_SQ_DEPTH 0x8
#define DEVDRV_DMA_QUEUE_SQ_TAIL 0xc
#define DEVDRV_DMA_QUEUE_CQ_BASE_L 0x10
#define DEVDRV_DMA_QUEUE_CQ_BASE_H 0x14
#define DEVDRV_DMA_QUEUE_CQ_DEPTH 0x18
#define DEVDRV_DMA_QUEUE_CQ_HEAD 0x1c
#define DEVDRV_DMA_QUEUE_CTRL0 0x20
#define DEVDRV_DMA_QUEUE_CTRL1 0x24
#define DEVDRV_DMA_QUEUE_FSM_STS 0x30
#define DEVDRV_DMA_QUEUE_SQ_STS 0x34
#define DEVDRV_DMA_QUEUE_CQ_TAIL 0x3c
#define DEVDRV_DMA_QUEUE_INT_STS 0x40
#define DEVDRV_DMA_QUEUE_INT_MSK 0x44
#define DEVDRV_DMA_QUEUE_DESP0 0x50
#define DEVDRV_DMA_QUEUE_DESP1 0x54
#define DEVDRV_DMA_QUEUE_DESP2 0x58
#define DEVDRV_DMA_QUEUE_DESP3 0x5c
#define DEVDRV_DMA_QUEUE_ERR_ADDR_L 0x60
#define DEVDRV_DMA_QUEUE_ERR_ADDR_H 0x64
#define DEVDRV_DMA_QUEUE_SQ_READ_ERR_PTR 0x68
#define DEVDRV_DMA_QUEUE_INT_RO 0x6c
#define DEVDRV_DMA_INIT_SET 0x70
#define DEVDRV_DMA_QUEUE_DESP4 0x74
#define DEVDRV_DMA_QUEUE_DESP5 0x78
#define DEVDRV_DMA_QUEUE_DESP6 0x7c
#define DEVDRV_DMA_QUEUE_DESP7 0x80
#define DEVDRV_DMA_QUEUE_CTRL2 0x9c

#define AGENTDRV_AP_SDI_AXIM_REG 0x13000

#define AGENTDRV_DMA_URCA_ERR  0x7c8d
#define AGENTDRV_DMA_URCA_MASK BIT(1) /* Submission descriptor read response error */

struct devdrv_dma_cfg_info {
    u32 pf_num;
    u32 vf_num;
    u64 sq_addr;
    u64 cq_addr;
    u32 sq_depth;
    u32 cq_depth;
    u32 sqcq_side;
};

void devdrv_dma_reg_wr(void __iomem *io_base, u32 offset, u32 val);
void devdrv_dma_reg_rd(const void __iomem *io_base, u32 offset, u32 *val);

int devdrv_get_dma_err_chan(const void __iomem *io_base, u32 *chan_id, u32 *side);

void devdrv_get_dma_queue_sts(const void __iomem *io_base, u32 *val);
int devdrv_dma_check_fsm_sts(const void __iomem *io_base, unsigned long timeout);

int devdrv_dma_ch_cfg_reset(void __iomem *io_base, u32 sriov_flag);
void devdrv_dma_ch_cfg_init(void __iomem *io_base, const struct devdrv_dma_cfg_info *dma_info);
void devdrv_record_dma_dxf_info(void __iomem *io_base, u32 *queue_init_sts);

void __iomem * devdrv_get_dma_chan_base(const void __iomem *dma_base);

void devdrv_set_va_enable(void __iomem *io_base);
void devdrv_set_dma_chan_en(void __iomem *io_base, u32 val);
void devdrv_dma_err_interrupt_unmask(void __iomem *io_base);
void devdrv_set_dma_arb_weight(void __iomem *io_base, u32 val);
void devdrv_dma_chan_err_interrupt_mask(void __iomem *io_base, u32 val);
void devdrv_set_dma_pfvf_num(void __iomem *io_base, u32 pfval, u32 vfval);

#endif
