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
#include "dms_dev_node.h"
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/pfn.h>
#include <linux/mm.h>
#include "memory_log.h"
#include "hbm_multi.h"
#include "hbm_isolation_common.h"
#include "safety_ras_isolation_710.h"
#include "drvmem_base_info.h"

#define mask(n)		((0x1UL << (n)) - 1UL)

mem_info_str volatile *g_mem_info[MEMORY_NODE_ID_MAX] = {NULL};
mem_cfg_str g_mem_cfg[MEMORY_NODE_ID_MAX] = {0};

STATIC inline void memory_kfree(void **ptr)
{
	if (*ptr != NULL) {
		kfree(*ptr);
		*ptr = NULL;
	}
}

// remap{dha ingress}\bank\dmc
const uint32_t g_chn_map[MSD_REMAP_NO][MSD_BANK_NO][MSD_DMC_NO] = {
	{ { 20U, 19U, 18U }, { 23U, 22U, 21U }, },	// remap 0
	{ { 12U, 13U, 14U }, { 15U, 16U, 17U }, },	// remap 1
	{ { 8U, 7U, 6U }, { 11U, 10U, 9U }, },		// remap 2
	{ { 0U, 1U, 2U }, { 3U, 4U, 5U }, },		// remap 3
};

STATIC void mem_cfg_debug(mem_cfg_str *mem_cfg)
{
	memory_drv_info("static infomations\n");
	memory_drv_info("row_num\t\t:%u\n", mem_cfg->row_num);
	memory_drv_info("col_num\t\t:%u\n", mem_cfg->col_num);
	memory_drv_info("rank_start_bit\t:%u\n", mem_cfg->rank_start_bit);
	memory_drv_info("rank_bit_num\t:%u\n", mem_cfg->rank_bit_num);
	memory_drv_info("bank_start_bit\t:%u\n", mem_cfg->bank_start_bit);
	memory_drv_info("bank_bit_num\t:%u\n", mem_cfg->bank_bit_num);
	memory_drv_info("col_start_bit_l\t:%u\n", mem_cfg->col_start_bit_l);
	memory_drv_info("col_bit_num_l\t:%u\n", mem_cfg->col_bit_num_l);
	memory_drv_info("col_start_bit_h\t:%u\n", mem_cfg->col_start_bit_h);
	memory_drv_info("col_bit_num_h\t:%u\n", mem_cfg->col_bit_num_h);
	memory_drv_info("row_start_bit_l\t:%u\n", mem_cfg->row_start_bit_l);
	memory_drv_info("row_bit_num_l\t:%u\n", mem_cfg->row_bit_num_l);
	memory_drv_info("row_start_bit_h\t:%u\n", mem_cfg->row_start_bit_h);
	memory_drv_info("row_bit_num_h\t:%u\n", mem_cfg->row_bit_num_h);
}

#ifndef DRV_MEM_GTEST
STATIC uint32_t check_dram_size(uint32_t dram_size, uint32_t rank_num)
{
	uint32_t i;
	uint32_t rank_size = dram_size / rank_num;
	static uint32_t dram_size_map[0x5U][0x2U] = {
			{0x200U, 15U}, {0x400U, 16U}, {0x600U, 17U}, {0x800U, 17U}, {0x1000U, 18U}
		}; // 单rank

	for (i = 0; i < 0x5U; i++) {
		if (rank_size == dram_size_map[i][0]) {
			return dram_size_map[i][1];
		}
	}

	return 0;
}

STATIC bool mem_cfg_check(mem_info_str volatile *mem_info)
{
	mem_check_act_info_log(mem_info->version != IECC_ISOLATION_MAGIC,
		return false, "version(%x) not correct\n", mem_info->version);

	mem_check_act_info_log(mem_info->rank_num > 0x2U, // 最大2rank
		return false, "rank_num(%x) not correct\n", mem_info->rank_num);

	mem_check_act_info_log(mem_info->iecc_en != 0x1U, // IECC功能必须开启
		return false, "iecc_en(%x) not enable\n", mem_info->iecc_en);

	mem_check_act_info_log(mem_info->chn_mask != 0xFFFFFFU,  // 通道满配3-way映射
		return false, "chn_mask(%x) not support\n", mem_info->chn_mask);

	mem_check_act_info_log(mem_info->dram_type != 0x0U, // 目前仅适配lp4
		return false, "dram_type(%x) not support\n", mem_info->dram_type);

	// 单通道容量512M到4GB
	mem_check_act_info_log(check_dram_size(mem_info->dram_size, mem_info->rank_num) == 0x0U,
		return false, "dram_size(%x) not support\n", mem_info->dram_size);

	return true;
}

