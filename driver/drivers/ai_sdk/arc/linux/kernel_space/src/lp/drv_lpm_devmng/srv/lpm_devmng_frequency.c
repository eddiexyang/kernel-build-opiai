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
#include "lpm_devmng_common.h"
#include "lpm_devmng_frequency_drv.h"
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"
#endif
#include "lpm_devmng_frequency.h"

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
STATIC int32_t lpm_frequency_debugfs_init(void);
#endif

int32_t lpm_frequency_query(char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	int32_t ret;
	uint32_t pf_id;
	uint32_t frequency = 0;
	struct lpm_devmng_dsmi_cfg_in *lpm_info = NULL;
	struct lpm_devmng_dsmi_in_param in_param;

	in_param.in              = in;
	in_param.in_len          = in_len;
	in_param.correct_in_len  = (uint32_t)sizeof(struct lpm_devmng_dsmi_cfg_in);
	in_param.out             = out;
	in_param.out_len         = out_len;
	in_param.correct_out_len = (uint32_t)sizeof(uint32_t);

	if (!lpm_common_check_dsmi_in_param(&in_param)) {
		lpm_log_err("lpm frequency query para check failed\n");
		return -EINVAL;
	}

	lpm_info = (struct lpm_devmng_dsmi_cfg_in *)in;

	ret = lpm_common_trans_logical_id_to_pf_id(lpm_info->dev_id, &pf_id);
	if (ret != 0) {
		lpm_log_err("lpm transform logical dev_id to pf_id failed, dev_id=%u, ret=%d\n",
			lpm_info->dev_id, ret);
		return ret;
	}

	ret = lpm_common_check_dev_and_core(pf_id, lpm_info->core_id);
	if (ret != 0) {
		lpm_log_err("lpm frequency query check dev_id and core_id failed\n");
		return ret;
	}

	ret = lpm_frequency_drv_query(pf_id, lpm_info->core_id, &frequency);
	if (ret != 0) {
		if (ret != -EOPNOTSUPP) {
			lpm_log_err("lpm query frequency failed, dev_id=%u, pf_id=%u, core_id=%u, ret=%d\n",
				lpm_info->dev_id, pf_id, lpm_info->core_id, ret);
		}
		return ret;
	}

	*(uint32_t *)out = frequency;
	return 0;
}

int32_t lpm_frequency_probe(uint64_t *param, uint32_t param_num)
{
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	int32_t ret = lpm_frequency_debugfs_init();
	if (ret != 0) {
		lpm_log_err("lpm frequency init debugfs failed, ret=%d\n", ret);
		return ret;
	}
#endif
	return lpm_frequency_drv_probe(param, param_num);
}

int32_t lpm_frequency_remove(uint64_t *param, uint32_t param_num)
{
	return lpm_frequency_drv_remove(param, param_num);
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)

STATIC int32_t lpm_frequency_debugfs_show(struct seq_file *seq, void *v)
{
	return lpm_frequency_drv_debugfs_show(seq, v);
}

STATIC int32_t lpm_frequency_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, lpm_frequency_debugfs_show, inode->i_private);
}

STATIC int32_t lpm_frequency_debugfs_init(void)
{
	int32_t ret;
	struct lpm_debugfs_init_cfg init_cfg = {
		.name = "frequency",
		.fn_open = lpm_frequency_debugfs_open,
		.fn_write = NULL
	};

	ret = lpm_debugfs_register_module(&init_cfg);
	if (ret != 0) {
		lpm_log_err("lpm frequency register debugfs failed, ret=%d\n", ret);
		return ret;
	}
	return 0;
}

#endif
