/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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

#ifndef LPM_FAULT_FIRMWARE_H
#define LPM_FAULT_FIRMWARE_H

#ifdef LPM_FAULT_STARTUP
int32_t lpm_fault_startup_init(uint32_t dev_num);
int32_t lpm_fault_startup_exit(uint32_t dev_num);

#else

static inline int32_t lpm_fault_startup_init(uint32_t dev_num)
{
	(void)dev_num;
	return 0;
}

static inline int32_t lpm_fault_startup_exit(uint32_t dev_num)
{
	(void)dev_num;
	return 0;
}
#endif

#endif