STATIC void mem_cfg_init(uint32_t dev_id, mem_cfg_str *mem_cfg, mem_info_str volatile *mem_info)
{
	// 1. 基本信息检查
	if (mem_cfg_check(mem_info) == false) {
		memory_drv_info("no need ddr isolation feature.\n");
		mem_cfg->ioslation_en = 0x0;
		return;
	}

	// 2. 基本信息初始化
	mem_cfg->dev_id = dev_id;
	mem_cfg->ioslation_en = 0x1U; // 通过检查, 使能隔离功能

	// 3.1 MSD转换信息初始化, 固定信息，同配置联动修改
	mem_cfg->dha_intlv_bit = 10U;
	mem_cfg->dha_intlv_xor = 1U;
	mem_cfg->dmc_intlv_bit = 9U;
	mem_cfg->dmc_intlv_xor = 1U;
	mem_cfg->intlv_banken = 1U;
	mem_cfg->intlv_dmcen = 2U; // 仅支持3-way
	mem_cfg->bank_dmc_intlv_xor = 1U;

	// 3.2 Matrix转换信息初始化, 默认支持映射模式：row-rank-row-col-bank-col
	mem_cfg->col_num = 10U;
	mem_cfg->row_num = check_dram_size(mem_info->dram_size, mem_info->rank_num);
	mem_cfg->rank_start_bit = mem_cfg->row_num + 10U; // 相对计算(14+rownum-4)
	mem_cfg->rank_bit_num = mem_info->rank_num - 1U;
	mem_cfg->bank_start_bit = 9U; // 固定LP4, 同配置联动修改
	mem_cfg->bank_bit_num = 3U;
	mem_cfg->col_start_bit_l = 1U;
	mem_cfg->col_bit_num_l = mem_cfg->bank_start_bit - 1U;
	mem_cfg->col_start_bit_h = mem_cfg->bank_start_bit + mem_cfg->bank_bit_num;
	mem_cfg->col_bit_num_h = mem_cfg->col_num - mem_cfg->col_bit_num_l;
	mem_cfg->row_start_bit_l = mem_cfg->col_start_bit_h + mem_cfg->col_bit_num_h;
	mem_cfg->row_bit_num_l = mem_cfg->rank_start_bit - mem_cfg->row_start_bit_l;
	mem_cfg->row_start_bit_h = mem_cfg->rank_start_bit + mem_cfg->rank_bit_num;
	mem_cfg->row_bit_num_h = mem_cfg->row_num - mem_cfg->row_bit_num_l;

	// 4. 打印配置信息
	mem_cfg_debug(mem_cfg);
}

STATIC void unmap_isolation_addr_info(uint32_t dev_num)
{
	uint32_t dev_id;

	for (dev_id = 0; dev_id < memory_dev_num(dev_num); dev_id++) {
		if (g_mem_info[dev_id] != NULL) {
			iounmap(g_mem_info[dev_id]);
			g_mem_info[dev_id] = NULL;
		}
	}
}

int32_t memory_failure_pre_handle(void)
{
	int32_t ret;
	uint32_t dev_num;
	uint32_t dev_id;

	ret = devdrv_get_devnum(&dev_num);
	if (ret != 0) {
		memory_drv_err("Get dev_num failed. (ret=%d)\n", ret);
		return ret;
	}

	for (dev_id = 0; dev_id < memory_dev_num(dev_num); dev_id++) {
		g_mem_info[dev_id] = ioremap(DDR_MEMORY_INFO_ADDR + dev_id * DDR_MSD_DIE1_OFFSET, sizeof(mem_info_str));
		if (g_mem_info[dev_id] == NULL) {
			memory_drv_err("ioremap ddr memory info failed.\n");
			unmap_isolation_addr_info(MEMORY_NODE_ID_MAX);
			return -1;
		}
		mem_cfg_init(dev_id, &g_mem_cfg[dev_id], g_mem_info[dev_id]);
	}

	return hbm_ecc_isolation_init(); // 始终执行隔离初始化
}

int32_t memory_failure_post_handle(void)
{
	unmap_isolation_addr_info(MEMORY_NODE_ID_MAX);

	hbm_ecc_isolation_exit();

	return 0;
}

STATIC bool lookup_intlv_info(long dmc_no_act, trans_info_str *trans_info)
{
	uint32_t remap, bank_no, dmc_no;

	for (remap = 0; remap < MSD_REMAP_NO; remap++) {
		for (bank_no = 0; bank_no < MSD_BANK_NO; bank_no++) {
			for (dmc_no = 0; dmc_no < MSD_DMC_NO; dmc_no++) {
				if (g_chn_map[remap][bank_no][dmc_no] == dmc_no_act) {
					trans_info->msd_remap = remap;
					trans_info->msd_bank_no = bank_no;
					trans_info->msd_dmc_no = dmc_no;
					return true;
				}
			}
		}
	}

	return false;
}

