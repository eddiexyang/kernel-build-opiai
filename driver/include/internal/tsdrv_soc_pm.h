/* *
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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

#ifndef TSDRV_SOC_PM_H
#define TSDRV_SOC_PM_H

#include <linux/types.h>
#include <linux/device.h>

void tsdrv_soc_startup(u32 devid, u64 boot_addr, u64 ts_subsysctl, u32 env_type);
int tsdrv_soc_config_gicr(u64 addr);

int tsdrv_soc_reset_tscpu(void __iomem *sysctrl, uint32_t tsid);
int tsdrv_soc_dereset_tscpu(void __iomem *sysctrl);

void tsdrv_enable_ts_disp(void __iomem *disp_base);
void tsdrv_soc_reset_prot_disable(void __iomem *ts_sysctl);
#endif
