/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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

#include <linux/securec.h>
#include "resource_drv.h"
#include "agentdrv_qos.h"
#include "agentdrv_qos_cloud_v2.h"

u32 agentdrv_get_pcie_core_num(void)
{
    return PCIE_CORE_NUM;
}

u32 agentdrv_get_pcie_dma_chan_num(void)
{
    return DMA_CHAN_REMOTE_USED_NUM + DMA_CHAN_REMOTE_USED_START_INDEX;
}

void __iomem *agentdrv_get_ap_iob_rx_base(void __iomem *apb_base, u32 core_id)
{
    return apb_base + PCIE_AP_IOB_RX_REG_OFFSET + core_id * PCIE_CORE_OFFSET;
}

void agentdrv_set_qos(void __iomem *iob_base, u32 qos)
{
    void __iomem *reg_base = iob_base + PCIE_AMB_QOS_CTRL;
    agentdrv_set_reg(reg_base, PCIE_AMB_QOS_MASK, PCIE_AMB_QOS_WR_OFFSET, qos);
    agentdrv_set_reg(reg_base, PCIE_AMB_QOS_MASK, PCIE_AMB_QOS_RD_OFFSET, qos);
}

void agentdrv_set_pmg(void __iomem *iob_base, u32 pmg)
{
    void __iomem *reg_base = iob_base + PCIE_AMB_ARUSER_SET;
    agentdrv_set_reg(reg_base, PCIE_AMB_ARUSER_PMG_MASK, PCIE_AMB_ARUSER_PMG_OFFSET, pmg);

    reg_base = iob_base + PCIE_AMB_AWUSER_SET;
    agentdrv_set_reg(reg_base, PCIE_AMB_AWUSER_PMG_MASK, PCIE_AMB_AWUSER_PMG_OFFSET, pmg);
}

void agentdrv_set_mpam_id(void __iomem *iob_base, u32 mpam_id)
{
    void __iomem *reg_base = iob_base + PCIE_AMB_ARUSER_SET;
    agentdrv_set_reg(reg_base, PCIE_AMB_ARUSER_MPAM_MASK, PCIE_AMB_ARUSER_MPAM_OFFSET, mpam_id);

    reg_base = iob_base + PCIE_AMB_AWUSER_SET;
    agentdrv_set_reg(reg_base, PCIE_AMB_AWUSER_MPAM_MASK, PCIE_AMB_AWUSER_MPAM_OFFSET, mpam_id);
}

u32 agentdrv_get_qos(void __iomem *iob_base)
{
    void __iomem *reg_base = iob_base + PCIE_AMB_QOS_CTRL;
    return agentdrv_get_reg(reg_base, PCIE_AMB_QOS_MASK, PCIE_AMB_QOS_WR_OFFSET);
}

u32 agentdrv_get_pmg(void __iomem *iob_base)
{
    void __iomem *reg_base = iob_base + PCIE_AMB_ARUSER_SET;
    return agentdrv_get_reg(reg_base, PCIE_AMB_ARUSER_PMG_MASK, PCIE_AMB_ARUSER_PMG_OFFSET);
}

u32 agentdrv_get_mpam_id(void __iomem *iob_base)
{
    void __iomem *reg_base = iob_base + PCIE_AMB_ARUSER_SET;
    return agentdrv_get_reg(reg_base, PCIE_AMB_ARUSER_MPAM_MASK, PCIE_AMB_ARUSER_MPAM_OFFSET);
}

STATIC void agentdrv_set_dma_qos_enable(void __iomem *reg_base, u32 val)
{
    agentdrv_set_reg(reg_base, PCIE_DMA_QOS_ENABLE_MASK, PCIE_DMA_QOS_ENABLE_OFFSET, val);
}

STATIC void agentdrv_set_dma_qos_ns(void __iomem *reg_base, u32 val)
{
    agentdrv_set_reg(reg_base, PCIE_DMA_QOS_NS_MASK, PCIE_DMA_QOS_NS_OFFSET, val);
}

STATIC void agentdrv_set_dma_qos_id(void __iomem *reg_base, u32 qos)
{
    agentdrv_set_reg(reg_base, PCIE_DMA_QOS_ID_MASK, PCIE_DMA_QOS_ID_OFFSET, qos);
}

STATIC void agentdrv_set_dma_pmg(void __iomem *reg_base, u32 pmg)
{
    agentdrv_set_reg(reg_base, PCIE_DMA_QOS_PMG_MASK, PCIE_DMA_QOS_PMG_OFFSET, pmg);
}

STATIC void agentdrv_set_dma_mpam_id(void __iomem *reg_base, u32 mpam_id)
{
    agentdrv_set_reg(reg_base, PCIE_DMA_QOS_MPAM_MASK, PCIE_DMA_QOS_MPAM_OFFSET, mpam_id);
}

STATIC u32 agentdrv_get_dma_qos_id(void __iomem *reg_base)
{
    return agentdrv_get_reg(reg_base, PCIE_DMA_QOS_ID_MASK, PCIE_DMA_QOS_ID_OFFSET);
}

STATIC u32 agentdrv_get_dma_pmg(void __iomem *reg_base)
{
    return agentdrv_get_reg(reg_base, PCIE_DMA_QOS_PMG_MASK, PCIE_DMA_QOS_PMG_OFFSET);
}

STATIC u32 agentdrv_get_dma_mpam_id(void __iomem *reg_base)
{
    return agentdrv_get_reg(reg_base, PCIE_DMA_QOS_MPAM_MASK, PCIE_DMA_QOS_MPAM_OFFSET);
}

void agentdrv_set_qos_config_to_dma(void __iomem *base, u32 chan_id, const struct qos_master_config_type *cfg)
{
    void __iomem *reg_base = base + PCIE_AP_DMA_REG_OFFSET + PCIE_DMA_QUEUE_CTRL1 + chan_id * PCIE_DMA_CHAN_OFFSET;

    agentdrv_set_dma_qos_enable(reg_base, 1);
    agentdrv_set_dma_qos_ns(reg_base, 1);
    agentdrv_set_dma_qos_id(reg_base, cfg->qos);
    agentdrv_set_dma_pmg(reg_base, cfg->pmg);
    agentdrv_set_dma_mpam_id(reg_base, cfg->mpamid);
}

void agentdrv_get_qos_config_from_dma(void __iomem *base, u32 chan_id, struct qos_master_config_type *cfg)
{
    void __iomem *reg_base = base + PCIE_AP_DMA_REG_OFFSET + PCIE_DMA_QUEUE_CTRL1 + chan_id * PCIE_DMA_CHAN_OFFSET;

    cfg->qos = agentdrv_get_dma_qos_id(reg_base);
    cfg->pmg = agentdrv_get_dma_pmg(reg_base);
    cfg->mpamid = agentdrv_get_dma_mpam_id(reg_base);
}

void agentdrv_set_allow_to_core(void __iomem *base, u32 core_id, const struct qos_allow_config_type *cfg)
{
    return;
}

void agentdrv_get_allow_from_core(void __iomem *base, u32 core_id, struct qos_allow_config_type *cfg)
{
    return;
}

void agentdrv_set_otsd_to_core(void __iomem *base, u32 core_id, const struct qos_otsd_config_type *cfg)
{
    return;
}

void agentdrv_get_otsd_from_core(void __iomem *base, u32 core_id, struct qos_otsd_config_type *cfg)
{
    return;
}