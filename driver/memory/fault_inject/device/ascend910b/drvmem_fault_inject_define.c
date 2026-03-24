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

#include <linux/slab.h>
#include "drvmem_fault_inject_define.h"
#include "drvmem_hbm_fault_inject_define.h"
#include "drvmem_hbmc_fault_inject_define.h"
#include "drvmem_mata_fault_inject_define.h"

struct fault_node_fault g_hbm_fault_table[] = {
	DRVMEM_FAULT_DEFINE(HBM_FAULT_TYPE_CE_TH, "hbm ce th", drvmem_hbm_ce_th_inject),
	DRVMEM_FAULT_DEFINE(HBM_FAULT_TYPE_FATAL_HIGH_TEMPER, "hbm fatal high temper", drvmem_hbm_fatal_high_temper_inject),
	DRVMEM_FAULT_DEFINE(HBM_FAULT_TYPE_HIGH_TEMPER, "hbm high temper", drvmem_hbm_high_temper_inject),
	DRVMEM_FAULT_DEFINE(HBM_FAULT_TYPE_UE_EXCEED_1, "hbm ue exceed 1", drvmem_hbm_ue_exceed_1_inject),
	DRVMEM_FAULT_DEFINE(HBM_FAULT_TYPE_UE_EXCEED_16, "hbm ue exceed 16", drvmem_hbm_ue_exceed_16_inject),
	DRVMEM_FAULT_DEFINE(HBM_FAULT_TYPE_UE_EXCEED_64, "hbm ue exceed 64", drvmem_hbm_ue_exceed_64_inject),
	DRVMEM_FAULT_DEFINE(HBM_FAULT_TYPE_UE_WITH_PROCESS_USED, "hbm ue with process used",
		drvmem_hbm_ue_with_process_used_inject),
	DRVMEM_FAULT_DEFINE(HBM_FAULT_TYPE_UE_DEMAND_SCRUB, "hbm demand scrub error",
		drvmem_hbm_ue_demand_scrub_inject),
};

struct fault_node_fault g_hbmc_fault_table[] = {
	DRVMEM_FAULT_DEFINE(HBMC_FAULT_TYPE_SBRAM_CE, "hbmc sbram ce", drvmem_hbmc_sbram_ce_inject),
	DRVMEM_FAULT_DEFINE(HBMC_FAULT_TYPE_SBRAM_UE, "hbmc sbram ue", drvmem_hbmc_sbram_ue_inject),
	DRVMEM_FAULT_DEFINE(HBMC_FAULT_TYPE_RPRAM_UE, "hbmc rpram ue", drvmem_hbmc_rpram_ue_inject),
	DRVMEM_FAULT_DEFINE(HBMC_FAULT_TYPE_SRAM_CE, "hbmc sram ce", drvmem_hbmc_sram_ce_inject),
	DRVMEM_FAULT_DEFINE(HBMC_FAULT_TYPE_SRAM_UE, "hbmc sram ue", drvmem_hbmc_sram_ue_inject),
	DRVMEM_FAULT_DEFINE(HBMC_FAULT_TYPE_RDQ_PARITY, "hbmc rdq parity", drvmem_hbmc_rdq_parity_inject),
	DRVMEM_FAULT_DEFINE(HBMC_FAULT_TYPE_WDQ_PARITY, "hbmc wdq parity", drvmem_hbmc_wdq_parity_inject),
	DRVMEM_FAULT_DEFINE(HBMC_FAULT_TYPE_CA_PARITY, "hbmc ca parity", drvmem_hbmc_ca_parity_inject),
	DRVMEM_FAULT_DEFINE(HBMC_FAULT_TYPE_LACK_SREF_ERROR, "hbmc lack sref error", drvmem_hbmc_lack_sref_error_inject),
	DRVMEM_FAULT_DEFINE(HBMC_FAULT_TYPE_RDATA_TIMEOUT_ERROR, "hbmc rdata timeout error",
		drvmem_hbmc_rdata_timeout_error_inject),
};

struct fault_node_fault g_mata_fault_table[] = {
	DRVMEM_FAULT_DEFINE(MATA_FAULT_TYPE_CE, "mata ce", drvmem_mata_ce_inject),
	DRVMEM_FAULT_DEFINE(MATA_FAULT_TYPE_UE, "mata ue", drvmem_mata_ue_inject),
	DRVMEM_FAULT_DEFINE(MATA_FAULT_TYPE_CFG_ERR, "mata cfg", drvmem_mata_cfg_err_inject),
	DRVMEM_FAULT_DEFINE(MATA_FAULT_TYPE_MIR_ERR, "mata mir", drvmem_mata_mirror_err_inject),
};

struct fault_node g_fault_node_table[] = {
	DRVMEM_NODE_DEFINE(FAULT_NODE_TYPE_HBM,  "hbm",  g_hbm_fault_table, \
		sizeof(g_hbm_fault_table)/ sizeof(g_hbm_fault_table[0])),
	DRVMEM_NODE_DEFINE(FAULT_NODE_TYPE_HBMC, "hbmc", g_hbmc_fault_table, \
		sizeof(g_hbmc_fault_table)/ sizeof(g_hbmc_fault_table[0])),
	DRVMEM_NODE_DEFINE(FAULT_NODE_TYPE_MATA, "mata", g_mata_fault_table, \
		sizeof(g_mata_fault_table)/ sizeof(g_mata_fault_table[0])),
};

/* @brief get fault node table
* @param [out] table: fault node table
* @return void
*/
void drvmem_fault_get_fault_node_table(struct fault_node_table_info *table)
{
	table->node = g_fault_node_table;
	table->num = sizeof(g_fault_node_table) / sizeof(g_fault_node_table[0]);
}