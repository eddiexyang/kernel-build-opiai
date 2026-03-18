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

#include "agentdrv_pci.h"
#include "agentdrv_platform.h"
#include "agentdrv_pm.h"
#include "agentdrv_linkdown.h"
#include "agentdrv_reset.h"
#include "devdrv_interface.h"
#include "agentdrv_dfx.h"
#include "agentdrv_unit.h"
#include "agentdrv_msg.h"
#include "devdrv_util.h"

void __iomem *subctrl_base;
void __iomem *sysctrl_base;
extern struct agentdrv_dfx_log_info g_dfx_info;

extern int pm_autosleep_set_state(suspend_state_t state);
void agentdrv_pcie_soft_reset(void);

int agentdrv_check_flr_reset_finish(u32 dev_id)
{
    (void)dev_id;
    return 0;
}

void agentdev_set_smmu_bypass(void)
{
    void __iomem *smmuby = NULL;

    smmuby = ioremap(DEVDRV_SMMU_BYPASS_REG, sizeof(u32));
    if (smmuby == NULL) {
        devdrv_err("Ioremap smmu bypass address error.\n");
        return;
    }
    writel(0x0, smmuby);

    iounmap(smmuby);
    smmuby = NULL;
}

void agentdrv_pcie_set_core_business_spi1(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    /* default:pcie global abnormal interrupt use SPI0;
     * business interrupt use SPI1
     */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_GLOBAL + AGENTDRV_PCIE_INT_SPI0_EN;
    writel(0x00007171, reg_addr);
}
void agentdrv_pci_unmask_core_business_spi0_int(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    /* only unmask SPI0 core1 business mask */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_GLOBAL + AGENTDRV_PCIE_SPI0_INT_MASK;
    writel(0x3ffdf, reg_addr);
}

void agentdrv_pci_unmask_core_business_spi1_int(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    /* only unmask SPI1 core1 business mask */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_GLOBAL + AGENTDRV_PCIE_SPI1_INT_MASK;
    writel(0x5ff, reg_addr);
}

/* SPI0 abnormal */
void agentdrv_pcie_global_spi0_int_status_clear(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_GLOBAL + AGENTDRV_PCIE_SPI0_INT_STATUS;
    writel(1 << 5, reg_addr);
}
/* SPI1 business */
void agentdrv_pcie_global_spi1_int_status_clear(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    reg_addr = p_dev->io_base + AGENTDRV_PCIE_GLOBAL + AGENTDRV_PCIE_SPI1_INT_STATUS;
    writel(1 << 9, reg_addr);
}

void agentdrv_pcie_global_int_init(struct agentdrv_platform_dev *p_dev)
{
    agentdrv_pcie_global_spi0_int_status_clear(p_dev);
    agentdrv_pcie_global_spi1_int_status_clear(p_dev);
    agentdrv_pcie_set_core_business_spi1(p_dev);
    agentdrv_pci_unmask_core_business_spi0_int(p_dev);
    agentdrv_pci_unmask_core_business_spi1_int(p_dev);
}
void agentdrv_pcie_global_spi0_int_unmask(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    /* pcie global unmask interrupt */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_GLOBAL + AGENTDRV_PCIE_SPI0_INT_MASK;
    reg_val = readl(reg_addr);
    reg_val = reg_val & (~AGENTDRV_CORE1_ABNOR_INT_BIT);
    writel(reg_val, reg_addr);
}

void agentdrv_pcie_global_spi1_int_unmask(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    /* pcie global unmask interrupt */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_GLOBAL + AGENTDRV_PCIE_SPI1_INT_MASK;
    reg_val = readl(reg_addr);
    reg_val = reg_val & (~AGENTDRV_CORE1_BUSI_INT_BIT);
    writel(reg_val, reg_addr);
}
void agentdrv_pcie_global_spi0_int_mask(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    /* pcie global unmask interrupt */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_GLOBAL + AGENTDRV_PCIE_SPI0_INT_MASK;
    reg_val = readl(reg_addr);
    reg_val = reg_val | AGENTDRV_CORE1_ABNOR_INT_BIT;
    writel(reg_val, reg_addr);
}

void agentdrv_pcie_global_spi1_int_mask(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    /* pcie global unmask interrupt */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_GLOBAL + AGENTDRV_PCIE_SPI1_INT_MASK;
    reg_val = readl(reg_addr);
    reg_val = reg_val | AGENTDRV_CORE1_BUSI_INT_BIT;
    writel(reg_val, reg_addr);
}

