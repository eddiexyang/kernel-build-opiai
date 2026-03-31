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
#include "devdrv_interface.h"
#include "devdrv_util.h"
#include "nvme_comm_drv.h"
#include "devdrv_dma.h"
#include "agentdrv_unit.h"
#include "resource_comm_drv.h"
#include "agentdrv_ctrl.h"
#include "agentdrv_flr.h"

int agentdrv_check_flr_reset_finish(u32 dev_id)
{
#ifndef DRV_UT
    struct agentdrv_devctrl *agent_dev = agentdrv_get_dev(dev_id);
    u32 pf_id = 0;
    u32 vf_id = 0;

    if ((agent_dev == NULL) || (agent_dev->platform_dev == NULL)) {
        devdrv_err("param NULL.\n");
        return -EINVAL;
    }

    if (agent_dev->pdev->is_physfn) {
        devdrv_err("Not support pf.\n");
        return -EINVAL;
    }

    devdrv_dev2chipfunc(dev_id, &pf_id, &vf_id);

    if ((agent_dev->platform_dev->flr.flr_state[vf_id] == AGENTDRV_FLR_STATE_OK) &&
        (agent_dev->platform_dev->flr.thread_state[vf_id] == AGENTDRV_THREAD_STATE_END)) {
        return 0;
    }

    return -EINVAL;
#else
    return 0;
#endif
}

void agentdrv_get_borad_type(struct agentdrv_platform_dev *p_dev)
{
    u32 board_type;
    u32 version = 0;

    board_type = PLATFORM_BOARD_TYPE;
    devdrv_info("board_type=%u version=%x\n", board_type, version);

    /* init board type */
    p_dev->board_type = board_type;
}

STATIC void agentdrv_pcie_config_pcie_err_mapping(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = p_dev->io_base + AGENTDRV_AP_GLOBAL_REG + AGENTDRV_PCIE_ERR_MAPPING;
    reg_val = readl(reg_addr);
    reg_val &= ~LOCAL_NI_TO_SPI0_MASK;
    reg_val |= LOCAL_NI_TO_SPI0_VAL;
    writel(reg_val, reg_addr);
}

STATIC void agentdrv_pcie_enable_ap_ni(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    reg_addr = p_dev->io_base + AGENTDRV_AP_GLOBAL_REG + AGENTDRV_PCIE_NI_ENA;
    writel(0x1, reg_addr);
}

STATIC void agentdrv_pcie_disable_ap_ni(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    reg_addr = p_dev->io_base + AGENTDRV_AP_GLOBAL_REG + AGENTDRV_PCIE_NI_ENA;
    writel(0x0, reg_addr);
}

STATIC void agentdrv_pcie_clear_status(struct agentdrv_platform_dev *p_dev)
{
    agentdrv_flr_clear_status(p_dev);
}

STATIC void agentdrv_init_spi_int(struct agentdrv_platform_dev *p_dev)
{
    agentdrv_pcie_config_pcie_err_mapping(p_dev);

    /* clear status */
    agentdrv_pcie_clear_status(p_dev);

    /* ENABLE FLR INT */
    agentdrv_handle_pcie_flr_unmask(p_dev);

    /* ENABLE AP_GLOBAL INT */
    agentdrv_pcie_enable_ap_ni(p_dev);
}

STATIC void agentdrv_uninit_spi_int(struct agentdrv_platform_dev *p_dev)
{
    /* ENABLE AP_GLOBAL INT */
    agentdrv_pcie_disable_ap_ni(p_dev);

    /* ENABLE FLR INT */
    agentdrv_handle_pcie_flr_mask(p_dev);

    /* clear status */
    agentdrv_pcie_clear_status(p_dev);
}

STATIC irqreturn_t agentdrv_pcie_flr_interrupt(int irq, void *data)
{
    struct agentdrv_platform_dev *p_dev = (struct agentdrv_platform_dev *)data;

    agentdrv_handle_pcie_flr_mask(p_dev);

    queue_work(p_dev->platform_workqueue, &p_dev->flr.flr_work);

    return IRQ_HANDLED;
}

