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

int agentdrv_hot_reset_int_status_check(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *status_reg = NULL;
    u32 reg_val;

    status_reg = p_dev->io_base + AGENTDRV_PCIE_MAC + AGRNTDRV_MAC_REG_MAC_INT_STATUS;
    reg_val = readl(status_reg);
    if (reg_val & AGRNTDRV_REG_HOT_RESET_INT_STATUS_BIT)
        return AGENTDRV_HOT_RESET_STATUS;

    return 0;
}
