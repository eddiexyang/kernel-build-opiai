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
#include <linux/property.h>
#include <linux/of_address.h>
#include <linux/of.h>

#include "resource_drv.h"
#include "devdrv_msg_def.h"
#include "dma_drv.h"
#include "nvme_drv.h"

#define DEVDRV_MAX_DMA_CH_SQ_DEPTH   0xc000
#define DEVDRV_MAX_DMA_CH_CQ_DEPTH   0xc000
#define DEVDRV_DMA_CH_SQ_DESC_RSV    0x400

void agentdrv_get_dma_total_chan_info(struct devdrv_dma_chan_dist_info *dma_chan)
{
    u32 pf_num = agentdrv_res_get_func_pf();
    if (pf_num > 1) {
        dma_chan->local_use_num = DMA_CHAN_LOCAL_TOTAL_NUM;
    } else {
        dma_chan->local_use_num = DMA_CHAN_LOCAL_USED_NUM;
    }
    dma_chan->local_start_index = DMA_CHAN_LOCAL_USED_START_INDEX;
    dma_chan->remote_use_num = DMA_CHAN_REMOTE_USED_NUM;
    dma_chan->remote_start_index = DMA_CHAN_REMOTE_USED_START_INDEX;
    dma_chan->ts_use_num = DMA_CHAN_TS_USED_NUM;
    dma_chan->ts_start_index = DMA_CHAN_TS_USED_START_INDEX;
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
        out->chan_num = DMA_CHAN_LOCAL_TOTAL_NUM / pf_num;
    } else {
        out->chan_num = DMA_CHAN_LOCAL_USED_NUM;
    }
    /* device DMA done is INT0~INTX, device use 0~X/2 chan, and continuous, then each function
    add prev function number. 0~1+2~3
    device use one error irq for all channel, don't hang err irq in function. */
    out->done_irq_base = dma_irq_base + DMA_DONE_IRQ_BASE;
    out->err_irq_base = dma_irq_base + DMA_ERR_IRQ_BASE;
    /* cloud only one err irq */
    out->err_flag = (DMA_ERR_IRQ_NUM == 1) ? 0 : 1;
    out->sq_cq_info.sq_depth = DEVDRV_MAX_DMA_CH_SQ_DEPTH;
    out->sq_cq_info.sq_rsv_num = DEVDRV_DMA_CH_SQ_DESC_RSV;
    out->sq_cq_info.cq_depth = DEVDRV_MAX_DMA_CH_CQ_DEPTH;
}

void agentdrv_get_dma_max_msg_chan(u32 func_id, u32 *chan_num)
{
    struct agentdrv_pfvf_info pfvf_info = { 0 };

    devdrv_soc_func2pfvf(0, func_id, 0, &pfvf_info);
    if (pfvf_info.vf_en == DEVDRV_SRIOV_TYPE_VF) {
        *chan_num = AGENTDRV_VF_MAX_IO_MSG_CHAN;
    } else {
        *chan_num = AGENTDRV_PF_MAX_IO_MSG_CHAN;
    }
}

void agentdrv_get_p2p_msg_db_range(u32 func_id, u32 *p2p_db_start, u32 *p2p_db_end)
{
    struct agentdrv_pfvf_info pfvf_info = { 0 };

    devdrv_soc_func2pfvf(0, func_id, 0, &pfvf_info);
    if (pfvf_info.vf_en == DEVDRV_SRIOV_TYPE_VF) {
        *p2p_db_start = AGENTDRV_VF_P2P_MSG_USED_DB_START;
        *p2p_db_end = AGENTDRV_VF_P2P_MSG_USED_DB_END;
    } else {
        *p2p_db_start = AGENTDRV_P2P_MSG_USED_DB_START;
        *p2p_db_end = AGENTDRV_P2P_MSG_USED_DB_END;
    }
}

STATIC u64 agentdrv_cloudv2_rsv_mem_base(u32 vf_num, u32 vf_en)
{
    if (vf_en == DEVDRV_SRIOV_TYPE_VF) {
        return (u64)(DEVDRV_RSV_VF_MEM_BASE + (u64)DEVDRV_RSV_VF_MEM_OFFSET * (u64)(vf_num - 1));
    } else {
        return (u64)(DEVDRV_RSV_MEM_BASE);
    }
}

