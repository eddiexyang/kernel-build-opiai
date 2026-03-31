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
#ifndef HBM_SINGLE_H
#define HBM_SINGLE_H
#include <linux/types.h>
#include "hbm_isolation_common.h"

#define OS_MAX_SINGLE_ECC_COUNT	 128
#define FLASH_MAX_SINGLE_ECC_COUNT  64

#define SINGLE_ECC_INFO_RESERVE	 28
#define SINGLE_ECC_HEADER_VERSION   0x1
struct single_ecc_header_s {
	uint32_t record_count;
	uint8_t signature;
	uint8_t version;
#ifdef CFG_HBM_FLASH
	uint16_t crc_check;
#endif
	uint8_t reserve[SINGLE_ECC_HEADER_RESERVE];
};

struct single_ecc_record_s {
	/* HBMC no. */
	uint32_t hbmc_id;
	uint32_t single_bit_count;
	uint32_t single_bit_low_addr;
	uint32_t single_bit_high_addr;
	uint32_t last_appear_time_stamp;
	uint8_t reserve[SINGLE_ECC_INFO_RESERVE];
};

struct single_ecc_tool_s {
	struct single_ecc_header_s single_header;
	struct single_ecc_record_s *single_ecc_infos;
	uint32_t process_failed_count;
	uint32_t write_failed_count;
	uint8_t is_single_ecc_to_flash;
};

void wait_single_ecc_data_resource(uint32_t dev_id);
void release_single_ecc_data_resource(uint32_t dev_id);
int hbm_init_single_ecc_info(void);
void hbm_free_single_ecc_info(void);
#ifdef CFG_HBM_FLASH
int hbm_clear_single_ecc_info(uint32_t dev_id);
#else
int hbm_clear_single_ecc_info(void);
#endif
void hbm_write_single_ecc_data_to_flash(void);
void hbm_process_single_count(void);
int hbm_get_one_single_ecc_info(uint32_t dev_id, uint32_t data_index,
	struct single_ecc_record_s *single_ecc_info);
uint32_t hbm_get_single_ecc_addr_count(uint32_t dev_id);
#endif
