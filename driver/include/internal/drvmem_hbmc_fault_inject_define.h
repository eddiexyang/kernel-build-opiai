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
* Description: hbmc fault inject declaration
* Author: Huawei Technologies Co.Ltd
* Create: 2023-11-1
*/

#ifndef DRVMEM_HBMC_FAULT_INJECT_DEFINE_H
#define DRVMEM_HBMC_FAULT_INJECT_DEFINE_H

#include <linux/types.h>

int32_t drvmem_hbmc_sbram_ce_inject(uint64_t base);
int32_t drvmem_hbmc_sbram_ue_inject(uint64_t base);
int32_t drvmem_hbmc_rpram_ue_inject(uint64_t base);
int32_t drvmem_hbmc_sram_ce_inject(uint64_t base);
int32_t drvmem_hbmc_sram_ue_inject(uint64_t base);
int32_t drvmem_hbmc_rdq_parity_inject(uint64_t base);
int32_t drvmem_hbmc_wdq_parity_inject(uint64_t base);
int32_t drvmem_hbmc_ca_parity_inject(uint64_t base);
int32_t drvmem_hbmc_lack_sref_error_inject(uint64_t base);
int32_t drvmem_hbmc_rdata_timeout_error_inject(uint64_t base);

#endif