/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
* Description:
* Author: huawei
* Create: 2019-10-15
*/

#ifndef DRV_MEMORY_ECC_H
#define DRV_MEMORY_ECC_H

#include "memory_ddr_cfg.h"

int memory_get_ecc_statistics(void *feature, char *in, unsigned int in_len, char *out, unsigned int out_len);
#ifdef CFG_MEMORY_ECC_INFO_FROM_LP
int get_ecc_statistics_from_lp(struct memory_info *memory_arg);
#endif

#endif