STATIC uint64_t matrix_to_sysaddr(struct cper_sec_mem_err *mem_err, mem_cfg_str *mem_cfg, trans_info_str *trans_info)
{
	uint64_t sysaddr;

	// 1. iecc_exp_trans计算实际的行列信息
	trans_info->col_unwind = ((mem_err->row * BANK_NUM) + mem_err->bank) * DATA_COL_NUM + mem_err->column;
	trans_info->row_act = trans_info->col_unwind / ROW_MOD_VAL;
	trans_info->bank_act = (trans_info->col_unwind % ROW_MOD_VAL) / BANK_MOD_VAL;
	trans_info->col_act = (trans_info->col_unwind % ROW_MOD_VAL) % BANK_MOD_VAL;
	trans_info->rank_act = mem_err->rank;
	trans_info->dmc_no_act = mem_err->module - ECC_MODULE_HBM_MIN; // dmc通道号

	// 2. matrix_to_sysaddr计算DMC入口地址
	trans_info->col_l = trans_info->col_act & mask(mem_cfg->col_bit_num_l);
	trans_info->col_h = (trans_info->col_act >> mem_cfg->col_bit_num_l) & mask(mem_cfg->col_bit_num_h);
	trans_info->bank = trans_info->bank_act & mask(mem_cfg->bank_bit_num);
	trans_info->row_l = trans_info->row_act & mask(mem_cfg->row_bit_num_l);
	trans_info->row_h = (trans_info->row_act >> mem_cfg->row_bit_num_l) & mask(mem_cfg->row_bit_num_h);
	trans_info->rank = trans_info->rank_act & mask(mem_cfg->rank_bit_num);
	sysaddr = (trans_info->col_l << mem_cfg->col_start_bit_l) | (trans_info->col_h << mem_cfg->col_start_bit_h) |
			(trans_info->row_l << mem_cfg->row_start_bit_l) | (trans_info->row_h << mem_cfg->row_start_bit_h) |
			(trans_info->bank << mem_cfg->bank_start_bit) | (trans_info->rank << mem_cfg->rank_start_bit);

	return sysaddr;
}

STATIC uint64_t iecc_info_to_phyaddr(struct cper_sec_mem_err *mem_err, mem_cfg_str *mem_cfg)
{
	uint32_t i;
	uint64_t phyaddr;
	trans_info_str *trans_info;

	trans_info = (trans_info_str *)kzalloc(sizeof(trans_info_str), GFP_KERNEL | __GFP_ACCOUNT);
	if (trans_info == NULL) {
		memory_drv_err("trans_info kzalloc failed.\n");
		return 0;
	}

	// 1. iecc_to_sysaddr计算DMC入口地址
	phyaddr = matrix_to_sysaddr(mem_err, mem_cfg, trans_info);

	// 2. sysaddr_to_phyaddr计算物理地址
	if (lookup_intlv_info(trans_info->dmc_no_act, trans_info) == false) {
		memory_kfree((void **)&trans_info);
		memory_drv_err("lookup intlv info failed.\n");
		return 0;
	}
	// 2.1 add dmc info
	phyaddr = (phyaddr & mask(0x9U)) | (((phyaddr >> 0x9U) * 0x3U + trans_info->msd_dmc_no) << 0x9U);
	// 2.2 add bank info
	phyaddr = (phyaddr & mask(0x9U)) | ((phyaddr >> 0x9U) << (0x9U + 0x1)); // 增加bit9
	for (i = 0; i < 12U; i++) { // 默认使能异或，全部bit参与
		trans_info->msd_bank_no ^= (phyaddr >> (10U + i)) & 0x1;
	}
	phyaddr |= trans_info->msd_bank_no << 0x9U;

	// 2.3 add dha info, dmc bit, dha bit
	phyaddr = (phyaddr & mask(mem_cfg->dmc_intlv_bit)) |
			((phyaddr >> mem_cfg->dmc_intlv_bit) << (mem_cfg->dmc_intlv_bit + 1)); // 增加dmc bit
	phyaddr = (phyaddr & mask(mem_cfg->dha_intlv_bit)) |
			((phyaddr >> mem_cfg->dha_intlv_bit) << (mem_cfg->dha_intlv_bit + 1)); // 增加dha bit
	phyaddr |= (((phyaddr >> mem_cfg->dmc_intlv_bit) & 0x1) ^
			((phyaddr >> (mem_cfg->dmc_intlv_bit + 0x5U)) & 0x1) ^
			(trans_info->msd_remap & 0x1)) << mem_cfg->dmc_intlv_bit; // 注入dmc bit
	phyaddr |= (((phyaddr >> mem_cfg->dha_intlv_bit) & 0x1) ^
			((phyaddr >> (mem_cfg->dha_intlv_bit + 0x5U)) & 0x1) ^
			((trans_info->msd_remap >> 0x1) & 0x1)) << mem_cfg->dha_intlv_bit; // 注入dha bit

	// 2.4 转换出物理地址如果高于2GB，需要添加高位的基地址
	phyaddr += (phyaddr >= DDR_MSD_2G_OFFSET) ? DDR_MSD_HIGH_OFFSET : 0x0U; // msd0 base addr

	// 4 增加device地址偏移, 最多2P
	phyaddr += (mem_cfg->dev_id == 0x1U) ? DDR_MSD_DIE1_OFFSET : 0x0U;

	memory_drv_info("trans result phyaddr:0x%llx\n", phyaddr);

	memory_kfree((void **)&trans_info);

	return phyaddr;
}

