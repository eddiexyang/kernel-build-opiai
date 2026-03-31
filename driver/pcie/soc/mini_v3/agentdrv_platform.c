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
#include "agentdrv_multi_chip.h"
#include "devdrv_interface.h"
#include "devdrv_util.h"
#include "nvme_comm_drv.h"

int agentdrv_check_flr_reset_finish(u32 dev_id)
{
    (void)dev_id;
    return 0;
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
struct agentdrv_platform_dev *agentdrv_platform_init(struct platform_device *apb_pdev, int agent_id,
                                                     void __iomem *apb_base)
{
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

    agentdrv_get_borad_type(platform_dev);

    /* ioremap qos_sche reg */
    platform_dev->sche_base = ioremap(PCIE_SCHE_REG_BASE, PCIE_SCHE_REG_SIZE);
    if (platform_dev->sche_base == NULL) {
        devdrv_err("Sche_base ioremap failed. (dev_id=%d)\n", agent_id);
        devm_kfree(&pdev->dev, platform_dev);
        platform_dev = NULL;
        return NULL;
    }

    devdrv_info("Call agentdrv_platform_probe success. (dev_id=%d)\n", agent_id);

    return platform_dev;
}

int agentdrv_platform_uninit(struct agentdrv_platform_dev *platform_dev, struct platform_device *apb_pdev)
{
    struct platform_device *pdev = apb_pdev;

    iounmap(platform_dev->sche_base);
    platform_dev->sche_base = NULL;

    devm_kfree(&pdev->dev, platform_dev);
    platform_dev = NULL;

    return 0;
}

#ifdef CFG_SOC_FEATURE_SR
int agentdrv_platform_suspend(struct platform_device *pdev, pm_message_t state)
{
    struct agentdrv_devctrl *agent_dev;

    devdrv_info("Platform driver suspend start. (event=%d)\n", state.event);
    agent_dev = agentdrv_get_dev_by_apb_dev(pdev);
    if (agent_dev == NULL) {
        devdrv_info("agent_dev is NULL.\n");
        return 0;
    }
    agentdrv_platform_unbind_irq(agent_dev);
    devdrv_info("Platform driver suspend success.\n");
    return 0;
}

int agentdrv_platform_resume(struct platform_device *pdev)
{
    return 0;
}

void agentdrv_mask_suspend_fault(void __iomem *io_base)
{
    writel(0XFFFFFFFFU, io_base + AP_IOB_TX_INT_MSK2_REG);
    writel(0XFFFFFFFFU, io_base + AP_IOB_TX_INT_MSK3_REG);
    writel(0X1FF, io_base + AP_IOB_RX_INT_MSK_REG);
    writel(0XF, io_base + AP_ABNORMAL_INT_MSK_REG);
    writel(0xFF, io_base + AP_INT_ECC_ERR_MSK_REG);
    writel(0XFFFFFFFFU, io_base + CORE_TL_INT_MSK0_REG);
    writel(0XFFFFFFFFU, io_base + CORE_MAC_INT_MSK_REG);
    writel(0x7FFFF, io_base + CORE_DL_INT_MSK_REG);

    return;
}
#endif
