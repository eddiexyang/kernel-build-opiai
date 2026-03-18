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
#include <linux/delay.h>
#include "lpm_devmng_ipc.h"
#include "lpm_devmng_sharemem_drv.h"
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"
#endif
#include "lpm_devmng_idle_drv.h"
#include "lpm_devmng_idle_ascend310b.h"

#define LPM_DEFAULT_IDLE_CLK_CYCLE_TH 0x8000U
#define LPM_MAX_IDLE_CLK_CYCLE_TH     0xffffU
#define LPM_MIN_IDLE_CLK_CYCLE_TH     1U

STATIC struct lpm_idle_drv_priv *lpm_idle_drv_priv_info(void)
{
	static struct lpm_idle_drv_priv lpm_idle_priv = {0};
	return &lpm_idle_priv;
}

STATIC int32_t lpm_idle_drv_subsys_config_init(void)
{
	uint32_t sys_num;
	struct lpm_idle_drv_priv *idle_priv = lpm_idle_drv_priv_info();
	// definition order needs to be the same as enum lpm_idle_sys_id
	static struct lpm_idle_subsys_config reg_config[] = {
		{"isp",   0x1, 0x00},
		{"media", 0x1, 0x01},
		{"ao",    0x1, 0x02},
		{"mn",    0x1, 0x03},
		{"stars", 0x0, 0x04},
		{"mata",  0x1, 0x05},
		{"cpu",   0x1, 0x06},
		{"peri",  0x1, 0x07},
		{"aic",   0x1, 0x08},
		{"io",    0x1, 0x09},
		{"dvpp",  0x1, 0x0a},
		{"ring",  0x1, 0x1F}
	};

	sys_num = (uint32_t)ARRAY_SIZE(reg_config);
	if (sys_num != LPM_IDLE_SUBSYSTEMS_MAX) {
		lpm_log_err("sys_num=%u is out of range, shoule be %u \n",
			sys_num, LPM_IDLE_SUBSYSTEMS_MAX);
		return -1;
	}

	idle_priv->sys_num = sys_num;
	idle_priv->subsys_config = &reg_config[0];

	return 0;
}

STATIC bool lpm_idle_drv_skip_config(uint32_t sys_id)
{
	struct lpm_idle_drv_priv *idle_priv = lpm_idle_drv_priv_info();
	return (idle_priv->subsys_config[sys_id].support == LPM_DEVMNG_NOT_SUPPORT) ? true : false;
}

STATIC int32_t lpm_idle_drv_get_sys_id_by_bit(uint32_t bit_in_ring, uint32_t *sys_id)
{
	uint32_t tmp_id;
	struct lpm_idle_drv_priv *idle_priv = lpm_idle_drv_priv_info();
	struct lpm_idle_subsys_config *subsys_config = NULL;

	for (tmp_id = 0; tmp_id < idle_priv->sys_num; tmp_id++) {
		subsys_config = &idle_priv->subsys_config[tmp_id];
		if (subsys_config->bit_in_ring != bit_in_ring) {
			continue;
		}

		*sys_id = tmp_id;
		return 0;
	}

	// not found
	return -1;
}

STATIC void lpm_idle_drv_init_default_config(void)
{
	uint32_t sys_id;
	struct lpm_idle_drv_priv *idle_priv = lpm_idle_drv_priv_info();
	struct lpm_idle_config *idle_config = NULL;

	for (sys_id = 0; sys_id < idle_priv->sys_num; sys_id++) {
		idle_config = &idle_priv->user_config[sys_id];
		idle_config->valid         = false;
		idle_config->idle_en       = true;
		idle_config->force_idle    = 0;
		idle_config->force_busy    = 0;
		idle_config->idle_cycle_th = LPM_DEFAULT_IDLE_CLK_CYCLE_TH;
	}
}

