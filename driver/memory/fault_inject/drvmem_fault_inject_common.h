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
*
* Description: fault inject common functions
* Author: Huawei Technologies Co.Ltd
* Create: 2022-11-22
*/

#ifndef DRVMEM_FAULT_INJECT_COMMON_H
#define DRVMEM_FAULT_INJECT_COMMON_H

#include <linux/types.h>

#ifndef STATIC_SKIP
#define STATIC static
#else
#define STATIC
#endif

struct fault_inject_process {
	uint64_t addr;
	uint32_t val;
};

int32_t drvmem_write_reg(uint64_t addr, uint32_t val);
int32_t drvmem_read_reg(uint64_t addr, uint32_t *val);
int32_t drvmem_fault_inject_write_reg(uint64_t base, size_t proc_length, struct fault_inject_process *proc);

#endif
