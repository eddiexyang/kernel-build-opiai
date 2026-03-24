/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
* Create: 2023-4-25
*/
#ifndef MEMORY_ADAPTER_H
#define MEMORY_ADAPTER_H

#include <linux/types.h>

struct hbm_hw_addr_s {
	uint32_t chip;
	uint32_t die;
	uint32_t stack;
	uint32_t pc;
	uint32_t sid;
	uint32_t ba;
	uint32_t bg;
	uint32_t row;
	uint32_t col;
	uint32_t bank;
	uint32_t module_id;
};

struct memory_adapter_t {
	int (*get_chip_die)(uint64_t addr, unsigned int *chip, unsigned int *die);
	int (*filter_addr)(unsigned int devid, uint64_t physical_addr);
	int (*get_base_addr)(uint32_t dev_id, uint64_t *base);
	int (*get_mirror_cfg)(uint32_t dev_num);
	int (*check_mirror_addr)(uint64_t addr, uint32_t dev_id);
	void (*hbm_init_trans_info)(void);
	void (*get_hbm_hwaddr)(uint32_t dev_id, uint64_t phy_addr, struct hbm_hw_addr_s *hbm_hw_addr);
};

struct memory_adapter_t *memory_get_adapter(void);

#endif