STATIC int32_t lpm_idle_drv_verify_config(
	uint32_t *value, uint32_t num, struct lpm_idle_config* idle_config)
{
	if (num != LPM_IDLE_CONFIG_MAX) {
		lpm_log_err("lpm config item num is out of range, num=%u\n", num);
		return -1;
	}

	// the config value must be greater than or equal to 1, otherwise an exception may occur.
	if ((value[LPM_IDLE_CONFIG_CYCLE_TH] < LPM_MIN_IDLE_CLK_CYCLE_TH) ||
		(value[LPM_IDLE_CONFIG_CYCLE_TH] > LPM_MAX_IDLE_CLK_CYCLE_TH)) {
		lpm_log_err("lpm idle_cycle_th is out of range, value=%u\n", value[LPM_IDLE_CONFIG_CYCLE_TH]);
		return -1;
	}

	idle_config->idle_en       = (value[LPM_IDLE_CONFIG_EN] != 0) ? true: false;
	idle_config->force_busy    = value[LPM_IDLE_CONFIG_FORCE_BUSY] & 0xffff;
	idle_config->force_idle    = value[LPM_IDLE_CONFIG_FORCE_IDLE] & 0xffff;
	idle_config->idle_cycle_th = value[LPM_IDLE_CONFIG_CYCLE_TH] & 0xffff;

	return 0;
}

STATIC int32_t lpm_idle_drv_get_dts_config(
	struct device_node *child, uint32_t sys_id, struct lpm_idle_config* idle_config)
{
	uint32_t item_id;
	uint32_t value[LPM_IDLE_CONFIG_MAX];
	int32_t ret;
	// definition order needs to be the same as enum lpm_idle_config_type
	char *config_item[LPM_IDLE_CONFIG_MAX] = {
		"idle_en", "force_idle", "force_busy", "idle_cycle_th"
	};

	if (idle_config->valid) {
		lpm_log_err("repeat config, sys_id=%u\n", sys_id);
		return -1;
	}

	for (item_id = 0; item_id < LPM_IDLE_CONFIG_MAX; item_id++) {
		ret = of_property_read_u32(child, config_item[item_id], &value[item_id]);
		if (ret != 0) {
			lpm_log_err("read %s from dtsi failed, ret=%d\n", config_item[item_id], ret);
			return ret;
		}
	}

	ret = lpm_idle_drv_verify_config(value, LPM_IDLE_CONFIG_MAX, idle_config);
	if (ret != 0) {
		lpm_log_err("verify lpm idle config read from dtsi failed, ret=%d\n", ret);
		return ret;
	}

	idle_config->valid = true;

	lpm_log_info("lpm idle parse dtsi done, sys_id=%u "
		"idle_en=%u, cycle_th=%u, force_idle=0x%x, force_busy=0x%x\n",
		sys_id, idle_config->idle_en, idle_config->idle_cycle_th,
		idle_config->force_idle, idle_config->force_busy);
	return 0;
}

STATIC int32_t lpm_idle_drv_parse_one_node_dts(struct device_node *child)
{
	uint32_t sys_id;
	uint32_t bit_in_ring = 0;
	int32_t ret;
	struct lpm_idle_drv_priv *idle_priv = lpm_idle_drv_priv_info();

	if (of_property_read_u32(child, "id", &bit_in_ring) != 0) {
		lpm_log_err("read id from dtsi failed\n");
		return -1;
	}

	ret = lpm_idle_drv_get_sys_id_by_bit(bit_in_ring, &sys_id);
	if (ret != 0) {
		lpm_log_err("id=%u read from dtsi is out of range\n", bit_in_ring);
		return -1;
	}

	if (lpm_idle_drv_skip_config(sys_id)) {
		lpm_log_err("this system not support idle config, sys_id=%u\n", sys_id);
		return -1;
	}

	ret = lpm_idle_drv_get_dts_config(child, sys_id, &idle_priv->user_config[sys_id]);
	if (ret != 0) {
		lpm_log_err("get dtsi config failed, sys_id=%u, ret=%d\n", sys_id, ret);
		return -1;
	}
	return 0;
}

