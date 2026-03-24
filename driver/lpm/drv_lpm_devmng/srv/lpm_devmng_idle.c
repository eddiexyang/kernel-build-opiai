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

#include <linux/of.h>
#include <linux/io.h>
#include <linux/securec.h>
#include <linux/uaccess.h>
#include "lpm_devmng_sharemem_drv.h"
#include "lpm_devmng_idle_drv.h"
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"
#endif
#include "lpm_devmng_idle.h"

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
STATIC int32_t lpm_idle_debugfs_init(void);
#endif

STATIC int32_t lpm_idle_get_dsmi_input(
	const char __user *in, uint32_t in_len, struct lpm_idle_dsmi_cfg_in *para)
{
	int32_t ret;

	if ((in == NULL) || (in_len != sizeof(struct lpm_idle_dsmi_cfg_in))) {
		lpm_log_err("get idle input failed, input char is NULL or in_len is wrong, in_len=%u\n", in_len);
		return -EINVAL;
	}

	// in points to user space
	ret = copy_from_user((void *)para, (const void *)in, in_len);
	if (ret != 0) {
		lpm_log_err("get idle input copy_from_user failed, ret=%d, in_len=%u\n",
			ret, in_len);
		return -EIO;
	}

	return 0;
}

STATIC int32_t lpm_idle_query_status(uint32_t dev_id, uint32_t *idle_status)
{
	int32_t ret;
	struct lpm_sys_status_info sys_status = {0};

	// read sys status from sharemem
	ret = lpm_get_system_status_from_sharemem(dev_id, &sys_status);
	if (ret != 0) {
		lpm_log_err("dev_id=%u query idle status failed, ret=%d\n", dev_id, ret);
		return ret;
	}

	// 0: busy, 1:idle
	// for miniv3:
	// when aicore, CPU, and dvpp enter the idle state,
	// the result of the DSMI interface query is the idle state
	// the value of the idle state reg read by the Taishan is different from the result read by lp,
	// so the idle state is written by lp to the sharememory
	*idle_status = (sys_status.idle_status == 0) ? (uint32_t)0x0 : (uint32_t)0x1;

	return 0;
}

int32_t lpm_idle_get_status(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	unsigned long cpy_len;
	int32_t ret;
	uint32_t idle_status = 0; // 1:means idle, 0: means not idle

	ret = lpm_check_dsmi_get_param(dev_id, in, in_len, (uint32_t)sizeof(idle_status), out_len);
	if (ret != 0) {
		lpm_log_err("get idle status failed, para check failed, ret=%d\n", ret);
		return -EINVAL;
	}

	ret = lpm_idle_query_status(dev_id, &idle_status);
	if (ret != 0) {
		lpm_log_err("lpm idle query ring status failed, ret=%d\n", ret);
		return ret;
	}

	cpy_len = copy_to_user(in, (void *)&idle_status, sizeof(idle_status));
	if (cpy_len != 0) {
		lpm_log_err("get idle status, copy to user failed, cpy_len=%lu, idle_status=%u\n",
			cpy_len, idle_status);
		return -EIO;
	}
	*out_len = (uint32_t)sizeof(idle_status);

	return 0;
}

int32_t lpm_idle_set_switch(uint32_t dev_id, const char __user *in, uint32_t in_len)
{
	int32_t ret;
	struct lpm_idle_dsmi_cfg_in para = {0};

	// convert dsmi input information according to the specified format
	ret = lpm_idle_get_dsmi_input(in, in_len, &para);
	if (ret != 0) {
		lpm_log_err("lpm idle set switch, get input failed, ret=%d\n", ret);
		return -EINVAL;
	}

	// configure based on user input
	ret = lpm_idle_drv_set_switch(dev_id, &para);
	if (ret != 0) {
		if (ret != -EOPNOTSUPP) {
			lpm_log_err("lpm idle set switch failed, ret=%d, dev_id=%u, idle_switch=%u\n",
				ret, dev_id, para.idle_switch);
		}
		return ret;
	}

	lpm_log_info("lpm idle set switch success, dev_id=%u, idle_switch=%u\n",
		dev_id, para.idle_switch);

	return 0;
}

int32_t lpm_idle_probe(uint64_t *param, uint32_t param_num)
{
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	int32_t ret;
	ret = lpm_idle_debugfs_init();
	if (ret != 0) {
		lpm_log_err("lpm idle init debugfs failed, ret=%d\n", ret);
		return ret;
	}
#endif
	return lpm_idle_drv_probe(param, param_num);
}

int32_t lpm_idle_remove(uint64_t *param, uint32_t param_num)
{
	return lpm_idle_drv_remove(param, param_num);
}

int32_t lpm_idle_suspend_prepare(void)
{
	return lpm_idle_drv_suspend_prepare();
}

int32_t lpm_idle_post_suspend(void)
{
	return lpm_idle_drv_post_suspend();
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)

STATIC ssize_t lpm_idle_debugfs_write(
	struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
	return lpm_idle_drv_debugfs_write(file, buf, len, pos);
}

STATIC int32_t lpm_idle_debugfs_show(struct seq_file *seq, void *v)
{
	return lpm_idle_drv_debugfs_show(seq, v);
}

STATIC int32_t lpm_idle_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, lpm_idle_debugfs_show, inode->i_private);
}

STATIC int32_t lpm_idle_debugfs_init(void)
{
	int32_t ret;
	struct lpm_debugfs_init_cfg init_cfg = {
		.name = "idle",
		.fn_open = lpm_idle_debugfs_open,
		.fn_write = lpm_idle_debugfs_write
	};

	ret = lpm_debugfs_register_module(&init_cfg);
	if (ret != 0) {
		lpm_log_err("lpm idle register debugfs failed, ret=%d\n", ret);
		return ret;
	}
	return 0;
}

#endif