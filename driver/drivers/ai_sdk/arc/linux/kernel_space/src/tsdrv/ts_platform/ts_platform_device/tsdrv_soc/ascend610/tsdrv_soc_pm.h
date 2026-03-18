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

#ifdef CFG_SOC_PLATFORM_MINIV3

#else
void tsdrv_soc_startup(u32 tsid, u32 devid, u64 boot_addr);
#endif
int tsdrv_soc_reset_tscpu(void __iomem *sysctrl, uint32_t tsid);

#endif
