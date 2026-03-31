/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
#include "lpm_devmng_common.h"
#include "lpm_devmng_debugfs.h"

STATIC struct lpm_debugfs_root *lpm_devmng_get_debugfs_root(void)
{
	static struct lpm_debugfs_root lpm_devmng_debugfs_root = {0};
	return &lpm_devmng_debugfs_root;
}

int32_t lpm_debugfs_register_module(struct lpm_debugfs_init_cfg *init_cfg)
{
	struct lpm_debugfs_root* debugfs_root = lpm_devmng_get_debugfs_root();
	return lpm_debugfs_create_file(debugfs_root, init_cfg);
}

int32_t lpm_devmng_debugfs_init(uint64_t *param, uint32_t param_num)
{
	struct lpm_debugfs_root* debugfs_root = lpm_devmng_get_debugfs_root();
	(void)param;
	(void)param_num;

	(void)memset_s(debugfs_root, sizeof(struct lpm_debugfs_root), 0, sizeof(struct lpm_debugfs_root));
	return lpm_debugfs_create("lpm_devmng", debugfs_root);
}

int32_t lpm_devmng_debugfs_exit(uint64_t *param, uint32_t param_num)
{
	struct lpm_debugfs_root* debugfs_root = lpm_devmng_get_debugfs_root();
	(void)param;
	(void)param_num;

	return lpm_debugfs_remove(debugfs_root);
}

#endif
