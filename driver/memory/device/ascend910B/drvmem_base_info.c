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
#include <linux/io.h>
#include "memory_log.h"
#include "devdrv_manager_comm.h"
#include "memory_adapter.h"
#include "devdrv_interface.h"
#include <asm/io.h>

void __iomem *g_hbm_mirror_base[MAX_OS_DEVICE_COUNT] = {NULL};
struct hbm_mirror_addr_cfg g_hbm_mirror_addr_cfg[MAX_OS_DEVICE_COUNT] = {0};

#define BIOS_RESERVED_ZONE_START	 0x9800000ULL
#define BIOS_RESERVED_ZONE_END	   0x19E00000ULL
#define KERNEL_ZONE_START			0x51370000ULL
#define KERNEL_ZONE_END			  0x80000000ULL

#define DEVICE_OFFSET_MASK		   (HCCS_DIE_OFFSET - 1)

static const uint64_t g_hbm_stack_base[MAX_HBM_STACK_PER_SOCKET] = {
	HBM_STACK0_BASE_ADDR, HBM_STACK1_BASE_ADDR, HBM_STACK2_BASE_ADDR, HBM_STACK3_BASE_ADDR,
};

#define MSD_CTRL_WIN1_OFFSET 0x11000000000ULL
#define MSD_CTRL_WIN0_OFFSET 0x1000000000ULL
#define CHIP_OFFSET 0x80000000000ULL

#define HBM_ADDR_BANK_XOR_BIT   14U
#define STACKPC_ADDR_POS 9U
#define STACKPC_SRC_WIDTH 3U
#define HBMC_HIGH_ADDR_POS 12U
#define HBMC_LOW_ADDR_WITDH 9U
#define BA_WIDTH 2U
#define BG_WIDTH 2U
#define DMC_INTLV_XOR_START_POS 9U
#define DMC_INTLV_XOR_END_POS 29U
#define DMC_INTLV_XOR_UINT 3U
#define MSD_CONFIG_CHN_INTLV_4WAY 3U
#define MSD_CONFIG_CHN_INTLV_2WAY 1U

#define MAX_MATA_NUM 8U
#define MAX_MATACHN_NUM 4U

STATIC struct hbm_addr_trans_info g_addr_trans_info;

STATIC const uint32_t g_mata_chn_2_hbm_ch_map[MAX_MATA_NUM][MAX_MATACHN_NUM] = {
	{HBM0_CHN0, HBM0_CHN1, HBM0_CHN4, HBM0_CHN5},
	{HBM1_CHN7, HBM1_CHN6, HBM1_CHN3, HBM1_CHN2},
	{HBM0_CHN7, HBM0_CHN6, HBM0_CHN3, HBM0_CHN2},
	{HBM1_CHN0, HBM1_CHN1, HBM1_CHN4, HBM1_CHN5},
	{HBM2_CHN0, HBM2_CHN1, HBM2_CHN4, HBM2_CHN5},
	{HBM3_CHN7, HBM3_CHN6, HBM3_CHN3, HBM3_CHN2},
	{HBM2_CHN7, HBM2_CHN6, HBM2_CHN3, HBM2_CHN2},
	{HBM3_CHN0, HBM3_CHN1, HBM3_CHN4, HBM3_CHN5},
};
STATIC int32_t memory_get_hbm_base_addr(uint32_t dev_id, uint64_t *base);

STATIC void memory_hbm_init_trans_info(void)
{
	uint64_t base = 0;
	void __iomem *trams_info_virtaddr = NULL;
	int32_t ret;

	ret = memory_get_hbm_base_addr(0, &base);
	if (ret != 0) {
		memory_drv_err("get hbm base addr failed.(ret=%u)\n", ret);
		return;
	}

	trams_info_virtaddr = ioremap(base + HBM_ADDR_TRANS_INFO_START_ADDR, HBM_ADDR_TRANS_INFO_SIZE);
	if (trams_info_virtaddr == NULL) {
		memory_drv_err("sram_virtaddr ioremap failed\n");
		return;
	}

	ret = memcpy_s(&g_addr_trans_info, sizeof(struct hbm_addr_trans_info),
		trams_info_virtaddr, sizeof(struct hbm_addr_trans_info));
	if (ret != 0)
		memory_drv_err("memcpy_s failed ret = %d\n", ret);

	iounmap(trams_info_virtaddr);

	return;
}