STATIC int32_t lpm_idle_drv_verify_node_cfg(void)
{
	int32_t final_ret = 0;
	uint32_t sys_id;
	struct lpm_idle_drv_priv *idle_priv = lpm_idle_drv_priv_info();

	for (sys_id = 0; sys_id < idle_priv->sys_num; sys_id++) {
		if (lpm_idle_drv_skip_config(sys_id)) {
			continue;
		}
		if (!idle_priv->user_config[sys_id].valid) {
			lpm_log_err("not found %s idle config in dtsi!\n",
				idle_priv->subsys_config[sys_id].sys_name);
			final_ret = -1;
		}
	}

	return final_ret;
}

STATIC int32_t lpm_idle_drv_parse_dts(void)
{
	int32_t ret;
	struct device_node *child = NULL;
	const struct device_node *np =
		of_find_compatible_node(NULL, NULL, MODULE_LPM_DEVMNG_MATCH);

	if (np == NULL) {
		lpm_log_err("read dtsi failed, not found device node\n");
		return -1;
	}

	for_each_child_of_node(np, child) {
		ret = lpm_idle_drv_parse_one_node_dts(child);
		if (ret != 0) {
			lpm_log_err("parse dts config failed, ret=%d\n", ret);
			return -1;
		}
	}

	ret = lpm_idle_drv_verify_node_cfg();
	if (ret != 0) {
		lpm_log_err("verify idle config failed, ret=%d\n", ret);
		return -1;
	}

	lpm_log_info("lpm devmng driver parse dts done\n");
	return 0;
}

STATIC int32_t lpm_idle_drv_config_init(void)
{
	int32_t ret;

	// should init first
	ret = lpm_idle_drv_subsys_config_init();
	if (ret != 0) {
		lpm_log_err("lpm idle init subsys config failed, ret=%d\n", ret);
		return ret;
	}

	lpm_idle_drv_init_default_config();

	ret = lpm_idle_drv_parse_dts();
	if (ret != 0) {
		lpm_log_err("lpm idle parse dts failed, ret=%d\n", ret);
		return ret;
	}

	return 0;
}

STATIC int32_t lpm_idle_drv_send_one_ipc(
	uint32_t dev_id, uint32_t start_id, uint32_t send_num, struct lpm_idle_config *user_config)
{
	uint32_t i;
	uint32_t mask;
	int32_t ret;
	struct lpm_idle_ipc_ack ack_data = {0};
	struct lpm_idle_ipc_send_data send_data = {0};
	struct lpm_idle_config *tmp_config = NULL;
	struct lpm_devmng_ipc_msg ipc_msg;

	send_data.start_id     = (uint8_t)start_id;
	send_data.sys_num      = (uint8_t)send_num;
	send_data.idle_en_mask = 0;

	for (i = 0; i < send_num; i++) {
		tmp_config = &user_config[i];
		mask = (tmp_config->idle_en ? 0x1 : 0x0) << (start_id + i);
		send_data.idle_en_mask |= (uint16_t)mask;
		send_data.force_idle[i] = tmp_config->force_idle;
		send_data.force_busy[i] = tmp_config->force_busy;
		send_data.cycle_th[i]   = tmp_config->idle_cycle_th;
	}

	ipc_msg.send_type = LPM_IPC_SET_IDLE;
	ipc_msg.in        = (void *)&send_data;
	ipc_msg.in_len    = (uint32_t)sizeof(send_data);
	ipc_msg.out       = (void *)&ack_data;
	ipc_msg.out_pos   = 0;
	ipc_msg.out_len   = (uint32_t)sizeof(ack_data);

	ret = lpm_ipc_send_sync_msg(dev_id, &ipc_msg);
	if (ret != 0) {
		lpm_log_err("lpm send idle ipc failed, ret=%d\n", ret);
		return ret;
	}

	if (ack_data.result != 0) {
		lpm_log_err("lpm idle ipc ack failed, result=%u\n", ack_data.result);
		return -EREMOTEIO;
	}

	return 0;
}

