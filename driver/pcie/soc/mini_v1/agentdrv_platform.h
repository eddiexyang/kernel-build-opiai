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
#include <linux/version.h>

#include "devdrv_dfm.h"
#include "agentdrv_unit.h"

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
#define AGENTDRV_PCIE_MAC (0x80000 * (AGENTDRV_CORE_NUM + 1) + 0x7000 + (0x4000 * AGENTDRV_PORT_NUM))
#define AGENTDRV_PCIE_TL_REG (0x80000 * ((AGENTDRV_CORE_NUM) + 1) + 0x4000 + (0x4000 * (AGENTDRV_PORT_NUM)))
#define AGENTDRV_PCIE_DL_REG (0x80000 * ((AGENTDRV_CORE_NUM) + 1) + 0x6000 + (0x4000 * (AGENTDRV_PORT_NUM)))

/* PCIE_GLOBAL_REG */
#define AGENTDRV_PCIE_SPI0_INT_MASK 0x4
#define AGENTDRV_PCIE_SPI0_INT_STATUS 0xc
#define AGENTDRV_PCIE_SPI1_INT_MASK 0x050
#define AGENTDRV_PCIE_SPI1_INT_STATUS 0x058
#define AGENTDRV_PCIE_INT_SPI0_EN 0x05c
#define AGENTDRV_CORE1_ABNOR_INT_BIT (1 << 5)
#define AGENTDRV_CORE1_BUSI_INT_BIT (1 << 9)

/* CORE_GLOBAL_REG */
#define AGENTDRV_PCIE_CORE_PORT_EN 0x4
#define AGENTDRV_PCIE_CORE_PHY_RESET 0x8
#define AGENTDRV_PCIE_CORE_PORT_RESET_CFG 0xC
#define AGENTDRV_CORE_INT_MASK_A 0x0098
#define AGENTDRV_CORE_INT_MASK_B 0xD0

/* DL_REG */
#define AGENTDRV_DL_INT_STATUS 0x88
#define AGENTDRV_DL_INT_MASK 0x8c

/* PCIE mask for spi0 enable at outside of pcie global */
#define AGENTDRV_PERI_SUBCTRL_REG 0x10c000000
#define AGENTDRV_WHOLE_GLOBAL_SPI0_MASK 0x3804
#define AGENTDRV_WHOLE_GLOBAL_SPI0_BIT (1 << 2)

#define DEVDRV_SMMU_BYPASS_REG 0x110213000

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
struct wake_lock {
        struct wakeup_source *ws;
};
#else
struct wake_lock {
        struct wakeup_source ws;
};
#endif

struct agentdrv_platform_dev {
    struct device *dev;
    void __iomem *io_base;
    int irq_spi0;
    int irq_spi1;
    int __iomem *hot_reset_flag_addr;
    u32 board_type; /* 0-FPGA 1-EMU 2-ESL 3-ASIC */
    u32 pm_enable;
    u32 aspm_enable;
#ifdef CFG_FEATURE_PM
    struct work_struct pm_gosleep_work;
    struct wake_lock pm_wakelock;
#endif
};

int agentdrv_check_flr_reset_finish(u32 dev_id);

void agentdrv_notify_blackbox_exception(u32 dev_id, u32 excep_id, const char *str);
int agentdrv_platform_suspend(struct platform_device *pdev, pm_message_t state);
int agentdrv_platform_resume(struct platform_device *pdev);
struct agentdrv_platform_dev *agentdrv_platform_init(struct platform_device *apb_pdev, int agent_id,
                                                     void __iomem *apb_base);
int agentdrv_platform_uninit(struct agentdrv_platform_dev *platform_dev, struct platform_device *apb_pdev);
void agentdrv_nvme_doorbell_lock_init(void);
#endif