STATIC u64 agentdrv_cloudv2_db_state_base(u32 vf_num, u32 vf_en)
{
    if (vf_en == DEVDRV_SRIOV_TYPE_VF) {
        return (u64)(AGENTDRV_VF_DB_STATE_ADDRESS + (vf_num - 1) * AGENTDRV_VF_DB_STATE_OFFSET);
    } else {
        return (u64)(AGENTDRV_PF_DB_STATE_ADDRESS);
    }
}

/* If 2DIE use two pcie, die0's dev_id is 0, die1's dev_id is 1, func_id is always 0;
 * If 2DIE use one pcie, die0's func_id is  0, die1's func_id is 1, dev_id is always 0;
 * If 1P 1DIE, dev_id and func_id are both 0;
*/
void agentdrv_res_sdi_addr(u32 dev_id, u32 func_id, struct agent_res_sdi_addr *sdi_addr)
{
    struct agentdrv_pfvf_info pfvf_info = { 0 };
    u64 msi_irq_addr = AGENTDRV_SOC_DB_RAISE_INIT_ADDR;
    u64 peh_npu_addr_start = 0;
    u64 chip_offset = 0;
    int ret;

    devdrv_soc_func2pfvf(dev_id, func_id, 0, &pfvf_info);

    /* get diff os's chip offset addr size */
    ret = agentdrv_get_chip_offset_by_devid((u32)dev_id, &chip_offset);
    if (ret != 0) {
        devdrv_err("Get chip offset fail by dev id\n");
        return;
    }

    sdi_addr->msg_size = DEVDRV_PCIE_RESERVE_MEM_SIZE;
    sdi_addr->msg_base = peh_npu_addr_start * dev_id + peh_npu_addr_start  * pfvf_info.func_num +
        chip_offset * dev_id + chip_offset * pfvf_info.func_num +
        dev_id * DEVDRV_DIE_ADDR_SPACE_SIZE + pfvf_info.func_num * DEVDRV_DIE_ADDR_SPACE_SIZE +
        dev_id * DEVDRV_DIE_RESERVE_MEM_OFFSET + pfvf_info.func_num * DEVDRV_DIE_RESERVE_MEM_OFFSET +
        agentdrv_cloudv2_rsv_mem_base(pfvf_info.vf_num, pfvf_info.vf_en);

    sdi_addr->rsv_mem_size = DEVDRV_PCIE_RESERVE_MEM_SIZE;
    sdi_addr->rsv_mem_base = peh_npu_addr_start * dev_id + peh_npu_addr_start  * pfvf_info.func_num +
        chip_offset * dev_id + chip_offset * pfvf_info.func_num +
        dev_id * DEVDRV_DIE_ADDR_SPACE_SIZE + pfvf_info.func_num * DEVDRV_DIE_ADDR_SPACE_SIZE +
        dev_id * DEVDRV_DIE_RESERVE_MEM_OFFSET + pfvf_info.func_num * DEVDRV_DIE_RESERVE_MEM_OFFSET +
        agentdrv_cloudv2_rsv_mem_base(pfvf_info.vf_num, pfvf_info.vf_en);

    sdi_addr->topic_sched_resmem_size = DEVDRV_TOPIC_SCHED_RES_MEM_SIZE;
    sdi_addr->topic_sched_resmem_base = peh_npu_addr_start * dev_id + peh_npu_addr_start  * pfvf_info.func_num +
        chip_offset * dev_id + chip_offset * pfvf_info.func_num +
        dev_id * DEVDRV_DIE_ADDR_SPACE_SIZE + pfvf_info.func_num * DEVDRV_DIE_ADDR_SPACE_SIZE +
        dev_id * DEVDRV_DIE_RESERVE_MEM_OFFSET + pfvf_info.func_num * DEVDRV_DIE_RESERVE_MEM_OFFSET +
        DEVDRV_TOPIC_SCHED_RES_MEM_BASE;

    sdi_addr->hbm_size = DEVDRV_PCIE_HBM_MEM_SIZE;
    sdi_addr->hbm_base = peh_npu_addr_start + dev_id * DEVDRV_DIE_ADDR_SPACE_SIZE +
        dev_id * DEVDRV_DIE_RESERVE_MEM_OFFSET + pfvf_info.func_num * DEVDRV_DIE_ADDR_SPACE_SIZE +
        pfvf_info.func_num * DEVDRV_DIE_RESERVE_MEM_OFFSET + DEVDRV_PCIE_HBM_MEM_ADDR;

    sdi_addr->shr_size = DEVDRV_SHR_PARA_ADDR_SIZE;
    sdi_addr->shr_base = peh_npu_addr_start + chip_offset + dev_id * DEVDRV_DIE_ADDR_SPACE_SIZE +
        pfvf_info.func_num * DEVDRV_DIE_ADDR_SPACE_SIZE + pfvf_info.vf_num * DEVDRV_VF_SHR_PARA_ADDR_OFFSET +
        DEVDRV_SHR_PARA_ADDR;
    sdi_addr->shr_type = DEVDRV_SHR_MEM_NORMAL;

    sdi_addr->db_size = AGENTDRV_DB_MEM_SIZE;
    sdi_addr->db_base = peh_npu_addr_start + chip_offset + dev_id * DEVDRV_DIE_ADDR_SPACE_SIZE +
        pfvf_info.func_num * DEVDRV_DIE_ADDR_SPACE_SIZE + pfvf_info.vf_num * AGENTDRV_VF_SOC_DOORBELL_OFFSET +
        AGENTDRV_DB_MEM_ADDRESS;

    sdi_addr->db_state_size = AGENTDRV_DB_MEM_SIZE;
    sdi_addr->db_state_base = peh_npu_addr_start +  chip_offset + dev_id * DEVDRV_DIE_ADDR_SPACE_SIZE +
        pfvf_info.func_num * DEVDRV_DIE_ADDR_SPACE_SIZE +
        agentdrv_cloudv2_db_state_base(pfvf_info.vf_num, pfvf_info.vf_en);

    sdi_addr->raise_int_size = AGENTDRV_SOC_DB_RAISE_INIT_SIZE;
    sdi_addr->raise_int_base = peh_npu_addr_start + chip_offset + dev_id * DEVDRV_DIE_ADDR_SPACE_SIZE + msi_irq_addr;

    sdi_addr->bw_ctrl_size = 0;
    sdi_addr->bw_ctrl_base = 0;
}

