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
#include <linux/pci.h>

#include "resource_drv.h"
#include "devdrv_util.h"
#include "dma_drv.h"
#include "devdrv_msg_def.h"
#include "agentdrv_platform.h"

#define DEVDRV_MAX_DMA_CH_SQ_DEPTH   0x8000
#define DEVDRV_MAX_DMA_CH_CQ_DEPTH   0x6000
#define DEVDRV_DMA_CH_SQ_DESC_RSV    0x200

void agentdrv_get_dma_total_chan_info(struct devdrv_dma_chan_dist_info *dma_chan)
{
    u32 pf_num = agentdrv_res_get_func_pf();
    if (pf_num > 1) {
        dma_chan->local_use_num = DMA_CHAN_DOUBLE_LOCAL_USED_NUM;
        dma_chan->local_start_index = DMA_CHAN_DOUBLE_LOCAL_USED_START_INDEX;
        dma_chan->remote_use_num = DMA_CHAN_DOUBLE_REMOTE_USED_NUM;
        dma_chan->remote_start_index = DMA_CHAN_DOUBLE_REMOTE_USED_START_INDEX;
        dma_chan->ts_use_num = DMA_CHAN_DOUBLE_TS_USED_NUM;
        dma_chan->ts_start_index = DMA_CHAN_DOUBLE_TS_USED_START_INDEX;
    } else {
        dma_chan->local_use_num = DMA_CHAN_LOCAL_USED_NUM;
        dma_chan->local_start_index = DMA_CHAN_LOCAL_USED_START_INDEX;
        dma_chan->remote_use_num = DMA_CHAN_REMOTE_USED_NUM;
        dma_chan->remote_start_index = DMA_CHAN_REMOTE_USED_START_INDEX;
        dma_chan->ts_use_num = DMA_CHAN_TS_USED_NUM;
        dma_chan->ts_start_index = DMA_CHAN_TS_USED_START_INDEX;
    }
}

void agentdrv_res_pf_dma_chan(struct devdrv_dma_dev *dma_dev)
{
    struct devdrv_dma_chan_dist_info dist_info = {0};
    u32 pf_num = agentdrv_res_get_func_pf();
    u32 local_num, remote_num, ts_num;
    u32 func_id = dma_dev->func_id;
    u32 i;

    agentdrv_get_dma_total_chan_info(&dist_info);

    local_num = dist_info.local_use_num / pf_num;
    for (i = 0; i < local_num; i++) {
        dma_dev->local_chan[i] = dist_info.local_start_index + (local_num * func_id) + i;
    }
    dma_dev->local_chan_num = local_num;

    remote_num = dist_info.remote_use_num / pf_num;
    for (i = 0; i < remote_num; i++) {
        dma_dev->remote_chan[i] = dist_info.remote_start_index + (remote_num * func_id) + i;
    }
    dma_dev->remote_chan_num = remote_num;

    ts_num = dist_info.ts_use_num / pf_num;
    for (i = 0; i < ts_num; i++) {
        dma_dev->ts_chan[i] = dist_info.ts_start_index + (ts_num * func_id) + i;
    }
    dma_dev->ts_chan_num = ts_num;
}

void agentdrv_dma_res_common(u32 func_id, u32 dma_irq_base, struct devdrv_res_dma_common_info *out)
{
    u32 pf_num = agentdrv_res_get_func_pf();
    if (pf_num > 1) {
        out->chan_num = DMA_CHAN_DOUBLE_LOCAL_USED_NUM / pf_num;
    } else {
        out->chan_num = DMA_CHAN_LOCAL_USED_NUM;
    }
    /* device DMA done is INT0~INTX, device use 0~X/2 chan, and continuous, then each function
    add prev function number. 0~1+2~3
    device use one error irq for all channel, don't hang err irq in function. */
    out->done_irq_base = dma_irq_base + DMA_DONE_IRQ_BASE;
    out->err_irq_base = dma_irq_base + DMA_ERR_IRQ_BASE;
    out->err_flag = 0;
    out->sq_cq_info.sq_depth = DEVDRV_MAX_DMA_CH_SQ_DEPTH;
    out->sq_cq_info.sq_rsv_num = DEVDRV_DMA_CH_SQ_DESC_RSV;
    out->sq_cq_info.cq_depth = DEVDRV_MAX_DMA_CH_CQ_DEPTH;
}

void agentdrv_get_dma_max_msg_chan(u32 func_id, u32 *chan_num)
{
    *chan_num = AGENTDRV_MAX_IO_MSG_CHAN;
}

void agentdrv_get_p2p_msg_db_range(u32 func_id, u32 *p2p_db_start, u32 *p2p_db_end)
{
    *p2p_db_start = AGENTDRV_P2P_MSG_USED_DB_START;
    *p2p_db_end = AGENTDRV_P2P_MSG_USED_DB_END;
}

