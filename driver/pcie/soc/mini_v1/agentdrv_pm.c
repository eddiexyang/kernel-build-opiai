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

#include "agentdrv_linkdown.h"
#include "agentdrv_pm.h"
#include "agentdrv_dfx.h"
#include "devdrv_util.h"

extern int agentdrv_linkdown_int_status_check(struct agentdrv_platform_dev *p_dev);
extern int pm_autosleep_set_state(suspend_state_t state);
extern struct agentdrv_dfx_log_info g_dfx_info;

void agentdrv_pm_clear_power_state_change_int_status(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_INT_STATUS1;
    writel(AGENTDRV_PME_STATE_CHG_BIT, reg_addr);
}

void agentdrv_pm_clear_pme_turn_off_int_status(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_INT_STATUS1;
    writel(AGENTDRV_DC_READY_L2_INT_BIT, reg_addr);
}

void agentdrv_pm_clear_enter_l2l3ready_int_status(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_INT_STATUS1;
    writel(AGENTDRV_DC_ENTER_L2_INT_BIT, reg_addr);
}

void agentdrv_pm_read_enter_l2l3ready_int_status(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_INT_STATUS1;
    reg_val = readl(reg_addr);
    devdrv_info("Enter l2l3ready int status. (reg_val=0x%x)\n", reg_val);

    return;
}

void agentdrv_pm_clear_enter_l1_int_status(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_INT_STATUS1;
    writel(AGENTDRV_DC_ENTER_L1_INT_BIT, reg_addr);
}

void agentdrv_pm_tl_set_enter_l0(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_ENTER_L0_CTRL;
    reg_val = readl(reg_addr);
    reg_val |= (1 << 0);
    writel(reg_val, reg_addr);
}
void agentdrv_pm_tl_aspm_idlecnt_disable(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_ASPM_IDLE_EN;
    reg_val = 0;
    writel(reg_val, reg_addr);
}
void agentdrv_pm_tl_aspm_idlecnt_enable(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_ASPM_IDLE_EN;
    reg_val = 3;
    writel(reg_val, reg_addr);
}

void agentdrv_pm_tl_set_enter_l1(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_PM_DC_CTRL;
    reg_val = readl(reg_addr);
    reg_val |= AGENTDRV_TL_DC_PCIPM_ENTER_L1_EN_BIT;
    writel(reg_val, reg_addr);
}

void agentdrv_pm_tl_set_enter_l2(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_PM_DC_CTRL;
    reg_val = readl(reg_addr);
    reg_val |= AGENTDRV_TL_DC_PCIPM_ENTER_L2_EN_BIT;
    writel(reg_val, reg_addr);
}

void agentdrv_pm_tl_clear_enter_l0(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_ENTER_L0_CTRL;
    reg_val = readl(reg_addr);
    reg_val &= ~(1 << 0);
    writel(reg_val, reg_addr);
}

void agentdrv_pm_tl_clear_enter_l1(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_PM_DC_CTRL;
    reg_val = readl(reg_addr);
    reg_val &= ~AGENTDRV_TL_DC_PCIPM_ENTER_L1_EN_BIT;
    writel(reg_val, reg_addr);
}

void agentdrv_pm_tl_clear_enter_l2(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_PM_DC_CTRL;
    reg_val = readl(reg_addr);
    reg_val &= ~AGENTDRV_TL_DC_PCIPM_ENTER_L2_EN_BIT;
    writel(reg_val, reg_addr);
}

void agentdrv_pm_power_state_change_int_handle(struct agentdrv_platform_dev *p_dev)
{
    u32 d_state;

    agentdrv_pm_clear_power_state_change_int_status(p_dev);

    /* only when D0->D3 */
    d_state = readl(p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_PME_STATUS);
    d_state = d_state & 0x3;
    if (d_state) {
        agentdrv_pm_tl_set_enter_l1(p_dev);
        agentdrv_pm_tl_clear_enter_l1(p_dev);
    }
}

void agentdrv_pm_dc_enter_l1_int_handle(struct agentdrv_platform_dev *p_dev)
{
    agentdrv_pm_clear_enter_l1_int_status(p_dev);
    devdrv_info("OK, our chip come into L1 sucess.\n");
}

