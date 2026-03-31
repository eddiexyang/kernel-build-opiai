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
#include "lpm_devmng_ipc.h"
#include "lpm_devmng_sharemem_drv.h"
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"
#endif
#include "lpm_devmng_idle_drv.h"
#include "lpm_devmng_idle_ascend910b.h"

STATIC bool lpm_idle_drv_cfg_para_check(uint32_t dev_id, struct lpm_idle_dsmi_cfg_in *in_param)
{
	if (!lpm_common_check_dev_id(dev_id)) {
		lpm_log_err("check cfg idle switch failed, dev_id=%u is out of range\n", dev_id);
		return false;
	}

	// 0: close, 1: open
	if (in_param->idle_switch > 0x1) {
		lpm_log_err("dev_id=%u check idle switch failed, idle_switch=%u is out of range\n",
			dev_id, in_param->idle_switch);
		return false;
	}

	return true;
}

STATIC int32_t lpm_idle_drv_ipc_send_cfg(uint32_t dev_id, uint32_t idle_switch)
{
	struct lpm_idle_switch_ipc_send send_data = {0};
	struct lpm_devmng_ipc_msg ipc_msg;
	int32_t ret;

	send_data.idle_switch = idle_switch;
	ipc_msg.send_type     = LPM_IPC_SET_IDLE;
	ipc_msg.in            = (void *)&send_data;
	ipc_msg.in_len        = (uint32_t)sizeof(send_data);
	ipc_msg.out           = NULL;
	ipc_msg.out_pos       = 0;
	ipc_msg.out_len       = 0;

	// out is used to point to ack, async ipc no need care about out
	// user determines whether the config is successful
	// by paying attention to the frequency of lp,
	// so here the ipc does not pay attention to ack
	ret = lpm_ipc_send_async_msg(dev_id, &ipc_msg);
	if (ret != 0) {
		lpm_log_err("lpm idle set switch send ipc failed, ret=%d, idle_switch=%u\n", ret, idle_switch);
		return ret;
	}

	return 0;
}

int32_t lpm_idle_drv_set_switch(uint32_t dev_id, struct lpm_idle_dsmi_cfg_in *in_param)
{
	int32_t ret;

	// check the legality of input parameters
	if (!lpm_idle_drv_cfg_para_check(dev_id, in_param)) {
		lpm_log_err("lpm idle set switch para check failed\n");
		return -EINVAL;
	}

	// notify lp to modify the config through ipc
	ret = lpm_idle_drv_ipc_send_cfg(dev_id, in_param->idle_switch);
	if (ret != 0) {
		lpm_log_err("lpm idle set switch send ipc failed, ret=%d\n", ret);
		return ret;
	}

	lpm_log_info("lpm idle set switch success, dev_id=%u, idle_switch=%u\n",
		dev_id, in_param->idle_switch);
	return 0;
}

int32_t lpm_idle_drv_probe(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	lpm_log_info("lpm idle probe success\n");
	return 0;
}

int32_t lpm_idle_drv_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	lpm_log_info("lpm idle remove success\n");
	return 0;
}

int32_t lpm_idle_drv_suspend_prepare(void)
{
	// do nothing
	return 0;
}
int32_t lpm_idle_drv_post_suspend(void)
{
	// do nothing
	return 0;
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)

STATIC void lpm_idle_drv_debugfs_set_config(uint32_t *user_cfg, uint32_t param_num)
{
	int32_t ret;
	struct lpm_idle_dsmi_cfg_in in_param = {0};

	// user_cfg[0] means dev_id
	// user_cfg[1] means idle switch, 0: close, 1:open
	in_param.idle_switch = user_cfg[1];
	ret = lpm_idle_drv_set_switch(user_cfg[0], &in_param);
	if (ret != 0) {
		lpm_log_err("set idle switch failed, ret=%d\n", ret);
		return;
	}

	lpm_log_info("set idle switch success, idle_switch=%u\n", in_param.idle_switch);
}

ssize_t lpm_idle_drv_debugfs_write(
	struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
	int32_t ret;
	uint32_t user_cfg[LPM_DEBUGFS_MAX_USER_INPUT_NUM] = {0};

	(void)file;
	(void)pos;

	// dev_id, idle_switch
	ret = lpm_debugfs_get_input(buf, len, user_cfg, 0x2);
	if (ret != 0) {
		lpm_log_err("debugfs write idle failed, ret=%d\n", ret);
		return -1;
	}

	lpm_idle_drv_debugfs_set_config(user_cfg, 0x2);
	return (ssize_t)len;
}

int32_t lpm_idle_drv_debugfs_show(struct seq_file *seq, void *v)
{
	(void)v;
	(void)seq;
	return 0;
}
#endif