void agentdrv_pcie_spi0_core_unmask_port0_int(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    /* core global unmask SPI0 interrupt */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_MASK_A;
    writel(0xFFE, reg_addr);
}
void agentdrv_pcie_spi1_core_unmask_port0_int(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    /* core global unmask SPI1 interrupt */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_CORE_GLOBAL + AGENTDRV_CORE_INT_MASK_B;
    writel(0x2, reg_addr);
}

void agentdrv_pcie_core_int_init(struct agentdrv_platform_dev *p_dev)
{
    agentdrv_pcie_spi0_core_unmask_port0_int(p_dev);
    agentdrv_pcie_spi1_core_unmask_port0_int(p_dev);
}

void agentdrv_pcie_tl_clear_spi0_int_status(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    /* clear SPI0 status */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_INT_STATUS0;
    writel(0xffffffff, reg_addr);
}
void agentdrv_pcie_tl_clear_spi1_int_status(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    /* clear SPI1 status */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_TL_REG + AGENTDRV_TL_INT_STATUS1;
    writel(0xffffffff, reg_addr);
}

void agentdrv_pcie_mac_clear_int_status(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    /* clear status, except hotreset, lpm3 will handle it */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_MAC + AGENTDRV_LINK_INT_STATUS_REG;
    writel(0xFFFFFFBF, reg_addr);
}

void agentdrv_pcie_tl_init(struct agentdrv_platform_dev *p_dev)
{
    /* clear tl layber interrupt status got from booting period */
    agentdrv_pcie_tl_clear_spi0_int_status(p_dev);
    agentdrv_pcie_tl_clear_spi1_int_status(p_dev);

    /* pm related interrupts init */
    agentdrv_pcie_pm_unmask_tl_int(p_dev);
}

void agentdrv_pcie_mac_init(struct agentdrv_platform_dev *p_dev)
{
    agentdrv_pcie_mac_clear_int_status(p_dev);
    agentdrv_pcie_mac_linkdown_up_init(p_dev);
}

void agentdrv_pcie_dl_mask_int_all(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    /* clear status */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_DL_REG + AGENTDRV_DL_INT_STATUS;
    writel(0xFFFFFFFF, reg_addr);
}

void agentdrv_pcie_dl_clear_int_status(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    /* currently, we don't use any DL layer interrupts,
     * so just mask all the sources and clear the status.
     */
    reg_addr = p_dev->io_base + AGENTDRV_PCIE_DL_REG + AGENTDRV_DL_INT_MASK;
    writel(0x0001FFFF, reg_addr);
}

void agentdrv_pcie_dl_init(struct agentdrv_platform_dev *p_dev)
{
    agentdrv_pcie_dl_clear_int_status(p_dev);
    agentdrv_pcie_dl_mask_int_all(p_dev);
}
void agentdrv_unmask_subctrl_spi0_int(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = subctrl_base + AGENTDRV_WHOLE_GLOBAL_SPI0_MASK;
    reg_val = readl(reg_addr);
    reg_val = reg_val & (~AGENTDRV_WHOLE_GLOBAL_SPI0_BIT);

    writel(reg_val, reg_addr);
}
void agentdrv_init_spi_int(struct agentdrv_platform_dev *p_dev)
{
    devdrv_info("Enter agentdrv_init_spi_int.\n");

    /* tl layler init */
    agentdrv_pcie_tl_init(p_dev);

    /* dl layber init */
    agentdrv_pcie_dl_init(p_dev);

    /* mac layber init */
    agentdrv_pcie_mac_init(p_dev);

    /* core global layer init */
    agentdrv_pcie_core_int_init(p_dev);

    /* pcie global layer init */
    agentdrv_pcie_global_int_init(p_dev);

    /* unmask subctrl spi0 mask */
    agentdrv_unmask_subctrl_spi0_int(p_dev);
}

irqreturn_t agentdrv_pcie_spi0_abnormal_interrupt(int irq, void *data)
{
    struct agentdrv_platform_dev *p_dev = data;

    /* global mask interrupt */
    agentdrv_pcie_global_spi0_int_mask(p_dev);

    /* check and handle linkdown/up interrupts */
    agentdrv_handle_link_int_sts(p_dev);

    /* global clear int status(only 1 core in mini) */
    agentdrv_pcie_global_spi0_int_status_clear(p_dev);

    /* global unmask interrupt */
    agentdrv_pcie_global_spi0_int_unmask(p_dev);

    return IRQ_HANDLED;
}