void agentdrv_pm_receive_pmeturnoff_int_handle(struct agentdrv_platform_dev *p_dev)
{
    agentdrv_pm_clear_pme_turn_off_int_status(p_dev);

    /* bbox report enter */
    agentdrv_dfx_put_string("During pme_turn_off get in: ");
    agentdrv_dfx_linkdown_put_into_bbox(p_dev);

    /* close ASPM to avoid affect, set to L0 if in L1 */
    agentdrv_pm_tl_aspm_idlecnt_disable(p_dev);
    agentdrv_pm_tl_set_enter_l0(p_dev);
    udelay(100);
    agentdrv_pm_tl_clear_enter_l0(p_dev);
    agentdrv_pm_tl_aspm_idlecnt_enable(p_dev);

    agentdrv_pm_tl_set_enter_l2(p_dev);
    agentdrv_pm_tl_clear_enter_l2(p_dev);

    /* bbox report out */
    agentdrv_dfx_put_string("During pme_turn_off get out: ");
    agentdrv_dfx_linkdown_put_into_bbox(p_dev);

    g_dfx_info.dfx_linkdown_info.pme_turn_off_times++;
}

void agentdrv_pm_enter_l2l3ready_int_handle(struct agentdrv_platform_dev *p_dev)
{
    agentdrv_pm_clear_enter_l2l3ready_int_status(p_dev);
    /* read once to make sure pcie write complete */
    agentdrv_pm_read_enter_l2l3ready_int_status(p_dev);
    (void)schedule_work(&p_dev->pm_gosleep_work);
}

void agentdrv_pcie_pm_spi1_tl_pcipm_ready_l2_unmask(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_INT_MASK1;
    reg_val = readl(reg_addr);
    reg_val &= ~BIT(7);
    writel(reg_val, reg_addr);
}

void agentdrv_pcie_pm_spi1_tl_pcipm_enter_l2_unmask(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_INT_MASK1;
    reg_val = readl(reg_addr);
    reg_val &= ~BIT(5);
    writel(reg_val, reg_addr);
}

void agentdrv_pcie_pm_spi1_tl_pcipm_pme_state_chg_unmask(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_INT_MASK1;
    reg_val = readl(reg_addr);
    reg_val &= ~BIT(4);
    writel(reg_val, reg_addr);
}

void agentdrv_pcie_pm_spi1_tl_pcipm_enter_l1_unmask(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_INT_MASK1;
    reg_val = readl(reg_addr);
    reg_val &= ~BIT(1);
    writel(reg_val, reg_addr);
}

void agentdrv_pcie_pm_unmask_tl_int(struct agentdrv_platform_dev *p_dev)
{
    agentdrv_pcie_pm_spi1_tl_pcipm_ready_l2_unmask(p_dev);
    agentdrv_pcie_pm_spi1_tl_pcipm_enter_l2_unmask(p_dev);
    agentdrv_pcie_pm_spi1_tl_pcipm_pme_state_chg_unmask(p_dev);
    agentdrv_pcie_pm_spi1_tl_pcipm_enter_l1_unmask(p_dev);
}

void agentdrv_pcie_pm_clr_all_int_status(struct agentdrv_platform_dev *p_dev)
{
    agentdrv_pm_clear_power_state_change_int_status(p_dev);
    agentdrv_pm_clear_pme_turn_off_int_status(p_dev);
    agentdrv_pm_clear_enter_l1_int_status(p_dev);
    agentdrv_pm_clear_enter_l2l3ready_int_status(p_dev);
}

void agentdrv_handle_pm_int(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;
    int linkdown_status;

    /* power state change */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_INT_STATUS1;
    reg_val = readl(reg_addr);

    /* if linkdown, no need to handle old pm events anymore */
    linkdown_status = agentdrv_linkdown_int_status_check(p_dev);
    if (linkdown_status == 1) {
        agentdrv_pcie_pm_clr_all_int_status(p_dev);
        devdrv_info("Detect linkdown, no need to handle old pm events.\n");
        return;
    }

    if (is_pm_receive_pmeturnoff_int(reg_val)) {
        agentdrv_pm_receive_pmeturnoff_int_handle(p_dev);
        devdrv_info("Call agentdrv_pm_receive_pmeturnoff_int_handle.\n");
    } else if (is_pm_enter_l2l3reay_int(reg_val)) {
        agentdrv_pm_enter_l2l3ready_int_handle(p_dev);
        devdrv_info("Call agentdrv_pm_enter_l2l3ready_int_handle.\n");
    } else if (is_pm_power_state_change_int(reg_val)) {
        agentdrv_pm_power_state_change_int_handle(p_dev);
        devdrv_info("Call agentdrv_pm_power_state_change_int_handle.\n");
    } else if (is_pm_dc_enter_l1_int(reg_val)) {
        agentdrv_pm_dc_enter_l1_int_handle(p_dev);
        devdrv_info("Call agentdrv_pm_dc_enter_l1_int_handle.\n");
    }
}

