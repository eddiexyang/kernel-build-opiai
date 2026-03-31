/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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

#include <linux/io.h>
#include "ascend_kernel_hal.h"
#include "pcie_qos_common.h"

#define PCIE_CORE_BITMAP 0
#define DMA_CHAN_BITMAP 1

void agentdrv_set_reg(void __iomem *reg_addr, u32 mask, u32 offset, u32 val);
u32 agentdrv_get_reg(const void __iomem *reg_addr, u32 mask, u32 offset);
u32 agentdrv_get_pcie_core_num(void);
u32 agentdrv_get_pcie_dma_chan_num(void);
void __iomem *agentdrv_get_ap_iob_rx_base(void __iomem *apb_base, u32 core_id);
void agentdrv_set_qos(void __iomem *iob_base, u32 qos);
void agentdrv_set_pmg(void __iomem *iob_base, u32 pmg);
void agentdrv_set_mpam_id(void __iomem *iob_base, u32 mpam_id);
u32 agentdrv_get_qos(void __iomem *iob_base);
u32 agentdrv_get_pmg(void __iomem *iob_base);
u32 agentdrv_get_mpam_id(void __iomem *iob_base);
void agentdrv_set_qos_config_to_dma(void __iomem *base, u32 chan_id, const struct qos_master_config_type *cfg);
void agentdrv_get_qos_config_from_dma(void __iomem *base, u32 chan_id, struct qos_master_config_type *cfg);
void agentdrv_resume_qos_config(void);

void agentdrv_set_allow_to_core(void __iomem *base, u32 core_id, const struct qos_allow_config_type *cfg);
void agentdrv_get_allow_from_core(void __iomem *base, u32 core_id, struct qos_allow_config_type *cfg);
void agentdrv_set_otsd_to_core(void __iomem *base, u32 core_id, const struct qos_otsd_config_type *cfg);
void agentdrv_get_otsd_from_core(void __iomem *base, u32 core_id, struct qos_otsd_config_type *cfg);