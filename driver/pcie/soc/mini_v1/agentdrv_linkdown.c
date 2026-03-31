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

#include "agentdrv_platform.h"
#include "agentdrv_linkdown.h"
#include "agentdrv_reset.h"
#include "agentdrv_dfx.h"

extern void __iomem *sysctrl_base;
extern struct agentdrv_dfx_log_info g_dfx_info;

void agentdrv_pm_init_aspm(struct agentdrv_platform_dev *p_dev);

void agentdrv_linkdown_reset_en(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    /* port0 linkdown_clr_port_en=1 in PHY_RESET */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_PCIE_CORE_PHY_RESET;
    reg_val = readl(reg_addr);
    reg_val |= (1 << 16);
    writel(reg_val, reg_addr);

    /* linkdown_rst_en in PORT_RESET_CFG=0 */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_PCIE_CORE_PORT_RESET_CFG;
    reg_val = readl(reg_addr);
    reg_val &= ~(1 << 16);
    writel(reg_val, reg_addr);
}

void agentdrv_pcie_mac_linkdown_up_init(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    /* unmask linkdown/linkup */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_LINK_INT_MASK_REG;
    writel(0xfffffffc, reg_addr);

    /* linkdown/linkup report to business int */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_MAC_INT_TYPE_SEL;
    writel(0x3F3F80FC, reg_addr);

    agentdrv_linkdown_reset_en(p_dev);
}

void agentdrv_linkdown_int_unmask(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *mask_reg = NULL;
    u32 reg_val;

    mask_reg = p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_LINK_INT_MASK_REG;
    reg_val = readl(mask_reg);
    reg_val &= ~(AGENTDRV_LINKDOWN_MASK_BIT);
    writel(reg_val, mask_reg);
}

void agentdrv_linkup_int_unmask(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *mask_reg = NULL;
    u32 reg_val;

    mask_reg = p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_LINK_INT_MASK_REG;
    reg_val = readl(mask_reg);
    reg_val &= ~(AGENTDRV_LINKUP_MASK_BIT);
    writel(reg_val, mask_reg);
}

void agentdrv_linkdown_int_mask(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *mask_reg = NULL;
    u32 reg_val;

    mask_reg = p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_LINK_INT_MASK_REG;
    reg_val = readl(mask_reg);
    reg_val |= AGENTDRV_LINKDOWN_MASK_BIT;
    writel(reg_val, mask_reg);
}

void agentdrv_linkup_int_mask(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *mask_reg = NULL;
    u32 reg_val;

    mask_reg = p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_LINK_INT_MASK_REG;
    reg_val = readl(mask_reg);
    reg_val |= AGENTDRV_LINKUP_MASK_BIT;
    writel(reg_val, mask_reg);
}

int agentdrv_linkdown_int_status_check(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *status_reg = NULL;
    u32 reg_val;

    status_reg = p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_LINK_INT_STATUS_REG;
    reg_val = readl(status_reg);
    if (reg_val & AGENTDRV_LINKDOWN_STATUS_BIT)
        return 1;

    return 0;
}

int agentdrv_linkup_int_status_check(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *status_reg = NULL;
    u32 reg_val;

    status_reg = p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_LINK_INT_STATUS_REG;
    reg_val = readl(status_reg);
    if (reg_val & AGENTDRV_LINKUP_STATUS_BIT)
        return 1;

    return 0;
}

void agentdrv_linkup_int_handle(struct agentdrv_platform_dev *p_dev)
{
    /* recovery ASPM config */
    agentdrv_pm_init_aspm(p_dev);
    devdrv_info("Got a linkup interrupt and handled over.\n");
}

