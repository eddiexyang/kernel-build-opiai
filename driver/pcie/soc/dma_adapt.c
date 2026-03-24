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
#include "devdrv_dma.h"
#include "devdrv_util.h"

#define DMA_QUEUE_SQ_TAIL 0xc
#define DMA_QUEUE_CQ_HEAD 0x1c
#define DMA_QUEUE_CQ_TAIL 0x3c
#define DMA_QUEUE_SQ_READ_ERR_PTR 0x68

void devdrv_dma_chan_ptr_show(struct devdrv_dma_channel *dma_chan)
{
    u32 sq_tail, cq_tail, cq_head;
    void __iomem *io_base = dma_chan->io_base;

    sq_tail = readl(io_base + DMA_QUEUE_SQ_TAIL);
    cq_head = readl(io_base + DMA_QUEUE_CQ_HEAD);
    cq_tail = readl(io_base + DMA_QUEUE_CQ_TAIL);

    devdrv_err("Get dma_chan.(hardware_sq_tail=%u; cq_head=%u; cq_tail=%u; software_sq_tail=%u; sq_head=%u; "
        "cq_head=%u)\n", sq_tail, cq_head, cq_tail, dma_chan->sq_tail, dma_chan->sq_head, dma_chan->cq_head);
}

void devdrv_set_dma_sq_tail(void __iomem *io_base, u32 val)
{
    writel(val, io_base + DMA_QUEUE_SQ_TAIL);
}

void devdrv_set_dma_cq_head(void __iomem *io_base, u32 val)
{
    writel(val, io_base + DMA_QUEUE_CQ_HEAD);
}

u32 devdrv_get_sq_err_ptr(const void __iomem *io_base)
{
    return readl(io_base + DMA_QUEUE_SQ_READ_ERR_PTR);
}

void devdrv_dma_set_sq_addr_info(struct devdrv_dma_sq_node *sq_desc, u64 src_addr, u64 dst_addr, u32 length)
{
    sq_desc->src_addr_l = (u32)src_addr;
    sq_desc->src_addr_h = (u32)(src_addr >> DEVDRV_ADDR_SHIFT_32);

    sq_desc->dst_addr_l = (u32)dst_addr;
    sq_desc->dst_addr_h = (u32)(dst_addr >> DEVDRV_ADDR_SHIFT_32);

    sq_desc->length = length;
}

void devdrv_dma_set_sq_attr(struct devdrv_dma_sq_node *sq_desc, u32 opcode, u32 attr,
    const struct devdrv_dma_dev *dma_dev, u32 wd_barrier, u32 rd_barrier)
{
    sq_desc->opcode = opcode;
    /* RO.remote flag for RD.remote np and WD.remote p */
    sq_desc->attr = attr;
    /* RO.local flag for RD.local p and WD.local np */
    sq_desc->attr_d = attr;
    sq_desc->pf = dma_dev->dma_pf_num;
    sq_desc->vfen = dma_dev->dma_vf_en;
    sq_desc->vf = dma_dev->dma_vf_num;
    sq_desc->wd_barrier = wd_barrier;
    sq_desc->rd_barrier = rd_barrier;
    sq_desc->pa_loc = DEVDRV_DMA_DES_PA_LOC_VA;
    sq_desc->addrt_d = DEVDRV_DMA_DES_AT_LOC_VA;

    devdrv_debug_spinlock("Get sq_desc information. (opcode=%x; attr=%x; pf=%x; vf=%x; wb_barrier=%x; "
        "rd_barrier=%x)\n", opcode, attr, dma_dev->dma_pf_num, dma_dev->dma_vf_num, wd_barrier, rd_barrier);
}

void devdrv_dma_set_sq_irq(struct devdrv_dma_sq_node *sq_desc, u32 rdie, u32 ldie, u32 msi)
{
    sq_desc->rdie = rdie;
    sq_desc->ldie = ldie;
    sq_desc->msi_l = msi & DMA_MSI_L_MASK;
    sq_desc->msi_h = (msi >> DMA_MSI_H_BIT_OFFSET) & DMA_MSI_H_MASK;
    devdrv_debug_spinlock("Get sq_desc information. (rdie=%x; ldie=%x; msi=%x)\n", rdie, ldie, msi);
}

void devdrv_dma_set_passid(struct devdrv_dma_sq_node *sq_desc, u32 loc_passid)
{
#ifdef CFG_FEATURE_PASSID
    if (loc_passid == DEVDRV_DMA_PASSID_DEFAULT) {
        return;
    }

    sq_desc->pa_rmt = DEVDRV_DMA_DES_PA_RMT_VA;
    sq_desc->pa_loc = DEVDRV_DMA_DES_PA_LOC_VA;
    sq_desc->addrt_d = DEVDRV_DMA_DES_AT_LOC_VA;
    sq_desc->flow_id_rmt = loc_passid & DEVDRV_DMA_DES_FLOW_ID_RMT_MASK;
    sq_desc->flow_id_loc_l = (loc_passid >> DEVDRV_DMA_DES_FLOW_ID_LOC_L_SHIFT) & DEVDRV_DMA_DES_FLOW_ID_LOC_L_MASK;
    sq_desc->flow_id_loc_h = (loc_passid >> DEVDRV_DMA_DES_FLOW_ID_LOC_H_SHIFT) & DEVDRV_DMA_DES_FLOW_ID_LOC_H_MASK;
#endif
}

bool devdrv_dma_get_cq_valid_flip(struct devdrv_dma_cq_node *cq_desc, u32 rounds)
{
    /* In order to avoid the software clearing the valid flag,
    the hardware will change from 1 to 0 after the cq is used up,
    and so on. 1 is valid for the first time */
    if (rounds & 0x1) {
        return (cq_desc->vld == 0);
    } else {
        return (cq_desc->vld == 1);
    }
}

void devdrv_dma_set_cq_invalid_flip(struct devdrv_dma_cq_node *cq_desc)
{
    /* no need to set */
    (void)cq_desc;
}

bool devdrv_dma_get_cq_valid_normal(struct devdrv_dma_cq_node *cq_desc, u32 rounds)
{
    (void)rounds;
    return (cq_desc->vld == 1);
}

void devdrv_dma_set_cq_invalid_normal(struct devdrv_dma_cq_node *cq_desc)
{
    cq_desc->vld = 0;
}

u32 devdrv_dma_get_cq_sqhd(const struct devdrv_dma_cq_node *cq_desc)
{
    return (u32)cq_desc->sqhd;
}

u32 devdrv_dma_get_cq_status(const struct devdrv_dma_cq_node *cq_desc)
{
    return (u32)cq_desc->status;
}

void devdrv_dma_ops_init(struct devdrv_dma_dev *dma_dev, u32 chip_type)
{
    if (chip_type == HISI_MINI_V1) {
        dma_dev->ops.devdrv_dma_get_cq_valid = devdrv_dma_get_cq_valid_normal;
        dma_dev->ops.devdrv_dma_set_cq_invalid = devdrv_dma_set_cq_invalid_normal;
    } else {
        dma_dev->ops.devdrv_dma_get_cq_valid = devdrv_dma_get_cq_valid_flip;
        dma_dev->ops.devdrv_dma_set_cq_invalid = devdrv_dma_set_cq_invalid_flip;
    }
}