STATIC int32_t lpm_idle_drv_send_notify(uint32_t dev_id, enum lpm_notify_type notify_type)
{
	struct lpm_idle_ipc_notify_data send_data = {0};
	struct lpm_idle_ipc_ack ack_data = {0};
	struct lpm_devmng_ipc_msg ipc_msg;
	int32_t ret;

	send_data.flag     = (uint8_t)notify_type;

	ipc_msg.send_type = LPM_IPC_NOTIFY_IDLE;
	ipc_msg.in        = (void *)&send_data;
	ipc_msg.in_len    = (uint32_t)sizeof(send_data);
	ipc_msg.out       = (void *)&ack_data;
	ipc_msg.out_pos   = 0;
	ipc_msg.out_len   = (uint32_t)sizeof(ack_data);

	ret = lpm_ipc_send_sync_msg(dev_id, &ipc_msg);
	if (ret != 0) {
		lpm_log_err("lpm send idle notify ipc failed, ret=%d, notify_type=%u\n",
			ret, notify_type);
		return ret;
	}

	if (ack_data.result != 0) {
		lpm_log_err("lpm send idle notify ipc ack failed: result=%u, notify_type=%u\n",
			ack_data.result, notify_type);
		return -EREMOTEIO;
	}

	return 0;
}

STATIC void lpm_idle_drv_get_sys_id(uint32_t total_num, uint32_t *start_id, uint32_t *send_num)
{
	uint32_t start_tmp;
	uint32_t num_tmp;
	uint32_t i;

	for (start_tmp = *start_id; start_tmp < total_num; start_tmp++) {
		if (!lpm_idle_drv_skip_config(start_tmp)) {
			break;
		}
	}

	if (start_tmp >= total_num) {
		*send_num = 0;
		return;
	}

	num_tmp = 0x1;

	for (i = 1; i < LPM_IDLE_MAX_SYSTEM_EACH_IPC; i++) {
		if ((start_tmp + i) >= total_num) {
			break;
		}
		if (lpm_idle_drv_skip_config(start_tmp + i)) {
			break;
		}
		num_tmp++;
	}

	*start_id = start_tmp;
	*send_num = num_tmp;
}

STATIC int32_t lpm_idle_drv_send_config_ipc(
	uint32_t dev_id, struct lpm_idle_config *user_config, uint32_t total_num)
{
	int32_t ret;
	uint32_t start_id = 0;
	uint32_t send_num = 0;

	while (start_id < total_num) {
		lpm_idle_drv_get_sys_id(total_num, &start_id, &send_num);
		if (send_num == 0) {
			break;
		}

		ret = lpm_idle_drv_send_one_ipc(dev_id, start_id, send_num, &user_config[start_id]);
		if (ret != 0) {
			lpm_log_err("lpm idle send ipc failed, dev_id=%u, start_id=%u, send_num=%u, ret=%d\n",
				dev_id, start_id, send_num, ret);
			return ret;
		}
		start_id += send_num;
	}

	return 0;
}

STATIC int32_t lpm_idle_drv_send_config_to_lpmcu(
	uint32_t dev_num, struct lpm_idle_config *user_config, uint32_t total_num)
{
	int32_t ret;
	uint32_t dev_id;

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		ret = lpm_idle_drv_send_config_ipc(dev_id, user_config, total_num);
		if (ret != 0) {
			lpm_log_err("lpm send idle ipc failed, dev_id=%u, total_num=%u, ret=%d\n",
				dev_id, total_num, ret);
			return ret;
		}
	}

	return 0;
}

