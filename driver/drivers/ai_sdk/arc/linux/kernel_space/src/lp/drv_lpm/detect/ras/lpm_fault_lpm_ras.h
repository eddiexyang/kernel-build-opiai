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
#ifndef LPM_FAULT_LPM_RAS_H
#define LPM_FAULT_LPM_RAS_H

#ifdef LPM_FAULT_RAS_PLL

#include "drv_ras_common.h"

#define PLL_NUM_MAX      (uint8_t)11U

typedef void (*lpm_fault_common_ras_proc_func)(const hisi_common_error_info *hisi_error);

struct common_ras_node {
	uint8_t module_id;
	lpm_fault_common_ras_proc_func handler;
};

int32_t lpm_fault_ras_lpm_init(void);
int32_t lpm_fault_ras_lpm_uninit(void);

#else

static inline int32_t lpm_fault_ras_lpm_init(void)
{
	return 0;
}
static inline int32_t lpm_fault_ras_lpm_uninit(void)
{
	return 0;
}
#endif

#endif
