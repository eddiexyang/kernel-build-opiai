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
*/

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include <linux/kernel.h>
#include "lpm_fault_common.h"
#include "lpm_fault_debugfs.h"

STATIC struct lpm_debugfs_root* lpm_fault_get_debugfs_root(void)
{
	static struct lpm_debugfs_root lpm_fault_debugfs_root = {0};
	return &lpm_fault_debugfs_root;
}

int32_t lpm_debugfs_register_module(struct lpm_debugfs_init_cfg *init_cfg)
{
	struct lpm_debugfs_root* debugfs_root = lpm_fault_get_debugfs_root();
	return lpm_debugfs_create_file(debugfs_root, init_cfg);
}

int32_t lpm_fault_debugfs_init(uint32_t dev_num)
{
	struct lpm_debugfs_root *debugfs_root = lpm_fault_get_debugfs_root();
	(void)dev_num;

	(void)memset_s(debugfs_root, sizeof(struct lpm_debugfs_root), 0, sizeof(struct lpm_debugfs_root));
	return lpm_debugfs_create("lpm_fault", debugfs_root);
}

int32_t lpm_fault_debugfs_exit(uint32_t dev_num)
{
	struct lpm_debugfs_root* debugfs_root = lpm_fault_get_debugfs_root();
	(void)dev_num;

	return lpm_debugfs_remove(debugfs_root);
}
#endif