int32_t lpm_idle_drv_set_switch(uint32_t dev_id, struct lpm_idle_dsmi_cfg_in *in_param)
{
	(void)dev_id;
	(void)in_param;
	// Operation not supported
	return -EOPNOTSUPP;
}

int32_t lpm_idle_drv_probe(uint64_t *param, uint32_t param_num)
{
	int32_t ret;
	uint32_t dev_num = lpm_common_get_dev_num();
	struct lpm_idle_drv_priv *idle_priv = lpm_idle_drv_priv_info();

	ret = lpm_idle_drv_config_init();
	if (ret != 0) {
		lpm_log_err("lpm idle parse dts failed, ret=%d\n", ret);
		return ret;
	}

	ret = lpm_idle_drv_send_config_to_lpmcu(
		dev_num, &idle_priv->user_config[0], LPM_IDLE_SUBSYSTEMS_MAX);
	if (ret != 0) {
		lpm_log_err("lpm send idle config to lp failed, dev_num=%u, ret=%d\n",
			dev_num, ret);
		return ret;
	}

	(void)param;
	(void)param_num;
	lpm_log_info("lpm idle probe success, dev_num=%u\n", dev_num);
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
	int32_t ret;
	uint32_t dev_id;
	uint32_t dev_num = lpm_common_get_dev_num();

	// send ipc msg to notify all substem to disable idle
	// this operation needs to be done before all subsystems go to suspend
	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		ret = lpm_idle_drv_send_notify(dev_id, LPM_IDLE_SUSPEND);
		if (ret != 0) {
			lpm_log_err("lpm notify suspend to lp failed, dev_id=%u, ret=%d\n",
				dev_id, ret);
			return ret;
		}
	}

	lpm_log_info("lpm idle suspend prepare success, dev_num=%u.\n", dev_num);
	return 0;
}

int32_t lpm_idle_drv_post_suspend(void)
{
	int32_t ret;
	uint32_t dev_id;
	uint32_t dev_num = lpm_common_get_dev_num();

	// send ipc msg to notify all substem to recovery idle config
	// this operation needs to be completed after all subsystems complete the resume operation
	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		// Wait up to 5s to ensure that lp firmware completes the resume process
		// Wait for 1ms each time, up to 5000 times
		// solve the problem that the ipc message processing fails
		// caused by the lp has not completed the resume
		ret = lpm_wait_lp_suspend_status_done(dev_id, LPM_IDLE_WAIT_RESUME_CNT,
			LPM_IDLE_WAIT_RESUME_TIME, LPM_SUSPEND_STATUS_RESUME);
		if (ret != 0) {
			lpm_log_err("lpm notify resume to lp failed for lp has not complete resume, dev_id=%u\n",
				dev_id);
			return ret;
		}

		ret = lpm_idle_drv_send_notify(dev_id, LPM_IDLE_RESUME);
		if (ret != 0) {
			lpm_log_err("lpm notify resume to lp failed, dev_id=%u, ret=%d\n",
				dev_id, ret);
			return ret;
		}
	}

	lpm_log_info("lpm idle post resume success, dev_num=%u.\n", dev_num);
	return 0;
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
STATIC void lpm_idle_drv_show_config(struct seq_file *seq)
{
	struct lpm_idle_drv_priv *idle_priv = lpm_idle_drv_priv_info();
	uint32_t sys_id;
	char *src_name[0x2] = {"default cfg", "dts cfg"};
	char *support_info[0x2] = {"not support idle", "support idle"};

	seq_printf(seq, "[idle config]\n");
	for (sys_id = 0; sys_id < LPM_IDLE_SUBSYSTEMS_MAX; sys_id++) {
		// base info
		seq_printf(seq, " [%-5s] %11s, %16s\n",
			idle_priv->subsys_config[sys_id].sys_name,
			src_name[idle_priv->user_config[sys_id].valid ? 0x1 : 0x0],
			support_info[(idle_priv->subsys_config[sys_id].support != 0) ? 0x1 : 0x0]);

		// cfg info
		seq_printf(seq,
			"        idle_en=%u, force_idle=0x%04x, force_busy=0x%04x, cycle_th=%-5u\n",
			idle_priv->user_config[sys_id].idle_en,
			idle_priv->user_config[sys_id].force_idle,
			idle_priv->user_config[sys_id].force_busy,
			idle_priv->user_config[sys_id].idle_cycle_th);
	}
}

