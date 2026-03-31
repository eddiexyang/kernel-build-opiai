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

#include <linux/delay.h>
#include <linux/errno.h>

#include "dma_drv.h"
#include "dma_adapt.h"
#include "nvme_comm_drv.h"
#include "devdrv_interface.h"
#include "resource_drv.h"

void devdrv_dma_reg_wr(void __iomem *io_base, u32 offset, u32 val)
{
    writel(val, io_base + offset);
}

void devdrv_dma_reg_rd(const void __iomem *io_base, u32 offset, u32 *val)
{
    *val = readl(io_base + offset);
}

STATIC void devdrv_clear_dma_err_status(void __iomem *io_base)
{
    devdrv_dma_reg_wr(io_base, DEVDRV_DMA_QUEUE_ERR_STS, DMA_ERR_MASK);
}

void devdrv_set_dma_sq_base(void __iomem *io_base, u64 val)
{
    devdrv_dma_reg_wr(io_base, DEVDRV_DMA_QUEUE_SQ_BASE_H, (u32)(val >> 32));
    devdrv_dma_reg_wr(io_base, DEVDRV_DMA_QUEUE_SQ_BASE_L, (u32)val);
}

void devdrv_set_dma_sq_depth(void __iomem *io_base, u32 val)
{
    /* the controller will +1 at the set depth by software */
    devdrv_dma_reg_wr(io_base, DEVDRV_DMA_QUEUE_SQ_DEPTH, val - 1);
}

void devdrv_set_dma_cq_base(void __iomem *io_base, u64 val)
{
    devdrv_dma_reg_wr(io_base, DEVDRV_DMA_QUEUE_CQ_BASE_H, (u32)(val >> 32));
    devdrv_dma_reg_wr(io_base, DEVDRV_DMA_QUEUE_CQ_BASE_L, (u32)val);
}

void devdrv_set_dma_cq_depth(void __iomem *io_base, u32 val)
{
    /* the controller will +1 at the set depth by software */
    devdrv_dma_reg_wr(io_base, DEVDRV_DMA_QUEUE_CQ_DEPTH, val - 1);
}

void devdrv_set_dma_queue_pause(void __iomem *io_base, u32 val)
{
    u32 regval;

    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_CTRL0, &regval);
    regval &= ~DMA_CTRL0_PAUSE_MASK;
    regval |= (val << DMA_CTRL0_PAUSE_OFFSET) & DMA_CTRL0_PAUSE_MASK;

    devdrv_dma_reg_wr(io_base, DEVDRV_DMA_QUEUE_CTRL0, regval);
}

void devdrv_set_dma_err_abort_en(void __iomem *io_base, u32 val)
{
    u32 regval;

    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_CTRL0, &regval);
    regval &= ~DMA_CTRL0_ERR_ABORT_EN_MASK;
    regval |= (val << DMA_CTRL0_ERR_ABORT_EN_OFFSET) & DMA_CTRL0_ERR_ABORT_EN_MASK;

    devdrv_dma_reg_wr(io_base, DEVDRV_DMA_QUEUE_CTRL0, regval);
}

void devdrv_set_dma_chan_en(void __iomem *io_base, u32 val)
{
    u32 regval;

    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_CTRL0, &regval);
    regval &= ~DMA_CTRL0_CHAN_EN_MASK;
    regval |= (val << DMA_CTRL0_CHAN_EN_OFFSET) & DMA_CTRL0_CHAN_EN_MASK;

    devdrv_dma_reg_wr(io_base, DEVDRV_DMA_QUEUE_CTRL0, regval);
}

void devdrv_get_dma_chan_en(const void __iomem *io_base, u32 *val)
{
    u32 regval;

    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_CTRL0, &regval);
    *val = (regval & DMA_CTRL0_CHAN_EN_MASK) >> DMA_CTRL0_CHAN_EN_OFFSET;
}

void devdrv_set_dma_sqcq_dir(void __iomem *io_base, u32 val)
{
    u32 regval;

    if (val == 1) {
        val = DMA_CTRL0_CQSQ_DIR_MASK >> DMA_CTRL0_CQSQ_DIR_OFFSET;
    } else {
        val = 0;
    }

    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_CTRL0, &regval);
    regval &= ~DMA_CTRL0_CQSQ_DIR_MASK;
    regval |= (val << DMA_CTRL0_CQSQ_DIR_OFFSET) & DMA_CTRL0_CQSQ_DIR_MASK;

    devdrv_dma_reg_wr(io_base, DEVDRV_DMA_QUEUE_CTRL0, regval);
}