int agentdrv_check_business_stop(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;
    u32 reg_val;

    /* Wait 10ms for the system bus to return the response
     * of bursted command
     */
    mdelay(10);

    /* Discover the IDLE of DMA module */
    reg_offset = p_dev->io_base + AGENTDRV_AP_DMA_REG + AGENTDRV_AP_DMA_PORT_IDLE_STS;
    reg_val = readl(reg_offset);
    if ((reg_val & 1) == 0) {
        devdrv_err("Linkdown init flow wait DMA IDLE failed.\n");
        return 0;
    }

    /* Discover the IDLE of OUTBOUND module */
    reg_offset = p_dev->io_base + AGENTDRV_AP_IOB_TX_REG + AGENTDRV_AP_IOB_TX_PORT_STATUS;
    reg_val = readl(reg_offset);
    if ((reg_val & (1 << 8)) == 0) {
        devdrv_err("Linkdown init flow wait outbound IDLE failed.\n");
        return 0;
    }

    /* Discover the IDLE of INBOUND module */
    reg_offset = p_dev->io_base + AGENTDRV_AP_IOB_RX_ODR_REG + AGENTDRV_AP_IB_ODR_PORT_IDLE_STATUS;
    reg_val = readl(reg_offset);
    if ((reg_val & 0x7) != 0x7) {
        devdrv_err("Linkdown init flow wait inbound IDLE failed.\n");
        return 0;
    }

    /* Discover the IDLE of INT module */
    reg_offset = p_dev->io_base + AGENTDRV_AP_INT_REG + AGENTDRV_AP_INT_LINK_DOWN_IDLE_STATUS;
    reg_val = readl(reg_offset);
    if ((reg_val & 0x1) != 1) {
        devdrv_err("Linkdown init flow wait INT IDLE failed.\n");
        return 0;
    }

    return 1;
}

void agentdrv_enable_port_en(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;
    u32 reg_val;

    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_PCIE_CORE_PORT_EN;
    reg_val = readl(reg_offset);
    reg_val |= 0x1;
    writel(reg_val, reg_offset);
}

void agentdrv_enable_port_reset(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;
    u32 reg_val;

    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + 0x0;

    /* set the reset en bit */
    reg_val = readl(reg_offset);
    reg_val = ((reg_val & 0xffff) | (1 << 16));
    writel(reg_val, reg_offset);

    /* clear the reset en bit */
    reg_val = (reg_val & 0xfffe);
    writel(reg_val, reg_offset);

    udelay(200);
}

/* cfg device ID */
void agentdrv_cfgspace_cfg_device_id(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;

    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_PCIHDR_ID;
    writel((DEVICE_ID << 16) | VENDOR_ID, reg_offset);
}

/* cfg single device */
void agentdrv_cfgspace_cfg_single_device(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;
    u32 reg_val;

    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_PCIHDR_MISC;
    reg_val = readl(reg_offset);
    writel(reg_val & (~(1 << 23)), reg_offset);
}

/* cfg BAR0/2/4 width in PCIHDR_BAR0/2/4 */
void agentdrv_cfgspace_cfg_bar_width(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;
    u32 reg_val;
    u32 i;

    for (i = 0; i < 6; i += 2) {
        reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + 0x10 + 0x4 * (long)i;
        reg_val = readl(reg_offset);
        writel(reg_val | (1 << 2), reg_offset);
    }
}

/* bar0~5 cfg_bar~n_type = 0  in PCIHDR_BAR0~5 */
void agentdrv_cfgspace_cfg_bar_type(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;
    u32 reg_val;
    u32 i;

    for (i = 0; i < 6; i++) {
        reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + 0x10 + 0x4 * (long)i;
        reg_val = readl(reg_offset);
        reg_val &= ~0x1;
        writel(reg_val, reg_offset);
    }
}

/* cfg_bar0/2/4_prefetch_en=0 in PCIHDR_BAR0/2/4 */
void agentdrv_cfgspace_cfg_bar_prefetch(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;
    u32 reg_val;

    /* cfg_bar0_prefetch_en=0 in PCIHDR_BAR0 */
    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_PCIHDR_BAR0;
    reg_val = readl(reg_offset);
    reg_val |= (1 << 3);
    writel(reg_val, reg_offset);

    /* cfg_bar2_prefetch_en=0 in PCIHDR_BAR2 */
    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_PCIHDR_BAR2;
    reg_val = readl(reg_offset);
    reg_val &= ~(1 << 3);
    writel(reg_val, reg_offset);

    /* cfg_bar4_prefetch_en=0 in PCIHDR_BAR4 */
    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_PCIHDR_BAR4;
    reg_val = readl(reg_offset);
    reg_val |= (1 << 3);
    writel(reg_val, reg_offset);
}

STATIC void agentdrv_cfgspace_aux_power_en(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;
    u32 reg_val;

    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_DEVICE_CTRL_STATUS;
    reg_val = readl(reg_offset);
    reg_val |= BIT(20);
    writel(reg_val, reg_offset);
}