STATIC void lpm_idle_drv_debugfs_set_config(uint32_t *user_cfg, uint32_t param_num)
{
	int32_t ret;
	uint32_t sys_id = 0;
	uint32_t dev_id;
	uint32_t bit_in_ring;
	struct lpm_idle_config idle_config = {0};
	uint32_t config_value[LPM_IDLE_CONFIG_MAX] = {0};
	struct lpm_idle_drv_priv *idle_priv = lpm_idle_drv_priv_info();

	// inner function, no need check param_num
	(void)param_num;

	dev_id              = user_cfg[0x0];
	bit_in_ring         = user_cfg[0x1];
	config_value[0x0]   = user_cfg[0x2];
	config_value[0x1]   = user_cfg[0x3];
	config_value[0x2]   = user_cfg[0x4];
	config_value[0x3]   = user_cfg[0x5];

	if (!lpm_common_check_dev_id(dev_id)) {
		lpm_log_err("dev_id=%u is out of range\n", dev_id);
		return;
	}

	ret = lpm_idle_drv_get_sys_id_by_bit(bit_in_ring, &sys_id);
	if (ret != 0) {
		lpm_log_err("bit_in_ring=%u is out of range\n", bit_in_ring);
		return;
	}

	if (lpm_idle_drv_skip_config(sys_id)) {
		lpm_log_warn("%s not support idle config\n",
			idle_priv->subsys_config[sys_id].sys_name);
		return;
	}

	ret = lpm_idle_drv_verify_config(config_value, LPM_IDLE_CONFIG_MAX, &idle_config);
	if (ret != 0) {
		lpm_log_err("verify lpm idle config read from dtsi failed=%d\n", ret);
		return;
	}

	ret = lpm_idle_drv_send_one_ipc(dev_id, sys_id, 0x1, &idle_config);
	if (ret != 0) {
		lpm_log_err("lpm idle send ipc failed, dev_id=%u start_id=%u ret=%d\n",
			dev_id, sys_id, ret);
		return;
	}

	lpm_log_info("lpm idle send ipc success, dev_id=%u, sys_id=%u "
		"idle_en=%u, cycle_th=%u, force_idle=0x%x, force_busy=0x%x\n",
		dev_id, sys_id, idle_config.idle_en, idle_config.idle_cycle_th,
		idle_config.force_idle, idle_config.force_busy);
}

ssize_t lpm_idle_drv_debugfs_write(
	struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
	int32_t ret;
	uint32_t user_cfg[LPM_DEBUGFS_MAX_USER_INPUT_NUM] = {0};

	(void)file;
	(void)pos;

	// dev_id, bit_in_ring idle_en, force_idle, force_busy, idle_cycle_th
	// idle cfg definition order needs to be the same as enum lpm_idle_config_type
	ret = lpm_debugfs_get_input(buf, len, user_cfg, (uint32_t)LPM_IDLE_CONFIG_MAX + (uint32_t)0x2);
	if (ret != 0) {
		lpm_log_err("debugfs write idle failed, ret=%d\n", ret);
		return -1;
	}

	lpm_idle_drv_debugfs_set_config(user_cfg, (uint32_t)LPM_IDLE_CONFIG_MAX + (uint32_t)0x2);
	return (ssize_t)len;
}

int32_t lpm_idle_drv_debugfs_show(struct seq_file *seq, void *v)
{
	(void)v;

	lpm_idle_drv_show_config(seq);
	return 0;
}

#endif