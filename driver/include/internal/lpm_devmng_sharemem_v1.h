/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
*/

#ifndef LPM_DEVMNG_SHAREMEM_V1_H
#define LPM_DEVMNG_SHAREMEM_V1_H

#include <linux/types.h>
#include <linux/spinlock.h>
#include "lpm_devmng_common.h"
#include "lpm_devmng_sharemem_drv.h"

#if defined(LPM_SHAREMEM_V1)

// for CFG_SOC_PLATFORM_MINIV2 or CFG_SOC_PLATFORM_HELPER
// LP Aicore frequency adjust Reason
#define LPM_SHAREMEM_AIC_FREQ_ADJ_OFFSET 0x20040ULL
#define LPM_SHAREMEM_AIC_FREQ_ADJ_SIZE   64ULL
#define LPM_SHAREMEM_AIC_FREQ_ADJ_MAGIC  0x4C504152 // 'LPAR'

int32_t lpm_read_aic_freq_adj_from_sharemem(uint32_t dev_id, struct lpm_aic_freq_adj_info *adj_info);

#endif
#endif