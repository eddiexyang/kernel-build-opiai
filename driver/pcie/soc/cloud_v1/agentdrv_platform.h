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

#ifndef _AGENTDRV_PLATFORM_H_
#define _AGENTDRV_PLATFORM_H_

#include <linux/platform_device.h>
#include "devdrv_dfm.h"
#include "agentdrv_unit.h"

#define AGENTDRV_CHIP_BASE 0x200000000000

#define PLATFORM_OFFSET_THREE 3
#define PLATFORM_OFFSET_FIVE 5
#define PLATFORM_OFFSET_SEVEN 7
#define PLATFORM_OFFSET_EIGHT 8
#define PLATFORM_OFFSET_TEN 10
#define PLATFORM_OFFSET_ELEVEN 11
#define PLATFORM_OFFSET_THIRTEN 13
#define PLATFORM_OFFSET_SEVENTEN 17
#define PLATFORM_OFFSET_TWENTY 20

#define AGENTDRV_PF_SEL_NUM 0x100

#define AGENTDRV_REG_BASE_REG 0x201080000
#define AGENTDRV_REG_BASE_SIZE 0x1000
#define AGENTDRV_SYSCTRL_CFG_REG 0x80000000
#define AGENTDRV_SYSCTRL_CFG_SIZE 0x10000
#define AGENTDRV_SUBCTRL_CFG_REG 0x80d00000
#define AGENTDRV_SUBCTRL_CFG_SIZE 0x4000

#define AGENTDRV_AP_MSG_REG_BASE 0x8000
#define AGENTDRV_PCIE_ERR_MAPPING 0x0
#define AGENTDRV_PCIE_NI_ENA 0x38
#define AGENTDRV_PCIE_CE_ENA 0x8
#define AGENTDRV_PCIE_UNF_ENA 0x10
#define AGENTDRV_PCIE_UF_ENA 0x18
/* CORE GLOBAL */
#define AGENTDRV_CORE_INT_NI_MSK_0 0xE4
#define AGENTDRV_CORE_INT_NI_MSK_1 0xF4
#define AGENTDRV_CORE_INT_CE_MSK_0 0x104
#define AGENTDRV_CORE_INT_CE_MSK_1 0x114
#define AGENTDRV_CORE_INT_NFE_MSK_0 0x124
#define AGENTDRV_CORE_INT_NFE_MSK_1 0x134
#define AGENTDRV_CORE_INT_FE_MSK_0 0x144
#define AGENTDRV_CORE_INT_FE_MSK_1 0x154
/* pcie GLOBAL */
#define AGENTDRV_APB_TIMEOUT_INT_MASK 0x4
/* ap_iob_tx_reg */
#define AGENTDRV_IOB_TX_INT_MASK1 0x810
#define AGENTDRV_IOB_TX_INT_MASK2 0x814
#define AGENTDRV_IOB_TX_INT_MASK3 0x818
#define AGENTDRV_IOB_TX_INT_MASK4 0x81c
#define AGENTDRV_IOB_TX_MISC_CTRL 0x8E0
#define AGENTDRV_IOB_TX_CHI_CFG_MISC 0x200C
#define AGENTDRV_IOB_TX_CHI_MAX_ENTRIES_CFG 0x2010
#define AGENTDRV_IOB_TX_POISON_CTRL 0x1800
#define AGENTDRV_IOB_TX_CHI_CFG_ORDERING 0x2000
#define AGENTDRV_IOB_TX_CFG_RESET 0x904
/* AGENTDRV_AP_IOB_RX_COM_REG */
#define AGENTDRV_AP_IOB_RX_COM_REG 0x4000
#define AGENTDRV_APAT_INT_MASK 0x10F4
/* AP_SDI_AXIM_REG */
#define AGENTDRV_SDI_AXIM_INT_MASK 0x84
#define AGENTDRV_SDI_ODR_INT_MASK 0x7E4
/* AGENTDRV_AP_INT_MASK */
#define AGENTDRV_AP_INT_REG (0x10000 + 0 * 0x200)
#define AGENTDRV_ECC_ERR_MASK 0x7C

