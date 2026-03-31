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

#define AGENTDRV_APB_REGION_BASE_INDEX 0
#define AGENTDRV_ATU_REGION1_BASE_INDEX 2
#define AGENTDRV_ATU_REGION2_BASE_INDEX 1

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

#define DEVDRV_SMMU_BYPASS_REG 0xa2813000  /* mini v2 smmu bypass reg addr */

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
struct agentdrv_platform_dev *agentdrv_platform_init(struct platform_device *apb_pdev, int agent_id,
                                                     void __iomem *apb_base);
int agentdrv_platform_uninit(struct agentdrv_platform_dev *platform_dev, struct platform_device *apb_pdev);
#endif
