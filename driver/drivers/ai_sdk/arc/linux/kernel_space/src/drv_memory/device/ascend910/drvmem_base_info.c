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
#include "drvmem_base_info.h"
#include "memory_log.h"
#include "memory_adapter.h"
#include "memory_fault_init_cfg.h"

static const uint64_t g_hbm_stack_base[MAX_OS_DEVICE_COUNT][MAX_HBM_STACK_PER_SOCKET] = {
	{ SK0_HBM_STACK0_BASE_ADDR, SK0_HBM_STACK1_BASE_ADDR, SK0_HBM_STACK2_BASE_ADDR, SK0_HBM_STACK3_BASE_ADDR },
	{ SK1_HBM_STACK0_BASE_ADDR, SK1_HBM_STACK1_BASE_ADDR, SK1_HBM_STACK2_BASE_ADDR, SK1_HBM_STACK3_BASE_ADDR },
	{ SK2_HBM_STACK0_BASE_ADDR, SK2_HBM_STACK1_BASE_ADDR, SK2_HBM_STACK2_BASE_ADDR, SK2_HBM_STACK3_BASE_ADDR },
	{ SK3_HBM_STACK0_BASE_ADDR, SK3_HBM_STACK1_BASE_ADDR, SK3_HBM_STACK2_BASE_ADDR, SK3_HBM_STACK3_BASE_ADDR }
};

#define SOCKET_OFFSET 0x200000000000ULL

#define REMAP_ID_BITS 2U
#define HBM_ADDR_BANK_XOR_BIT 14U
#define STACKPC_ADDR_POS 12U
#define STACKPC_SRC_WIDTH 3U
#define HBMC_HIGH_ADDR_POS 15U
#define HBMC_LOW_ADDR_WITDH 9U
#define HBM_MSD_ID_POS 9U
#define BA_BG_START_POS 9U
#define BA_WIDTH 2U
#define BG_WIDTH 2U
#define MSD_ID_WIDTH 3U
#define DMC_INTLV_XOR_START_POS 9U
#define DMC_INTLV_XOR_END_POS 26U
#define DMC_INTLV_XOR_UINT 3U
#define MAX_REMAP_ID 4U
#define MAX_DHA_NUM 8U
#define MAX_DHACHN_NUM 4U

#define HBM_DHA_0 0U
#define HBM_DHA_1 1U
#define HBM_DHA_2 2U
#define HBM_DHA_3 3U
#define HBM_DHA_4 4U
#define HBM_DHA_5 5U
#define HBM_DHA_6 6U
#define HBM_DHA_7 7U

STATIC struct hbm_addr_trans_info g_addr_trans_info;
STATIC const uint32_t g_dha_die_remap[MAX_DIE][MAX_REMAP_ID] = {
	{HBM_DHA_4, HBM_DHA_5, HBM_DHA_6, HBM_DHA_7},
	{HBM_DHA_0, HBM_DHA_1, HBM_DHA_2, HBM_DHA_3}
};

STATIC const uint32_t g_dha_chn_2_hbm_ch_map[MAX_DHA_NUM][MAX_DHACHN_NUM] = {
	{HBM0_CHN0, HBM0_CHN1, HBM0_CHN4, HBM0_CHN5},
	{HBM1_CHN7, HBM1_CHN6, HBM1_CHN3, HBM1_CHN2},
	{HBM2_CHN5, HBM2_CHN4, HBM2_CHN1, HBM2_CHN0},
	{HBM3_CHN2, HBM3_CHN3, HBM3_CHN6, HBM3_CHN7},
	{HBM0_CHN7, HBM0_CHN6, HBM0_CHN3, HBM0_CHN2},
	{HBM1_CHN0, HBM1_CHN1, HBM1_CHN4, HBM1_CHN5},
	{HBM2_CHN7, HBM2_CHN6, HBM2_CHN3, HBM2_CHN2},
	{HBM3_CHN0, HBM3_CHN1, HBM3_CHN4, HBM3_CHN5},
};

STATIC void memory_hbm_init_trans_info(void)
{
	g_addr_trans_info.dmc_intlv_en = 7;
	g_addr_trans_info.dmc_intlv_xor = 1;
	g_addr_trans_info.bank_xor = 2;
	g_addr_trans_info.ch_intlv = 3;
	g_addr_trans_info.group_intlv = 1;

	g_addr_trans_info.sid_start_bit = 14;
	g_addr_trans_info.bnk_start_bit = 8;
	g_addr_trans_info.col_start_bit_l = 3;
	g_addr_trans_info.col_start_bit_h = 12;
	g_addr_trans_info.col_width_l = 5;
	g_addr_trans_info.col_width_h = 2;
	g_addr_trans_info.row_start_bit_l = 15;
	g_addr_trans_info.row_start_bit_h = 0;
	g_addr_trans_info.row_width_l = 14;
	g_addr_trans_info.row_width_h = 0;
	return;
}

