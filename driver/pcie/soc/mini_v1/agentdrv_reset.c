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
#include <linux/module.h>
#include <linux/msi.h>
#include <linux/netdevice.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/pci.h>
#include <linux/suspend.h>

#include "agentdrv_reset.h"

extern void __iomem *sysctrl_base;
extern void __iomem *subctrl_base;

int agentdrv_pf_flr_int_status_check(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *status_reg = NULL;
    u32 reg_val;

    status_reg = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_PF_FLR_INT_STATUS;
    reg_val = readl(status_reg);
    if (reg_val & AGENTDRV_TL_PF_FLR_INT_STATUS_BIT)
        return 1;

    return 0;
}

void agentdrv_clear_pf_flr_int_status(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_PF_FLR_INT_STATUS;
    writel(AGENTDRV_TL_PF_FLR_INT_STATUS_BIT, reg_addr);
}

void agentdrv_pf_flr_int_mask(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    /* tl reg unmask pf flr interrupt */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_PF_FLR_INT_MASK;
    /* only mask pf0 */
    writel(0xf, reg_addr);
}

void agentdrv_pf_flr_int_unmask(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    /* tl reg unmask pf flr interrupt */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_PF_FLR_INT_MASK;
    /* only unmask pf0 */
    writel(0xe, reg_addr);
}

void agentdrv_tl_cfgcpl_crs_en(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_CFG_SPACE_CTRL;
    reg_val = readl(reg_addr);
    reg_val |= AGENTDRV_TL_CFGCPL_CRS_BIT;
    writel(reg_val, reg_addr);
}

void agentdrv_tl_flr_int_sel(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    /* tl flr int select SPI1 */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_FLR_INT_SEL;
    writel(0x1, reg_addr);
}

void agentdrv_handle_flr_int(struct agentdrv_platform_dev *p_dev)
{
    u32 flr_status;

    flr_status = agentdrv_pf_flr_int_status_check(p_dev);
    if (flr_status == 1) {
        agentdrv_pf_flr_int_mask(p_dev);
        agentdrv_clear_pf_flr_int_status(p_dev);

        agentdrv_pf_flr_int_unmask(p_dev);
        return;
    }
}

int agentdrv_hot_reset_int_status_check(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *status_reg = NULL;
    u32 reg_val;

    status_reg = p_dev->io_base + AGENTDRV_PCIE_MAC + AGRNTDRV_MAC_REG_MAC_INT_STATUS;
    reg_val = readl(status_reg);
    if (reg_val & AGRNTDRV_REG_HOT_RESET_INT_STATUS_BIT)
        return 1;

    return 0;
}