irqreturn_t agentdrv_pcie_spi1_business_interrupt(int irq, void *data)
{
    struct agentdrv_platform_dev *p_dev = data;

    /* global mask interrupt */
    agentdrv_pcie_global_spi1_int_mask(p_dev);

    /* check and handle pm interrupts */
    agentdrv_handle_pm_int(p_dev);

    /* check and handle flr interrupts */
    agentdrv_handle_flr_int(p_dev);

    /* global clear int status(only 1 core in mini) */
    agentdrv_pcie_global_spi1_int_status_clear(p_dev);

    /* global unmask interrupt */
    agentdrv_pcie_global_spi1_int_unmask(p_dev);

    return IRQ_HANDLED;
}

void agentdrv_init_flr_int(struct agentdrv_platform_dev *p_dev)
{
    devdrv_info("Enter agentdrv_init_flr_int.\n");

    /* tl clear flr int status */
    agentdrv_clear_pf_flr_int_status(p_dev);

    /* tl_cfgcpl_crs_en */
    agentdrv_tl_cfgcpl_crs_en(p_dev);

    /* tl reg unmask pf flr interrupt */
    agentdrv_pf_flr_int_unmask(p_dev);

    /* tl flr int select SPI1 */
    agentdrv_tl_flr_int_sel(p_dev);
}

