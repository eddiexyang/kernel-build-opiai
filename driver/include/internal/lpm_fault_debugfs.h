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
#ifndef LPM_FAULT_DEBUGFS_H
#define LPM_FAULT_DEBUGFS_H

#include <linux/types.h>
#include "lpm_debugfs_base.h"

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
int32_t lpm_fault_debugfs_init(uint32_t dev_num);
int32_t lpm_fault_debugfs_exit(uint32_t dev_num);

int32_t lpm_debugfs_register_module(struct lpm_debugfs_init_cfg *init_cfg);
#else
static inline int32_t lpm_fault_debugfs_init(uint32_t dev_num)
{
	(void)dev_num;
	return 0;
}
static inline int32_t lpm_fault_debugfs_exit(uint32_t dev_num)
{
	(void)dev_num;
	return 0;
}
#endif


#endif