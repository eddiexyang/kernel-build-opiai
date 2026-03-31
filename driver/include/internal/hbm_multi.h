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
#ifndef HBM_MULTI_H
#define HBM_MULTI_H
#include <linux/timer.h>
#include <linux/types.h>
#include <linux/cper.h>
#include "hbm_user_type.h"

#define ECC_ADDR_INFO_RECORDED	  1
#define ECC_ADDR_INFO_NOT_RECORDED  0

// 710上走HBM模式
#define DDR_ALIGN_MAX_ADDR_COUNT	2
#define SECOND_DDR_START_BASE		(0x4080000000)
#define DDR_MEMERY_PAGE_SIZE		(0x1000)
#define DDR_MEMERY_PAGE_MASK		(~0xFFF)

#define SING_ECC_INTERRUPT_COUNT	1
#define MULTI_ECC_INTERRUPT_COUNT   1

#define ECC_CONFIG_RESERVED_LENGTH	6
#define ECC_EXTERNAL_RESERVED_LENGTH	24

/* update ecc statistics every 5s */
#define PREIOD_STATISTIC_TIME		   5
/* write single bit ECC count to flash every 24 hours */
#define SINGLE_ECC_WRITE_FLASH_INTERVAL (24 * 3600 / PREIOD_STATISTIC_TIME)
/* write multi bit ECC count  every 1h hour */
#define SAVE_MULTI_ECC_COUNT_INTERVAL	(3600 / PREIOD_STATISTIC_TIME)
/* write single bit ECC count change per hour */
#define SINGLE_ECC_PRINT_INTERVAL	   (3600 / PREIOD_STATISTIC_TIME)
/* read single bit ECC addr and process pfns every 10s */
#define TEN_SECOND_INTERVAL	(10 / PREIOD_STATISTIC_TIME)
/* save single bit ECC addr  every 1h hour */
#define SAVE_SINGLE_ECC_ADDR_INTERVAL	(3600 / PREIOD_STATISTIC_TIME)

#define ECC_MODULE_COUNT				2
#define OFFSET_EIGHT					8
#define OFFSET_SIXTEEN				  16
#define OFFSET_TWENTY_FOUR			  24
#define DRVFAULT_IPC_MSG_LENGTH		 22

#pragma pack(4)
struct ecc_config_header_s {
	uint8_t signature;
	uint8_t version;
	/* Cumulative number of DDRC single bit ECC error reports */
	uint16_t ddr_single_bit_count;
	/* Cumulative number of DDRC multiple bit ECC error reports */
	uint16_t ddr_mul_bit_count;
	/* Cumulative number of HBM single bit ECC error reports */
	uint16_t hbm_single_bit_count;
	/* Cumulative number of HBM multiple bit ECC error reports */
	uint16_t hbm_mul_bit_count;
	uint8_t isolation_policy;
	/* recored ecc_addr_info length */
	uint16_t length;
	uint16_t ddr_single_ecc_length;
	uint16_t ddr_multi_ecc_length;
	uint16_t hbm_single_ecc_length;
	uint16_t hbm_multi_ecc_length;
	uint16_t crc_check;
	uint16_t statistics_info_offset;
	uint8_t reserved[ECC_CONFIG_RESERVED_LENGTH];
};

struct ecc_bit_count_info {
	uint32_t dev_id;
	uint32_t hbm_stack_i;
	uint32_t hbm_pc_i;
	uint32_t single_bit_count;
	uint32_t multi_bit_count;
};

struct external_info {
	uint64_t timer_stamp;
	uint8_t reserved[ECC_EXTERNAL_RESERVED_LENGTH];
};
#pragma pack()

extern pid_t *collect_process_by_pfn(unsigned long pfn, int *size);
int hbm_multi_ecc_record(struct cper_sec_mem_err *mem_err);
int hbm_ecc_isolation_init(void);
void hbm_ecc_isolation_exit(void);
#endif