void agentdrv_res_sdi_addr(u32 dev_id, u32 func_id, struct agent_res_sdi_addr *sdi_addr)
{
    sdi_addr->msg_size = DEVDRV_PCIE_RESERVE_MEM_SIZE;
    sdi_addr->msg_base = func_id * DEVDRV_CHIP_ADDR_SPACE_SIZE + AGENTDRV_MSG_QUEUE_ADDRESS;
    sdi_addr->db_size = AGENTDRV_DB_MEM_SIZE;
    sdi_addr->db_base = func_id * DEVDRV_CHIP_ADDR_SPACE_SIZE + AGENTDRV_DB_MEM_ADDRESS;
    sdi_addr->shr_size = DEVDRV_SHR_PARA_ADDR_SIZE;
    sdi_addr->shr_base = func_id * DEVDRV_CHIP_ADDR_SPACE_SIZE + DEVDRV_SHR_PARA_ADDR;
    sdi_addr->shr_type = DEVDRV_SHR_MEM_NORMAL;
    sdi_addr->bw_ctrl_size = DEVDRV_VF_BANDWIDTH_SIZE;
    sdi_addr->bw_ctrl_base = func_id * DEVDRV_CHIP_ADDR_SPACE_SIZE + DEVDRV_TS_SRAM_ADDR + DEVDRV_VF_BANDWIDTH_OFFSET;
    sdi_addr->rsv_mem_base = func_id * DEVDRV_CHIP_ADDR_SPACE_SIZE + DEVDRV_RSV_MEM_BASE;
    sdi_addr->rsv_mem_size = DEVDRV_PCIE_RESERVE_MEM_SIZE;
    sdi_addr->raise_int_size = AGENTDRV_SOC_DB_RAISE_INIT_SIZE;
    sdi_addr->raise_int_base = AGENTDRV_SOC_DB_RAISE_INIT_ADDR;
    sdi_addr->db_state_size = AGENTDRV_SOC_DB_IRQ_SIZE;
    sdi_addr->db_state_base = AGENTDRV_SOC_DB_IRQ_ADDR;
    sdi_addr->topic_sched_resmem_size = 0;
    sdi_addr->topic_sched_resmem_base = 0;
}

int agentdrv_ioremap_doorbell_base(void __iomem **doorbell_base, phys_addr_t db_base, u64 db_size)
{
    *doorbell_base = ioremap_cache(db_base, db_size);
    return (*doorbell_base == NULL) ? -ENOMEM : 0;
}

void agentdrv_res_db_queue_id2func_id(u32 db_id, u32 *db_id_in_func)
{
    *db_id_in_func = db_id % DEVDRV_RM_DOORBELL_QUEUE_PER_PF;
}

void agentdrv_res_dma_chan_id2func_id(u32 dma_chan_id, u32 *func_id, u32 *dma_chan_id_in_func)
{
    struct devdrv_dma_chan_dist_info dma_chan_dist = {0};
    u32 func_totl = agentdrv_res_get_func_total();
    if (func_totl == 0) {
        devdrv_warn("Func_totl is zero, no dev ready.\n");
        return;
    }

    agentdrv_get_dma_total_chan_info(&dma_chan_dist);

    if (dma_chan_id < dma_chan_dist.local_use_num) {
        *func_id = dma_chan_id / (dma_chan_dist.local_use_num / func_totl);
        *dma_chan_id_in_func = dma_chan_id % (dma_chan_dist.local_use_num / func_totl);
    } else {
        *func_id = (dma_chan_id - dma_chan_dist.local_use_num) / (dma_chan_dist.remote_use_num / func_totl);
        *dma_chan_id_in_func = (dma_chan_id - dma_chan_dist.local_use_num) %
            (dma_chan_dist.remote_use_num / func_totl);
    }
}

void __iomem *agentdrv_get_phy_match_flag_addr(struct agentdrv_devctrl *agent_dev, u32 func_idx)
{
    return (u8 *)agent_dev->shr_para[func_idx] + PHY_MATCH_FLAG_OFFSET_IN_SHR_MEM;
}

int agentdrv_platform_get_devid(struct platform_device *pdev)
{
    return 0;
}

int agentdrv_get_tx_atu_addr(struct platform_device *pdev, u32 func_id, u32 region_index, u64 *start_addr, u64 *size)
{
    if (region_index == AGENTDRV_ATU_REGION1_BASE_INDEX) {
        if (func_id == 0x0) {
            *start_addr = DEVDRV_CHIP0_PCIE_MEM_BASE_ADDR_HIGH;
        } else {
            *start_addr = DEVDRV_CHIP1_PCIE_MEM_BASE_ADDR_HIGH;
        }

        *size = DEVDRV_CHIP_PCIE_MEM_SIZE_HIGH;
    } else {
        if (func_id == 0x0) {
            *start_addr = DEVDRV_CHIP0_PCIE_MEM_BASE_ADDR_LOW;
        } else {
            *start_addr = DEVDRV_CHIP1_PCIE_MEM_BASE_ADDR_LOW;
        }

        *size = DEVDRV_CHIP_PCIE_MEM_SIZE_LOW;
    }

    return 0;
}


