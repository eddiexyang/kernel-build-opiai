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
#include "agentdrv_qos_mini_v3.h"
#include "devdrv_util.h"

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
    return apb_base + PCIE_AP_IOB_RX_REG_OFFSET + (core_id * PCIE_CORE_OFFSET);
}

void agentdrv_set_qos(void __iomem *iob_base, u32 qos)
{
    void __iomem *reg_base = iob_base + PCIE_AML_QOS_CTRL;
    agentdrv_set_reg(reg_base, PCIE_AML_QOS_MASK, PCIE_AML_QOS_WR_OFFSET, qos);
    agentdrv_set_reg(reg_base, PCIE_AML_QOS_MASK, PCIE_AML_QOS_RD_OFFSET, qos);
}

void agentdrv_set_pmg(void __iomem *iob_base, u32 pmg)
{
    void __iomem *reg_base = iob_base + PCIE_WR_AML_AXUSER_CTRL;
    agentdrv_set_reg(reg_base, PCIE_AXUSER_CTRL_PMG_MASK, PCIE_AXUSER_CTRL_PMG_OFFSET, pmg);

    reg_base = iob_base + PCIE_RD_AML_AXUSER_CTRL;
    agentdrv_set_reg(reg_base, PCIE_AXUSER_CTRL_PMG_MASK, PCIE_AXUSER_CTRL_PMG_OFFSET, pmg);
}

void agentdrv_set_mpam_id(void __iomem *iob_base, u32 mpam_id)
{
    void __iomem *reg_base = iob_base + PCIE_WR_AML_AXUSER_CTRL;
    agentdrv_set_reg(reg_base, PCIE_AXUSER_CTRL_MPAM_MASK, PCIE_AXUSER_CTRL_MPAM_OFFSET, mpam_id);

    reg_base = iob_base + PCIE_RD_AML_AXUSER_CTRL;
    agentdrv_set_reg(reg_base, PCIE_AXUSER_CTRL_MPAM_MASK, PCIE_AXUSER_CTRL_MPAM_OFFSET, mpam_id);
}

u32 agentdrv_get_qos(void __iomem *iob_base)
{
    void __iomem *reg_base = iob_base + PCIE_AML_QOS_CTRL;
    return agentdrv_get_reg(reg_base, PCIE_AML_QOS_MASK, PCIE_AML_QOS_WR_OFFSET);
}

u32 agentdrv_get_pmg(void __iomem *iob_base)
{
    void __iomem *reg_base = iob_base + PCIE_WR_AML_AXUSER_CTRL;
    return agentdrv_get_reg(reg_base, PCIE_AXUSER_CTRL_PMG_MASK, PCIE_AXUSER_CTRL_PMG_OFFSET);
}

u32 agentdrv_get_mpam_id(void __iomem *iob_base)
{
    void __iomem *reg_base = iob_base + PCIE_WR_AML_AXUSER_CTRL;
    return agentdrv_get_reg(reg_base, PCIE_AXUSER_CTRL_MPAM_MASK, PCIE_AXUSER_CTRL_MPAM_OFFSET);
}

void agentdrv_set_qos_config_to_dma(void __iomem *base, u32 chan_id, const struct qos_master_config_type *cfg)
{
    return;
}

void agentdrv_get_qos_config_from_dma(void __iomem *base, u32 chan_id, struct qos_master_config_type *cfg)
{
    return;
}

STATIC void agentdrv_disable_allow_to_core(void __iomem *base, u32 core_id, const struct qos_allow_config_type *cfg)
{
    void __iomem *reg_base = NULL;
    reg_base = base + SCHE_RD_ALLOW_REG_OFFSET + (PCIE_RW_ALLOW_REG_OFFSET * (core_id >> 1));
    agentdrv_set_reg(reg_base, PCIE_RW_ALLOW_SEL_MASK, PCIE_RW_ALLOW_SEL_OFFSET, PCIE_RW_ALLOW_SEL_DISABLE);

    reg_base = base + SCHE_WR_ALLOW_REG_OFFSET + (PCIE_RW_ALLOW_REG_OFFSET * (core_id >> 1));
    agentdrv_set_reg(reg_base, PCIE_RW_ALLOW_SEL_MASK, PCIE_RW_ALLOW_SEL_OFFSET, PCIE_RW_ALLOW_SEL_DISABLE);

    return;
}