STATIC uint64_t get_chip_die(uint32_t dev_id, uint64_t phy_addr, struct hbm_hw_addr_s *hbm_hw_addr)
{
	uint64_t sys_addr;
	int32_t ret;
	uint32_t chip_id = 0;
	uint32_t die_id = 0;
	uint64_t base;

	ret = devdrv_get_chip_die_id(dev_id, &chip_id, &die_id);	/* get chip id and die id by device id */
	if (ret != 0)
		memory_drv_err("get chip id and die id failed, dev id = %u\n", dev_id);

	hbm_hw_addr->chip = chip_id;
	hbm_hw_addr->die = die_id;

	if (phy_addr <= MSD7_WIN_SIZE)
		return phy_addr;

	ret = memory_get_adapter()->get_base_addr(dev_id, &base);
	if (ret != 0) {
		memory_drv_err("get hbm base addr failed. (dev_id=%u)\n", dev_id);
		return sys_addr;
	}

	sys_addr = phy_addr - base;
	if (sys_addr > MSD_CTRL_WIN1_OFFSET) {
		sys_addr -= MSD_CTRL_WIN1_OFFSET;
	} else if (sys_addr > MSD_CTRL_WIN0_OFFSET) {
		sys_addr -= MSD_CTRL_WIN0_OFFSET;
	}
	return sys_addr;
}

STATIC uint64_t get_stackpc_src(uint64_t hbmc_stackpc_addr)
{
	return ((hbmc_stackpc_addr >> STACKPC_ADDR_POS) & mask(STACKPC_SRC_WIDTH));
}

STATIC uint64_t get_hbmc_addr(uint64_t hbmc_stackpc_addr)
{
	return (hbmc_stackpc_addr & mask(HBMC_LOW_ADDR_WITDH)) |
		((hbmc_stackpc_addr >> HBMC_HIGH_ADDR_POS) << STACKPC_ADDR_POS);
}


STATIC uint64_t remove_bit(uint64_t addr, uint8_t is_remove, uint32_t pos, uint32_t width)
{
	uint64_t temp_addr = addr;
	if (is_remove != 0)
		temp_addr = (temp_addr & mask(pos)) | ((temp_addr >> (pos + width)) << pos);

	return temp_addr;
}

STATIC uint64_t get_hbmc_stackpc_addr(uint64_t sys_addr)
{
	uint64_t hbmc_stackpc_addr = sys_addr;
	uint32_t mata_bits;

	if (g_addr_trans_info.ch_intlv == MSD_CONFIG_CHN_INTLV_4WAY)
		mata_bits = 0x2; // 4-way, use 2 bits
	else if (g_addr_trans_info.ch_intlv == MSD_CONFIG_CHN_INTLV_2WAY)
		mata_bits = 0x1U; // 2-way, use 1 bits
	else // never run to here but for gtest
		mata_bits = 0x2U; // use 1 bits

	if (g_addr_trans_info.group_intlv_bit > g_addr_trans_info.dmc_intlv_bit) {
		hbmc_stackpc_addr = remove_bit(hbmc_stackpc_addr,
			((g_addr_trans_info.group_intlv == 0x1U) && (g_addr_trans_info.group_intlv_xor == 1U)),
			g_addr_trans_info.group_intlv_bit, 1);
		hbmc_stackpc_addr = remove_bit(hbmc_stackpc_addr,
			((g_addr_trans_info.dmc_intlv_xor == 1U) && (g_addr_trans_info.mata_intlv_xor == 1U)),
			g_addr_trans_info.dmc_intlv_bit, mata_bits);
	} else {
		hbmc_stackpc_addr = remove_bit(hbmc_stackpc_addr,
			((g_addr_trans_info.dmc_intlv_xor == 1U) && (g_addr_trans_info.mata_intlv_xor == 1U)),
			g_addr_trans_info.dmc_intlv_bit, mata_bits);
		hbmc_stackpc_addr = remove_bit(hbmc_stackpc_addr,
			((g_addr_trans_info.group_intlv == 0x1U) && (g_addr_trans_info.group_intlv_xor == 1U)),
			g_addr_trans_info.group_intlv_bit, 1);
	}
	return hbmc_stackpc_addr;
}

