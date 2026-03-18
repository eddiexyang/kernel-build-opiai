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
#ifndef _AGENTDRV_RESET_H_
#define _AGENTDRV_RESET_H_

#include <linux/module.h>
#include <linux/msi.h>
#include <linux/netdevice.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/pci.h>
#include <linux/suspend.h>

#include "agentdrv_platform.h"

/* MAC_REG */
#define AGRNTDRV_MAC_REG_MAC_INT_STATUS 0x54
#define AGENTDRV_HOT_RESET_STATUS 1
/* MAC_REG HOT_RESET_INT_STATUS */
#define AGRNTDRV_REG_HOT_RESET_INT_STATUS_BIT (1 << 6)

int agentdrv_hot_reset_int_status_check(struct agentdrv_platform_dev *p_dev);
extern int drv_cpld_rst_ctrl_all(void);

#endif
