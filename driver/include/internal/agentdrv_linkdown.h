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

#ifndef _AGENTDRV_LINKDOWN_H_
#define _AGENTDRV_LINKDOWN_H_

#include <linux/module.h>
#include <linux/msi.h>
#include <linux/netdevice.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/pci.h>
#include <linux/suspend.h>

#include "agentdrv_platform.h"

#define AGENTDRV_AP_DMA_REG 0xc000
#define AGENTDRV_AP_IOB_TX_REG 0x00
#define AGENTDRV_AP_IOB_RX_ODR_REG 0x6400
#define AGENTDRV_AP_INT_REG 0x10000

/* MAC_REG */
#define AGENTDRV_LINK_INT_STATUS_REG 0x54
#define AGENTDRV_LINK_INT_MASK_REG 0x58
#define AGENTDRV_MAC_LINK_INFO 0x60
#define AGENTDRV_MAC_INT_TYPE_SEL 0x3a8

#define AGENTDRV_LINKDOWN_MASK_BIT (1 << 1)
#define AGENTDRV_LINKUP_MASK_BIT (1 << 0)
#define AGENTDRV_LINKDOWN_STATUS_BIT (1 << 1)
#define AGENTDRV_LINKUP_STATUS_BIT (1 << 0)

/* CFG SPACE */
#define AGENTDRV_CFG_SPACE_PCIHDR_ID 0x0
#define AGENTDRV_CFG_SPACE_PCIHDR_MISC 0xc
#define AGENTDRV_CFG_SPACE_PCIHDR_BAR0 0x10
#define AGENTDRV_CFG_SPACE_PCIHDR_BAR2 0x18
#define AGENTDRV_CFG_SPACE_PCIHDR_BAR4 0x20
#define AGENTDRV_CFG_SPACE_DEVICE_CTRL_STATUS 0x48
#define AGENTDRV_CFG_SPACE_LINK_CAP 0x4C
#define AGENTDRV_CFG_SPACE_LINK_CTRL_STATUS 0x50
#define AGENTDRV_CFG_SPACE_DEVICE_CAPABILITY2 0x64
#define AGENTDRV_CFG_SPACE_LINK_CAPABILITY2 0x6c
#define AGENTDRV_CFG_SPACE_PME_STATUS 0xb4
#define AGENTDRV_CFG_SPACE_LINK_CTRL_STATUS2 0x70
#define AGENTDRV_CFG_SPACE_ADVACD_CAP_CTRL 0x118
#define AGENTDRV_CFG_SPACE_DEVICE_SERIAL_NUMBER_CAP_HEADER 0x4e0
#define AGENTDRV_CFG_SPACE_DEVICE_CAP 0x44
#define AGENTDRV_CFG_SPACE_DEVICE_FLR_CAP 28

/* AP_REG */
#define AGENTDRV_AP_INT_LINK_DOWN_IDLE_STATUS 0x74
#define AGENTDRV_AP_DMA_PORT_IDLE_STS 0x2150
#define AGENTDRV_AP_IOB_TX_PORT_STATUS 0x2200
#define AGENTDRV_AP_IB_ODR_PORT_IDLE_STATUS (0x1a0 + (0x400 * AGENTDRV_CORE_NUM))
/* for PHY reset */
#define AGENTDRV_SYSCTRL_BASE_ADDR 0x1100c0000
#define AGENTDRV_PCIE_PHY_RESET_REG 0x34

/* snopsys phy reg */
#define AGENTDRV_PCIE_PHY (0x80000 * ((AGENTDRV_CORE_NUM) + 1))
#define AGENTDRV_PHY_PCS_POWER_CTRL 0x840
#define AGENTDRV_PHY_PMA_POWER_CTRL 0x844
#define AGENTDRV_PHY_ANA_PWR_CTRL 0x850
#define AGENTDRV_PHY_PHY_RESET 0x800
#define AGENTDRV_PHY_REF_CLK_CTRL 0x85c
#define AGENTDRV_PHY_PCS_LANE_RESET 0x804
#define AGENTDRV_PHY_UPCS_PWR_CTRL 0x848
#define AGENTDRV_PHY_LNAEX_POWER_PRESENT 0x84c

/* sysctrl regs */
#define AGENTDRV_SYSCTRL_SC_PCIE_RESET_REQ 0xB10
#define AGENTDRV_SYSCTRL_SC_PCIE_RESET_DREQ 0xB14

#define DEVICE_ID 0xd100
#define VENDOR_ID 0x19e5
#define CLASS_CODE 0x12
#define SUB_CLASS_CODE 0x00
#define PROGRAM_INT 0x00
#define REVERSION_ID 0x20

void agentdrv_linkdown_int_mask(struct agentdrv_platform_dev *p_dev);
void agentdrv_pcie_mac_int_status_clear(struct agentdrv_platform_dev *p_dev);
int agentdrv_check_business_stop(struct agentdrv_platform_dev *p_dev);
void agentdrv_linkdown_int_unmask(struct agentdrv_platform_dev *p_dev);
void agentdrv_enable_port_reset(struct agentdrv_platform_dev *p_dev);
void agentdrv_tl_cfgspace_config(struct agentdrv_platform_dev *p_dev);
void agentdrv_enable_port_en(struct agentdrv_platform_dev *p_dev);
void agentdrv_pcie_mac_linkdown_up_init(struct agentdrv_platform_dev *p_dev);
void agentdrv_handle_link_int_sts(struct agentdrv_platform_dev *p_dev);
int agentdrv_pcie_linkdown_handle(struct agentdrv_platform_dev *p_dev);

#endif
