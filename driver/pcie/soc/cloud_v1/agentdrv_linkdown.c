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
#include "agentdrv_unit.h"
#include "devdrv_util.h"
#include "agentdrv_msg.h"
#include "nvme_comm_drv.h"

#define AGENTDRV_HOT_RESET_FLAG 0x5a5aa5a5

struct agentdrv_msg_dev *agentdrv_get_msgdev(int dev_id)
{
    struct agentdrv_devctrl *p_agentdrv_devctrl = NULL;

    p_agentdrv_devctrl = agentdrv_get_dev(dev_id);
    if (p_agentdrv_devctrl == NULL) {
        devdrv_err("Input parameter is invalid. (dev_id=%d)\n", dev_id);
        return NULL;
    }

    return p_agentdrv_devctrl->p_agentdrv_msg_dev[0];
}

void agentdrv_linkdown_reset_en(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    /* port0 linkdown_clr_port_en=1 in PHY_RESET */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_PCIE_CORE_PHY_RESET;
    reg_val = readl(reg_addr);
    reg_val |= LINKDOWN_REG_VAL_SELECT;
    writel(reg_val, reg_addr);

    /* linkdown_rst_en in PORT_RESET_CFG=0 */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_PCIE_CORE_PORT_RESET_CFG;
    reg_val = readl(reg_addr);
    reg_val &= ~LINKDOWN_REG_VAL_SELECT;
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
    writel(0x3F038077, reg_addr);
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
        return AGENTDRV_LINK_STATUS_VAL;

    return 0;
}

int agentdrv_linkup_int_status_check(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *status_reg = NULL;
    u32 reg_val;

    status_reg = p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_LINK_INT_STATUS_REG;
    reg_val = readl(status_reg);
    if (reg_val & AGENTDRV_LINKUP_STATUS_BIT)
        return AGENTDRV_LINK_STATUS_VAL;

    return 0;
}

void agentdrv_linkup_int_handle(struct agentdrv_platform_dev *p_dev)
{
    /* Currently, nothing should do for linkup */
    agentdrv_linkup_int_mask(p_dev);
    devdrv_info("Got a linkup interrupt and handled over. (dev_id=%d)\n", p_dev->agent_id);
    agentdrv_linkup_int_unmask(p_dev);
}

