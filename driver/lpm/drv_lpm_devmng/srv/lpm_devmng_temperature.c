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
*/

#include <linux/securec.h>
#include <linux/uaccess.h>
#include "lpm_devmng_ipc.h"
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"
#endif
#include "lpm_devmng_temperature_drv.h"
#include "lpm_devmng_temperature.h"

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
STATIC int32_t lpm_temperature_debugfs_init(void);
#endif

STATIC int32_t lpm_temperature_check_soc_param(uint32_t dev_id, enum lpm_devmng_core_id core_id)
{
	if (!lpm_common_check_dev_id(dev_id)) {
		lpm_log_err("check soc max temperature query param failed, dev_id=%u error, core_id=%u\n",
			dev_id, (uint32_t)core_id);
		return -ENODEV;
	}

	if (core_id != LPM_SOC_ID) {
		lpm_log_err("check soc max temperature query param failed, dev_id=%u, core_id=%u error\n",
			dev_id, (uint32_t)core_id);
		return -EINVAL;
	}
	return 0;
}

int32_t lpm_temperature_query_soc(char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	int32_t ret;
	uint32_t pf_id = 0;
	uint32_t temperature = 0;
	struct lpm_devmng_dsmi_cfg_in *lpm_info = NULL;
	struct lpm_devmng_dsmi_in_param in_param;

	in_param.in              = in;
	in_param.in_len          = in_len;
	in_param.correct_in_len  = (uint32_t)sizeof(struct lpm_devmng_dsmi_cfg_in);
	in_param.out             = out;
	in_param.out_len         = out_len;
	in_param.correct_out_len = (uint32_t)sizeof(int32_t);

	if (!lpm_common_check_dsmi_in_param(&in_param)) {
		lpm_log_err("lpm temperature query para check failed\n");
		return -EINVAL;
	}

	lpm_info = (struct lpm_devmng_dsmi_cfg_in *)in;

	ret = lpm_common_trans_logical_id_to_pf_id(lpm_info->dev_id, &pf_id);
	if (ret != 0) {
		lpm_log_err("lpm transform logical dev_id to pf_id failed when query soc temperature, "
			"dev_id=%u, ret=%d\n",
			lpm_info->dev_id, ret);
		return ret;
	}

	ret = lpm_temperature_check_soc_param(pf_id, (enum lpm_devmng_core_id)lpm_info->core_id);
	if (ret != 0) {
		lpm_log_err("lpm temperature query check param failed, dev_id=%u, pf_id=%u, core_id=%u, ret=%d\n",
			lpm_info->dev_id, pf_id, lpm_info->core_id, ret);
		return ret;
	}

	ret = lpm_temperature_drv_get_soc_temp(
		pf_id, (enum lpm_devmng_core_id)lpm_info->core_id, &temperature);
	if (ret != 0) {
		lpm_log_err("lpm get soc temperature failed, dev_id=%u, pf_id=%u, core_id=%u, ret=%d\n",
			lpm_info->dev_id, pf_id, lpm_info->core_id, ret);
		return ret;
	}

	*(int32_t *)out = (int32_t)temperature;
	return 0;
}

int32_t lpm_temperature_probe(uint64_t *param, uint32_t param_num)
{
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	int32_t ret;
	ret = lpm_temperature_debugfs_init();
	if (ret != 0) {
		lpm_log_err("lpm idle init debugfs failed, ret=%d\n", ret);
		return ret;
	}
#endif
	return lpm_temperature_drv_probe(param, param_num);
}

int32_t lpm_temperature_remove(uint64_t *param, uint32_t param_num)
{
	return lpm_temperature_drv_remove(param, param_num);
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)

STATIC int32_t lpm_temperature_debugfs_show(struct seq_file *seq, void *v)
{
	return lpm_temperature_drv_debugfs_show(seq, v);
}

STATIC int32_t lpm_temperature_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, lpm_temperature_debugfs_show, inode->i_private);
}

STATIC int32_t lpm_temperature_debugfs_init(void)
{
	int32_t ret;
	struct lpm_debugfs_init_cfg init_cfg = {
		.name = "temperature",
		.fn_open = lpm_temperature_debugfs_open,
		.fn_write = NULL
	};

	ret = lpm_debugfs_register_module(&init_cfg);
	if (ret != 0) {
		lpm_log_err("lpm temperature register debugfs failed, ret=%d\n", ret);
		return ret;
	}
	return 0;
}

#endif
