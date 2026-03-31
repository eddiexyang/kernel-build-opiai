/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
#ifndef LPM_DEVMNG_FREQUENCY_DRV_H
#define LPM_DEVMNG_FREQUENCY_DRV_H

#include <linux/types.h>

#ifdef LPM_FREQUENCY

#ifdef LPM_TURBO
int32_t lpm_frequency_get_aic_freq_range(uint32_t dev_id, uint32_t *min_freq, uint32_t *max_freq);
#endif

int32_t lpm_frequency_drv_query(uint32_t dev_id, uint32_t core_id, uint32_t *frequency);

int32_t lpm_frequency_drv_probe(uint64_t *param, uint32_t param_num);
int32_t lpm_frequency_drv_remove(uint64_t *param, uint32_t param_num);

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
int32_t lpm_frequency_drv_debugfs_show(struct seq_file *seq, void *v);
#endif

#endif

#endif
