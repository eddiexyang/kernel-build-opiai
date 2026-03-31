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
#ifndef LPM_FAULT_SAFETY_H
#define LPM_FAULT_SAFETY_H

#include "lpm_fault_common.h"

#ifdef LPM_FAULT_SAFETY
int32_t lpm_fault_safety_init(uint32_t dev_num);
int32_t lpm_fault_safety_exit(uint32_t dev_num);
void lpm_safety_suspend_prepare(void);
void lpm_safety_post_suspend(void);
#else
static inline int32_t lpm_fault_safety_init(uint32_t dev_num)
{
	lpm_log_info("no need do safety init\n");
	return 0;
}
static inline int32_t lpm_fault_safety_exit(uint32_t dev_num)
{
	lpm_log_info("no need do safety exit\n");
	return 0;
}
static inline void lpm_safety_suspend_prepare(void)
{
	return;
}
static inline void lpm_safety_post_suspend(void)
{
	return;
}
#endif
#endif /* LPM_FAULT_SAFETY_H */