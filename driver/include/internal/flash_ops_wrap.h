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
#ifndef FLASH_OPS_WRAP_H
#define FLASH_OPS_WRAP_H

#ifdef CFG_NOR_FLASH_OPS_VER1
#define ECC_CONFIG_FLASH_PART0			"Test_P0_N"
#define ECC_CONFIG_FLASH_PART1			"Test_P1_N"
#define ECC_CONFIG_FLASH_PART2			"Test_P2_N"
#define ECC_CONFIG_FLASH_PART3			"Test_P3_N"
#define ECC_CONFIG_FLASH_PART           ECC_CONFIG_FLASH_PART0
#endif

int hbm_flash_read(const unsigned char *part_name, unsigned int offset, size_t len, unsigned char *buf);
int hbm_flash_write(const unsigned char *part_name, unsigned int offset, size_t len, unsigned char *buf);
int hbm_flash_erase(const unsigned char *part_name, unsigned int offset, size_t len);

#endif