STATIC int agentdrv_flr_interrupt_init(struct agentdrv_platform_dev *platform_dev, struct platform_device *pdev)
{
    int irq, ret;
    int agent_id = platform_dev->agent_id;

    INIT_WORK(&platform_dev->flr.flr_work, agentdrv_flr_task);

    /* get irqs SPI0 */
    irq = platform_get_irq(pdev, AGENTDRV_INT_SPI0);
    if (irq < 0) {
        devdrv_err("Get spi0 irq fail! (dev_id=%d)\n", agent_id);
        agentdrv_notify_blackbox_exception((u32)agent_id, AGENTDRV_PM_PROBE_FAIL, "Get spi0 irq fail!\n");
        ret = -EINVAL;
        goto cancel_work;
    }
    platform_dev->irq_spi0 = irq;

    devdrv_info("Platform_dev->irq_spi0 info. (dev_id=%d; irq=%d)\n", agent_id, irq);

    ret = devm_request_irq(&pdev->dev, (u32)irq, agentdrv_pcie_flr_interrupt, 0,
                           "agentdrv_pcie_flr_interrupt", platform_dev);
    if (ret) {
        devdrv_err("Request pcie_flr_irq fail! (dev_id=%d; irq=%d)\n", agent_id, irq);
        agentdrv_notify_blackbox_exception((u32)agent_id, AGENTDRV_PM_PROBE_FAIL, "Request pcie_flr_irq fail!\n");
        goto cancel_work;
    }

    /* init SPI intrrupt */
    agentdrv_init_spi_int(platform_dev);

    return 0;

cancel_work:
    cancel_work_sync(&platform_dev->flr.flr_work);

    return ret;
}

STATIC void agentdrv_flr_interrupt_uninit(struct agentdrv_platform_dev *platform_dev, struct platform_device *pdev)
{
    agentdrv_uninit_spi_int(platform_dev);

    cancel_work_sync(&platform_dev->flr.flr_work);
    devm_free_irq(&pdev->dev, (u32)platform_dev->irq_spi0, platform_dev);
}

struct agentdrv_platform_dev *agentdrv_platform_init(struct platform_device *apb_pdev, int agent_id,
                                                     void __iomem *apb_base)
{
    struct platform_device *pdev = apb_pdev;
    struct agentdrv_platform_dev *platform_dev = NULL;
    int ret;

    /* alloc pm dev */
    platform_dev = devm_kzalloc(&pdev->dev, sizeof(struct agentdrv_platform_dev), GFP_KERNEL);
    if (unlikely(platform_dev == NULL)) {
        devdrv_err("Alloc agentdrv_platform_dev fail! (dev_id=%d)\n", agent_id);
        agentdrv_notify_blackbox_exception((u32)agent_id, AGENTDRV_PM_PROBE_FAIL, "Alloc gentdrv_platform_dev fail!\n");
        return NULL;
    }
    platform_dev->dev = &pdev->dev;
    platform_dev->hot_reset_flag_addr = NULL;
    platform_dev->io_base = apb_base;
    platform_dev->agent_id = agent_id;

    platform_dev->platform_workqueue = create_workqueue("pcie_platform_workqueue");
    if (platform_dev->platform_workqueue == NULL) {
        devdrv_err("Create platform_workqueue failed. (dev_id=%d)\n", agent_id);
        agentdrv_notify_blackbox_exception((u32)agent_id,
            AGENTDRV_PM_PROBE_FAIL, "Create platform_workqueue failed!\n");
        goto free_platform_dev;
    }

    agentdrv_handle_pcie_flr_mask(platform_dev);

    ret = agentdrv_flr_interrupt_init(platform_dev, pdev);
    if (ret) {
        devdrv_err("Flr interrupt init fail! (dev_id=%d)\n", agent_id);
        agentdrv_notify_blackbox_exception((u32)agent_id, AGENTDRV_PM_PROBE_FAIL, "Flr interrupt init fail!\n");
        goto destroy_platform_workqueue;
    }

    agentdrv_get_borad_type(platform_dev);

    devdrv_info("Agentdrv_platform_init success! (dev_id=%d)\n", agent_id);

    return platform_dev;

destroy_platform_workqueue:
    destroy_workqueue(platform_dev->platform_workqueue);

free_platform_dev:
    devm_kfree(&pdev->dev, platform_dev);
    platform_dev = NULL;

    return NULL;
}

int agentdrv_platform_uninit(struct agentdrv_platform_dev *platform_dev, struct platform_device *apb_pdev)
{
    struct platform_device *pdev = apb_pdev;

    agentdrv_flr_interrupt_uninit(platform_dev, pdev);
    destroy_workqueue(platform_dev->platform_workqueue);

    devm_kfree(&pdev->dev, platform_dev);
    platform_dev = NULL;

    return 0;
}