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

#include <linux/seq_file.h>
#include <linux/securec.h>
#include <linux/uaccess.h>
#include "lpm_devmng_sharemem_drv.h"
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"
#endif
#include "lpm_devmng_aic_freq_drv.h"
#include "lpm_devmng_aic_freq.h"

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
STATIC int32_t lpm_aic_freq_debugfs_init(void);
#endif

int32_t lpm_aic_freq_get_adj_cause(
	uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	unsigned long cpy_len;
	int32_t ret;
	struct lpm_aic_freq_adj_cause adj_cause = {0};

	ret = lpm_check_dsmi_get_param(
		dev_id, in, in_len, (uint32_t)sizeof(adj_cause.reason), out_len);
	if (ret != 0) {
		lpm_log_err("get aic freq adj cause, check param failed, ret=%d\n", ret);
		return -EINVAL;
	}

	// read from sharemem
	ret = lpm_get_adj_cause_from_sharemem(
		dev_id, LPM_ADJ_REDUCE_READ_RETRY_CNT, LPM_ADJ_REDUCE_READ_WAIT_TIME, &adj_cause);
	if (ret != 0) {
		lpm_log_err("get aic freq adj cause, query failed, ret=%d, dev_id=%u\n", ret, dev_id);
		return -EINVAL;
	}

	cpy_len = copy_to_user(in, (void *)&adj_cause.reason, sizeof(adj_cause.reason));
	if (cpy_len != 0) {
		lpm_log_err("get aic freq adj cause, copy to user failed, "
			"cpy_len=%lu, reason=0x%llx, sys_time=0x%llx, utc_time=0x%llx\n",
			cpy_len, adj_cause.reason, adj_cause.sys_time, adj_cause.utc_time);
		return -EIO;
	}
	*out_len = (uint32_t)sizeof(adj_cause.reason);

	return 0;
}

int32_t lpm_aic_freq_get_syscnt_info(uint32_t dev_id, void *out, uint32_t out_len, uint32_t *result_len)
{
	return lpm_aic_freq_drv_get_syscnt_info(dev_id, out, out_len, result_len);
}

int32_t lpm_aic_freq_probe(uint64_t *param, uint32_t param_num)
{
	int32_t ret;

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	ret = lpm_aic_freq_debugfs_init();
	if (ret != 0) {
		lpm_log_err("lpm aic freq init debugfs failed, ret=%d\n", ret);
		return ret;
	}
#endif

	ret = lpm_aic_freq_drv_probe(param, param_num);
	if (ret != 0) {
		lpm_log_err("lpm aic freq drv probe failed, ret=%d\n", ret);
		return ret;
	}

	lpm_log_info("lpm aic freq probe success\n");
	return 0;
}

int32_t lpm_aic_freq_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	lpm_log_info("lpm aic freq remove success\n");
	return 0;
}

void lpm_aic_freq_post_suspend(void)
{
	lpm_aic_freq_drv_post_suspend();
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
STATIC void lpm_aic_freq_debug_show_info(struct seq_file *seq)
{
	struct lpm_aic_freq_adj_cause adj_cause = {0};
	uint32_t dev_id;
	uint32_t dev_num = lpm_common_get_dev_num();
	int32_t ret;

	seq_printf(seq, "[dev_num=%u]\n", dev_num);
	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		seq_printf(seq, " [dev_id=%u]\n", dev_id);

		// read from sharemem
		ret = lpm_get_adj_cause_from_sharemem(
			dev_id, LPM_ADJ_REDUCE_READ_RETRY_CNT, LPM_ADJ_REDUCE_READ_WAIT_TIME, &adj_cause);
		if (ret != 0) {
			seq_printf(seq, "  query aic freq adj cause failed, ret=%d\n", ret);
			continue;
		}

		seq_printf(seq, "   adj_cause=0x%llx, sys_time=0x%llx, utc_time=0x%llx\n",
			adj_cause.reason, adj_cause.sys_time, adj_cause.utc_time);
	}
}

STATIC ssize_t lpm_aic_freq_debugfs_write(
	struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
	return lpm_aic_freq_drv_debugfs_write(file, buf, len, pos);
}

STATIC int32_t lpm_aic_freq_debugfs_show(struct seq_file *seq, void *v)
{
	(void)v;
	lpm_aic_freq_debug_show_info(seq);
	lpm_aic_freq_drv_debugfs_show(seq);
	return 0;
}

STATIC int32_t lpm_aic_freq_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, lpm_aic_freq_debugfs_show, inode->i_private);
}

STATIC int32_t lpm_aic_freq_debugfs_init(void)
{
	int32_t ret;
	struct lpm_debugfs_init_cfg init_cfg = {
		.name = "aic_freq",
		.fn_open = lpm_aic_freq_debugfs_open,
		.fn_write = lpm_aic_freq_debugfs_write
	};

	ret = lpm_debugfs_register_module(&init_cfg);
	if (ret != 0) {
		lpm_log_err("lpm aic freq register debugfs failed, ret=%d\n", ret);
		return ret;
	}
	return 0;
}
#endif
