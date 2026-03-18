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

#ifndef _AGENTDRV_PM_H_
#define _AGENTDRV_PM_H_

#include <linux/module.h>
#include <linux/msi.h>
#include <linux/netdevice.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/pci.h>
#include <linux/suspend.h>

#include "agentdrv_pci.h"
#include "agentdrv_platform.h"

/* TL_REG */
#define AGENTDRV_TL_ASPM_IDLE_EN 0x104
#define AGENTDRV_TL_PM_DC_CTRL 0x108
#define AGENTDRV_TL_PM_STATE 0x10c
#define AGENTDRV_TL_ENTER_L0_CTRL 0x118
#define AGENTDRV_TL_INT_STATUS0 0x56c
#define AGENTDRV_TL_INT_MASK0 0x574
#define AGENTDRV_TL_INT_STATUS1 0x570
#define AGENTDRV_TL_INT_MASK1 0x580

/* TL_REG TL_INT_STATUS1 */
#define AGENTDRV_DC_READY_L2_INT_BIT (1 << 7)
#define AGENTDRV_DC_ENTER_L2_INT_BIT (1 << 5)
#define AGENTDRV_PME_STATE_CHG_BIT (1 << 4)
#define AGENTDRV_DC_ENTER_L1_INT_BIT (1 << 1)
#define AGENTDRV_ASPM_ENTER_L1_BIT (1 << 0)

/* TL_REG TL_PM_DC_CTRL */
#define AGENTDRV_TL_DC_PCIPM_ENTER_L1_EN_BIT (1 << 16)
#define AGENTDRV_TL_DC_PCIPM_ENTER_L2_EN_BIT (1 << 24)

/* TL REG for ASPM */
#define AGENTDRV_TL_ASPM_IDLE_CNT_REG 0x100

/* DL REG for ASPM */
#define AGENTDRV_DL_TX_FC_UPDATE_P_CYCLE_G1 0x30
#define AGENTDRV_DL_TX_FC_UPDATE_NP_CYCLE_G1 0x128
#define AGENTDRV_DL_TX_FC_UPDATE_CPL_CYCLE_G1 0x138
#define AGENTDRV_DL_TX_FC_UPDATE_P_CYCLE_G2 0x34
#define AGENTDRV_DL_TX_FC_UPDATE_NP_CYCLE_G2 0x12c
#define AGENTDRV_DL_TX_FC_UPDATE_CPL_CYCLE_G2 0x13c
#define AGENTDRV_DL_TX_FC_UPDATE_P_CYCLE_G3 0x38
#define AGENTDRV_DL_TX_FC_UPDATE_NP_CYCLE_G3 0x130
#define AGENTDRV_DL_TX_FC_UPDATE_CPL_CYCLE_G3 0x140

/* CFG SPACE REG */
#define AGENTDRV_CFG_ASPM_CAP 0x4C
#define AGENTDRV_CFG_ASPM_CTRL 0x50
#define AGENTDRV_CFG_ASPM_L1_CAP_BIT (0x2 << 10)
#define AGENTDRV_CFG_ASPM_L0SL1_CAP_BIT (0x3 << 10)
#define AGENTDRV_CFG_ASPM_L1_CTRL_BIT (0x2 << 0)
#define AGENTDRV_CFG_ASPM_L0SL1_CTRL_BIT (0x3 << 0)

#define NVEID_Lp_Ap_NV 7

#define is_pm_power_state_change_int(reg_val) ((((reg_val) >> 4) & 1) == 1)
#define is_pm_enter_l2l3reay_int(reg_val) ((((reg_val) >> 5) & 1) == 1)
#define is_pm_receive_pmeturnoff_int(reg_val) ((((reg_val) >> 7) & 1) == 1)
#define is_pm_dc_enter_l1_int(reg_val) ((((reg_val) >> 1) & 1) == 1)

void agentdrv_handle_pm_int(struct agentdrv_platform_dev *p_dev);
void agentdrv_pcie_pm_unmask_tl_int(struct agentdrv_platform_dev *p_dev);
void agentdrv_aspm_set_aspm_cap(struct agentdrv_platform_dev *p_dev, int enable);
void agentdrv_aspm_set_aspm_ctrl(struct agentdrv_platform_dev *p_dev, int enable);
void agentdrv_aspm_set_l1_enter_time(struct agentdrv_platform_dev *p_dev);
void agentdrv_aspm_set_credit_update_time(struct agentdrv_platform_dev *p_dev);
void agentdrv_aspm_open_aspm(struct agentdrv_platform_dev *p_dev);
void agentdrv_aspm_close_aspm(struct agentdrv_platform_dev *p_dev);
void agentdrv_pm_init_aspm(struct agentdrv_platform_dev *p_dev);

#endif