STATIC uint64_t get_mata_id(uint64_t hbmc_addr, uint64_t sys_addr, struct hbm_hw_addr_s *hbm_hw_addr)
{
	uint32_t mata_id;
	uint32_t mata_group = 0;
	uint32_t idx = 0;
	uint32_t remap_val;
	uint32_t mata_bits;
	uint32_t die = hbm_hw_addr->die & mask(1);
	uint32_t intlv_step = 4U;

	if (g_addr_trans_info.ch_intlv == MSD_CONFIG_CHN_INTLV_4WAY)
		mata_bits = 0x2; // 4-way, use 2 bits
	else if (g_addr_trans_info.ch_intlv == MSD_CONFIG_CHN_INTLV_2WAY) // 2 way all enable group intlv
		mata_bits = 0x1U; // 2-way, use 1 bits
	else // never run to here but for gtest
		mata_bits = 0x2U; // use 1 bits

	if ((g_addr_trans_info.group_intlv == 1U) && (g_addr_trans_info.group_intlv_xor == 1U))
		mata_group = (((sys_addr >> g_addr_trans_info.group_intlv_bit) & mask(1)) ^
			((sys_addr >> (g_addr_trans_info.group_intlv_bit + intlv_step)) & mask(1))) & mask(1);
	else
		mata_group = g_addr_trans_info.group_tgt & mask(1);

	if ((g_addr_trans_info.dmc_intlv_xor == 1U) && (g_addr_trans_info.mata_intlv_xor == 1U))
		idx = (((sys_addr >> g_addr_trans_info.dmc_intlv_bit) & mask(2)) ^
			((sys_addr >> (g_addr_trans_info.dmc_intlv_bit + intlv_step)) & mask(2))) & mask(2);

	remap_val = g_addr_trans_info.mata_id_remap[die][mata_group][idx];
	if (remap_val >= MAX_MATA_NUM_PER_GROUP)
		remap_val = 0;

	mata_id = remap_val * 2U + mata_group;
	return mata_id;
}

STATIC void get_stack_pc(uint64_t sys_addr, uint64_t hbmc_addr,
		uint64_t stackpc_src, struct hbm_hw_addr_s *hbm_hw_addr)
{
	uint32_t mata_id = get_mata_id(hbmc_addr, sys_addr, hbm_hw_addr) & mask(3);
	uint32_t mata_hbm_pc = 0;
	uint32_t mata_hbm_chn;
	uint32_t hbm_chn, loop;
	uint64_t xor_data = 0;
	uint64_t temp;

	if ((g_addr_trans_info.dmc_intlv_en == 0x7U) && (g_addr_trans_info.dmc_intlv_xor == 1U)) {
		temp = hbmc_addr >> DMC_INTLV_XOR_START_POS;
		for (loop = DMC_INTLV_XOR_START_POS; loop < DMC_INTLV_XOR_END_POS; loop += DMC_INTLV_XOR_UINT) {
			xor_data ^= temp & mask(DMC_INTLV_XOR_UINT);
			temp = temp >> DMC_INTLV_XOR_UINT;
		}
	} else {
		xor_data = 0;
	}

	mata_hbm_pc = stackpc_src ^ xor_data;
	mata_hbm_chn = (mata_hbm_pc / 2U) & mask(2);
	hbm_hw_addr->stack = (g_mata_chn_2_hbm_ch_map[mata_id][mata_hbm_chn] >> 4U) & mask(4);
	hbm_chn = g_mata_chn_2_hbm_ch_map[mata_id][mata_hbm_chn] & mask(4);
	hbm_hw_addr->pc = hbm_chn * 2U + (mata_hbm_pc % 2U);

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
	hbm_hw_addr->bank = hbm_hw_addr->bg << BA_WIDTH | hbm_hw_addr->ba;
	hbm_hw_addr->col = col_low | (col_high << g_addr_trans_info.col_width_l);
	return;
}

STATIC void memory_get_hbm_hwaddr(uint32_t dev_id, uint64_t phy_addr, struct hbm_hw_addr_s *hbm_hw_addr)
{
	uint64_t sys_addr, hbmc_stackpc_addr, hbmc_addr, stackpc_src;

	sys_addr = get_chip_die(dev_id, phy_addr, hbm_hw_addr);
	hbmc_stackpc_addr = get_hbmc_stackpc_addr(sys_addr);
	hbmc_addr = get_hbmc_addr(hbmc_stackpc_addr);
	stackpc_src = get_stackpc_src(hbmc_stackpc_addr);

	get_stack_pc(sys_addr, hbmc_addr, stackpc_src, hbm_hw_addr);
	get_unit_addr(hbmc_addr, hbm_hw_addr);

	return;
}