int agentdrv_ioremap_doorbell_base(void __iomem **doorbell_base, phys_addr_t db_base, u64 db_size)
{
    return 0;
}

void agentdrv_res_db_queue_id2func_id(u32 db_id, u32 *db_id_in_func)
{
    *db_id_in_func = db_id % AGENTDRV_SINGLE_PF_DB_IRQ_NUM;
}

void agentdrv_res_dma_chan_id2func_id(u32 dma_chan_id, u32 *func_id, u32 *dma_chan_id_in_func)
{
    struct devdrv_dma_chan_dist_info dma_chan_dist = {0};
    u32 func_totl = agentdrv_res_get_func_total();
    if (func_totl == 0) {
        devdrv_warn("func_totl is zero\n");
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
#ifndef DRV_UT
    u32 dev_id = MAX_AGENTCHIP_CNT;
    struct device_node *node = NULL;
    int ret;

    node = pdev->dev.of_node;
    if (node == NULL) {
        devdrv_debug("node is null\n");
        return -EINVAL;
    }

    ret = of_property_read_u32(node, "dev_id", &dev_id);
    if (ret != 0) {
        dev_id = 0;
    }

    if (dev_id >= MAX_AGENTCHIP_CNT) {
        devdrv_err("Device ID error. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }
#else
    u32 dev_id = 0;
#endif
    return (int)dev_id;
}

int agentdrv_get_tx_atu_addr(struct platform_device *pdev, u32 func_id, u32 region_index, u64 *start_addr, u64 *size)
{
    struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, region_index);
    if (res == NULL) {
        devdrv_warn("func %u can not get region_index %u resource.\n", func_id, region_index);
        return -EINVAL;
    }

    *start_addr = (u64)res->start + func_id * DEVDRV_DIE_TX_ATU_SPACE_OFFSET;
    *size = (u64)res->end + 1 - (u64)res->start;

    return 0;
}

