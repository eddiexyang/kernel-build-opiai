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

#ifndef LPM_DEVMNG_TEMPERATURE_DRV_H
#define LPM_DEVMNG_TEMPERATURE_DRV_H

#include <linux/types.h>
#include "lpm_devmng_common.h"

#ifdef LPM_TEMPERATURE

int32_t lpm_temperature_drv_probe(uint64_t *param, uint32_t param_num);
int32_t lpm_temperature_drv_remove(uint64_t *param, uint32_t param_num);

int32_t lpm_temperature_drv_get_soc_temp(
	uint32_t dev_id, enum lpm_devmng_core_id core_id, uint32_t *temperature);

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
int32_t lpm_temperature_drv_debugfs_show(struct seq_file *seq, void *v);
#endif

#endif
#endif