STATIC void agentdrv_cfgspace_recovery_link_para(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;
    u32 reg_val;

    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_LINK_CAP;
    reg_val = readl(reg_offset);

    /* if on ASIC, cfg link para to x4,gen3 */
    if (p_dev->board_type == DEVDRV_PLATFORM_TYPE_ASIC) {
        /* cfg to x4 in ASIC */
        reg_val &= ~(0x3f << 4);
        reg_val |= (0x4 << 4);

        /* cfg to gen3 in ASIC */
        reg_val &= ~0xf;
        reg_val |= 0x3;
    } else {
        /* cfg to x4 in fpga */
        reg_val &= ~(0x3f << 4);
        reg_val |= (0x4 << 4);

        /* cfg to gen1 in fpga */
        reg_val &= ~0xf;
        reg_val |= 0x1;
    }

    writel(reg_val, reg_offset);
}

STATIC void agentdrv_cfgspace_close_aspm(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;
    u32 reg_val;

    /* close aspm support */
    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_LINK_CAP;
    reg_val = readl(reg_offset);
    reg_val &= ~(0x3 << 10);
    writel(reg_val, reg_offset);

    /* close aspm ctrl */
    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_LINK_CTRL_STATUS;
    reg_val = readl(reg_offset);
    reg_val &= ~0x3;
    writel(reg_val, reg_offset);
}

/* keep pcie gen3 and close gen4 */
STATIC void agentdrv_cfgspace_close_pcie_4(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;
    u32 reg_val;

    /* close sup_10bit_cpl_tag */
    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_DEVICE_CAPABILITY2;
    reg_val = readl(reg_offset);
    reg_val &= ~BIT(16);
    writel(reg_val, reg_offset);

    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_LINK_CAPABILITY2;
    writel(0x0000000e, reg_offset);

    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_ADVACD_CAP_CTRL;
    reg_val = readl(reg_offset);
    reg_val &= ~BIT(12);
    writel(reg_val, reg_offset);

    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_DEVICE_SERIAL_NUMBER_CAP_HEADER;
    reg_val = readl(reg_offset);
    reg_val &= ~(0xFFFu << 20);
    writel(reg_val, reg_offset);
}

STATIC void agentdrv_cfgspace_cfg_msix(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;
    u32 reg_val;

    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + 0xa0;
    reg_val = readl(reg_offset);
    reg_val |= (0x7f << 16);
    writel(reg_val, reg_offset);
}

STATIC void agentdrv_cfgspace_cfg_flr_cap(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;
    u32 reg_val;

    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_DEVICE_CAP;
    reg_val = readl(reg_offset);
    reg_val &= (~(0x1 << AGENTDRV_CFG_SPACE_DEVICE_FLR_CAP));
    writel(reg_val, reg_offset);
}

void agentdrv_tl_cfgspace_config(struct agentdrv_platform_dev *p_dev)
{
    agentdrv_cfgspace_cfg_device_id(p_dev);
    agentdrv_cfgspace_cfg_single_device(p_dev);
    agentdrv_cfgspace_cfg_bar_width(p_dev);
    agentdrv_cfgspace_cfg_bar_type(p_dev);
    agentdrv_cfgspace_cfg_bar_prefetch(p_dev);
    agentdrv_cfgspace_aux_power_en(p_dev);
    agentdrv_cfgspace_recovery_link_para(p_dev);
    agentdrv_cfgspace_close_aspm(p_dev);
    agentdrv_cfgspace_close_pcie_4(p_dev);
    agentdrv_cfgspace_cfg_msix(p_dev);
    agentdrv_cfgspace_cfg_flr_cap(p_dev);
}

void agentdrv_asic_phy_reset(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg = NULL;
    u32 val;

    reg = p_dev->io_base + AGENTDRV_PCIE_PHY + AGENTDRV_PHY_PCS_POWER_CTRL;
    val = readl(reg);
    val &= ~0xf;
    writel(val, reg);

    reg = p_dev->io_base + AGENTDRV_PCIE_PHY + AGENTDRV_PHY_PMA_POWER_CTRL;
    val = readl(reg);
    val &= ~0xf;
    writel(val, reg);

    reg = p_dev->io_base + AGENTDRV_PCIE_PHY + AGENTDRV_PHY_ANA_PWR_CTRL;
    val = readl(reg);
    val |= BIT(0);
    writel(val, reg);

    reg = p_dev->io_base + AGENTDRV_PCIE_PHY + AGENTDRV_PHY_PHY_RESET;
    val = readl(reg);
    val |= BIT(0);
    writel(val, reg);

    reg = p_dev->io_base + AGENTDRV_PCIE_PHY + AGENTDRV_PHY_REF_CLK_CTRL;
    val = readl(reg);
    val |= BIT(0);
    writel(val, reg);

    reg = p_dev->io_base + AGENTDRV_PCIE_PHY + AGENTDRV_PHY_PCS_LANE_RESET;
    val = readl(reg);
    val |= 0xF;
    writel(val, reg);

    reg = p_dev->io_base + AGENTDRV_PCIE_PHY + AGENTDRV_PHY_UPCS_PWR_CTRL;
    val = readl(reg);
    val |= BIT(1);
    writel(val, reg);

    reg = p_dev->io_base + AGENTDRV_PCIE_PHY + AGENTDRV_PHY_LNAEX_POWER_PRESENT;
    writel(0, reg);

    reg = p_dev->io_base + AGENTDRV_PCIE_PHY + AGENTDRV_PHY_PHY_RESET;
    val = readl(reg);
    val &= ~BIT(0);
    writel(val, reg);
}