void devdrv_set_dma_queue_reset(void __iomem *io_base, u32 val)
{
    u32 regval;

    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_CTRL1, &regval);
    regval &= ~DMA_CTRL1_QUE_RESET_MASK;
    regval |= (val << DMA_CTRL1_QUE_RESET_OFFSET) & DMA_CTRL1_QUE_RESET_MASK;

    devdrv_dma_reg_wr(io_base, DEVDRV_DMA_QUEUE_CTRL1, regval);
}

void devdrv_get_dma_queue_sts(const void __iomem *io_base, u32 *val)
{
    u32 regval;

    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_FSM_STS, &regval);
    *val = (regval & DMA_FSM_QUE_STS_MASK) >> DMA_FSM_QUE_STS_OFFSET;
}

void devdrv_get_dma_queue_not_work(const void __iomem *io_base, u32 *val)
{
    u32 regval;

    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_FSM_STS, &regval);
    *val = (regval & DMA_FSM_QUE_WORK_MASK) >> DMA_FSM_QUE_WORK_OFFSET;
}

void devdrv_dma_interrupt_mask(void __iomem *io_base, u32 val)
{
    devdrv_dma_reg_wr(io_base, DEVDRV_DMA_QUEUE_INT_MSK, val);
}

void devdrv_set_dma_pfvf_num(void __iomem *io_base, u32 pfval, u32 vfval)
{
    u32 regval;

    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_CTRL2, &regval);
    regval &= ~DMA_CTRL2_PF_NUM_MASK;
    regval |= (pfval << DMA_CTRL2_PF_NUM_OFFSET) & DMA_CTRL2_PF_NUM_MASK;
    regval &= ~DMA_CTRL2_VF_NUM_MASK;
    regval |= (vfval << DMA_CTRL2_VF_NUM_OFFSET) & DMA_CTRL2_VF_NUM_MASK;

    devdrv_dma_reg_wr(io_base, DEVDRV_DMA_QUEUE_CTRL2, regval);
}

void devdrv_record_sq_err_ptr(const void __iomem *io_base)
{
    u32 regval;

    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_SQ_READ_ERR_PTR, &regval);
    devdrv_err("Get the sq read error pointer. (regval=%d)\n", regval);
}

void devdrv_record_err_addr(const void __iomem *io_base)
{
    u32 regval;
    u64 val;

    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_ERR_ADDR_H, &regval);
    val = ((u64)regval << 32);

    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_ERR_ADDR_L, &regval);
    val |= regval;

    devdrv_err("Get the error address. (val=%lld)\n", val);
}

void devdrv_record_dma_queue_desp(const void __iomem *io_base)
{
    u32 reg_val;

    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_DESP0, &reg_val);
    devdrv_err("The 1st DW of current descriptor is reg_val. (reg_val=0x%x)\n", reg_val);
    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_DESP1, &reg_val);
    devdrv_err("The 2st DW of current descriptor is reg_val. (reg_val=0x%x)\n", reg_val);
    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_DESP2, &reg_val);
    devdrv_err("The 3st DW of current descriptor is reg_val. (reg_val=0x%x)\n", reg_val);
    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_DESP3, &reg_val);
    devdrv_err("The 4st DW of current descriptor is reg_val. (reg_val=0x%x)\n", reg_val);
    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_DESP4, &reg_val);
    devdrv_err("The 5st DW of current descriptor is reg_val. (reg_val=0x%x)\n", reg_val);
    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_DESP5, &reg_val);
    devdrv_err("The 6st DW of current descriptor is reg_val. (reg_val=0x%x)\n", reg_val);
    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_DESP6, &reg_val);
    devdrv_err("The 7st DW of current descriptor is reg_val. (reg_val=0x%x)\n", reg_val);
    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_DESP7, &reg_val);
    devdrv_err("The 8st DW of current descriptor is reg_val. (reg_val=0x%x)\n", reg_val);
}

/* printk the dma channel dxf register */
void devdrv_record_dma_dxf_info(void __iomem *io_base, u32 *queue_init_sts)
{
    u32 val = 0;
    devdrv_record_dma_queue_desp(io_base);
    devdrv_record_err_addr(io_base);
    devdrv_record_sq_err_ptr(io_base);
    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_SQ_STS, &val);
    devdrv_err("Get the hardware sq head. (value=%u)\n", val & 0xFFFF);
    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_SQ_TAIL, &val);
    devdrv_err("Get the hardware sq tail. (value=%u)\n", val);
    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_CQ_HEAD, &val);
    devdrv_err("Get the hardware cq head. (value=%u)\n", val);
    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_CQ_TAIL, &val);
    devdrv_err("Get the hardware cq tail. (value=%u)\n", val);
    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_ERR_STS, &val);
    devdrv_err("Get the err type. (value=0x%x)\n", val);
    *queue_init_sts = val;
    devdrv_dma_reg_wr(io_base, DEVDRV_DMA_QUEUE_ERR_STS, val & DMA_ERR_MASK);
}

