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
#include <linux/sched.h>

#include "devdrv_dfm.h"

#define DEVDRV_SMMU_BYPASS_REG 0x80100B020

#define PLATFORM_BOARD_TYPE 2

#define AGENTDRV_INT_SPI0 0

#define AGENTDRV_APB_REGION_BASE_INDEX 0
#define AGENTDRV_ATU_REGION1_BASE_INDEX 1
#define AGENTDRV_ATU_REGION2_BASE_INDEX 2

#define AGENTDRV_DMA_PROBE_FAIL 0xA8021000
#define AGENTDRV_SDIO_PROBE_FAIL 0xA8021001
#define AGENTDRV_PM_PROBE_FAIL 0xA8021002
#define AGENTDRV_LINKDOWN_FAIL 0xA8021003
#define AGENTDRV_DMA_FAIL 0xA8021004

#define AGENTDRV_THREAD_STATE_START 0x1
#define AGENTDRV_THREAD_STATE_END 0x0
#define AGENTDRV_THREAD_STATE_ERROR -1

/* AP_GLOBAL_REG */
#define AGENTDRV_AP_GLOBAL_REG 0x8000
#define AGENTDRV_PCIE_ERR_MAPPING 0x0
#define LOCAL_NI_TO_SPI0_MASK (0x3 << 20)
#define LOCAL_NI_TO_SPI0_VAL (0x1 << 20)
#define AGENTDRV_PCIE_NI_ENA 0x160
#define AGENTDRV_PCIE_NI_MASK 0x170
#define AGENTDRV_PCIE_CORE_NI_STATUS 0x180

/* CORE_GLOBAL_REG */
#define AGENTDRV_CORE_GLOBAL_CTRL_REG 0x5e000
#define AGENTDRV_CORE_INT_NI_MSK_0 0x534
#define AGENTDRV_CORE_INT_NI_MSK_1 0x544

/* TL_REG */
#define AGENTDRV_PCIE_TL_REG (0x20000 + 0x1000 * AGENTDRV_PORT_NUM)
#define AGENTDRV_TL_CFG_HCK_EN 0x4e8
#define TL_CFGCPL_CRS_EN (0x1 << 8)
#define AGENTDRV_TL_INT_MASK_0 0x574
#define AGENTDRV_TL_INT_MASK_1 0x580
#define AGENTDRV_TL_INT_STATUS0 0x56c
#define AGENTDRV_TL_INT_STATUS1 0x570

/* TL_CORE_REG */
#define AGENTDRV_TL_CORE_REG 0x30000
#define AGENTDRV_PF_FLR_INT_MASK 0x940
#define AGENTDRV_PF_FLR_INT_STATUS 0x944

/* TL_CORE_VF_REG */
#define AGENTDRV_TL_CORE_PF0_VF_REG (0x30000 + 0xa000)
#define AGENTDRV_VF_FLR_INT_MASK 0x0
#define AGENTDRV_VF_FLR_INT_STATUS 0x4
#define AGENTDRV_VF_FLR_EN 0x8

#define AGENTDRV_PCIE_CFG_SPACE 0x105000
#define AGENTDRV_PCIE_GLOBAL 0x05C000
#define AGENTDRV_PCIE_MAC (AGENTDRV_CORE_GLOBAL_CTRL_REG + 0x70000 + (0x1000 * AGENTDRV_PORT_NUM))
#define AGENTDRV_PCIE_DL_REG (0x60000 + 0x1000 * AGENTDRV_PORT_NUM)
#define AGENTDRV_TL_CORE_PF_REG (0x30000 + 0x1000)

#define AGENTDRV_PF_NUM_PER_AGENT_DEV 1
#define AGENTDRV_VF_NUM_PER_AGENT_DEV 12
#define AGENTDRV_PFVF_NUM_PER_AGENT_DEV (AGENTDRV_PF_NUM_PER_AGENT_DEV + AGENTDRV_VF_NUM_PER_AGENT_DEV)

enum agentdrv_flr_state {
    AGENTDRV_FLR_STATE_OK = 0x0,
    AGENTDRV_FLR_STATE_FLRING,
    AGENTDRV_FLR_STATE_THREAD_RUN_FAIL,
    AGENTDRV_FLR_STATE_STOP_BUSINESS_TIMEOUT,
    AGENTDRV_FLR_STATE_STOP_BUSINESS_ERROR,
    AGENTDRV_FLR_STATE_DMA_ERROR
};

struct agentdrv_flr_info {
    unsigned long long vf_flr_sum_cnt[AGENTDRV_VF_NUM_PER_AGENT_DEV];
    unsigned long long vf_flr_fail_cnt[AGENTDRV_VF_NUM_PER_AGENT_DEV];
};

struct agentdrv_flr {
    struct task_struct *task[AGENTDRV_VF_NUM_PER_AGENT_DEV];
    int thread_state[AGENTDRV_VF_NUM_PER_AGENT_DEV];
    enum agentdrv_flr_state flr_state[AGENTDRV_VF_NUM_PER_AGENT_DEV];
    struct work_struct flr_work;
    struct agentdrv_flr_info flr_info;
};

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
    void __iomem *sche_base;
    struct workqueue_struct *platform_workqueue;
    struct agentdrv_flr flr;
    struct work_struct reset;
};

int agentdrv_check_flr_reset_finish(u32 dev_id);

void agentdrv_notify_blackbox_exception(u32 dev_id, u32 excep_id, const char *str);
struct agentdrv_platform_dev *agentdrv_platform_init(struct platform_device *apb_pdev, int agent_id,
                                                     void __iomem *apb_base);
int agentdrv_platform_uninit(struct agentdrv_platform_dev *platform_dev, struct platform_device *apb_pdev);
#endif