void agentdrv_set_allow_to_core(void __iomem *base, u32 core_id, const struct qos_allow_config_type *cfg)
{
    u32 reg_val = 0;
    void __iomem *reg_base = base;

    if (cfg->qos_allow_mode == PCIE_ALLOW_MODE_DISABLE) {
        agentdrv_disable_allow_to_core(reg_base, core_id, cfg);
        return;
    }

    reg_val |= (cfg->qos_allow_lvl[PCIE_QOS_LVL1] & PCIE_RW_ALLOW_LVL_MASK) << PCIE_RW_ALLOW_LVL1_OFFSET;
    reg_val |= (cfg->qos_allow_lvl[PCIE_QOS_LVL2] & PCIE_RW_ALLOW_LVL_MASK) << PCIE_RW_ALLOW_LVL2_OFFSET;
    reg_val |= (cfg->qos_allow_lvl[PCIE_QOS_LVL3] & PCIE_RW_ALLOW_LVL_MASK) << PCIE_RW_ALLOW_LVL3_OFFSET;
    reg_val |= (PCIE_RW_ALLOW_SEL_ENABLE & PCIE_RW_ALLOW_SEL_MASK) << PCIE_RW_ALLOW_SEL_OFFSET;

    if (cfg->qos_allow_ctrl == PCIE_QOS_ALLOW_CTRL_READ) {
        reg_base = base + SCHE_RD_ALLOW_REG_OFFSET + (PCIE_RW_ALLOW_REG_OFFSET * (core_id >> 1));
        agentdrv_set_reg(reg_base, PCIE_QOS_REG_MASK, 0, reg_val);
    } else if (cfg->qos_allow_ctrl == PCIE_QOS_ALLOW_CTRL_WRITE) {
        reg_base = base + SCHE_WR_ALLOW_REG_OFFSET + (PCIE_RW_ALLOW_REG_OFFSET * (core_id >> 1));
        agentdrv_set_reg(reg_base, PCIE_QOS_REG_MASK, 0, reg_val);
    } else {
        reg_base = base + SCHE_RD_ALLOW_REG_OFFSET + (PCIE_RW_ALLOW_REG_OFFSET * (core_id >> 1));
        agentdrv_set_reg(reg_base, PCIE_QOS_REG_MASK, 0, reg_val);

        reg_base = base + SCHE_WR_ALLOW_REG_OFFSET + (PCIE_RW_ALLOW_REG_OFFSET * (core_id >> 1));
        agentdrv_set_reg(reg_base, PCIE_QOS_REG_MASK, 0, reg_val);
    }

    return;
}

void agentdrv_get_allow_from_core(void __iomem *base, u32 core_id, struct qos_allow_config_type *cfg)
{
    u32 reg_val = 0;
    void __iomem *reg_base = NULL;

    if (cfg->qos_allow_ctrl == PCIE_QOS_ALLOW_CTRL_READ) {
        reg_base = base + SCHE_RD_ALLOW_REG_OFFSET + (PCIE_RW_ALLOW_REG_OFFSET * (core_id >> 1));
    } else {
        reg_base = base + SCHE_WR_ALLOW_REG_OFFSET + (PCIE_RW_ALLOW_REG_OFFSET * (core_id >> 1));
    }

    reg_val = agentdrv_get_reg(reg_base, PCIE_QOS_REG_MASK, 0);
    cfg->qos_allow_lvl[PCIE_QOS_LVL1] = (reg_val >> PCIE_RW_ALLOW_LVL1_OFFSET) & PCIE_RW_ALLOW_LVL_MASK;
    cfg->qos_allow_lvl[PCIE_QOS_LVL2] = (reg_val >> PCIE_RW_ALLOW_LVL2_OFFSET) & PCIE_RW_ALLOW_LVL_MASK;
    cfg->qos_allow_lvl[PCIE_QOS_LVL3] = (reg_val >> PCIE_RW_ALLOW_LVL3_OFFSET) & PCIE_RW_ALLOW_LVL_MASK;

    return;
}