void __iomem * devdrv_get_dma_chan_base(const void __iomem *dma_base)
{
    return (void __iomem *)dma_base + DEVDRV_DMA_CHAN_ADDR_OFFSET;
}

int devdrv_dma_check_queue_sts(const void __iomem *io_base, unsigned long timeout)
{
    u32 dma_queue_sts;
    int ret = 0;

    devdrv_get_dma_queue_sts(io_base, &dma_queue_sts);
    while (dma_queue_sts == DEVDRV_DMA_CHANNEL_RUN_STATE) {
        if (timeout-- == 0) {
            ret = -ETIMEDOUT;
            break;
        }

        devdrv_get_dma_queue_sts(io_base, &dma_queue_sts);
        devdrv_debug("Get DMA queue sts. (dma_queue_sts=%x)\n", dma_queue_sts);
        udelay(1);
    }

    return ret;
}

int devdrv_dma_check_fsm_sts(const void __iomem *io_base, unsigned long timeout)
{
    u32 dma_queue_sts, dma_queue_not_work;
    int ret = 0;

    devdrv_get_dma_queue_sts(io_base, &dma_queue_sts);
    devdrv_get_dma_queue_not_work(io_base, &dma_queue_not_work);
    while ((dma_queue_sts != DEVDRV_DMA_CHANNEL_IDLE_STATE) || (dma_queue_not_work != DEVDRV_DMA_QUEUE_NOT_WORK)) {
        if (timeout-- == 0) {
            ret = -ETIMEDOUT;
            break;
        }

        devdrv_get_dma_queue_sts(io_base, &dma_queue_sts);
        devdrv_get_dma_queue_not_work(io_base, &dma_queue_not_work);
        devdrv_debug("Get DMA queue information. (dma_queue_sts=%x; dma_queue_not_work=%x)\n",
                     dma_queue_sts, dma_queue_not_work);
        udelay(1);
    }

    return ret;
}

/* reset the DMA channel configuration */
int devdrv_dma_ch_cfg_reset(void __iomem *io_base, u32 sriov_flag)
{
    u32 dma_queue_sts;
    u32 dma_queue_en;
    int ret;

    /* Clear DMA error status in case of remaining from last OS reset */
    devdrv_clear_dma_err_status(io_base);

    /* If the DMA Queue is in IDLE_STATE, the channel reset flow
     * can be finished
     */
    devdrv_get_dma_queue_sts(io_base, &dma_queue_sts);
    devdrv_debug("Get dma_queue_sts. (dma_queue_sts=%x)\n", dma_queue_sts);
    if ((sriov_flag == DEVDRV_SRIOV_DISABLE) && (dma_queue_sts == DEVDRV_DMA_CHANNEL_IDLE_STATE)) {
        /* The VA and PA select for transferring addresses are added to the latest DMA channel.
           when va is enabled, hardware aslo need check whether the SMMU is enabled. */
        devdrv_set_va_enable(io_base);
        return 0;
    }

    /* if sriov enable or disable, the dma must be reset */
    if ((sriov_flag == DEVDRV_SRIOV_ENABLE) || (dma_queue_sts == DEVDRV_DMA_CHANNEL_RUN_STATE)) {
        /* set dma_queue_pause to 0x1 */
        devdrv_set_dma_queue_pause(io_base, 1);

        /* disable dma_queue_en bits of DEVDRV_DMA_QUEUE_CTRL0 */
        devdrv_set_dma_chan_en(io_base, 0);

        /* check dma channel queue sts */
        ret = devdrv_dma_check_queue_sts(io_base, DEVDRV_DMA_TIMEOUT);
        if (ret) {
            devdrv_err("Execute devdrv_dma_check_queue_sts failed.\n");
            return -ETIMEDOUT;
        }
    }

    devdrv_get_dma_chan_en(io_base, &dma_queue_en);
    if (dma_queue_en == 1)
        devdrv_set_dma_chan_en(io_base, 0);

    /* set dma_queue_reset bits of DEVDRV_DMA_QUEUE_CTRL1 */
    devdrv_set_dma_queue_reset(io_base, 1);

    /* set sq tail and cq head */
    devdrv_set_dma_sq_tail(io_base, 0);
    devdrv_set_dma_cq_head(io_base, DEVDRV_DMA_CQ_HEAD(1024));

    /* set dma_queue_pause to 0x0 */
    devdrv_set_dma_queue_pause(io_base, 0);

    /* The VA and PA select for transferring addresses are added to the latest DMA channel.
       when va is enabled, hardware aslo need check whether the SMMU is enabled. */
    devdrv_set_va_enable(io_base);

    /* check the DMA queue fsm sts */
    ret = devdrv_dma_check_fsm_sts(io_base, DEVDRV_DMA_TIMEOUT);
    if (ret) {
        devdrv_err("Execute dma_ch_check_fsm_sts failed.\n");
        return -ETIMEDOUT;
    }

    return 0;
}

