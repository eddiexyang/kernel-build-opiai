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
 * Description:
 * Author: huawei
 * Create: 2022-01-20
 */
#ifndef SAFETY_RAS_ISOLATION_710_H
#define SAFETY_RAS_ISOLATION_710_H
#include <linux/types.h>
#include "safety_ras_node.h"

#define mem_check_act_info_log(expr, action, fmt, ...)			\
	if (expr) {													\
		memory_drv_info(fmt, ##__VA_ARGS__);					\
		action;													\
	}

#define DDR_MEMORY_INFO_ADDR	0xC6F13C00U
#define IECC_UERR_ADDR_MAX		0xAU
#define IECC_ISOLATION_MAGIC	0x5A3C0000U // 低16bit标识版本号
#define IECC_ADDR_INFO_VALID	0x4F435552U // 代表OCUR

// Matrix解析
#define COL_NUM					1024U
#define DATA_COL_NUM			960U
#define BANK_NUM				8U
#define ROW_MOD_VAL				(COL_NUM * BANK_NUM)
#define BANK_MOD_VAL			COL_NUM
// MSD映射
#define MSD_REMAP_NO			4U
#define MSD_BANK_NO				2U
#define MSD_DMC_NO				3U

typedef struct {
	// 故障基本信息记录，兼容不改动
	uint32_t ce_count;
	uint32_t ue_count;
	uint32_t iecc_cerr_chan_mask;
	uint32_t iecc_uerr_chan_mask;
	uint32_t reset_system_reason;
	// 隔离用颗粒基本信息
	uint32_t version;
	uint32_t dram_size;
	uint32_t dram_type;
	uint32_t chn_mask;
	uint32_t rank_num;
	uint32_t iecc_en;
	// 隔离用故障地址信息
	uint32_t cerr_vaild;	// cerr vaild,chn: high 16 bits valid, low 16bits chn info
	uint32_t cerr_dfx_st8;	// cerr chn,type,col
	uint32_t cerr_dfx_st9;	// cerr rank,bank&bg,row
	uint32_t uerr_vaild;	// uerr vaild,chn: high 16 bits valid, low 16bits chn info
	uint32_t uecc_dfx_st10;	// uerr chn,type,col
	uint32_t uecc_dfx_st11;	// uerr rank,bank&bg,row
	uint32_t resverd[0xFU];
} mem_info_str;

typedef struct {
	// 基本信息
	uint32_t ioslation_en;
	uint32_t dev_id;

	// MSD配置信息
	uint32_t dha_intlv_bit;
	uint32_t dha_intlv_xor;
	uint32_t dmc_intlv_bit;
	uint32_t dmc_intlv_xor;
	uint32_t intlv_banken;
	uint32_t intlv_dmcen;
	uint32_t bank_dmc_intlv_xor;

	// Matrix信息
	uint32_t row_num;
	uint32_t col_num;
	uint32_t rank_start_bit;
	uint32_t rank_bit_num;
	uint32_t bank_start_bit;
	uint32_t bank_bit_num;
	uint32_t row_start_bit_l;
	uint32_t row_bit_num_l;
	uint32_t row_start_bit_h;
	uint32_t row_bit_num_h;
	uint32_t col_start_bit_l;
	uint32_t col_bit_num_l;
	uint32_t col_start_bit_h;
	uint32_t col_bit_num_h;
} mem_cfg_str;

typedef struct {
	// remap相关
	uint32_t msd_remap;
	uint32_t msd_bank_no;
	uint32_t msd_dmc_no;

	// Matrix相关
	uint64_t col_unwind;
	uint64_t row_act;
	uint64_t bank_act;
	uint64_t col_act;
	uint64_t rank_act;
	uint64_t dmc_no_act;
	uint64_t col_l;
	uint64_t col_h;
	uint64_t bank;
	uint64_t row_l;
	uint64_t row_h;
	uint64_t rank;
} trans_info_str;

uint32_t memory_dev_num(uint32_t dev_num);
struct drv_memory_devices *memory_get_devices(void);

void memory_failure_handle(int32_t dev_id);
int32_t memory_failure_pre_handle(void);
int32_t memory_failure_post_handle(void);

#endif