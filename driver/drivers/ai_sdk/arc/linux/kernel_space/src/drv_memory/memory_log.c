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
 *
 * Description: drv memory log
 * Author: Huawei Technologies Co.Ltd
 * Create: 2023-10-20
 */
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include "memory_log.h"

#ifdef DRV_MEMORY_GTEST
int32_t drv_memory_vprintk_emit(int32_t level, const char *fmt, ...)
{
	return 0;
}
#else
int32_t drv_memory_vprintk_emit(int32_t level, const char *fmt, ...)
{
	va_list args;
	int32_t r;

	va_start(args, fmt);
	r = vprintk_emit(MEMORY_VPRINTK_FACILITY, level, NULL, fmt, args);
	va_end(args);

	return r;
}
#endif
