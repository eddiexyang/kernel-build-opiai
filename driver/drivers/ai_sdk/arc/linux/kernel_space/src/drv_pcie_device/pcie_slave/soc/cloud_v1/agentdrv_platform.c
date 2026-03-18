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
#include <linux/timer.h>

#include "agentdrv_platform.h"
#include "agentdrv_linkdown.h"
#include "devdrv_interface.h"
#include "devdrv_util.h"
#include "nvme_comm_drv.h"

int agentdrv_check_flr_reset_finish(u32 dev_id)
{
    (void)dev_id;
    return 0;
}

/* SPI0 abnormal */
void agentdrv_pcie_global_spi_int_status_clear(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = p_dev->io_base + AGENTDRV_PCS_INT + AGENTDRV_PCS_INTR_STATUS;
    reg_val = readl(reg_addr);
    writel(reg_val, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_HILINK_INT_STATUS;
    reg_val = readl(reg_addr);
    writel(reg_val, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_NI_STATUS_0;
    reg_val = readl(reg_addr);
    writel(reg_val, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_NI_RO_0;
    reg_val = readl(reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_NI_STATUS_1;
    reg_val = readl(reg_addr);
    writel(reg_val, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_CE_STATUS_0;
    reg_val = readl(reg_addr);
    writel(reg_val, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_CE_STATUS_1;
    reg_val = readl(reg_addr);
    writel(reg_val, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_NFE_STATUS_0;
    reg_val = readl(reg_addr);
    writel(reg_val, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_NFE_STATUS_1;
    reg_val = readl(reg_addr);
    writel(reg_val, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_AP_MSG_REG_BASE + AGENTDRV_PCIE_NI_STATUS;
    reg_val = readl(reg_addr);
    writel(reg_val, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_AP_MSG_REG_BASE + AGENTDRV_PCIE_CE_STATUS;
    reg_val = readl(reg_addr);
    writel(reg_val, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_AP_MSG_REG_BASE + AGENTDRV_PCIE_UNF_STATUS;
    reg_val = readl(reg_addr);
    writel(reg_val, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_AP_MSG_REG_BASE + AGENTDRV_PCIE_UF_STATUS;
    reg_val = readl(reg_addr);
    writel(reg_val, reg_addr);
}

void agentdrv_pcie_global_spi_int_unmask(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_NFE_MSK_0;
    writel(0x0, reg_addr);
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_NI_MSK_0;
    writel(0x0, reg_addr);
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_CE_MSK_0;

    writel(0x0, reg_addr);
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_FE_MSK_0;
    writel(0x0, reg_addr);

    return;
}

void agentdrv_pcie_global_spi_int_mask(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_NFE_MSK_0;
    writel(0xFFFFFFFF, reg_addr);
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_NI_MSK_0;
    writel(0xFFFFFFFF, reg_addr);
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_CE_MSK_0;

    writel(0xFFFFFFFF, reg_addr);
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_FE_MSK_0;
    writel(0xFFFFFFFF, reg_addr);

    return;
}

void agentdrv_pcie_mac_info_show(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_LINK_INT_STATUS_REG;
    reg_val = readl(reg_addr);
    devdrv_info("Get MAC Interrupt status. (reg_val=0x%x)\n", reg_val);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_LINK_INT_MASK_REG;
    reg_val = readl(reg_addr);
    devdrv_info("Get MAC Interrupt mask. (reg_val=0x%x)\n", reg_val);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_MAC_LINK_INFO;
    reg_val = readl(reg_addr);
    devdrv_info("Get MAC link information. (reg_val=0x%x)\n", reg_val);
}

void agentdrv_pcie_mac_clear_int_status(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    /* clear status */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_LINK_INT_STATUS_REG;
    writel(0xFFFFFFFF, reg_addr);
}

void agentdrv_pcie_mac_init(struct agentdrv_platform_dev *p_dev)
{
    agentdrv_pcie_mac_clear_int_status(p_dev);
    agentdrv_pcie_mac_linkdown_up_init(p_dev);
}

void agentdrv_pcie_set_pf_num(struct agentdrv_platform_dev *p_dev, u32 val)
{
    void __iomem *reg_addr = NULL;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_CFG_ACCESS_CTRL;
    writel(val, reg_addr);
}

void agentdrv_pcie_config_pcie_err_mapping(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = p_dev->io_base + AGENTDRV_AP_MSG_REG_BASE + AGENTDRV_PCIE_ERR_MAPPING;
    reg_val = readl(reg_addr);
    reg_val = reg_val | (0x1 << PLATFORM_OFFSET_TWENTY);
    writel(reg_val, reg_addr);
}

void agentdrv_pcie_enable_ena_register(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    reg_addr = p_dev->io_base + AGENTDRV_AP_MSG_REG_BASE + AGENTDRV_PCIE_NI_ENA;
    writel(0x2200000, reg_addr);
}

void agentdrv_pcie_core_int_mask(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_NI_MSK_0;
    writel(0x3FFFFFB, reg_addr);
}

void agentdrv_pcie_virtio_queue_reg(struct agentdrv_platform_dev *p_dev)
{
    struct agentdrv_msg_dev *p_agentdrv_msg_dev = NULL;

    p_agentdrv_msg_dev = agentdrv_get_msgdev(p_dev->agent_id);
    if (p_agentdrv_msg_dev == NULL) {
        devdrv_err("Device ID is invalid, p_agentdrv_msg_dev is NULL. (dev_id=%d)\n", p_dev->agent_id);
        return;
    }
}

void agentdrv_pcie_reg_clear_status(void __iomem *reg_addr)
{
    u32 reg_val;

    reg_val = readl(reg_addr);
    writel(reg_val, reg_addr);
}

void agentdrv_pcie_clear_status(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_INT_STATUS0;
    agentdrv_pcie_reg_clear_status(reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_INT_STATUS1;
    agentdrv_pcie_reg_clear_status(reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_DL_REG + AGENTDRV_DL_INT_STATUS;
    agentdrv_pcie_reg_clear_status(reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_MAC_INT_STATUS;
    writel(0x3, reg_addr);

    reg_addr = p_dev->io_base + 0x1C0000 + 1 * 0x10000 + AGENTDRV_PCS_INTR_STATUS;
    agentdrv_pcie_reg_clear_status(reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_HILINK_INT_STATUS;
    agentdrv_pcie_reg_clear_status(reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_NI_STATUS_0;
    agentdrv_pcie_reg_clear_status(reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_NI_STATUS_1;
    agentdrv_pcie_reg_clear_status(reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_CE_STATUS_0;
    agentdrv_pcie_reg_clear_status(reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_CE_STATUS_1;
    agentdrv_pcie_reg_clear_status(reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_NFE_STATUS_0;
    agentdrv_pcie_reg_clear_status(reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_NFE_STATUS_1;
    agentdrv_pcie_reg_clear_status(reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_AP_MSG_REG_BASE + AGENTDRV_PCIE_NI_STATUS;
    agentdrv_pcie_reg_clear_status(reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_AP_MSG_REG_BASE + AGENTDRV_PCIE_CE_STATUS;
    agentdrv_pcie_reg_clear_status(reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_AP_MSG_REG_BASE + AGENTDRV_PCIE_UNF_STATUS;
    agentdrv_pcie_reg_clear_status(reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_AP_MSG_REG_BASE + AGENTDRV_PCIE_UF_STATUS;
    agentdrv_pcie_reg_clear_status(reg_addr);
}

void agentdrv_init_spi_int(struct agentdrv_platform_dev *p_dev)
{
    struct agentdrv_pfvf_info pfvf_info = { 0 };

    devdrv_info("Enter agentdrv_init_spi_int start. (dev_id=%d)\n", p_dev->agent_id);

    devdrv_soc_func2pfvf((u32)p_dev->agent_id, 0, 0, &pfvf_info);
    agentdrv_pcie_set_pf_num(p_dev, pfvf_info.pf_num | AGENTDRV_PF_SEL_NUM);
    agentdrv_pcie_config_pcie_err_mapping(p_dev);

    /* ENABLE ENA register */
    agentdrv_pcie_enable_ena_register(p_dev);

    /* CORE GLOBAL REGISTER AGENTDRV_PCIE_CORE_GLOBAL */
    agentdrv_pcie_core_int_mask(p_dev);

    /* clear status */
    agentdrv_pcie_clear_status(p_dev);

    agentdrv_pcie_mac_init(p_dev);
}

irqreturn_t agentdrv_pcie_spi0_abnormal_interrupt(int irq, void *data)
{
    struct agentdrv_platform_dev *p_dev = (struct agentdrv_platform_dev *)data;

    devdrv_info("Enter spi0 interrupt. (dev_id=%d )\n", p_dev->agent_id);

    /* global mask interrupt */
    agentdrv_pcie_global_spi_int_mask(p_dev);

    /* check and handle linkdown/up interrupts */
    agentdrv_handle_link_int_sts(p_dev);

    agentdrv_pcie_mac_info_show(p_dev);

    /* global clear int status(only 1 core in mini) */
    agentdrv_pcie_global_spi_int_status_clear(p_dev);

    /* global unmask interrupt */
    agentdrv_pcie_global_spi_int_unmask(p_dev);

    return IRQ_HANDLED;
}

irqreturn_t agentdrv_pcie_spi1_business_interrupt(int irq, void *data)
{
    struct agentdrv_platform_dev *p_dev = (struct agentdrv_platform_dev *)data;

    agentdrv_pcie_global_spi_int_mask(p_dev);
    agentdrv_pcie_global_spi_int_status_clear(p_dev);
    agentdrv_pcie_global_spi_int_unmask(p_dev);

    return IRQ_HANDLED;
}

void agentdrv_get_borad_type(struct agentdrv_platform_dev *p_dev)
{
    u32 board_type = 0;
    u32 version = 0;

    (void)devdrv_get_platform_type(&board_type, &version);

    devdrv_info("Get platform type. (board_type=%u; version=%x)\n", board_type, version);

    /* init board type */
    p_dev->board_type = board_type;
}

int agentdrv_reg_addr_init(struct agentdrv_platform_dev *platform_dev, struct platform_device *pdev)
{
    int agent_id = platform_dev->agent_id;
    platform_dev->subctrl_base = devm_ioremap(&pdev->dev, agent_id * AGENTDRV_CHIP_BASE + AGENTDRV_SUBCTRL_CFG_REG,
                                              AGENTDRV_SUBCTRL_CFG_SIZE);
    if (IS_ERR_OR_NULL(platform_dev->subctrl_base)) {
        devdrv_err("Fail to ioremap subctrl base. (dev_id=%d)\n", agent_id);
        agentdrv_notify_blackbox_exception(agent_id, AGENTDRV_PM_PROBE_FAIL, "fail to ioremap subctrl base!\n");
        return -ENODEV;
    }
    platform_dev->reg_base = devm_ioremap(&pdev->dev, agent_id * AGENTDRV_CHIP_BASE + AGENTDRV_REG_BASE_REG,
                                          AGENTDRV_REG_BASE_SIZE);
    if (IS_ERR_OR_NULL(platform_dev->reg_base)) {
        devdrv_err("Fail to ioremap reg base. (dev_id=%d)\n", agent_id);
        agentdrv_notify_blackbox_exception(agent_id, AGENTDRV_PM_PROBE_FAIL, "fail to ioremap reg base!\n");
        goto iounmap_subctrl;
    }
    platform_dev->sysctrl_base = devm_ioremap(&pdev->dev, agent_id * AGENTDRV_CHIP_BASE + AGENTDRV_SYSCTRL_CFG_REG,
                                              AGENTDRV_SYSCTRL_CFG_SIZE);
    if (IS_ERR_OR_NULL(platform_dev->sysctrl_base)) {
        devdrv_err("Fail to ioremap sysctrl base.\n");
        agentdrv_notify_blackbox_exception(agent_id, AGENTDRV_PM_PROBE_FAIL, "fail to ioremap sysctrl base!\n");
        goto iounmap_regbase;
    }
    return 0;

iounmap_regbase:
    devm_iounmap(&pdev->dev, platform_dev->reg_base);
    platform_dev->reg_base = NULL;
iounmap_subctrl:
    devm_iounmap(&pdev->dev, platform_dev->subctrl_base);
    platform_dev->subctrl_base = NULL;

    return -ENODEV;
}

int agentdrv_linkdown_interrupt_init(struct agentdrv_platform_dev *platform_dev, struct platform_device *pdev)
{
    /* init SPI0&SPI1 intrrupt */
    agentdrv_init_spi_int(platform_dev);

    /* spi whole interrupt mask */
    writel(0xab0, platform_dev->reg_base + 0xc);
    writel(0x1, platform_dev->reg_base);

    return 0;
}

struct agentdrv_platform_dev *agentdrv_platform_init(struct platform_device *apb_pdev, int agent_id,
                                                     void __iomem *apb_base)
{
    int ret;
    struct platform_device *pdev = apb_pdev;
    struct agentdrv_platform_dev *platform_dev = NULL;

    /* alloc pm dev */
    platform_dev = devm_kzalloc(&pdev->dev, sizeof(struct agentdrv_platform_dev), GFP_KERNEL);
    if (unlikely(platform_dev == NULL)) {
        devdrv_err("Alloc pm device memory space failed. (dev_id=%d)\n", agent_id);
        agentdrv_notify_blackbox_exception(agent_id, AGENTDRV_PM_PROBE_FAIL, "alloc pm dev mem space fail!\n");
        return NULL;
    }
    platform_dev->dev = &pdev->dev;
    platform_dev->hot_reset_flag_addr = NULL;
    platform_dev->io_base = apb_base;
    platform_dev->agent_id = agent_id;

    ret = agentdrv_reg_addr_init(platform_dev, pdev);
    if (ret) {
        goto free_platform_dev;
    }

    ret = agentdrv_linkdown_interrupt_init(platform_dev, pdev);
    if (ret) {
        goto iounmap_reg_addr;
    }

    agentdrv_get_borad_type(platform_dev);

    devdrv_info("Call agentdrv_platform_probe success. (dev_id=%d)\n", agent_id);

    return platform_dev;

iounmap_reg_addr:
    devm_iounmap(&pdev->dev, platform_dev->sysctrl_base);
    platform_dev->sysctrl_base = NULL;
    devm_iounmap(&pdev->dev, platform_dev->reg_base);
    platform_dev->reg_base = NULL;
    devm_iounmap(&pdev->dev, platform_dev->subctrl_base);
    platform_dev->subctrl_base = NULL;

free_platform_dev:
    devm_kfree(&pdev->dev, platform_dev);
    platform_dev = NULL;

    return NULL;
}

int agentdrv_platform_uninit(struct agentdrv_platform_dev *platform_dev, struct platform_device *apb_pdev)
{
    struct platform_device *pdev = apb_pdev;

    devm_iounmap(&pdev->dev, platform_dev->sysctrl_base);
    platform_dev->sysctrl_base = NULL;

    devm_iounmap(&pdev->dev, platform_dev->reg_base);
    platform_dev->reg_base = NULL;

    devm_iounmap(&pdev->dev, platform_dev->subctrl_base);
    platform_dev->subctrl_base = NULL;

    devm_kfree(&pdev->dev, platform_dev);
    platform_dev = NULL;

    return 0;
}
