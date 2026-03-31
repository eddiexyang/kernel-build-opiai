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
* Description: hbm fault inject declaration
* Author: Huawei Technologies Co.Ltd
* Create: 2023-11-1
*/

#ifndef DRVMEM_HBM_FAULT_INJECT_DEFINE_H
#define DRVMEM_HBM_FAULT_INJECT_DEFINE_H

#include <linux/types.h>

int32_t drvmem_hbm_ce_th_inject(uint64_t base);
int32_t drvmem_hbm_ue_exceed_1_inject(uint64_t base);
int32_t drvmem_hbm_ue_exceed_16_inject(uint64_t base);
int32_t drvmem_hbm_ue_exceed_64_inject(uint64_t base);
int32_t drvmem_hbm_fatal_high_temper_inject(uint64_t base);
int32_t drvmem_hbm_high_temper_inject(uint64_t base);
int32_t drvmem_hbm_ue_with_process_used_inject(uint64_t base);
int32_t drvmem_hbm_ue_demand_scrub_inject(uint64_t base);

#endif