void agentdrv_pcie_phy_reset(struct agentdrv_platform_dev *p_dev)
{
    /* Not support phy reset on FPGA */
    if (p_dev->board_type == DEVDRV_PLATFORM_TYPE_ASIC) {
        agentdrv_asic_phy_reset(p_dev);
    }
}

void agentdrv_pcie_soft_reset(void)
{
    void __iomem *reg = NULL;
    u32 val;

    /* assert pcie soft reset */
    reg = sysctrl_base + AGENTDRV_SYSCTRL_SC_PCIE_RESET_REQ;
    val = readl(reg);
    val |= BIT(1);
    writel(val, reg);

    /* deassert pcie soft reset */
    reg = sysctrl_base + AGENTDRV_SYSCTRL_SC_PCIE_RESET_DREQ;
    val = readl(reg);
    val |= BIT(1);
    writel(val, reg);

    udelay(200);
}

/* only clear linkdown&linkup int status */
void agentdrv_pcie_mac_int_status_clear(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg = NULL;
    u32 reg_val;

    reg = p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_LINK_INT_STATUS_REG;
    /* write 1 to clear */
    reg_val = BIT(0) | BIT(1);
    writel(reg_val, reg);
}

int agentdrv_pcie_linkdown_handle(struct agentdrv_platform_dev *p_dev)
{
    int ret;

    /* mask the interrupt */
    agentdrv_linkdown_int_mask(p_dev);

    if (g_dfx_info.linkdown_in_hotreset != 1) {
        /* get ltssm for bbox report */
        agentdrv_dfx_linkdown_collect_link_state(p_dev);
        /* log bbox msg */
        agentdrv_dfx_put_string("During pcie linkdown: ");
        agentdrv_dfx_linkdown_put_into_bbox(p_dev);
        agentdrv_notify_blackbox_exception(0, AGENTDRV_LINKDOWN_FAIL, g_dfx_info.bbox_storage_str);
    }

    /* wait all the business stopped */
    ret = agentdrv_check_business_stop(p_dev);
    if (!ret) {
        agentdrv_linkdown_int_unmask(p_dev);
        return -EIO;
    }

    /* enable port reset */
    agentdrv_enable_port_reset(p_dev);

    /* reconfig TL config space regs */
    agentdrv_tl_cfgspace_config(p_dev);

    /* enable port en */
    agentdrv_enable_port_en(p_dev);

    agentdrv_linkdown_int_unmask(p_dev);

    devdrv_info("Got a linkdown interrupt and handled over.\n");

    return 0;
}

void agentdrv_handle_link_int_sts(struct agentdrv_platform_dev *p_dev)
{
    int linkdown_status;
    int linkup_status;
    int hot_reset_status;

    /* linkdown interrupt handle */
    linkdown_status = agentdrv_linkdown_int_status_check(p_dev);
    linkup_status = agentdrv_linkup_int_status_check(p_dev);
    hot_reset_status = agentdrv_hot_reset_int_status_check(p_dev);

    if (hot_reset_status == 1) {
        g_dfx_info.linkdown_in_hotreset = 1;
    }
    if (linkdown_status == 1) {
        agentdrv_pcie_mac_int_status_clear(p_dev);
        /* if hot reset intr status is 1, drv and os not handle it;lpm3 will handle */
        if (hot_reset_status == 1) {
            devdrv_info("A hot reset got, drv not handle.\n");
        } else {
            (void)agentdrv_pcie_linkdown_handle(p_dev);
            devdrv_info("A linkdown interrupt handled over.\n");
        }
        return;
    }

    if (linkup_status == 1) {
        agentdrv_pcie_mac_int_status_clear(p_dev);
        agentdrv_linkup_int_handle(p_dev);
        return;
    }
}