void agentdrv_aspm_set_aspm_cap(struct agentdrv_platform_dev *p_dev, int enable)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    /* open L1, but close l0s in cap reg */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_ASPM_CAP;
    reg_val = readl(reg_addr);

    if (enable) {
        reg_val &= (~AGENTDRV_CFG_ASPM_L0SL1_CAP_BIT);
        reg_val |= AGENTDRV_CFG_ASPM_L1_CAP_BIT;
    } else {
        reg_val &= (~AGENTDRV_CFG_ASPM_L0SL1_CAP_BIT);
    }
    writel(reg_val, reg_addr);
}

void agentdrv_aspm_set_aspm_ctrl(struct agentdrv_platform_dev *p_dev, int enable)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    /* open L1, but close l0s in ctrl reg */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_ASPM_CTRL;
    reg_val = readl(reg_addr);

    if (enable) {
        reg_val &= (~AGENTDRV_CFG_ASPM_L0SL1_CTRL_BIT);
        reg_val |= AGENTDRV_CFG_ASPM_L1_CTRL_BIT;
    } else {
        reg_val &= (~AGENTDRV_CFG_ASPM_L0SL1_CTRL_BIT);
    }
    writel(reg_val, reg_addr);
}

void agentdrv_aspm_set_l1_enter_time(struct agentdrv_platform_dev *p_dev)
{
    /* l0s l1 enter time must match the credit update time. */
    void __iomem *reg_addr = NULL;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_ASPM_IDLE_CNT_REG;

    /* 2us idel will go to L1 */
    writel(0x20001, reg_addr);
}

void agentdrv_aspm_set_credit_update_time(struct agentdrv_platform_dev *p_dev)
{
    /* In fact, different speed and width needs different values,
     * but in mini, recommend values are all the same.
     */
    void __iomem *reg_addr = NULL;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_DL_REG + AGENTDRV_DL_TX_FC_UPDATE_P_CYCLE_G1;
    /* 0x201:0x201*16ns=8.208us */
    writel(0x201, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_DL_REG + AGENTDRV_DL_TX_FC_UPDATE_NP_CYCLE_G1;
    writel(0x201, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_DL_REG + AGENTDRV_DL_TX_FC_UPDATE_CPL_CYCLE_G1;
    writel(0x201, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_DL_REG + AGENTDRV_DL_TX_FC_UPDATE_P_CYCLE_G2;
    writel(0x201, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_DL_REG + AGENTDRV_DL_TX_FC_UPDATE_NP_CYCLE_G2;
    writel(0x201, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_DL_REG + AGENTDRV_DL_TX_FC_UPDATE_CPL_CYCLE_G2;
    writel(0x201, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_DL_REG + AGENTDRV_DL_TX_FC_UPDATE_P_CYCLE_G3;
    writel(0x201, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_DL_REG + AGENTDRV_DL_TX_FC_UPDATE_NP_CYCLE_G3;
    writel(0x201, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_DL_REG + AGENTDRV_DL_TX_FC_UPDATE_CPL_CYCLE_G3;
    writel(0x201, reg_addr);
}

void agentdrv_aspm_open_aspm(struct agentdrv_platform_dev *p_dev)
{
    agentdrv_aspm_set_aspm_cap(p_dev, AGENTDRV_DISABLE);
    agentdrv_aspm_set_aspm_ctrl(p_dev, AGENTDRV_DISABLE);
    agentdrv_aspm_set_l1_enter_time(p_dev);
    agentdrv_aspm_set_credit_update_time(p_dev);
    agentdrv_aspm_set_aspm_cap(p_dev, AGENTDRV_ENABLE);
    agentdrv_aspm_set_aspm_ctrl(p_dev, AGENTDRV_ENABLE);
}

void agentdrv_aspm_close_aspm(struct agentdrv_platform_dev *p_dev)
{
    agentdrv_aspm_set_aspm_cap(p_dev, AGENTDRV_DISABLE);
    agentdrv_aspm_set_aspm_ctrl(p_dev, AGENTDRV_DISABLE);
}

void agentdrv_pm_init_aspm(struct agentdrv_platform_dev *p_dev)
{
    if (p_dev->aspm_enable) {
        agentdrv_aspm_open_aspm(p_dev);
    } else {
        agentdrv_aspm_close_aspm(p_dev);
    }
}