STATIC bool construct_mem_err(struct cper_sec_mem_err *mem_err, mem_info_str volatile *mem_info)
{
	if (mem_info->uerr_vaild != IECC_ADDR_INFO_VALID) {
		memory_drv_info("no valid iecc info, need do nothing\n");
		return false;
	}

	memory_drv_info("uecc_dfx_st10 0x%x, uecc_dfx_st11 0x%x\n",
		mem_info->uecc_dfx_st10, mem_info->uecc_dfx_st11);

	mem_err->validation_bits = 0xFFFFFFFFU; // 地址信息全部有效
	mem_err->physical_addr = 0x0U; // 单独转换
	mem_err->module = ((mem_info->uecc_dfx_st10 >> 24U) & 0xFFU) + ECC_MODULE_HBM_MIN; // 适配hbm判断调过两个ddr通道
	mem_err->rank = (mem_info->uecc_dfx_st11 >> 23U) & 0x1U;
	mem_err->bank = (mem_info->uecc_dfx_st11 >> 20U) & 0x7U;
	mem_err->row = mem_info->uecc_dfx_st11 & mask(18U);
	mem_err->column = mem_info->uecc_dfx_st10 & mask(11U);
	mem_err->error_type = 0x3U; // 将被转为类型ECC_TYPE_MULTI_BIT

	// 最后写标记，有故障地址丢失的概率
	mem_info->uerr_vaild = 0x0U;

	return true;
}

STATIC void memory_failure_isolation(uint64_t paddr, int flags)
{
	uint64_t physical_addr = paddr;
	uint64_t pfn;

	pfn = PHYS_PFN(physical_addr);
	if (!pfn_valid(pfn)) {
		memory_drv_info("invalid address in generic error data\n");
		return;
	}

	if (pfn_to_online_page(pfn) == NULL) {
		memory_drv_info("invalid onine pfn\n");
		return;
	}

	memory_failure_queue(pfn, flags);
}

// 多bit错误后处理入口函数
void memory_failure_handle(int32_t dev_id)
{
	int32_t flags = 0;
	struct cper_sec_mem_err mem_err = {0};

	// 1. Sanity检查
	if (g_mem_cfg[dev_id].ioslation_en == 0x0) {
		memory_drv_info("ioslation not support\n");
		return;
	}

	// 2. 获取基础信息
	if (construct_mem_err(&mem_err, g_mem_info[dev_id]) == false) {
		return;
	}

	// 3. 解析物理地址
	mem_err.physical_addr = iecc_info_to_phyaddr(&mem_err, &g_mem_cfg[dev_id]);
	if (mem_err.physical_addr == 0) {
		return;
	}

	// 4. 在线隔离函数
	memory_failure_isolation(mem_err.physical_addr, flags);

	// 5. 复用hbm驱动后处理
	hbm_multi_ecc_record(&mem_err);
}

// 对外导出，多bit上报过程中, 周期轮询中获取
void get_ecc_count_record(uint32_t dev_id, uint32_t *single_bit_count, uint32_t *multi_bit_count)
{
	*single_bit_count = 0x0U;
	*multi_bit_count = 0x0U;

	// 目前仅更新多bit记录
	if ((g_mem_cfg[dev_id].ioslation_en != 0) && (g_mem_info[dev_id] != NULL)) {
		*multi_bit_count = g_mem_info[dev_id]->ue_count;
	}
}
#endif
