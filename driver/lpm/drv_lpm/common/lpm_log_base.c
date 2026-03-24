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

#include <linux/kernel.h>
#include <linux/printk.h>
#include "lpm_log_base.h"

#define LPM_VPRINTK_FACILITY ((int32_t)90) // facility = kernel15

int32_t lpm_log_vprintk_emit(int32_t level, const char *fmt, ...)
{
	va_list args;
	int32_t r;

	va_start(args, fmt);
	r = vprintk_emit(LPM_VPRINTK_FACILITY, level, NULL, fmt, args);
	va_end(args);

	return r;
}
