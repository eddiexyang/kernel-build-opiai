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
#include <linux/types.h>
#include "lpm_devmng_common.h"
#include "lpm_devmng_sharemem_drv.h"
#include "lpm_devmng_health_status.h"

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"

STATIC int32_t lpm_health_status_debugfs_init(void);
#endif

STATIC int32_t lpm_health_status_check_soc_param(uint32_t dev_id, enum lpm_devmng_core_id core_id)
{
	if (!lpm_common_check_dev_id(dev_id)) {
		lpm_log_err("Check get health status param failed, dev_id=%u error, core_id=%u\n",
			dev_id, (uint32_t)core_id);
		return -ENODEV;
	}

	if (core_id != LPM_SOC_ID) {
		lpm_log_err("Check get health status param failed, dev_id=%u, core_id=%u error\n",
			dev_id, (uint32_t)core_id);
		return -EINVAL;
	}

	return 0;
}

STATIC int32_t lpm_devmng_get_health_status(uint32_t dev_id, struct lpm_health_status *health_status)
{
	int32_t ret;
	struct lpm_sys_status_info sys_status = {0};

	ret = lpm_get_system_status_from_sharemem(dev_id, &sys_status);
	if (ret != 0) {
		lpm_log_err("get lp health_status from sharemem failed, dev_id=%u ret=%d\n", dev_id, ret);
		return ret;
	}

	health_status->status = sys_status.health_status;
	health_status->status_info = sys_status.health_status_info;
	return 0;
}

int32_t lpm_get_health_status(char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	int32_t ret;
	uint32_t pf_id = 0;
	struct lpm_health_status health_status = {0};
	struct lpm_devmng_dsmi_cfg_in *lpm_info = NULL;
	struct lpm_devmng_dsmi_in_param in_param;

	in_param.in              = in;
	in_param.in_len          = in_len;
	in_param.correct_in_len  = (uint32_t)sizeof(struct lpm_devmng_dsmi_cfg_in);
	in_param.out             = out;
	in_param.out_len         = out_len;
	in_param.correct_out_len = (uint32_t)sizeof(struct lpm_health_status);

	if (!lpm_common_check_dsmi_in_param(&in_param)) {
		lpm_log_err("lpm health status query para check failed\n");
		return -EINVAL;
	}

	lpm_info = (struct lpm_devmng_dsmi_cfg_in *)in;
	ret = lpm_common_trans_logical_id_to_pf_id(lpm_info->dev_id, &pf_id);
	if (ret != 0) {
		lpm_log_err("lpm transform logical dev_id to pf_id failed when query health status, "
			"dev_id=%u, ret=%d\n", lpm_info->dev_id, ret);
		return ret;
	}

	ret = lpm_health_status_check_soc_param(pf_id, (enum lpm_devmng_core_id)lpm_info->core_id);
	if (ret != 0) {
		lpm_log_err("lpm health status query check param failed, dev_id=%u, pf_id=%u, core_id=%u, ret=%d\n",
			lpm_info->dev_id, pf_id, lpm_info->core_id, ret);
		return ret;
	}

	ret = lpm_devmng_get_health_status(pf_id, &health_status);
	if (ret != 0) {
		lpm_log_err("lpm get health status out failed, ret=%d\n", ret);
		return ret;
	}

	*(struct lpm_health_status *)out = health_status;
	return 0;
}

int32_t lpm_health_status_probe(uint64_t *param, uint32_t param_num)
{
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	int32_t ret = lpm_health_status_debugfs_init();
	if (ret != 0) {
		lpm_log_err("lpm health_status init debugfs failed, ret=%d\n", ret);
		return ret;
	}
#endif
	(void)param;
	(void)param_num;
	return 0;
}

int32_t lpm_health_status_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)

STATIC int32_t lpm_health_status_debugfs_show(struct seq_file *seq, void *v)
{
	int32_t ret;
	uint32_t dev_id;
	struct lpm_health_status health_status = {0};
	uint32_t dev_num = lpm_common_get_dev_num();

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		ret = lpm_devmng_get_health_status(dev_id, &health_status);
		if (ret != 0) {
			seq_printf(seq, "dev_id=%u get health status failed\n", dev_id);
		} else {
			seq_printf(seq, "dev_id=%u health status=%u\n", dev_id, health_status.status);
		}
	}

	(void)v;
	return 0;
}

STATIC int32_t lpm_health_status_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, lpm_health_status_debugfs_show, inode->i_private);
}

STATIC int32_t lpm_health_status_debugfs_init(void)
{
	int32_t ret;
	struct lpm_debugfs_init_cfg init_cfg = {
		.name = "health_status",
		.fn_open = lpm_health_status_debugfs_open,
		.fn_write = NULL
	};

	ret = lpm_debugfs_register_module(&init_cfg);
	if (ret != 0) {
		lpm_log_err("lpm health_status register debugfs failed, ret=%d\n", ret);
		return ret;
	}

	return 0;
}

#endif