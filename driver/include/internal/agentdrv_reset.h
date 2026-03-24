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

/* TL_REG */
#define AGENTDRV_TL_CFG_SPACE_CTRL 0x4d4
#define AGENTDRV_PF_FLR_INT_MASK 0x0540
#define AGENTDRV_TL_PF_FLR_INT_STATUS 0x544
#define AGENTDRV_TL_PF_FLR_EN 0x0548
#define AGENTDRV_TL_FLR_INT_SEL 0x058c

/* MAC_REG */
#define AGRNTDRV_MAC_REG_MAC_INT_STATUS 0x54

/* MAC_REG HOT_RESET_INT_STATUS */
#define AGRNTDRV_REG_HOT_RESET_INT_STATUS_BIT (1 << 6)

/* TL_REG PF_FLR_INT_STATUS */
#define AGENTDRV_TL_PF_FLR_INT_STATUS_BIT 1
#define AGENTDRV_TL_CFGCPL_CRS_BIT (1 << 16)

/* SYSCTRL REG */
#define AGENTDRV_SYSCTRL_CFG_REG 0x1100c0000
#define AGENTDRV_SYSCTRL_CFG_SIZE 0x10000
#define AGENTDRV_SC_BAK_DATA15 0x344c
#define AGENTDRV_SOFTRST_FLAG 0x1001

/* PERI SUBCTRL REG */
#define AGENTDRV_SC_M3_CTRL0 0x20B0
#define AGENTDRV_SC_M3_CTRL0_NMI_INT 0x46000000

void agentdrv_clear_pf_flr_int_status(struct agentdrv_platform_dev *p_dev);
void agentdrv_tl_cfgcpl_crs_en(struct agentdrv_platform_dev *p_dev);
void agentdrv_tl_flr_int_sel(struct agentdrv_platform_dev *p_dev);
void agentdrv_pf_flr_int_mask(struct agentdrv_platform_dev *p_dev);
void agentdrv_pf_flr_int_unmask(struct agentdrv_platform_dev *p_dev);
void agentdrv_handle_flr_int(struct agentdrv_platform_dev *p_dev);
int agentdrv_hot_reset_int_status_check(struct agentdrv_platform_dev *p_dev);

#endif