int32_t get_hbm_chip_die_offset(uint32_t dev_id, uint64_t *chip_base_addr, uint64_t *chip_offset,
	uint64_t *die_offset)
{
	int32_t connect_type;

	connect_type = devdrv_get_connect_protocol(dev_id);
	if ((connect_type !=  CONNECT_PROTOCOL_PCIE) && (connect_type != CONNECT_PROTOCOL_HCCS)) {
		return -EINVAL;
	}
	if (connect_type == CONNECT_PROTOCOL_PCIE) {
		*chip_base_addr = FULLMESH_CHIP_BASE_ADDR;
		*chip_offset = FULLMESH_CHIP_OFFSET;
		*die_offset = FULLMESH_DIE_OFFSET;
	} else if (connect_type == CONNECT_PROTOCOL_HCCS) {
		*chip_base_addr = HCCS_CHIP_BASE_ADDR;
		*chip_offset = HCCS_CHIP_OFFSET;
		*die_offset = HCCS_DIE_OFFSET;
	}

	return 0;
}

STATIC int32_t memory_get_hbm_base_addr(uint32_t dev_id, uint64_t *base)
{
	int32_t ret;
	uint32_t chip_id, die_id;
	uint64_t chip_base_addr, chip_offset, die_offset;

	ret = devdrv_get_chip_die_id(dev_id, &chip_id, &die_id);	/* get chip id and die id by device id */
	if (ret != 0) {
		memory_drv_err("get chip id and die id failed, dev id = %u\n", dev_id);
		return ret;
	}

	ret = get_hbm_chip_die_offset(dev_id, &chip_base_addr, &chip_offset, &die_offset);
	if (ret != 0) {
		memory_drv_err("get chip and die offset failed, dev id = %u\n", dev_id);
		return ret;
	}

	*base = chip_base_addr + (uint64_t)chip_id * chip_offset + (uint64_t)die_id * die_offset;

	return 0;
}

STATIC int memory_filter_addr(unsigned int devid, uint64_t physical_addr)
{
	int is_passed = 0;
	(void)devid;

	physical_addr &= DEVICE_OFFSET_MASK;

	// bios reserve space pass
	if (physical_addr >= BIOS_RESERVED_ZONE_START && physical_addr < BIOS_RESERVED_ZONE_END) {
		is_passed = 1;
	}
	// kernel space pass
	if (physical_addr >= KERNEL_ZONE_START) {
		is_passed = 1;
	}
	return is_passed;
}

STATIC int memory_get_chip_die_id(uint64_t addr, unsigned int *chip, unsigned int *die)
{
	int ret;
	unsigned int temp_die = (addr % HBM_PHYSIC_ADDR_CHIP_OFFSET) / HBM_PHYSIC_ADDR_DIE_OFFSET;
	unsigned int temp_chip = addr / HBM_PHYSIC_ADDR_CHIP_OFFSET;

	// addr belong to chip0 & die0 or belong to die0 & MSD7
	if (temp_die == 0 && temp_chip == 0) {
		ret = devdrv_get_chip_die_id(temp_die, &temp_chip, &temp_die);
		if (ret != 0) {
			memory_drv_err("get chip & die id fail.ret=%d\n", ret);
			return ret;
		}
	}
	*chip = temp_chip;
	*die = temp_die;

	return 0;
}

int32_t get_hbm_stack_base(uint32_t socket, uint32_t stack, uint64_t *base)
{
	int32_t ret;

	if (socket >= MAX_OS_DEVICE_COUNT || stack >= MAX_HBM_STACK_PER_SOCKET) {
		memory_drv_err("illegal socket %u or stack %u.\n", socket, stack);
		return -EINVAL;
	}
	ret = memory_get_hbm_base_addr(socket, base);
	if (ret != 0) {
		memory_drv_err("get hbm base addr failed.(ret=%u)\n", ret);
		return ret;
	}

	*base += g_hbm_stack_base[stack];

	return 0;
}

