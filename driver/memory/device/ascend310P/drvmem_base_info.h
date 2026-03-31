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
#ifndef MEM_BASE_INFO_H
#define MEM_BASE_INFO_H

#include <linux/types.h>

#define MAX_OS_DEVICE_COUNT			2
#define HBM_ROW_MEMERY_MASK			(~0xFFF) // 4K
#define ECC_CONFIG_START_OFFSET		0xF70000

#define DDR_MSD_2G_OFFSET			0x80000000UL // 2GB
#define DDR_MSD_HIGH_OFFSET			0x800000000UL
#define DDR_MSD_DIE1_OFFSET			0x8000000000UL

void hbm_mirror_cfg_iounmap(void);
void get_ecc_count_record(uint32_t dev_id, uint32_t *single_bit_count, uint32_t *multi_bit_count);

#endif
