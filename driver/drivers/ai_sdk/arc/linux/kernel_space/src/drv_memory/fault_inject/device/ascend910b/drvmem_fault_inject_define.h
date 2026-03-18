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
* Description: fault declaration
* Author: Huawei Technologies Co.Ltd
* Create: 2022-11-22
*/

#ifndef DRVMEM_FAULT_INJECT_DEFINE_H
#define DRVMEM_FAULT_INJECT_DEFINE_H

#include <linux/types.h>
#include <drvmem_fault_inject_node.h>
#include "memory_fault_init_cfg.h"

#define DRVMEM_FAULT_DEFINE(_type, _name, _op) \
	{.type = (_type), .name = (_name), .inject = (_op)}

#define DRVMEM_NODE_DEFINE(_type, _name, _fault, _num) \
	{.type = (_type), .name = (_name), .fault = (_fault), .fault_num = (_num)}

#define MASK_ALL_REG_BIT    0xFFFFFFFFU
#define INJECT_CE_BIT       0xFU
#define INJECT_UCE_BIT      0xFFFFFU

enum hbm_fault_type {
	HBM_FAULT_TYPE_CE_TH,
	HBM_FAULT_TYPE_FATAL_HIGH_TEMPER,
	HBM_FAULT_TYPE_HIGH_TEMPER,
	HBM_FAULT_TYPE_UE_EXCEED_1,
	HBM_FAULT_TYPE_UE_EXCEED_16,
	HBM_FAULT_TYPE_UE_EXCEED_64,
	HBM_FAULT_TYPE_UE_WITH_PROCESS_USED,
	HBM_FAULT_TYPE_UE_DEMAND_SCRUB
};

enum hbmc_fault_type {
	HBMC_FAULT_TYPE_SBRAM_CE,
	HBMC_FAULT_TYPE_SBRAM_UE,
	HBMC_FAULT_TYPE_RPRAM_UE,
	HBMC_FAULT_TYPE_SRAM_CE,
	HBMC_FAULT_TYPE_SRAM_UE,
	HBMC_FAULT_TYPE_RDQ_PARITY,
	HBMC_FAULT_TYPE_WDQ_PARITY,
	HBMC_FAULT_TYPE_CA_PARITY,
	HBMC_FAULT_TYPE_LACK_SREF_ERROR,
	HBMC_FAULT_TYPE_RDATA_TIMEOUT_ERROR
};

enum mata_fault_type {
	MATA_FAULT_TYPE_CE,
	MATA_FAULT_TYPE_UE,
	MATA_FAULT_TYPE_CFG_ERR,
	MATA_FAULT_TYPE_MIR_ERR
};

enum fault_node_type {
	FAULT_NODE_TYPE_HBM,
	FAULT_NODE_TYPE_HBMC,
	FAULT_NODE_TYPE_MATA
};

typedef struct {
	uint32_t stack;
	uint32_t pc;
	union {
		struct {
			uint32_t col   : 18;
			uint32_t row_l : 14;
		} reg;
		uint32_t val;
	} addr_l;
	union {
		struct {
			uint32_t row_h : 4;
			uint32_t bank  : 2;
			uint32_t bg    : 2;
			uint32_t sid   : 1;
			uint32_t resv  : 23;
		} reg;
		uint32_t val;
	} addr_h;
	uint64_t phy_addr;
} ecc_addr_record_s;

void drvmem_fault_get_fault_node_table(struct fault_node_table_info *table);
extern int memory_oem_fault_event_handler(struct memory_fault_event *event);

#endif
