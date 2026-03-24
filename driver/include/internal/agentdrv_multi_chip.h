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
#ifndef _AGENTDRV_MULTI_CHIP_H_
#define _AGENTDRV_MULTI_CHIP_H_

#include "agentdrv_unit.h"

struct agentdrv_cpu_data {
    u32 start_ccpuid;
    u32 ccpu_num;
    u32 start_dcpuid;
    u32 dcpu_num;
};

#define DEVDRV_PDEV_TYPE_PLATFORM    0 /* for pcie platform probe */
#define DEVDRV_PDEV_TYPE_PF_DOORBELL 1 /* for pf's soc doorbell */
#define DEVDRV_PDEV_TYPE_VF_DOORBELL 2 /* for vf's soc doorbell */
void agentdrv_get_pdev_type_info(struct platform_device *pdev, u32 *type, u32 *irq_func_idx);

int agentdrv_parse_cpu_num(int dev_id, char buf[], u32 *cpu_num);
int agentdrv_get_ts_cpu_info(struct agentdrv_devctrl *agent_dev);
int agentdrv_get_cpu_num(int dev_id, const char *cpu_type, u32 *cpu_num);
int agentdrv_init_cpu_info(struct agentdrv_devctrl *agent_dev);

void agentdrv_get_cpu_data_info(const struct agentdrv_devctrl *agent_dev, u32 func_index,
    struct agentdrv_cpu_data *drv_cpu_info);
void agentdrv_dma_bind_irq(const struct agentdrv_devctrl *agent_dev, u32 func_id,
    struct agentdrv_cpu_data *drv_cpu_info);
void agentdrv_dma_unbind_irq(const struct agentdrv_devctrl *agent_dev, u32 func_id);
void agentdrv_pci_bind_irq(const struct agentdrv_devctrl *agent_dev, u32 func_id,
    struct agentdrv_cpu_data *drv_cpu_info);
void agentdrv_pci_unbind_irq(const struct agentdrv_devctrl *agent_dev);
void agentdrv_platform_bind_irq(const struct agentdrv_devctrl *agent_dev, u32 func_id, u32 start_dcpuid, u32 dcpu_num);
void agentdrv_platform_unbind_irq(const struct agentdrv_devctrl *agent_dev);
void agentdrv_bind_irq(struct agentdrv_devctrl *agent_dev);
void agentdrv_init_multi_chip(void);

#endif