STATIC uint64_t get_chip_die(uint32_t dev_id, uint64_t phy_addr, struct hbm_hw_addr_s *hbm_hw_addr)
{
	uint32_t chip_id = phy_addr / SOCKET_OFFSET;
	uint64_t sys_addr = phy_addr - chip_id * SOCKET_OFFSET;
	int32_t ret;
	uint32_t die_id = 0;
	ret = devdrv_get_chip_die_id(dev_id, &chip_id, &die_id);	/* get chip id and die id by device id */
	if (ret != 0)
		memory_drv_err("get chip id and die id failed, dev id = %u\n", dev_id);

	hbm_hw_addr->chip = chip_id;
	hbm_hw_addr->die = die_id;
	return sys_addr;
}
STATIC uint64_t get_stackpc_src(uint64_t sys_addr)
{
	return (sys_addr >> STACKPC_ADDR_POS) & mask(STACKPC_SRC_WIDTH);
}

STATIC uint64_t get_hbmc_addr(uint64_t sys_addr)
{
	return (sys_addr & mask(HBMC_LOW_ADDR_WITDH)) |
		((sys_addr >> HBMC_HIGH_ADDR_POS) << HBM_MSD_ID_POS);
}

STATIC uint64_t get_dha_id(uint64_t hbmc_addr, uint64_t sys_addr, struct hbm_hw_addr_s *hbm_hw_addr)
{
	uint64_t tmp_val = (hbmc_addr >> BA_BG_START_POS) & mask(MSD_ID_WIDTH);
	uint64_t msd_id = (sys_addr >> HBM_MSD_ID_POS) & mask(MSD_ID_WIDTH);
	uint64_t remap_val = msd_id ^ tmp_val;
	uint64_t die_id = (remap_val >> REMAP_ID_BITS) & mask(1);
	uint32_t dha;

	remap_val = remap_val & mask(REMAP_ID_BITS);
	dha = g_dha_die_remap[die_id][remap_val];

	return dha;
}

STATIC void get_stack_pc(uint64_t sys_addr, uint64_t hbmc_addr,
		uint64_t stackpc_src, struct hbm_hw_addr_s *hbm_hw_addr)
{
	uint32_t dha_id = get_dha_id(hbmc_addr, sys_addr, hbm_hw_addr) & mask(3);
	uint32_t dha_hbm_pc = 0;
	uint32_t dha_hbm_chn;
	uint32_t hbm_chn, loop;
	uint64_t xor_data = 0;
	uint64_t temp;

	if (g_addr_trans_info.dmc_intlv_en != 0) {
		temp = hbmc_addr >> DMC_INTLV_XOR_START_POS;
		for (loop = DMC_INTLV_XOR_START_POS; loop < DMC_INTLV_XOR_END_POS; loop += DMC_INTLV_XOR_UINT) {
			xor_data ^= temp & mask(DMC_INTLV_XOR_UINT);
			temp = temp >> DMC_INTLV_XOR_UINT;
		}
	} else {
		xor_data = 0;
	}

	dha_hbm_pc = stackpc_src ^ xor_data;
	dha_hbm_chn = (dha_hbm_pc / 2U) & mask(2);
	hbm_hw_addr->stack = (g_dha_chn_2_hbm_ch_map[dha_id][dha_hbm_chn] >> 4U) & mask(4);
	hbm_chn = g_dha_chn_2_hbm_ch_map[dha_id][dha_hbm_chn] & mask(4);
	hbm_hw_addr->pc = hbm_chn * 2U + (dha_hbm_pc % 2U);

	hbm_hw_addr->module_id = hbm_hw_addr->stack * MAX_HBM_PC_PER_STACK + hbm_hw_addr->pc + 2U;
	return;
}

