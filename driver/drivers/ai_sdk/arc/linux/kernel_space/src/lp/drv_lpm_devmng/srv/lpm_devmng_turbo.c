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
#include "lpm_devmng_frequency_drv.h"
#include "lpm_devmng_turbo.h"

#ifdef LPM_BUILD_DEBUG

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
STATIC int32_t lpm_turbo_debugfs_init(void);
#endif


STATIC bool lpm_turbo_cfg_para_check(uint32_t dev_id, struct lpm_turbo_dsmi_cfg_in *in_cfg)
{
	if (!lpm_common_check_dev_id(dev_id)) {
		lpm_log_err("check cfg turbo failed, dev_id=%u is out of range\n", dev_id);
		return false;
	}

	// 0: close, 1: open
	if (in_cfg->enable > 0x1) {
		lpm_log_err("dev_id=%u check cfg turbo failed, enable=%u is out of range\n",
			dev_id, in_cfg->enable);
		return false;
	}

	return true;
}

STATIC int32_t lpm_turbo_ipc_send_cfg(uint32_t dev_id, uint32_t enable)
{
	struct lpm_turbo_ipc_send send_data = {0};
	struct lpm_devmng_ipc_msg ipc_msg;
	int32_t ret;

	send_data.enable  = enable;
	ipc_msg.send_type = LPM_IPC_SET_TURBO;
	ipc_msg.in        = (void *)&send_data;
	ipc_msg.in_len    = (uint32_t)sizeof(send_data);
	ipc_msg.out       = NULL;
	ipc_msg.out_pos   = 0;
	ipc_msg.out_len   = 0;

	// out is used to point to ack, async ipc no need care about out
	// user determines whether the config is successful
	// by paying attention to the frequency of lp,
	// so here the ipc does not pay attention to ack
	ret = lpm_ipc_send_async_msg(dev_id, &ipc_msg);
	if (ret != 0) {
		lpm_log_err("lpm turbo send ipc failed, ret=%d, enable=%u\n", ret, enable);
		return ret;
	}

	return 0;
}

STATIC int32_t lpm_turbo_change_mode(uint32_t dev_id, struct lpm_turbo_dsmi_cfg_in *in_cfg)
{
	int32_t ret;

	// check the legality of input parameters
	if (!lpm_turbo_cfg_para_check(dev_id, in_cfg)) {
		lpm_log_err("lpm turbo set para check failed\n");
		return -EINVAL;
	}

	// notify lp to modify the config through ipc
	ret = lpm_turbo_ipc_send_cfg(dev_id, in_cfg->enable);
	if (ret != 0) {
		lpm_log_err("lpm turbo set send ipc failed, ret=%d\n", ret);
		return ret;
	}

	lpm_log_info("lpm turbo set success, dev_id=%u, enable=%u\n", dev_id, in_cfg->enable);
	return 0;
}

STATIC int32_t lpm_turbo_check_max_freq_param(uint32_t dev_id, uint32_t max_freq)
{
	int32_t ret;
	uint32_t min_freq_th = 0;
	uint32_t max_freq_th = 0;

	if (!lpm_common_check_dev_id(dev_id)) {
		lpm_log_err("check max freq param failed, dev_id=%u is out of range, max_freq=%u\n",
			dev_id, max_freq);
		return -EINVAL;
	}

	ret = lpm_frequency_get_aic_freq_range(dev_id, &min_freq_th, &max_freq_th);
	if (ret != 0) {
		lpm_log_err("check max freq param failed when get aic freq range, dev_id=%u, max_freq=%u\n",
			dev_id, max_freq);
		return -EIO;
	}

	if ((max_freq < min_freq_th) || (max_freq > max_freq_th)) {
		lpm_log_err("check max freq param failed, dev_id=%u, max freq=%u, freq range=[%u, %u]\n",
			dev_id, max_freq, min_freq_th, max_freq_th);
		return -EINVAL;
	}

	return 0;
}

STATIC int32_t lpm_turbo_send_set_max_aic_freq_ipc(uint32_t dev_id, uint32_t max_freq)
{
	int32_t ret;
	struct lpm_turbo_set_max_freq_send_ipc send_data = {0};
	struct lpm_turbo_set_max_freq_ack_ipc ack_data = {0};
	struct lpm_devmng_ipc_msg msg_info;

	send_data.max_freq = (uint16_t)max_freq;

	msg_info.send_type = LPM_IPC_SET_MAX_AIC_FREQ;
	msg_info.in        = (void *)&send_data;
	msg_info.in_len    = (uint32_t)sizeof(send_data);
	msg_info.out       = (void *)&ack_data;
	msg_info.out_len   = (uint32_t)sizeof(ack_data);
	msg_info.out_pos   = 0;

	ret = lpm_ipc_send_sync_msg(dev_id, &msg_info);
	if (ret != 0) {
		lpm_log_err("lpm turbo send set max aic freq ipc failed, ret=%d, dev_id=%u, max_freq=%u\n",
			ret, dev_id, max_freq);
		return ret;
	}

	if (ack_data.result != 0) {
		lpm_log_err("lpm turbo set max aic freq ipc failed, dev_id=%u, max_freq=%u, ack result=%u\n",
			dev_id, max_freq, ack_data.result);
		return -EIO;
	}

	return 0;
}

