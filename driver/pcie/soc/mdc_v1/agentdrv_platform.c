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

void agentdrv_get_borad_type(struct agentdrv_platform_dev *p_dev)
{
    u32 board_type = 0;
    u32 version = 0;

    (void)devdrv_get_platform_type(&board_type, &version);

    devdrv_info("Got platform type. (board_type=%u; version=%x)\n", board_type, version);

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
        devdrv_err("Alloc platform device memory space failed. (dev_id=%d)\n", agent_id);
        agentdrv_notify_blackbox_exception(agent_id, AGENTDRV_PM_PROBE_FAIL, "alloc pm dev mem space fail!\n");
        return NULL;
    }
    platform_dev->dev = &pdev->dev;
    platform_dev->hot_reset_flag_addr = NULL;
    platform_dev->io_base = apb_base;
    platform_dev->agent_id = agent_id;

    agentdev_set_smmu_bypass();

    agentdrv_get_borad_type(platform_dev);

    devdrv_info("Call agentdrv_platform_probe success. (dev_id=%d)\n", agent_id);

    return platform_dev;
}

int agentdrv_platform_uninit(struct agentdrv_platform_dev *platform_dev, struct platform_device *apb_pdev)
{
    struct platform_device *pdev = apb_pdev;

    devm_kfree(&pdev->dev, platform_dev);
    platform_dev = NULL;

    return 0;
}
