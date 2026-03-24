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

#define LINKDOWN_REG_VAL_SELECT (1 << 16)
#define OUTBOUND_MODULE_REG_VALUE (1 << 8)
#define CFGSPACE_CFG_REG_VALUE (1 << 23)
#define CFGSPACE_AUX_REG_VALUE (1 << 20)
#define CFGSPACE_CLOSE_REG_VALUE (1 << 16)
#define REG_OFFSET_ONE 1
#define REG_OFFSET_TWO 2
#define REG_OFFSET_THREE 3
#define REG_OFFSET_FOUR 4
#define REG_OFFSET_FIVE 5
#define REG_OFFSET_SEVEN 7
#define REG_OFFSET_EIGHT 8
#define REG_OFFSET_TEN 10
#define REG_OFFSET_ELEVEN 11
#define REG_OFFSET_TWELVE 12
#define REG_OFFSET_THIRTEEN 13
#define REG_OFFSET_SIXTEEN 16
#define REG_OFFSET_TWENTYSIX 26
#define REG_OFFSET_THIRTY 30

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
#define AGENTDRV_DL_FEATURE_CAP_REG04 0x704
#define AGENTDRV_MSI_DATA 0x8C
#define AGENTDRV_UNCR_ERR_MASK 0x108
#define AGENTDRV_UNCR_ERR_SEVERITY 0x10C
#define AGENTDRV_COR_ERR_MASK 0x114
#define AGENTDRV_ADVACD_CAP_CTRL 0x118
#define AGENTDRV_MSIX_CAP_HEADER 0xA0
#define AGENTDRV_CFG_SPACE_DEVICE_SERIAL_NUMBER_CAP_HEADER 0x4e0
#define AGENTDRV_PF0_BAR0_MASK 0x300
#define AGENTDRV_PF1_BAR0_MASK 0x304
#define AGENTDRV_PF2_BAR0_MASK 0x308
#define AGENTDRV_PCIHDR_INT 0x3C
#define AGENTDRV_SRIOV_CAP 0x204
#define AGENTDRV_CFG_SPACE_DEVICE_CAP 0x44
#define AGENTDRV_CFG_SPACE_DEVICE_FLR_CAP 28

/* AP_REG */
#define AGENTDRV_AP_INT_LINK_DOWN_IDLE_STATUS 0x74
#define AGENTDRV_ERR_RESPONSE 0x114
#define AGENTDRV_AP_DMA_PORT_IDLE_STS 0x2150
#define AGENTDRV_AP_IOB_TX_PORT_STATUS 0x2200
#define AGENTDRV_AP_IB_ODR_PORT_IDLE_STATUS (0x1a0 + (0x400 * AGENTDRV_CORE_NUM))
#define AGENTDRV_IOB_TX_SLV_PORT_STATUS 0x220c
#define DEVICE_ID 0xD801
#define VENDOR_ID 0x19e5

/* nvme */
#define AGENTDRV_NVME_GLOBAL_REG 0x102000
#define AGENTDRV_NVME_PORT_IDLE 0xCF48

/* virtio */
#define AGENTDRV_VQ_NOTIFY_INI 0x150
#define AGENTDRV_LINK_CAPBILITY 0x4C
#define AGENTDRV_LINK_CTRL_STATUS2 0x70
#define AGENTDRV_PFn_BARn_ENABLE 0x478
#define AGENTDRV_PCIHDR_CLSREV 0x8
#define AGENTDRV_ENGN_FLR_REQ 0x154

/* pcie_icl_subctrl */
#define AGENTDRV_PCIE_ICL_SUBCTRL 0x0
#define AGENTDRV_SC_PCIE_TL_31_0_ICG_DIS 0x3CC
#define AGENTDRV_SC_PCIE_TL_39_32_ICG_DIS 0x3D4
#define AGENTDRV_SC_PCIE_TL_31_0_ICG_EN 0x3C8
#define AGENTDRV_SC_PCIE_TL_39_32_ICG_EN 0x3D0

#define AGENTDRV_LINK_STATUS_VAL 1
void agentdrv_linkdown_int_mask(struct agentdrv_platform_dev *p_dev);
void agentdrv_pcie_mac_int_status_clear(struct agentdrv_platform_dev *p_dev);
int agentdrv_check_business_stop(struct agentdrv_platform_dev *p_dev, struct agentdrv_msg_dev *p_agentdrv_msg_dev);
void agentdrv_linkdown_int_unmask(struct agentdrv_platform_dev *p_dev);
void agentdrv_tl_cfgspace_config(struct agentdrv_platform_dev *p_dev);
void agentdrv_enable_port_en(struct agentdrv_platform_dev *p_dev);
void agentdrv_pcie_mac_linkdown_up_init(struct agentdrv_platform_dev *p_dev);
void agentdrv_handle_link_int_sts(struct agentdrv_platform_dev *p_dev);
int agentdrv_pcie_linkdown_handle(struct agentdrv_platform_dev *p_dev);

#endif