/*
 * return value:
 * = 0 ：set max freq success
 * -EOPNOTSUPP: not support
 * other ：set max freq failed
 * Constraint information, please read carefully:
 * 1: device_id must be passed to physical device id
 * 2: max_freq the value calculated according to the following formula takes effect:
 *    set_aic_max_freq = ((max_freq % step) != 0) ? ((max_freq / step) * step) : max_freq;
 * ascend910b: step=50, it is recommended to set max_freq to an integer divisible by 50
 */
int32_t lpm_turbo_set_aic_max_freq(uint32_t dev_id, uint32_t max_freq)
{
	int32_t ret;

	ret = lpm_turbo_check_max_freq_param(dev_id, max_freq);
	if (ret != 0) {
		lpm_log_err("lpm turbo check aic max freq info failed, ret=%d, dev_id=%u, max_freq=%u\n",
			ret, dev_id, max_freq);
		return ret;
	}

	ret = lpm_turbo_send_set_max_aic_freq_ipc(dev_id, max_freq);
	if (ret != 0) {
		lpm_log_err("lpm turbo set aic max freq info failed, ret=%d, dev_id=%u, max_freq=%u\n",
			ret, dev_id, max_freq);
		return ret;
	}

	lpm_log_info("lpm turbo set aic max freq info success, dev_id=%u, max_freq=%u\n",
		dev_id, max_freq);
	return 0;
}

int32_t lpm_turbo_probe(uint64_t *param, uint32_t param_num)
{
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	int32_t ret;

	ret = lpm_turbo_debugfs_init();
	if (ret != 0) {
		lpm_log_err("lpm turbo init debugfs failed, ret=%d\n", ret);
		return ret;
	}
#endif

	(void)param;
	(void)param_num;
	lpm_log_info("lpm turbo probe success\n");
	return 0;
}

int32_t lpm_turbo_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	lpm_log_info("lpm turbo remove success\n");
	return 0;
}


#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
STATIC void lpm_turbo_debugfs_set_mode(uint32_t *user_cfg, uint32_t param_num)
{
	int32_t ret;
	struct lpm_turbo_dsmi_cfg_in turbo_cfg = {0};

	// inner function, no need check param_num
	(void)param_num;

	// user_cfg[0x1] means dev_id
	turbo_cfg.enable = user_cfg[0x2]; // 0: close, 1: open
	ret = lpm_turbo_change_mode(user_cfg[0x1], &turbo_cfg);
	if (ret != 0) {
		lpm_log_err("debugfs write turbo failed, ret=%d\n", ret);
	}
}

STATIC void lpm_turbo_debugfs_set_max_aic_freq(uint32_t *user_cfg, uint32_t param_num)
{
	int32_t ret;

	// inner function, no need check param_num
	(void)param_num;

	ret = lpm_turbo_set_aic_max_freq(user_cfg[0x1], user_cfg[0x2]);
	if (ret != 0) {
		lpm_log_err("debugfs set max aic freq failed, ret=%d\n", ret);
	}
}

STATIC ssize_t lpm_turbo_debugfs_write(
	struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
	int32_t ret;
	uint32_t user_cfg[LPM_DEBUGFS_MAX_USER_INPUT_NUM] = {0};

	(void)file;
	(void)pos;

	// param0: type: 0:turbo mode, 1:set max aic freq
	// param1~2
	// param0=0: dev_id, enable
	// param0=1: dev_id, max aic freq(unit:M)
	ret = lpm_debugfs_get_input(buf, len, user_cfg, 0x3);
	if (ret != 0) {
		lpm_log_err("debugfs write turbo failed, ret=%d\n", ret);
		return -1;
	}

	if (user_cfg[0] == 0x0) {
		lpm_turbo_debugfs_set_mode(user_cfg, 0x3);
	} else if (user_cfg[0] == 0x1) {
		lpm_turbo_debugfs_set_max_aic_freq(user_cfg, 0x3);
	} else {
		lpm_log_err("debugfs write turbo failed, user_cfg[0]=%u error"
			"(0: set turbo mode, 1: set max aic freq)\n",
			user_cfg[0]);
	}

	return (ssize_t)len;
}

STATIC int32_t lpm_turbo_debugfs_init(void)
{
	int32_t ret;
	struct lpm_debugfs_init_cfg init_cfg = {
		.name = "turbo",
		.fn_open = NULL,
		.fn_write = lpm_turbo_debugfs_write
	};

	ret = lpm_debugfs_register_module(&init_cfg);
	if (ret != 0) {
		lpm_log_err("lpm turbo register debugfs failed, ret=%d\n", ret);
		return ret;
	}
	return 0;
}
#endif

#endif
