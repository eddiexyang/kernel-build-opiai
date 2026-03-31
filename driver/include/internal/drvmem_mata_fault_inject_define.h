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
* Description: mata fault inject declaration
* Author: Huawei Technologies Co.Ltd
* Create: 2023-11-1
*/

#ifndef DRVMEM_MATA_FAULT_INJECT_DEFINE_H
#define DRVMEM_MATA_FAULT_INJECT_DEFINE_H

#include <linux/types.h>

int32_t drvmem_mata_ce_inject(uint64_t base);
int32_t drvmem_mata_ue_inject(uint64_t base);
int32_t drvmem_mata_cfg_err_inject(uint64_t base);
int32_t drvmem_mata_mirror_err_inject(uint64_t base);

#endif