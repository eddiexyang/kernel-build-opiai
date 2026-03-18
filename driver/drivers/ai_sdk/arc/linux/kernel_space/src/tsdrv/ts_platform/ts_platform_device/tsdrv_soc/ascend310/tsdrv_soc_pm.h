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

void tsdrv_soc_startup(u64 ts_subsysctrl_base, u64 boot_addr);
int tsdrv_soc_reset_tscpu(void __iomem *sysctrl, u32 tsid);
int tsdrv_soc_dereset_tscpu(void __iomem *ts_sysctrl);

void tsdrv_soc_reset_prot_disable(void __iomem *ts_sysctrl);
int tsdrv_config_gicr(u64 addr);

void tsdrv_enable_ts_disp(void __iomem *disp_base);
void tsdrv_enable_disp_nfe(void __iomem *disp_base);

#endif
