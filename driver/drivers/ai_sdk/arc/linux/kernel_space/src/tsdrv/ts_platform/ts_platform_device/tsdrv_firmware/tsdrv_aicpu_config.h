/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#ifndef DEVDRV_AICPU_CONFIG_H
#define DEVDRV_AICPU_CONFIG_H

#include <linux/init.h>
#include "dbl/chip_config.h"
#include "aicpu.h"

#define CHIP_BASE_PA_DDR 0x200000000000ULL

#define TS_MEMTYPE_MAX 2

int devdrv_construct_aicpu_dt(u64 vaddr, u32 dev_id, u32 tsid);
void devdrv_config_ts_dma_chan_info(struct devdrv_info *dev_info, u64 vaddr);
int devdrv_config_ts_valid_nid_type(u32 nid_type[][TS_MEMTYPE_MAX], u32 *nid_type_num);
typedef int (*get_tscpu_irq_info)(u32 dev_id, u32 *start_irq, u32 *irq_num);

#endif