int agentdrv_resume_dma(struct agentdrv_devctrl *agent_dev)
{
    struct devdrv_dma_dev *dma_dev = NULL;
    struct pci_dev *dev = NULL;
    int ret = 0;
    const int MINI_DMA_INDEX = 0;

    dev = agent_dev->pdev;

    ret = agentdrv_dma_proc_func_init(dev, agent_dev, MINI_DMA_INDEX);
    if (ret) {
        devdrv_err("DMA device alloc failed. (dev_id=%d; fun=%d)\n", agent_dev->agent_id, MINI_DMA_INDEX);
        ret = -ENOMEM;
        return ret;
    }
    agent_dev->p_agentdrv_dma_dev[0] = dma_dev;

    return ret;
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

/*
bit0:pm enable flag
bit1:aspm enable flag
*/
void agentdrv_get_pm_swtich_from_nve(struct agentdrv_platform_dev *p_dev)
{
    u32 value = 0;
    int ret = 0;

    ret = of_property_read_u32(p_dev->dev->of_node, "pm_enable", &value);
    if (ret) {
        value = 0;
    }

    p_dev->pm_enable = value & BIT(0);
    p_dev->aspm_enable = (value >> 1) & BIT(0);
    devdrv_info("Get pm_enable. (pm_enable=%d; ret=%d)\n", value, ret);
    devdrv_info("Get PCI pm switch value. (pm_enable=%u)\n", p_dev->pm_enable);
    devdrv_info("Get aspm switch value. (aspm_enable=%u)\n", p_dev->aspm_enable);
}

void agentdrv_pm_gotosleep_func(struct work_struct *p_work)
{
    struct agentdrv_platform_dev *p_dev = NULL;

    p_dev = container_of(p_work, struct agentdrv_platform_dev, pm_gosleep_work);
#ifndef DRV_UT
    if (p_dev->pm_enable)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
        __pm_relax(p_dev->pm_wakelock.ws);
#else
        __pm_relax(&(p_dev->pm_wakelock.ws));
#endif
#endif
}

void agentdrv_pm_init_autosleep(struct agentdrv_platform_dev *p_dev)
{
    /* turn on the autosleep */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    p_dev->pm_wakelock.ws = wakeup_source_create("pci_wakelock");
    if (p_dev->pm_wakelock.ws == NULL) {
        devdrv_info("invalid wakeup_source.\n");
        return;
    }
    wakeup_source_add(p_dev->pm_wakelock.ws);
    __pm_stay_awake(p_dev->pm_wakelock.ws);
#else
    wakeup_source_init(&(p_dev->pm_wakelock.ws), "pci_wakelock");
    __pm_stay_awake(&(p_dev->pm_wakelock.ws));
#endif

    (void)pm_autosleep_set_state(PM_SUSPEND_MEM);

    /* Start a work in the L2/L3entry interrupt to call the sleep,
     * or we will get a soft watchdog calltrace.
     */
    INIT_WORK(&p_dev->pm_gosleep_work, agentdrv_pm_gotosleep_func);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
void agentdrv_pm_uninit_autosleep(struct agentdrv_platform_dev *p_dev)
{
    __pm_relax(p_dev->pm_wakelock.ws);
    wakeup_source_remove(p_dev->pm_wakelock.ws);
    wakeup_source_destroy(p_dev->pm_wakelock.ws);
    p_dev->pm_wakelock.ws = NULL;
}
#endif

struct agentdrv_platform_dev *agentdrv_platform_init(struct platform_device *apb_pdev, int agent_id,
                                                     void __iomem *apb_base)
{
    int irq;
    int ret;
    struct platform_device *pdev = apb_pdev;
    struct agentdrv_platform_dev *platform_dev = NULL;

    agentdrv_nvme_doorbell_lock_init();

    /* alloc pm dev */
    platform_dev = devm_kzalloc(&pdev->dev, sizeof(struct agentdrv_platform_dev), GFP_KERNEL);
    if (unlikely(platform_dev == NULL)) {
        devdrv_err("Alloc pm device memory space failed. (dev_id=%d)\n", agent_id);
        agentdrv_dfx_put_string("alloc pm dev mem space fail!\n");
        agentdrv_notify_blackbox_exception(agent_id, AGENTDRV_PM_PROBE_FAIL, g_dfx_info.bbox_storage_str);
        return NULL;
    } else {
        platform_dev->dev = &pdev->dev;
    }

    platform_dev->io_base = apb_base;

    /* for whole global spi0 int mask */
    subctrl_base = devm_ioremap(&pdev->dev, AGENTDRV_PERI_SUBCTRL_REG, 0x4000);
    if (IS_ERR_OR_NULL(subctrl_base)) {
        devdrv_err("Failed to ioremap subctrl base.\n");
        agentdrv_dfx_put_string("fail to ioremap subctrl base!\n");
        agentdrv_notify_blackbox_exception(agent_id, AGENTDRV_PM_PROBE_FAIL, g_dfx_info.bbox_storage_str);
        goto free_platform_dev;
    }

    /* init SPI0&SPI1 intrrupt */
    agentdrv_init_spi_int(platform_dev);

    /* init flr intrrupt */
    agentdrv_init_flr_int(platform_dev);
    sysctrl_base = devm_ioremap(&pdev->dev, AGENTDRV_SYSCTRL_CFG_REG, AGENTDRV_SYSCTRL_CFG_SIZE);
    if (IS_ERR_OR_NULL(sysctrl_base)) {
        devdrv_err("Failed to ioremap sysctrl base.\n");
        agentdrv_dfx_put_string("fail to ioremap sysctrl base!\n");
        agentdrv_notify_blackbox_exception(agent_id, AGENTDRV_PM_PROBE_FAIL, g_dfx_info.bbox_storage_str);
        goto iounmap_subctrl;
    }

    /* get irqs SPI0 */
    irq = platform_get_irq(pdev, AGENTDRV_INT_SPI0);
    if (irq < 0) {
        devdrv_err("Got irq failed. (dev_id=%d; irq=%d)\n", agent_id, irq);
        ret = irq;
        agentdrv_dfx_put_string("get irq fail!\n");
        agentdrv_notify_blackbox_exception(agent_id, AGENTDRV_PM_PROBE_FAIL, g_dfx_info.bbox_storage_str);
        goto iounmap_sysctrl;
    }
    platform_dev->irq_spi0 = irq;

    devdrv_info("Get irqs SPI0. (irq_spi0=%d)\n", irq);

    ret = devm_request_irq(&pdev->dev, irq, agentdrv_pcie_spi0_abnormal_interrupt, 0,
                           "agentdrv_pcie_abnormal_interrupt", platform_dev);
    if (ret) {
        devdrv_err("Request irq failed. (dev_id=%d; irq=%d)\n", agent_id, AGENTDRV_INT_SPI0);
        agentdrv_dfx_put_string("request irq fail!\n");
        agentdrv_notify_blackbox_exception(agent_id, AGENTDRV_PM_PROBE_FAIL, g_dfx_info.bbox_storage_str);
        goto iounmap_sysctrl;
    }

    /* get irqs SPI1 */
    irq = platform_get_irq(pdev, AGENTDRV_INT_SPI1);
    if (irq < 0) {
        devdrv_err("Get irq failed. (dev_id=%d; irq=%d)\n", agent_id, AGENTDRV_INT_SPI1);
        ret = irq;
        agentdrv_dfx_put_string("get irq fail!\n");
        agentdrv_notify_blackbox_exception(agent_id, AGENTDRV_PM_PROBE_FAIL, g_dfx_info.bbox_storage_str);
        goto free_irq;
    }
    platform_dev->irq_spi1 = irq;

    devdrv_info("Get irqs SPI1. (irq_spi1=%d)\n", irq);

    ret = devm_request_irq(&pdev->dev, irq, agentdrv_pcie_spi1_business_interrupt, 0,
                           "agentdrv_pcie_pm_business_interrupt", platform_dev);
    if (ret) {
        devdrv_err("Request irq failed. (dev_id=%d; irq=%d)\n", agent_id, AGENTDRV_INT_SPI1);
        agentdrv_dfx_put_string("request irq fail!\n");
        agentdrv_notify_blackbox_exception(agent_id, AGENTDRV_PM_PROBE_FAIL, g_dfx_info.bbox_storage_str);
        goto free_irq;
    }

    agentdrv_get_borad_type(platform_dev);
    agentdev_set_smmu_bypass();
    agentdrv_get_pm_swtich_from_nve(platform_dev);
    agentdrv_pm_init_autosleep(platform_dev);
    agentdrv_pm_init_aspm(platform_dev);

    /* DFX always record even if memory is full and overwite the old data. */
    agentdev_pcie_set_reg_ltssm_tracer_recap(platform_dev, 0x2);

    devdrv_info("Call agentdrv_platform_probe success.\n");

    return platform_dev;

free_irq:
    devm_free_irq(&pdev->dev, platform_dev->irq_spi0, platform_dev);

iounmap_sysctrl:
    devm_iounmap(&pdev->dev, sysctrl_base);
    sysctrl_base = NULL;

iounmap_subctrl:
    devm_iounmap(&pdev->dev, subctrl_base);
    subctrl_base = NULL;

free_platform_dev:
    devm_kfree(&pdev->dev, platform_dev);

    return NULL;
}

int agentdrv_platform_uninit(struct agentdrv_platform_dev *platform_dev, struct platform_device *apb_pdev)
{
    struct platform_device *pdev = apb_pdev;

    devm_free_irq(&pdev->dev, platform_dev->irq_spi0, platform_dev);
    devm_free_irq(&pdev->dev, platform_dev->irq_spi1, platform_dev);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    agentdrv_pm_uninit_autosleep(platform_dev);
#endif

    devm_iounmap(&pdev->dev, sysctrl_base);
    sysctrl_base = NULL;

    devm_iounmap(&pdev->dev, subctrl_base);
    subctrl_base = NULL;

    devm_kfree(&pdev->dev, platform_dev);

    return 0;
}

/* dma not available when sleep, disable it */
void devdrv_set_dma_dev_invalid(struct agentdrv_devctrl *agent_dev)
{
    struct devdrv_dma_dev *p_agentdrv_dma_dev = NULL;

    p_agentdrv_dma_dev = agent_dev->p_agentdrv_dma_dev[0];
    agent_dev->p_agentdrv_dma_dev[0] = NULL;

    msleep(AGENTDRV_MSLEEP_10);
    devdrv_dma_exit(p_agentdrv_dma_dev, DEVDRV_SRIOV_DISABLE);
}

int agentdrv_platform_suspend(struct platform_device *pdev, pm_message_t state)
{
    int dev_id = 0;
    struct agentdrv_devctrl *agent_dev = NULL;

    agent_dev = agentdrv_get_dev(dev_id);
    if (agent_dev == NULL) {
        devdrv_err("Function agentdrv_get_dev failed, agent_dev is null.\n");
        return -EINVAL;
    }

    agentdrv_pcie_global_spi0_int_mask(agent_dev->platform_dev);
    agentdrv_pcie_global_spi1_int_mask(agent_dev->platform_dev);

    devdrv_set_dma_dev_invalid(agent_dev);

    agentdrv_pcie_mac_int_status_clear(agent_dev->platform_dev);
    agentdrv_pcie_global_spi0_int_status_clear(agent_dev->platform_dev);
    agentdrv_pcie_global_spi1_int_status_clear(agent_dev->platform_dev);

    agentdrv_pci_unmask_core_business_spi0_int(agent_dev->platform_dev);
    agentdrv_pci_unmask_core_business_spi1_int(agent_dev->platform_dev);

    devdrv_info("Platform driver suspend success. (event=%d)\n", state.event);

    return 0;
}

int agentdrv_pcie_resume_regs(struct agentdrv_platform_dev *p_dev)
{
    int ret;

    /* mask the interrupt */
    agentdrv_linkdown_int_mask(p_dev);

    agentdrv_pcie_mac_int_status_clear(p_dev);
    agentdrv_pcie_global_spi0_int_status_clear(p_dev);

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

    /* unmask subctrl spi0 mask */
    agentdrv_unmask_subctrl_spi0_int(p_dev);

    agentdrv_linkdown_int_unmask(p_dev);

    /* enable port en */
    agentdrv_enable_port_en(p_dev);

    return 0;
}

int agentdrv_gpio_ctrl(struct platform_device *pdev)
{
    struct resource *res = NULL;
    const u32 AGENTDRV_GPIO_INDEX = 1;
    int reg_offset = 0x4;
    u32 res_idx = AGENTDRV_GPIO_INDEX;
    void __iomem *gpio_base = NULL;
    u32 reg_val;

    res = platform_get_resource(pdev, IORESOURCE_MEM, res_idx);
    if (IS_ERR_OR_NULL(res)) {
        devdrv_err("Get resource mem failed. (res_idx=%u)\n", res_idx);
        return -ENOMEM;
    }

    /* do not use devm_ioremap_resource. can't request region for resource */
    gpio_base = ioremap(res->start, res->end - res->start + 1);
    if (gpio_base == NULL) {
        devdrv_err("Function ioremap failed.\n");
        return -ENOMEM;
    }

    reg_val = readl(gpio_base + reg_offset);
    /* set bit1 to 1. to avoid ci warn, do not use set_bit */
    reg_val = reg_val | 0x2;
    writel(reg_val, gpio_base + reg_offset);

    reg_val = readl(gpio_base);
    /* set bit1 to zero. to avoid ci warn, do not use set_bit */
    reg_val = reg_val & 0xfffffffd;
    writel(reg_val, gpio_base);
    iounmap(gpio_base);
    gpio_base = NULL;

    return 0;
}

int agentdrv_platform_resume(struct platform_device *pdev)
{
    int ret;
    int dev_id = 0;
    struct agentdrv_devctrl *agent_dev = NULL;

    agent_dev = agentdrv_get_dev(dev_id);
    if (agent_dev == NULL) {
        devdrv_err("Function agentdrv_get_dev failed, agent_dev is null.\n");
        return -EINVAL;
    }

    agentdrv_pcie_global_spi0_int_mask(agent_dev->platform_dev);
    agentdrv_pcie_global_spi1_int_mask(agent_dev->platform_dev);

    agentdrv_pcie_soft_reset();

    ret = agentdrv_pcie_resume_regs(agent_dev->platform_dev);
    if (ret) {
        devdrv_err("Linkdown resume regs failed.\n");
    }

    /* Once linkdown happened, the DMA hardware should reset and reconfig */
    ret = agentdrv_resume_dma(agent_dev);
    if (ret) {
        devdrv_err("DMA resume failed.\n");
    }

    /* recovery ASPM config */
    agentdrv_pm_init_aspm(agent_dev->platform_dev);
#ifndef DRV_UT
    if (agent_dev->platform_dev->pm_enable)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
        __pm_stay_awake(agent_dev->platform_dev->pm_wakelock.ws);
#else
        __pm_stay_awake(&(agent_dev->platform_dev->pm_wakelock.ws));
#endif
#endif
    agentdrv_pcie_mac_int_status_clear(agent_dev->platform_dev);
    agentdrv_pcie_global_spi0_int_status_clear(agent_dev->platform_dev);
    agentdrv_pcie_global_spi1_int_status_clear(agent_dev->platform_dev);

    agentdrv_pci_unmask_core_business_spi0_int(agent_dev->platform_dev);
    agentdrv_pci_unmask_core_business_spi1_int(agent_dev->platform_dev);

    ret = agentdrv_gpio_ctrl(pdev);
    if (ret != 0) {
        devdrv_err("Platform gpio control failed.\n");
        return ret;
    }

    /* set dl bp en */
    agentdrv_set_dl_bp_en(agent_dev);

    devdrv_info("Platform driver resume finished.\n");

    return 0;
}