void devdrv_dma_ch_cfg_init(void __iomem *io_base, const struct devdrv_dma_cfg_info *dma_info)
{
    devdrv_set_dma_sq_base(io_base, dma_info->sq_addr);
    devdrv_set_dma_cq_base(io_base, dma_info->cq_addr);

    /* set dma channel sq depth */
    devdrv_set_dma_sq_depth(io_base, dma_info->sq_depth);
    /* set dma channel cq depth */
    devdrv_set_dma_cq_depth(io_base, dma_info->cq_depth);
    /* set dma channel sq tail */
    devdrv_set_dma_sq_tail(io_base, 0);
    /* set dma channel cq head */
    devdrv_set_dma_cq_head(io_base, DEVDRV_DMA_CQ_HEAD(dma_info->cq_depth));
    /* set dma queue arb weight */
    devdrv_set_dma_arb_weight(io_base, 0);

    /* set sq and cq side */
    devdrv_set_dma_sqcq_dir(io_base, dma_info->sqcq_side);

    devdrv_dma_interrupt_mask(io_base, 0);
    devdrv_dma_chan_err_interrupt_mask(io_base, 0);
    /* set PF num */
    devdrv_set_dma_pfvf_num(io_base, dma_info->pf_num, dma_info->vf_num);
    devdrv_set_dma_err_abort_en(io_base, 0);
}

int devdrv_get_dma_err_chan(const void __iomem *io_base, u32 *chan_id, u32 *side)
{
    u32 val;
    u32 i;
    void __iomem *addr;

    for (i = DMA_CHAN_LOCAL_USED_START_INDEX; i < DMA_CHAN_LOCAL_USED_START_INDEX + DMA_CHAN_LOCAL_TOTAL_NUM; i++) {
        addr = (void __iomem *)((char *)io_base + (u64)i * DEVDRV_DMA_CHAN_OFFSET);
        devdrv_dma_reg_rd(addr, DEVDRV_DMA_QUEUE_INT_RO, &val);
        if ((val & DMA_ERR_MASK) != 0) {
            devdrv_info("Device DMA chan is abnormal. (chan=%u; val=0x%x)\n", i, val);
            *side = DEVDRV_DMA_LOCAL_SIDE;
            *chan_id = i;
            return 0;
        }
    }

    for (i = DMA_CHAN_REMOTE_USED_START_INDEX; i < DMA_CHAN_REMOTE_USED_START_INDEX + DMA_CHAN_REMOTE_USED_NUM; i++) {
        addr = (void __iomem *)((char *)io_base + (u64)i * DEVDRV_DMA_CHAN_OFFSET);
        devdrv_dma_reg_rd(addr, DEVDRV_DMA_QUEUE_INT_RO, &val);
        if ((val & DMA_ERR_MASK) != 0) {
            devdrv_info("Host DMA chan is abnormal. (chan=%u; val=0x%x)\n", i, val);
            *side = DEVDRV_DMA_REMOTE_SIDE;
            *chan_id = i;
            return 0;
        }
    }

    for (i = DMA_CHAN_TS_USED_START_INDEX; i < DMA_CHAN_TS_USED_START_INDEX + DMA_CHAN_TS_USED_NUM; i++) {
        addr = (void __iomem *)((char *)io_base + (u64)i * DEVDRV_DMA_CHAN_OFFSET);
        devdrv_dma_reg_rd(addr, DEVDRV_DMA_QUEUE_INT_RO, &val);
        if ((val & DMA_ERR_MASK) != 0) {
            devdrv_info("TS DMA chan abnormal. (chan=%u; val=0x%x)\n", i, val);
            *side = DEVDRV_DMA_TS_SIDE;
            *chan_id = i;
            return -EINVAL;
        }
    }

    devdrv_info("Not find abnormal DMA chan.\n");

    return -EINVAL;
}