int agentdrv_check_business_stop(struct agentdrv_platform_dev *p_dev, struct agentdrv_msg_dev *p_agentdrv_msg_dev)
{
    struct agentdrv_msg_dev *p_msg_dev = p_agentdrv_msg_dev;
    void __iomem *reg_offset = NULL;
    u32 reg_val;

    /* Wait 10ms for the system bus to return the response
     * of bursted command
     */
    mdelay(10); /* wait 10ms */

    /* Discover the IDLE of DMA module */
    reg_offset = p_dev->io_base + AGENTDRV_AP_DMA_REG + AGENTDRV_AP_DMA_PORT_IDLE_STS;
    reg_val = readl(reg_offset);
    if ((reg_val & 1) == 0) {
        devdrv_err("Device linkdown init flow wait DMA idle failed. (dev_id=%d)\n", p_dev->agent_id);
        return 0;
    }

    /* Discover the IDLE of OUTBOUND module */
    reg_offset = p_dev->io_base + AGENTDRV_AP_IOB_TX_REG + AGENTDRV_IOB_TX_SLV_PORT_STATUS;
    reg_val = readl(reg_offset);
    if ((reg_val & OUTBOUND_MODULE_REG_VALUE) == 0) {
        devdrv_err("Device linkdown init flow wait outbound idle failed. (dev_id=%d)\n", p_dev->agent_id);
        return 0;
    }

    /* Discover the IDLE of INBOUND module */
    reg_offset = p_dev->io_base + AGENTDRV_AP_IOB_RX_ODR_REG + AGENTDRV_AP_IB_ODR_PORT_IDLE_STATUS;
    reg_val = readl(reg_offset);
    if ((reg_val & 0x7) != 0x7) {
        devdrv_err("Device linkdown init flow wait inbound idle failed. (dev_id=%d)\n", p_dev->agent_id);
        return 0;
    }

    /* Discover the IDLE of INT module */
    /* Discover the IDLE of NVME_GLOBAL_REG */
    reg_offset = p_msg_dev->sdi_base + AGENTDRV_NVME_GLOBAL_REG + AGENTDRV_NVME_PORT_IDLE;
    reg_val = readl(reg_offset);
    if ((reg_val & 0x1) != 0x1) {
        devdrv_err("Execute nvme_port_idle failed. (devid=%d)\n", p_dev->agent_id);
        return 0;
    }

    /* Discover port_idle of INT0 in AP_INT_REG */
    reg_offset = p_dev->io_base + AGENTDRV_AP_INT_REG + AGENTDRV_AP_INT_LINK_DOWN_IDLE_STATUS;
    reg_val = readl(reg_offset);
    if ((reg_val & 0x1) != 0x1) {
        devdrv_err("AGENTDRV_AP_INT_LINK_DOWN_IDLE_STATUS idle failed. (dev_id=%d)\n", p_dev->agent_id);
        return 0;
    }

    reg_offset = p_dev->io_base + AGENTDRV_AP_INT_REG + AGENTDRV_ERR_RESPONSE;
    reg_val = readl(reg_offset);
    if ((reg_val & 0x8) != 0x8) {
        devdrv_err("ERR_RESPONSE INT idle failed. (devid=%d)\n", p_dev->agent_id);
        return 0;
    }
    /* Discover the IDLE of VIRTIO module */
    reg_offset = p_msg_dev->sdi_base + AGENTDRV_VIRTIO_QUEUE_REG + AGENTDRV_VQ_NOTIFY_INI;
    reg_val = readl(reg_offset);
    if ((reg_val & 0x40000000) != 0x40000000) {
#ifndef DRV_UT
        devdrv_info("IDLE of VIRTIO module wait VQ_NOTIFY_INI idle. (dev_id=%d)\n", p_dev->agent_id);
#endif
    }
    return AGENTDRV_LINK_STATUS_VAL;
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

void agentdrv_cfgspace_cfg_link_speed(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;
    u32 reg_val;

    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_LINK_CAPBILITY;
    if (p_dev->board_type == DEVDRV_PLATFORM_TYPE_ASIC) {
        reg_val = readl(reg_offset);
        reg_val = (reg_val & (~(0xF))) | 0x4;
        reg_val = (reg_val & (~(0x3F << REG_OFFSET_FOUR))) | (0x10 << REG_OFFSET_FOUR);
        writel(reg_val, reg_offset);
    } else {
        reg_val = readl(reg_offset);
        reg_val = (reg_val & (~(0xF))) | 0x1;
        reg_val = (reg_val & (~(0x3F << REG_OFFSET_FOUR))) | (0x10 << REG_OFFSET_FOUR);
        writel(reg_val, reg_offset);
    }
    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_LINK_CTRL_STATUS2;
    reg_val = readl(reg_offset);
    reg_val = (reg_val & (~(0xF))) | 0x1;
    writel(reg_val, reg_offset);
}

/* cfg device ID */
void agentdrv_cfgspace_cfg_device_id(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;

    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_PCIHDR_ID;
    writel((DEVICE_ID << REG_OFFSET_SIXTEEN) | VENDOR_ID, reg_offset);
}

/*  cfg single device cloud CONFIG 1,mini CONFIG 0 */
void agentdrv_cfgspace_cfg_single_device(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;
    u32 reg_val;

    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_PCIHDR_MISC;
    reg_val = readl(reg_offset);
    writel(reg_val | CFGSPACE_CFG_REG_VALUE, reg_offset);
}

void agentdrv_cfgspace_cfg_bar_enable(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;
    u32 i;
    const int count = 4;

    for (i = 0; i < count; i += 1) {
        reg_offset = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_PFn_BARn_ENABLE + 0x4 * (long)i;
        writel(0x7, reg_offset);
    }
    reg_offset = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_PFn_BARn_ENABLE + 0x4 * REG_OFFSET_FOUR;
    writel(0x4, reg_offset);
    reg_offset = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_PFn_BARn_ENABLE + 0x4 * REG_OFFSET_FIVE;
    writel(0x0, reg_offset);
}

/* bar0~5 cfg_bar~n_type = 0  in PCIHDR_BAR0~5 */
void agentdrv_cfgspace_cfg_bar_type(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;
    u32 reg_val;
    u32 i;
    const int count = 6;

    for (i = 0; i < count; i++) {
        reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_PCIHDR_BAR0 + 0x4 * (long)i;
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
    reg_val |= 0xc;
    writel(reg_val, reg_offset);

    /* cfg_bar2_prefetch_en=0 in PCIHDR_BAR2 */
    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_PCIHDR_BAR2;
    reg_val = readl(reg_offset);
    reg_val |= 0x4;
    writel(reg_val, reg_offset);

    /* cfg_bar4_prefetch_en=0 in PCIHDR_BAR4 */
    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_PCIHDR_BAR4;
    reg_val = readl(reg_offset);
    reg_val |= 0xc;
    writel(0x0, reg_offset);
}

STATIC void agentdrv_cfgspace_aux_power_en(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;
    u32 reg_val;

    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_DEVICE_CTRL_STATUS;
    reg_val = readl(reg_offset);
    reg_val |= CFGSPACE_AUX_REG_VALUE;
    writel(reg_val, reg_offset);
}

STATIC void agentdrv_nvme_config(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;
    u32 reg_val;

    /* PCIHDR_CLSREV */
    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_PCIHDR_CLSREV;
    writel(0x12000020, reg_offset);

    /* DEVICE_CTRL_STATUS */
    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_DEVICE_CTRL_STATUS;
    reg_val = readl(reg_offset);
    reg_val = (reg_val & (~(0x1 << REG_OFFSET_ELEVEN))) | (reg_val & (~(0x1 << REG_OFFSET_EIGHT)));
    writel(reg_val, reg_offset);
}

STATIC void agentdrv_cfgspace_close_aspm(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;
    u32 reg_val;

    /* close aspm support */
    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_LINK_CAP;
    reg_val = readl(reg_offset);
    reg_val &= ~(0x3 << REG_OFFSET_TEN);
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
    reg_val = (reg_val & (~CFGSPACE_CLOSE_REG_VALUE)) | (reg_val | (0x1 << REG_OFFSET_FOUR));
    writel(reg_val, reg_offset);

    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_MSI_DATA;
    reg_val = readl(reg_offset);
    reg_val = reg_val & 0xffff0000;
    writel(reg_val, reg_offset);

    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_UNCR_ERR_MASK;
    reg_val = readl(reg_offset);
    reg_val = reg_val & (~(0x1 << REG_OFFSET_TWENTYSIX));
    writel(reg_val, reg_offset);

    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_UNCR_ERR_SEVERITY;
    reg_val = readl(reg_offset);
    reg_val = reg_val & (~(0x1 << REG_OFFSET_FIVE));
    writel(reg_val, reg_offset);

    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_COR_ERR_MASK;
    reg_val = readl(reg_offset);
    reg_val = (reg_val & (~(0x7 << REG_OFFSET_THIRTEEN)));
    writel(reg_val, reg_offset);

    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_ADVACD_CAP_CTRL;
    reg_val = readl(reg_offset);
    reg_val = (reg_val & (~(0x1 << REG_OFFSET_TWELVE)));
    writel(reg_val, reg_offset);
}

STATIC void agentdrv_cfgspace_cfg_msix(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_offset = NULL;

    reg_offset = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_MSIX_CAP_HEADER;
    writel(0x83ffb011, reg_offset);
}

STATIC void agentdrv_pcie_tl_pf_mask(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg = NULL;

    reg = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_PF0_BAR0_MASK;
    writel(0xffff, reg);
    reg = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_PF1_BAR0_MASK;
    writel(0xffff, reg);
    reg = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_PF2_BAR0_MASK;
    writel(0x3ffffff, reg);
}

STATIC void agentdrv_cfgspace_cfg_no_soft_reset(void __iomem *io_base)
{
    void __iomem *reg = NULL;
    u32 reg_val;

    reg = io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_PME_STATUS;
    reg_val = readl(reg);
    reg_val |= (0x1 << REG_OFFSET_THREE);
    writel(reg_val, reg);
}

STATIC void agentdrv_pcie_credit_cfg(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg = NULL;

    /* config 0xb1c、0xb20、0xb24、0xb28 */
    reg = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_RX_POSTED_CREDIT;
    writel(0xc80028, reg);
    reg = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_RX_NON_POSTED_CREDIT;
    writel(0x40028, reg);
    reg = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_RX_CPL_CREDIT;
    writel(0x4000100, reg);
    reg = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_RX_CDT_INI_UP;
    writel(0x1, reg);

    /*  config TL_RX_POSTED_CREDIT_DF B88 */
    reg = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_RX_POSTED_CREDIT_DF;
    writel(0x785018, reg);

    reg = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_RX_NON_POSTED_CREDIT_DF;
    writel(0x25018, reg);

    reg = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_RX_CPL_CREDIT_DF;
    writel(0x205008, reg);

    reg = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_RX_CDT_INI_UP_DF;
    writel(0x100, reg);
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

void agentdrv_cfgspace_rcb_clear(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg = NULL;
    u32 reg_val;

    reg = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_CFG_SPACE_LINK_CTRL_STATUS;
    reg_val = readl(reg);
    reg_val = reg_val & (~(0x1 << REG_OFFSET_THREE));
    writel(reg_val, reg);
}

void agentdrv_cfgspace_not_support_io_cmd(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg = NULL;
    u32 reg_val;

    reg = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_DL_FEATURE_CAP_REG04;
    reg_val = readl(reg);
    reg_val &= 0xfffffffe;
    writel(reg_val, reg);
}

void agentdrv_tl_cfg_base_lim_clear(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg = NULL;
    u32 reg_val;

    reg = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_BASE_LIMT_CHECK_EN;
    reg_val = readl(reg);
    reg_val = reg_val & (~(0x1 << REG_OFFSET_ONE));
    writel(reg_val, reg);
}

void agentdrv_cfgspace_cfg_int_pin_clear(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg = NULL;
    u32 reg_val;

    reg = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_PCIHDR_INT;
    reg_val = readl(reg);
    reg_val = (reg_val & (~(0xff << REG_OFFSET_EIGHT))) | (reg_val | (0x1 << REG_OFFSET_EIGHT));
    writel(reg_val, reg);
}

void agentdrv_cfg_linkdown_autoreset(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg = NULL;
    u32 reg_val;

    reg = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_PCIE_CORE_PORT_RESET_CFG;
    reg_val = readl(reg);
    reg_val = reg_val & (~(0x1 << REG_OFFSET_SIXTEEN));
    writel(reg_val, reg);

    reg = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_PCIE_CORE_PHY_RESET;
    reg_val = readl(reg);
    reg_val = reg_val | (0x1 << REG_OFFSET_SIXTEEN);
    writel(reg_val, reg);
}

void agentdrv_tl_cfg_eco_en(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg = NULL;
    u32 reg_val;

    reg = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_ECO_TL;
    reg_val = readl(reg);
    reg_val = reg_val | (0x1 << REG_OFFSET_ONE);
    writel(reg_val, reg);
}

void agentdrv_tl_cfgspace_config(struct agentdrv_platform_dev *p_dev)
{
    struct agentdrv_pfvf_info pfvf_info = { 0 };

    devdrv_soc_func2pfvf((u32)p_dev->agent_id, 0, 0, &pfvf_info);
    agentdrv_pcie_set_pf_num(p_dev, pfvf_info.pf_num | AGENTDRV_PF_SEL_NUM);
    /* config max_link_speed and target_link_speed */
    agentdrv_cfgspace_cfg_link_speed(p_dev);
    agentdrv_cfgspace_cfg_device_id(p_dev);
    agentdrv_cfgspace_cfg_single_device(p_dev);
    agentdrv_cfgspace_cfg_bar_enable(p_dev);
    agentdrv_cfgspace_cfg_bar_prefetch(p_dev);
    agentdrv_pcie_tl_pf_mask(p_dev);

    /* vf mask is default */
    /* The field "no_soft_reset" of register PME_STATUS in EPF_CFGSPACE
     * is set to be 0x1
     */
    agentdrv_cfgspace_cfg_no_soft_reset(p_dev->io_base);
    agentdrv_pcie_credit_cfg(p_dev);
    agentdrv_cfgspace_not_support_io_cmd(p_dev);
    agentdrv_cfgspace_cfg_bar_type(p_dev);
    agentdrv_cfgspace_rcb_clear(p_dev);
    agentdrv_tl_cfg_base_lim_clear(p_dev);
    agentdrv_cfgspace_cfg_int_pin_clear(p_dev);
    agentdrv_cfg_linkdown_autoreset(p_dev);
    agentdrv_cfgspace_aux_power_en(p_dev);
    agentdrv_tl_cfg_eco_en(p_dev);
    agentdrv_cfgspace_close_aspm(p_dev);
    agentdrv_nvme_config(p_dev);
    agentdrv_cfgspace_close_pcie_4(p_dev);
    agentdrv_cfgspace_cfg_msix(p_dev);
    agentdrv_cfgspace_cfg_flr_cap(p_dev);
}

void agentdrv_pcie_mac_int_status_clear(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg = NULL;
    u32 reg_val;

    reg = p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_LINK_INT_STATUS_REG;
    /* write 1 to clear */
    reg_val = readl(reg);
    writel(reg_val, reg);
}

void agentdrv_epf_cfg_sriov_cap(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg = NULL;
    u32 reg_val;

    reg = p_dev->io_base + AGENTDRV_PCIE_CFG_SPACE + AGENTDRV_SRIOV_CAP;
    reg_val = readl(reg);
    reg_val = reg_val & (~(0x1 << REG_OFFSET_TWO));
    writel(reg_val, reg);
}

void agentdrv_ap_ep_mode_init(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg = NULL;
    u32 reg_val;
    u32 i;
    struct agentdrv_pfvf_info pfvf_info = { 0 };

    /*  iob_tx_misc_ctrl */
    reg = p_dev->io_base + AGENTDRV_AP_IOB_TX_REG + AGENTDRV_IOB_TX_MISC_CTRL;
    reg_val = readl(reg);
    reg_val = reg_val | (0x7 << REG_OFFSET_FOUR);
    writel(reg_val, reg);

    devdrv_soc_func2pfvf((u32)p_dev->agent_id, 0, 0, &pfvf_info);
    for (i = 0; i <= pfvf_info.pf_num; i++) {
        /* select pf num i */
        agentdrv_pcie_set_pf_num(p_dev, i | AGENTDRV_PF_SEL_NUM);
        /* EPFi_CFGSPACE SRIOV_CAP */
        agentdrv_epf_cfg_sriov_cap(p_dev);
    }

    /* cfg_chi_misc_ctrl bit10 */
    reg = p_dev->io_base + AGENTDRV_AP_IOB_TX_REG + AGENTDRV_IOB_TX_CHI_CFG_MISC;
    reg_val = readl(reg);
    reg_val = reg_val | (0x1 << REG_OFFSET_TEN);
    writel(reg_val, reg);

    /* cfg_p_max_entries_allowed  */
    reg = p_dev->io_base + AGENTDRV_AP_IOB_TX_REG + AGENTDRV_IOB_TX_CHI_MAX_ENTRIES_CFG;
    writel(0xF, reg);

    /* cfg_chi_misc_ctrl bit3-bit2 */
    reg = p_dev->io_base + AGENTDRV_AP_IOB_TX_REG + AGENTDRV_IOB_TX_CHI_CFG_MISC;
    reg_val = readl(reg);
    reg_val = reg_val | (0x3 << REG_OFFSET_TWO);
    writel(reg_val, reg);

    /* iob_tx_poison_ctrl */
    reg = p_dev->io_base + AGENTDRV_AP_IOB_TX_REG + AGENTDRV_IOB_TX_POISON_CTRL;
    writel(0xFFFFF, reg);

    /* cfg_chi_ordering_ctrl bit[3] */
    reg = p_dev->io_base + AGENTDRV_AP_IOB_TX_REG + AGENTDRV_IOB_TX_CHI_CFG_ORDERING;
    reg_val = readl(reg);
    reg_val = reg_val | (0x1 << REG_OFFSET_SEVEN);
    writel(reg_val, reg);
}

int agentdrv_pcie_linkdown_handle(struct agentdrv_platform_dev *p_dev)
{
    struct agentdrv_msg_dev *p_agentdrv_msg_dev = NULL;
    int ret;
    void __iomem *reg = NULL;
    u32 reg_val;

    /* mask the interrupt */
    agentdrv_linkdown_int_mask(p_dev);

    p_agentdrv_msg_dev = agentdrv_get_msgdev(p_dev->agent_id);
    if (p_agentdrv_msg_dev == NULL) {
        devdrv_err("Parameter is invalid. (dev_id=%d)\n", p_dev->agent_id);
        return -EINVAL;
    }
    /* VIRTIO_QUEUE_REG */
    reg = p_agentdrv_msg_dev->sdi_base + AGENTDRV_VIRTIO_QUEUE_REG + AGENTDRV_VQ_NOTIFY_INI;
    reg_val = readl(reg);
    if ((reg_val & (0x1 << REG_OFFSET_THIRTY)) != (0x1 << REG_OFFSET_THIRTY)) {
        devdrv_info("Get reg_val.(dev_id=%d; VQ_NOTIFY_INI_reg_val=0x%x)\n", p_dev->agent_id, reg_val);
    }

    /* wait all the business stopped */
    ret = agentdrv_check_business_stop(p_dev, p_agentdrv_msg_dev);
    if (!ret) {
        agentdrv_linkdown_int_unmask(p_dev);
        devdrv_err("Call agentdrv_check_business_stop failed. (dev_id=%d)\n", p_dev->agent_id);
        return -EIO;
    }

    /* AP EP mode initialization flow */
    agentdrv_ap_ep_mode_init(p_dev);

    /* Configure the associated bit of iob_tx_cfg_reset
     * in AP_IOB_TX_REG to 0x1
     */
    reg = p_dev->io_base + AGENTDRV_AP_IOB_TX_REG + AGENTDRV_IOB_TX_CFG_RESET;
    writel(0x1, reg);

    /* Local CPU execute CFGSPACE related logic reset by
     * configuring "port_ctrl_sft_rst"
     */
    reg = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_PORT_RESET;
    reg_val = readl(reg);
    reg_val = reg_val | (0x1 << REG_OFFSET_SIXTEEN);
    writel(reg_val, reg);

    /* Close TL clock, local CPU configure icg_dis_pcie_tl_xx_xx
     * in PCIe_ICL_Subctrl;
     */
    reg = p_dev->subctrl_base + AGENTDRV_PCIE_ICL_SUBCTRL + AGENTDRV_SC_PCIE_TL_31_0_ICG_DIS;
    writel(0xFFFFFFFF, reg);
    reg = p_dev->subctrl_base + AGENTDRV_PCIE_ICL_SUBCTRL + AGENTDRV_SC_PCIE_TL_39_32_ICG_DIS;
    writel(0xFFFFFFFF, reg);

    /* Release the soft reset */
    reg = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_PORT_RESET;
    reg_val = readl(reg);
    reg_val = reg_val & (~(0x1 << REG_OFFSET_SIXTEEN));
    writel(reg_val, reg);

    /* Open TL clock, local CPU configure icg_en_pcie_tl_xx_xx
     * in PCIe_ICL_Subctrl
     */
    reg = p_dev->subctrl_base + AGENTDRV_PCIE_ICL_SUBCTRL + AGENTDRV_SC_PCIE_TL_31_0_ICG_EN;
    writel(0xFFFFFFFF, reg);
    reg = p_dev->subctrl_base + AGENTDRV_PCIE_ICL_SUBCTRL + AGENTDRV_SC_PCIE_TL_39_32_ICG_EN;
    writel(0xFFFFFFFF, reg);

    /* reconfig TL config space regs */
    agentdrv_tl_cfgspace_config(p_dev);

    /* enable port en */
    agentdrv_enable_port_en(p_dev);

    agentdrv_linkdown_int_unmask(p_dev);

    devdrv_info("Got a linkdown interrupt and handled over. (dev_id=%d)\n", p_dev->agent_id);

    return 0;
}

void agentdrv_hot_reset(struct work_struct *p_work)
{
    devdrv_info("Hot reset start on cpu. (smp_processor_id=%d)\n", smp_processor_id());
    devdrv_info("PCIE hot reset record end.\n");
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

    devdrv_info("Got status data. (dev_id=%d; linkdown_status=%d; linkup_status=%d; hot_reset_status=%d)\n",
                p_dev->agent_id, linkdown_status, linkup_status, hot_reset_status);

    if (hot_reset_status == 1) {
        devdrv_info("Start work hot reset device on cpu. (smp_processor_id=%d)\n", smp_processor_id());

        if (p_dev->hot_reset_flag_addr != NULL) {
            *(p_dev->hot_reset_flag_addr) = AGENTDRV_HOT_RESET_FLAG;
            devdrv_info("Notice bios hot reset.\n");
        }
        agentdrv_pcie_mac_int_status_clear(p_dev);
        INIT_WORK(&p_dev->reset, agentdrv_hot_reset);
        schedule_work(&p_dev->reset);
        return;
    }

    if (linkdown_status == 1) {
        agentdrv_pcie_mac_int_status_clear(p_dev);
        devdrv_info("A linkdown interrupt handled over.\n");
    }

    if (linkup_status == 1) {
        devdrv_info("A linkup interrupt handled over.\n");
        agentdrv_pcie_mac_int_status_clear(p_dev);
        agentdrv_linkup_int_handle(p_dev);
    }
    return;
}