void agentdrv_set_otsd_to_core(void __iomem *base, u32 core_id, const struct qos_otsd_config_type *cfg)
{
    u32 reg_val = 0;
    void __iomem *reg_base = base + SCHE_OTSD_REG_OFFSET + (PCIE_OTSD_REG_OFFSET * (core_id >> 1));

    if (cfg->otsd_mode == PCIE_OTSD_MODE_DISABLE) {
        agentdrv_set_reg(reg_base, PCIE_RW_OTSD_EN_MASK, PCIE_WR_OTSD_EN_OFFSET, ~(PCIE_RW_OTSD_EN));
        agentdrv_set_reg(reg_base, PCIE_RW_OTSD_EN_MASK, PCIE_RD_OTSD_EN_OFFSET, ~(PCIE_RW_OTSD_EN));

        return;
    }

    if (cfg->otsd_mode == PCIE_OTSD_MODE_RDWR) {
        reg_val |= (cfg->otsd_lvl[PCIE_OTSD_RD] & PCIE_RW_OTSD_LVL_MASK) << PCIE_RD_OTSD_LVL_OFFSET;
        reg_val |= (PCIE_RW_OTSD_EN & PCIE_RW_OTSD_EN_MASK) << PCIE_RD_OTSD_EN_OFFSET;
        reg_val |= (cfg->otsd_lvl[PCIE_OTSD_WR] & PCIE_RW_OTSD_LVL_MASK) << PCIE_WR_OTSD_LVL_OFFSET;
        reg_val |= (PCIE_RW_OTSD_EN & PCIE_RW_OTSD_EN_MASK) << PCIE_WR_OTSD_EN_OFFSET;
        agentdrv_set_reg(reg_base, PCIE_QOS_REG_MASK, 0, reg_val);
    } else {
        reg_val |= (cfg->otsd_lvl[PCIE_OTSD_RD] & PCIE_RW_OTSD_LVL_MASK) << PCIE_RD_OTSD_LVL_OFFSET;
        reg_val |= (PCIE_RW_OTSD_EN & PCIE_RW_OTSD_EN_MASK) << PCIE_RD_OTSD_EN_OFFSET;
        reg_val |= (cfg->otsd_lvl[PCIE_OTSD_RD] & PCIE_RW_OTSD_LVL_MASK) << PCIE_WR_OTSD_LVL_OFFSET;
        reg_val |= (PCIE_RW_OTSD_EN & PCIE_RW_OTSD_EN_MASK) << PCIE_WR_OTSD_EN_OFFSET;
        agentdrv_set_reg(reg_base, PCIE_QOS_REG_MASK, 0, reg_val);
    }

    return;
}

void agentdrv_get_otsd_from_core(void __iomem *base, u32 core_id, struct qos_otsd_config_type *cfg)
{
    u32 reg_val = 0;
    void __iomem *reg_base = base + SCHE_OTSD_REG_OFFSET + (PCIE_OTSD_REG_OFFSET * (core_id >> 1));

    reg_val = agentdrv_get_reg(reg_base, PCIE_QOS_REG_MASK, 0);
    cfg->otsd_lvl[PCIE_OTSD_RD] = (reg_val >> PCIE_RD_OTSD_LVL_OFFSET) & PCIE_RW_OTSD_LVL_MASK;
    cfg->otsd_lvl[PCIE_OTSD_WR] = (reg_val >> PCIE_WR_OTSD_LVL_OFFSET) & PCIE_RW_OTSD_LVL_MASK;

    return;
}