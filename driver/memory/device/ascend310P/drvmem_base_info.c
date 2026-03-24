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

STATIC int memory_filter_addr(unsigned int devid, uint64_t physical_addr)
{
	(void)devid;
	(void)physical_addr;

	// never filter
	return 1;
}

STATIC int memory_get_chip_die_id(uint64_t addr, unsigned int *chip, unsigned int *die)
{
	(void)addr;

	*chip = 0x0U; // always set zero
	*die = (addr < DDR_MSD_DIE1_OFFSET) ? 0x0 : 0x1U;

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
	.filter_addr = memory_filter_addr,
	.get_chip_die = memory_get_chip_die_id,
	.get_base_addr = NULL,
	.get_mirror_cfg = memory_get_mirror_cfg,
	.check_mirror_addr = memory_check_mirror_addr,
	.hbm_init_trans_info = NULL,
	.get_hbm_hwaddr = NULL
};

struct memory_adapter_t *memory_get_adapter(void)
{
	return &adapter;
}