#define AGENTDRV_HILINK_INT_MASK 0xc8
#define AGENTDRV_TL_INT_MASK0 0x574
#define AGENTDRV_TL_INT_MASK1 0x580
#define AGENTDRV_DL_INT_MASK 0x8c
#define AGENTDRV_MAC_REG_MAC_INT_MASK 0X58
#define AGENTDRV_PCS_INTR_MSK 0xb0
/* STATUS REG */
#define AGENTDRV_TL_INT_STATUS0 0x56c
#define AGENTDRV_TL_INT_STATUS1 0x570
#define AGENTDRV_DL_INT_STATUS 0x88
#define AGENTDRV_MAC_INT_STATUS 0x54
#define AGENTDRV_PCS_INTR_STATUS 0xa8
#define AGENTDRV_PCS_INT 0x1C0000 + 1 * 0x10000
#define AGENTDRV_HILINK_INT_STATUS 0xd0
#define AGENTDRV_CORE_INT_NI_STATUS_0 0xe8
#define AGENTDRV_CORE_INT_NI_STATUS_1 0xf8
#define AGENTDRV_CORE_INT_CE_STATUS_0 0x108
#define AGENTDRV_CORE_INT_CE_STATUS_1 0x118
#define AGENTDRV_CORE_INT_NFE_STATUS_0 0x128
#define AGENTDRV_CORE_INT_NFE_STATUS_1 0x138
#define AGENTDRV_PCIE_NI_STATUS 0x3C
#define AGENTDRV_PCIE_CE_STATUS 0x20
#define AGENTDRV_PCIE_UNF_STATUS 0x28
#define AGENTDRV_PCIE_UF_STATUS 0x30
/* VIRTIO LIST */
#define AGENTDRV_VIRTIO_QUEUE_REG 0xE0000
#define AGENTDRV_VIRTIO_GLOBAL_REG 0xC0000
#define AGENTDRV_ENGN_INT_MSK 0x188
#define AGENTDRV_ECC_INT_MASK 0x708
#define AGENTDRV_CORE_INT_NI_RO_0 0xec
#define AGENTDRV_TL_CFG_ACCESS_CTRL 0x4fc
#define AGENTDRV_APB_REGION_BASE_INDEX 0
#define AGENTDRV_ATU_REGION1_BASE_INDEX 2
#define AGENTDRV_ATU_REGION2_BASE_INDEX 1

#define AGENTDRV_INT_SPI0 0
#define AGENTDRV_INT_SPI1 1

#define AGENTDRV_DMA_PROBE_FAIL 0xA8021000
#define AGENTDRV_SDIO_PROBE_FAIL 0xA8021001
#define AGENTDRV_PM_PROBE_FAIL 0xA8021002
#define AGENTDRV_LINKDOWN_FAIL 0xA8021003
#define AGENTDRV_DMA_FAIL 0xA8021004

#define AGENTDRV_PCIE_CFG_SPACE 0x105000
#define AGENTDRV_PCIE_GLOBAL 0x20000
#define AGENTDRV_PCIE_CORE_GLOBAL (0x80000 * ((AGENTDRV_CORE_NUM) + 1))
#define AGENTDRV_PCIE_MAC (0x80000 * (AGENTDRV_CORE_NUM + 1) + 0x7000 + (0x4000 * AGENTDRV_PORT_NUM * 2))
#define AGENTDRV_PCIE_TL_REG (0x80000 * ((AGENTDRV_CORE_NUM) + 1) + 0x4000 + (0x4000 * (AGENTDRV_PORT_NUM)*2))
#define AGENTDRV_PCIE_DL_REG (0x80000 * ((AGENTDRV_CORE_NUM) + 1) + 0x6000 + (0x4000 * (AGENTDRV_PORT_NUM)*2))

/* CORE_GLOBAL_REG */
#define AGENTDRV_PCIE_CORE_PORT_EN 0x4
#define AGENTDRV_PCIE_CORE_PHY_RESET 0x8
#define AGENTDRV_PCIE_CORE_PORT_RESET_CFG 0xC
#define AGENTDRV_PORT_RESET 0x0
/* TL_REG */
#define AGENTDRV_TL_RX_POSTED_CREDIT 0xB1C
#define AGENTDRV_TL_RX_NON_POSTED_CREDIT 0xB20
#define AGENTDRV_TL_RX_CPL_CREDIT 0xB24
#define AGENTDRV_TL_RX_CDT_INI_UP 0xB28
#define AGENTDRV_TL_RX_POSTED_CREDIT_DF 0xB88
#define AGENTDRV_TL_RX_NON_POSTED_CREDIT_DF 0xB8C
#define AGENTDRV_TL_RX_CPL_CREDIT_DF 0xB90
#define AGENTDRV_TL_RX_CDT_INI_UP_DF 0xB94
#define AGENTDRV_TL_BASE_LIMT_CHECK_EN 0xC
#define AGENTDRV_ECO_TL 0xFFC

struct agentdrv_platform_dev {
    int agent_id;
    struct device *dev;
    void __iomem *io_base;
    int irq_spi0;
    int irq_spi1;
    int __iomem *hot_reset_flag_addr;
    u32 board_type; /* 0-FPGA 1-EMU 2-ESL 3-ASIC */
    u32 pm_enable;
    u32 aspm_enable;
    void __iomem *sysctrl_base;
    void __iomem *subctrl_base;
    void __iomem *reg_base;
    struct work_struct reset;
};

int agentdrv_check_flr_reset_finish(u32 dev_id);

void agentdrv_notify_blackbox_exception(u32 dev_id, u32 excep_id, const char *str);
void agentdrv_pcie_set_pf_num(struct agentdrv_platform_dev *p_dev, u32 val);
extern struct agentdrv_msg_dev *agentdrv_get_msgdev(int dev_id);

struct agentdrv_platform_dev *agentdrv_platform_init(struct platform_device *apb_pdev, int agent_id,
                                                     void __iomem *apb_base);
int agentdrv_platform_uninit(struct agentdrv_platform_dev *platform_dev, struct platform_device *apb_pdev);
#endif