int32_t memory_check_mirror_addr(uint64_t addr, uint32_t dev_id)
{
	uint32_t idx;

	for (idx = 0; idx < MAX_MIRROR_WIN_NUM; idx++) {
		if (g_hbm_mirror_addr_cfg[dev_id].mirror_addr[idx].en == 0) {
			continue;
		}
		if ((addr >= g_hbm_mirror_addr_cfg[dev_id].mirror_addr[idx].start_addr) &&
			(addr < g_hbm_mirror_addr_cfg[dev_id].mirror_addr[idx].end_addr)) {
			return CUR_ADDR_IS_MIRROR_ADDR;
		}
	}

	return 0;
}

void hbm_mirror_cfg_iounmap(void)
{
	uint32_t dev_id;

	for (dev_id = 0; dev_id < MAX_OS_DEVICE_COUNT; dev_id++) {
		if (g_hbm_mirror_base[dev_id] != NULL) {
			iounmap(g_hbm_mirror_base[dev_id]);
			g_hbm_mirror_base[dev_id] = NULL;
		}
	}
}

STATIC int32_t hbm_mirror_cfg_ioremap(uint64_t base, uint32_t dev_id)
{
	g_hbm_mirror_base[dev_id] = ioremap(HBM_MIRROR_START_ADDR(base), HBM_MIRROR_SIZE);
	if (g_hbm_mirror_base[dev_id] == NULL) {
		memory_drv_err("ioremap mirror start addr failed. (dev_id=%u)", dev_id);
		return -ENOMEM;
	}

	return 0;
}

STATIC int32_t hbm_get_mirror_addr(uint64_t base, uint32_t dev_id)
{
	int32_t ret;
	uint32_t idx;
	uint64_t start_addr;
	uint64_t size;
	struct hbm_mirror_config hbm_mirror_cfg = {0};

	ret = memcpy_s((void *)(&hbm_mirror_cfg), sizeof(struct hbm_mirror_config),
		(void *)(g_hbm_mirror_base[dev_id]), sizeof(struct hbm_mirror_config));
	if (ret != 0) {
		memory_drv_err("memcpy mirror config failed. (dev_id=%u)", dev_id);
		return ret;
	}

	for (idx = 0; idx < MAX_MIRROR_WIN_NUM; idx++) {
		if (hbm_mirror_cfg.win[idx].en == 0) {
			continue;
		}
		g_hbm_mirror_addr_cfg[dev_id].mirror_addr[idx].en = hbm_mirror_cfg.win[idx].en;
		start_addr = (uint64_t)hbm_mirror_cfg.win[idx].addr_high << 32U;
		start_addr |= (uint64_t)hbm_mirror_cfg.win[idx].addr_low;
		if (start_addr >= (MSD_DIE0_BASEADDR + MSD7_WIN_SIZE)) {
			start_addr += base;
		}
		size = (uint64_t)hbm_mirror_cfg.win[idx].size_high << 32U;
		size |= (uint64_t)hbm_mirror_cfg.win[idx].size_low;
		g_hbm_mirror_addr_cfg[dev_id].mirror_addr[idx].start_addr = start_addr;
		g_hbm_mirror_addr_cfg[dev_id].mirror_addr[idx].end_addr = start_addr + size;
	}

	return 0;
}

int32_t memory_get_mirror_cfg(uint32_t dev_num)
{
	int32_t ret;
	uint32_t dev_id;
	uint64_t base;

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		ret = memory_get_adapter()->get_base_addr(dev_id, &base);
		if (ret != 0) {
			memory_drv_err("get hbm base addr failed. (dev_id=%u)\n", dev_id);
			goto FAIL;
		}

		ret = hbm_mirror_cfg_ioremap(base, dev_id);
		if (ret != 0) {
			memory_drv_err("hbm mirror cfg ioremap failed. (dev_id=%u)\n", dev_id);
			goto FAIL;
		}

		ret = hbm_get_mirror_addr(base, dev_id);
		if (ret != 0) {
			memory_drv_err("get mirror addr failed. (dev_id=%u)\n", dev_id);
			goto FAIL;
		}
	}

	return 0;

FAIL:
	hbm_mirror_cfg_iounmap();
	return ret;
}

static struct memory_adapter_t adapter = {
	.filter_addr = memory_filter_addr,
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