STATIC void get_unit_addr(uint64_t hbmc_addr, struct hbm_hw_addr_s *hbm_hw_addr)
{
	uint64_t xor_addr, xor_addr_expand;
	uint64_t row_high, row_low, col_high, col_low, ba_bg_merge;
	if (g_addr_trans_info.bank_xor == 0)
		xor_addr = 0;
	else
		xor_addr = (hbmc_addr >> (HBM_ADDR_BANK_XOR_BIT + g_addr_trans_info.bank_xor - 1U)) & mask(4);
	xor_addr_expand = xor_addr << g_addr_trans_info.bnk_start_bit;
	hbmc_addr = hbmc_addr ^ xor_addr_expand;

	row_high = (hbmc_addr >> g_addr_trans_info.row_start_bit_h) & mask(g_addr_trans_info.row_width_h);
	row_low = (hbmc_addr >> g_addr_trans_info.row_start_bit_l) & mask(g_addr_trans_info.row_width_l);
	hbm_hw_addr->row = (row_high << g_addr_trans_info.row_width_l) | row_low;
	hbm_hw_addr->sid = (hbmc_addr >> g_addr_trans_info.sid_start_bit) & mask(1);
	ba_bg_merge = (hbmc_addr >> g_addr_trans_info.bnk_start_bit) & mask(4);
	col_high = (hbmc_addr >> g_addr_trans_info.col_start_bit_h) & mask(g_addr_trans_info.col_width_h);
	col_low = (hbmc_addr >> g_addr_trans_info.col_start_bit_l) & mask(g_addr_trans_info.col_width_l);
	hbm_hw_addr->bg = ba_bg_merge & mask(BA_WIDTH);
	hbm_hw_addr->ba = (ba_bg_merge >> BG_WIDTH) & mask(BA_WIDTH);
	hbm_hw_addr->bank = ba_bg_merge;
	hbm_hw_addr->col = col_low | (col_high << g_addr_trans_info.col_width_l);
	return;
}

STATIC void memory_get_hbm_hwaddr(uint32_t dev_id, uint64_t phy_addr, struct hbm_hw_addr_s *hbm_hw_addr)
{
	uint64_t sys_addr, hbmc_addr, stackpc_src;

	sys_addr = get_chip_die(dev_id, phy_addr, hbm_hw_addr);
	hbmc_addr = get_hbmc_addr(sys_addr);
	stackpc_src = get_stackpc_src(sys_addr);
	get_stack_pc(sys_addr, hbmc_addr, stackpc_src, hbm_hw_addr);
	get_unit_addr(hbmc_addr, hbm_hw_addr);

	return;
}

int32_t get_hbm_chip_die_offset(uint32_t dev_id, uint64_t *chip_base_addr, uint64_t *chip_offset,
	uint64_t *die_offset)
{
	(void)dev_id;
	(void)chip_base_addr;
	(void)chip_offset;
	(void)die_offset;

	return 0;
}

int32_t get_hbm_stack_base(uint32_t socket, uint32_t stack, uint64_t *base)
{
	if (socket >= MAX_OS_DEVICE_COUNT || stack >= MAX_HBM_STACK_PER_SOCKET) {
		memory_drv_err("illegal socket %u or stack %u.\n", socket, stack);
		return -EINVAL;
	}

	*base = g_hbm_stack_base[socket][stack];
	return 0;
}

STATIC int32_t memory_get_hbm_base_addr(uint32_t dev_id, uint64_t *base)
{
	int32_t ret;
	uint32_t chip_id, die_id;

	ret = devdrv_get_chip_die_id(dev_id, &chip_id, &die_id);	/* get chip id and die id by device id */
	if (ret != 0) {
		memory_drv_err("get chip id and die id failed, dev id = %u\n", dev_id);
		return ret;
	}

	*base = CHIP_BASE_ADDR + (uint64_t)chip_id * CHIP_OFFSET + (uint64_t)die_id * DIE_OFFSET;

	return 0;
}

STATIC int memory_get_chip_die_id(uint64_t addr, unsigned int *chip, unsigned int *devid)
{
	*devid = addr / ONE_DEVICE_PAGE_START_OFFSET;
	return 0;
}

void hbm_mirror_cfg_iounmap(void)
{
	return;
}

int32_t memory_check_mirror_addr(uint64_t addr, uint32_t dev_id)
{
	(void)addr;
	(void)dev_id;

	return 0;
}

int32_t memory_get_mirror_cfg(uint32_t dev_num)
{
	(void)dev_num;

	return 0;
}

static struct memory_adapter_t adapter = {
	.filter_addr = NULL,
	.get_chip_die = memory_get_chip_die_id,
	.get_base_addr = memory_get_hbm_base_addr,
	.get_mirror_cfg = memory_get_mirror_cfg,
	.check_mirror_addr = memory_check_mirror_addr,
	.hbm_init_trans_info = memory_hbm_init_trans_info,
	.get_hbm_hwaddr = memory_get_hbm_hwaddr
};

struct memory_adapter_t *memory_get_adapter(void)
{
	return &adapter;
}
