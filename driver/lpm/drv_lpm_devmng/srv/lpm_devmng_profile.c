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
#include "lpm_devmng_sharemem_drv.h"
#include "lpm_devmng_flash.h"
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"
#endif
#include "lpm_devmng_profile.h"

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
STATIC int32_t lpm_profile_debugfs_init(void);
#endif

STATIC struct lpm_profile_priv *lpm_profile_priv_info(void)
{
	static struct lpm_profile_priv lpm_profile_priv = {0};
	return &lpm_profile_priv;
}

STATIC void lpm_profile_init_data(void)
{
	struct lpm_profile_priv *profile_priv = lpm_profile_priv_info();
	uint32_t dev_id;

	for (dev_id = 0; dev_id < LPM_DEVMNG_DEV_MAX_NUM; dev_id++) {
		profile_priv->dev_data[dev_id].last_flash_tops = LPM_PROFILE_TOPS_MAX;
		profile_priv->dev_data[dev_id].cur_tops = LPM_PROFILE_TOPS_MAX;
		profile_priv->dev_data[dev_id].is_recovery = false;

		mutex_init(&profile_priv->dev_data[dev_id].profile_mutex);
	}
}

STATIC void lpm_profile_destroy_data(void)
{
	struct lpm_profile_priv *profile_priv = lpm_profile_priv_info();
	uint32_t dev_id;

	for (dev_id = 0; dev_id < LPM_DEVMNG_DEV_MAX_NUM; dev_id++) {
		mutex_destroy(&profile_priv->dev_data[dev_id].profile_mutex);
	}
}

STATIC int32_t lpm_profile_ipc_send_cfg(uint32_t dev_id, uint32_t config_tops)
{
	struct lpm_profile_ipc_send send_data = {0};
	struct lpm_profile_ipc_ack ack_data = {0};
	struct lpm_devmng_ipc_msg ipc_msg;
	int32_t ret;

	send_data.profile_index = config_tops;

	ipc_msg.send_type = LPM_IPC_SET_PROFILE;
	ipc_msg.in        = (void *)&send_data;
	ipc_msg.in_len    = (uint32_t)sizeof(send_data);
	ipc_msg.out       = (void *)&ack_data;
	ipc_msg.out_pos   = 0;
	ipc_msg.out_len   = (uint32_t)sizeof(ack_data);

	ret = lpm_ipc_send_sync_msg(dev_id, &ipc_msg);
	if (ret != 0) {
		lpm_log_err("lpm profile send ipc failed, ret=%d, dev_id=%u, config_tops=%u\n",
			ret, dev_id, config_tops);
		return ret;
	}

	if (ack_data.result != 0) {
		lpm_log_err("lpm profile send ipc failed: ack result=%u, config_tops=%u\n",
			ack_data.result, config_tops);
		return -EREMOTEIO;
	}

	return 0;
}

STATIC void lpm_profile_save_cur_tops(uint32_t dev_id, uint32_t profile_tops)
{
	struct lpm_profile_priv *profile_priv = lpm_profile_priv_info();
	profile_priv->dev_data[dev_id].cur_tops = profile_tops;
}

STATIC uint32_t lpm_profile_get_cur_tops(uint32_t dev_id)
{
	struct lpm_profile_priv *profile_priv = lpm_profile_priv_info();
	return profile_priv->dev_data[dev_id].cur_tops;
}

STATIC void lpm_profile_save_flash_tops(uint32_t dev_id, uint32_t profile_tops)
{
	struct lpm_profile_priv *profile_priv = lpm_profile_priv_info();
	profile_priv->dev_data[dev_id].last_flash_tops = profile_tops;
}

STATIC uint32_t lpm_profile_get_flash_tops(uint32_t dev_id)
{
	struct lpm_profile_priv *profile_priv = lpm_profile_priv_info();
	return profile_priv->dev_data[dev_id].last_flash_tops;
}

STATIC int32_t lpm_profile_save_tops_to_flash(
	uint32_t dev_id, enum lpm_flash_op_way op_way, uint32_t profile_tops)
{
	int32_t ret;
	struct lpm_flash_op_info op_info = {0};
	struct lpm_flash_profile_data profile_data = {0};

	if (lpm_profile_get_flash_tops(dev_id) == profile_tops) {
		return 0;
	}

	profile_data.profile_id = profile_tops;
	profile_data.valid      = 0x1; // 1 measns valid

	op_info.dev_id    = dev_id;
	op_info.op_way    = op_way;
	op_info.item_type = LPM_FLASH_ITEM_PROFILE;
	op_info.data      = (uint8_t *)&profile_data;
	op_info.data_len  = (uint32_t)sizeof(struct lpm_flash_profile_data);
	ret = lpm_flash_write(&op_info);
	if (ret != 0) {
		lpm_log_err("lpm profile save tops to flash failed, ret=%d, dev_id=%u, profile_tops=%u\n",
			ret, dev_id, profile_tops);
		return ret;
	}

	lpm_profile_save_flash_tops(dev_id, profile_tops);
	return 0;
}

STATIC bool lpm_profile_check_profile_data(uint32_t dev_id, const uint8_t *data, uint32_t data_len)
{
	if (!lpm_common_check_dev_id(dev_id)) {
		lpm_log_err("recover tops failed, dev_id=%u is out of range\n", dev_id);
		return false;
	}
	if (data == NULL) {
		lpm_log_err("dev_id=%u recover tops failed, data is null, data_len=%u\n", dev_id, data_len);
		return false;
	}
	if (data_len != sizeof(struct lpm_flash_profile_data)) {
		lpm_log_err("dev_id=%u recover tops failed, data_len=%u error\n", dev_id, data_len);
		return false;
	}

	return true;
}

STATIC int32_t lpm_profile_config_tops(
	uint32_t dev_id, bool flash_valid, const uint8_t *data, uint32_t data_len)
{
	int32_t ret;
	const struct lpm_flash_profile_data *profile_data = NULL;
	struct lpm_profile_priv *profile_priv = lpm_profile_priv_info();

	lpm_log_info("start to recover tops from flash, dev_id=%u\n", dev_id);

	if (!lpm_profile_check_profile_data(dev_id, data, data_len)) {
		lpm_log_err("recover tops from flash failed\n");
		return -EINVAL;
	}

	mutex_lock(&profile_priv->dev_data[dev_id].profile_mutex);
	profile_priv->dev_data[dev_id].is_recovery = true;

	if (!flash_valid) {
		mutex_unlock(&profile_priv->dev_data[dev_id].profile_mutex);
		lpm_log_info("user has not config flash\n");
		return 0;
	}

	profile_data = (const struct lpm_flash_profile_data *)data;
	if (profile_data->valid == 0) {
		mutex_unlock(&profile_priv->dev_data[dev_id].profile_mutex);
		lpm_log_info("user has not config profile\n");
		return 0;
	}
	if (!lpm_profile_check_tops(profile_data->profile_id)) {
		mutex_unlock(&profile_priv->dev_data[dev_id].profile_mutex);
		lpm_log_err("recover tops from flash failed, profile_id=%u is out of range\n",
			profile_data->profile_id);
		return -EINVAL;
	}

	lpm_profile_save_flash_tops(dev_id, profile_data->profile_id);

	ret = lpm_profile_ipc_send_cfg(dev_id, profile_data->profile_id);
	if (ret != 0) {
		// not return
		lpm_log_err(" lpm profile config tops failed, ret=%d, profile_id=%u\n",
			ret, profile_data->profile_id);
	} else {
		lpm_profile_save_cur_tops(dev_id, profile_data->profile_id);
	}

	mutex_unlock(&profile_priv->dev_data[dev_id].profile_mutex);

	lpm_log_info("recover tops from flash success, dev_id=%u, profile_id=%u\n",
		dev_id, profile_data->profile_id);

	return 0;
}

STATIC int32_t lpm_profile_get_input(
	const char *in, uint32_t in_len, struct lpm_profile_dsmi_cfg_in *para)
{
	int32_t ret;

	if ((in == NULL) || (in_len != sizeof(struct lpm_profile_dsmi_cfg_in))) {
		lpm_log_err("get profile input failed, input char is NULL or in_len is wrong, in_len=%u\n", in_len);
		return -EINVAL;
	}

	ret = copy_from_user((void *)para, (const void *)in, in_len);
	if (ret != 0) {
		lpm_log_err("get profile input copy_from_user failed, ret=%d, in_len=%u\n",
			ret, in_len);
		return -EIO;
	}

	return 0;
}

STATIC int32_t lpm_profile_cfg_para_check(
	uint32_t dev_id, struct lpm_profile_dsmi_cfg_in *para)
{
	if (!lpm_common_check_dev_id(dev_id)) {
		lpm_log_err("check cfg profile failed, dev_id=%u is out of range\n", dev_id);
		return -ENODEV;
	}

	if (!lpm_profile_check_tops(para->profile_id)) {
		lpm_log_err("check cfg profile failed, profile_id=%u is out of range\n", para->profile_id);
		return -EINVAL;
	}

	return 0;
}

STATIC int32_t lpm_profile_get_tops_from_sharemem(
	uint32_t dev_id, struct lpm_profile_tops_list *tops_list, uint32_t *init_profile)
{
	int32_t ret;
	struct lpm_profile_tops_info tops_info = {0};

	ret = lpm_get_profile_from_sharemem(dev_id, &tops_info);
	if (ret != 0) {
		lpm_log_err("lpm profile get tops from sharemem failed, ret=%d\n", ret);
		return ret;
	}

	*init_profile = tops_info.init_profile;
	(void)memcpy_s(tops_list, sizeof(struct lpm_profile_tops_list),
		&tops_info.tops_list, sizeof(struct lpm_profile_tops_list));

	return 0;
}

STATIC void lpm_profile_set_tops_rsv_data(struct lpm_profile_tops_list *tops_list)
{
	uint32_t i;

	// as agreed with the dsmi module, the reserved field returns 0xff
	for (i = 0; i < tops_list->tops_num; i++) {
		(void)memset_s(tops_list->tops_table[i].rsv, sizeof(tops_list->tops_table[i].rsv),
			0xff, sizeof(tops_list->tops_table[i].rsv));
	}
}

int32_t lpm_profile_get_tops(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	unsigned long cpy_len;
	int32_t ret;
	uint32_t cur_tops;
	struct lpm_profile_tops_summary tops_summary = {0};
	struct lpm_profile_tops_list tops_list = {0};
	struct lpm_profile_priv *profile_priv = lpm_profile_priv_info();

	if (lpm_common_check_is_in_container()) {
		return -EOPNOTSUPP;
	}

	ret = lpm_check_dsmi_get_param(dev_id, in, in_len, (uint32_t)sizeof(tops_summary), out_len);
	if (ret != 0) {
		lpm_log_err("get tops, check param failed, ret=%d\n", ret);
		return -EINVAL;
	}

	ret = lpm_profile_get_tops_from_sharemem(dev_id, &tops_list, &tops_summary.cur_tops);
	if (ret != 0) {
		lpm_log_err("lpm profile get tops from sharemem failed, ret=%d\n", ret);
		return ret;
	}

	tops_summary.tops_num = tops_list.tops_num;

	/*
	 * case1: user has not config yet, no need recovery from flash, return default profile
	 * case2: no need recovery from flash, user update profile, return profile id configured by user
	 * case3: profile has not recovery from flash, user has not update profile, return default profile
	 * case4: profile recovery from flash, user has not update profile, return profile id recovery from flash
	 * case5: profile recovery from flash, user update profile, return profile id configured by user
	 * attention: before the config in the flash is restored, the user cannot update the config
	 */
	mutex_lock(&profile_priv->dev_data[dev_id].profile_mutex);
	cur_tops = lpm_profile_get_cur_tops(dev_id);
	if (lpm_profile_check_tops(cur_tops)) {
		// user has config profile, update cur_tops info
		tops_summary.cur_tops = cur_tops;
	}
	mutex_unlock(&profile_priv->dev_data[dev_id].profile_mutex);

	cpy_len = copy_to_user(in, (void *)&tops_summary, sizeof(tops_summary));
	if (cpy_len != 0) {
		lpm_log_err("get tops, copy to user failed, cpy_len=%lu, tops_num=%u, cur_tops=%u\n",
			cpy_len, tops_summary.tops_num, tops_summary.cur_tops);
		return -EIO;
	}
	*out_len = (uint32_t)sizeof(tops_summary);

	return 0;
}

STATIC int32_t lpm_profile_change_tops(
	uint32_t dev_id, enum lpm_flash_op_way op_way, struct lpm_profile_dsmi_cfg_in *in_cfg)
{
	int32_t ret;
	struct lpm_profile_priv *profile_priv = lpm_profile_priv_info();

	ret = lpm_profile_cfg_para_check(dev_id, in_cfg);
	if (ret != 0) {
		lpm_log_err("lpm profile set tops, para check failed, ret=%d\n", ret);
		return -EINVAL;
	}

	mutex_lock(&profile_priv->dev_data[dev_id].profile_mutex);
	// if the flash config before the last reset has not been restored, tops config is not allowed
	if (!profile_priv->dev_data[dev_id].is_recovery) {
		mutex_unlock(&profile_priv->dev_data[dev_id].profile_mutex);
		lpm_log_err("lpm profile set tops failed, flash config has not been recovered\n");
		return -EIO;
	}

	// filter repeat config
	if (lpm_profile_get_cur_tops(dev_id) != in_cfg->profile_id) {
		ret = lpm_profile_ipc_send_cfg(dev_id, in_cfg->profile_id);
		if (ret != 0) {
			mutex_unlock(&profile_priv->dev_data[dev_id].profile_mutex);
			lpm_log_err("lpm profile set tops, send ipc failed, ret=%d, dev_id=%u, profile_id=%u\n",
				ret, dev_id, in_cfg->profile_id);
			return ret;
		}

		lpm_profile_save_cur_tops(dev_id, in_cfg->profile_id);
	}

	if (in_cfg->save_flash != 0) {
		ret = lpm_profile_save_tops_to_flash(dev_id, op_way, in_cfg->profile_id);
		if (ret != 0) {
			mutex_unlock(&profile_priv->dev_data[dev_id].profile_mutex);
			lpm_log_err("lpm profile save tops to flash failed, ret=%d, dev_id=%u, profile_id=%u\n",
				ret, dev_id, in_cfg->profile_id);
			return ret;
		}
	}

	mutex_unlock(&profile_priv->dev_data[dev_id].profile_mutex);

	lpm_log_info("lpm profile set tops success, dev_id=%u, profile_id=%u, save_flash=%u\n",
		dev_id, in_cfg->profile_id, in_cfg->save_flash);

	return 0;
}

int32_t lpm_profile_set_tops(uint32_t dev_id, const char *in, uint32_t in_len)
{
	int32_t ret;
	struct lpm_profile_dsmi_cfg_in para = {0};

	ret = lpm_profile_get_input(in, in_len, &para);
	if (ret != 0) {
		lpm_log_err("lpm profile set tops, get input failed, ret=%d\n", ret);
		return -EINVAL;
	}

	ret = lpm_profile_change_tops(dev_id, LPM_FLASH_OP_WAY_DIRECT, &para);
	if (ret != 0) {
		lpm_log_err("lpm profile set tops failed, ret=%d, dev_id=%u, profile_id=%u, save_flash=%u\n",
			ret, dev_id, para.profile_id, para.save_flash);
		return ret;
	}

	return 0;
}

int32_t lpm_profile_get_tops_detail(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	unsigned long cpy_len;
	int32_t ret;
	uint32_t init_profile = 0;
	struct lpm_profile_tops_list tops_list = {0};

	if (lpm_common_check_is_in_container()) {
		return -EOPNOTSUPP;
	}

	ret = lpm_check_dsmi_get_param(dev_id, in, in_len, (uint32_t)sizeof(tops_list), out_len);
	if (ret != 0) {
		lpm_log_err("check get tops list in param failed, ret=%d\n", ret);
		return -EINVAL;
	}

	ret = lpm_profile_get_tops_from_sharemem(dev_id, &tops_list, &init_profile);
	if (ret != 0) {
		lpm_log_err("lpm profile get tops from sharemem failed, ret=%d\n", ret);
		return ret;
	}

	lpm_profile_set_tops_rsv_data(&tops_list);

	cpy_len = copy_to_user(in, (void *)&tops_list, sizeof(tops_list));
	if (cpy_len != 0) {
		lpm_log_err("get tops, copy to user failed, cpy_len=%lu, tops_num=%u\n", cpy_len, tops_list.tops_num);
		return -EIO;
	}
	*out_len = (uint32_t)sizeof(tops_list);

	return 0;
}

int32_t lpm_profile_probe(uint64_t *param, uint32_t param_num)
{
	int32_t ret;
	struct lpm_flash_recover_module module_cfg = {0};

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	ret = lpm_profile_debugfs_init();
	if (ret != 0) {
		lpm_log_err("lpm profile init debugfs failed, ret=%d\n", ret);
		return ret;
	}
#endif

	lpm_profile_init_data();

	module_cfg.item_type  = LPM_FLASH_ITEM_PROFILE;
	module_cfg.fn_recover = lpm_profile_config_tops;
	ret = lpm_flash_register_recover_module(&module_cfg);
	if (ret != 0) {
		lpm_log_err("lpm profile register flash recover failed, ret=%d\n", ret);
		return ret;
	}

	(void)param;
	(void)param_num;
	lpm_log_info("lpm profile probe success\n");
	return 0;
}

int32_t lpm_profile_remove(uint64_t *param, uint32_t param_num)
{
	lpm_profile_destroy_data();

	(void)param;
	(void)param_num;
	lpm_log_info("lpm profile remove success\n");
	return 0;
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
STATIC void lpm_profile_debug_show_tops_list(
	struct seq_file *seq, struct lpm_profile_tops_list *tops_list)
{
	uint32_t i;

	seq_printf(seq, " tops_num=%u\n", tops_list->tops_num);
	seq_printf(seq, "  profile_id aic_tops   aic_freq   aic_vol    cpu_freq   cpu_vol\n");
	for (i = 0; i < tops_list->tops_num; i++) {
		seq_printf(seq, "  %-10u %-10u %-10u %-10u %-10u %-10u\n",
			tops_list->tops_table[i].profile_id, tops_list->tops_table[i].aic_tops,
			tops_list->tops_table[i].aic_freq, tops_list->tops_table[i].aic_vol,
			tops_list->tops_table[i].cpu_freq, tops_list->tops_table[i].cpu_vol);
	}
}

STATIC int32_t lpm_profile_read_tops_from_flash(
	uint32_t dev_id, enum lpm_flash_op_way op_way, bool *is_config, uint32_t *config_tops)
{
	int32_t ret;
	struct lpm_flash_op_info op_info = {0};
	struct lpm_flash_profile_data profile_data = {0};

	*is_config = false;
	*config_tops = LPM_PROFILE_TOPS_MAX;

	op_info.dev_id    = dev_id;
	op_info.op_way    = op_way;
	op_info.item_type = LPM_FLASH_ITEM_PROFILE;
	op_info.data      = (uint8_t *)&profile_data;
	op_info.data_len  = (uint32_t)sizeof(profile_data);

	ret = lpm_flash_read(&op_info);
	if (ret == -ENOENT) {
		return 0;
	}

	if (ret != 0) {
		lpm_log_err("read profile tops from flash failed, ret=%d\n", ret);
		return ret;
	}
	if (profile_data.valid == 0) {
		return 0;
	}
	if (!lpm_profile_check_tops(profile_data.profile_id)) {
		lpm_log_err("profile tops read from flash out of range, profile_id=%u\n",
			profile_data.profile_id);
		return -EINVAL;
	}

	*is_config   = true;
	*config_tops = profile_data.profile_id;
	return 0;
}

STATIC void lpm_profile_debug_get_tops_list(struct seq_file *seq)
{
	int32_t ret;
	uint32_t dev_id;
	bool is_config = false;
	uint32_t dev_num = lpm_common_get_dev_num();
	uint32_t init_profile = 0;
	uint32_t config_tops = LPM_PROFILE_TOPS_MAX;
	struct lpm_profile_tops_list tops_list = {0};

	seq_printf(seq, "[dev_num=%u]\n", dev_num);
	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		seq_printf(seq, " [dev_id=%u]\n", dev_id);

		// read from sharemem
		ret = lpm_profile_get_tops_from_sharemem(dev_id, &tops_list, &init_profile);
		if (ret != 0) {
			seq_printf(seq, " lpm profile get tops table from sharemem failed, ret=%d\n", ret);
		} else {
			seq_printf(seq, " [profile tops read from sharemem]\n");
			seq_printf(seq, " init_profile=%u\n", init_profile);
			lpm_profile_debug_show_tops_list(seq, &tops_list);
		}

		// read from flash
		ret = lpm_profile_read_tops_from_flash(dev_id, LPM_FLASH_OP_WAY_THREAD, &is_config, &config_tops);
		if (ret != 0) {
			seq_printf(seq, " read profile tops from flash failed failed, ret=%d\n", ret);
		} else {
			seq_printf(seq, " [profile tops read from flash]\n");
			if (is_config) {
				seq_printf(seq, "   config_tops = %u\n", config_tops);
			} else {
				seq_printf(seq, "   user has not set yet\n");
			}
		}

		// read from dev_data
		seq_printf(seq, " [profile tops read from soft data]\n");
		seq_printf(seq, "   last_flash_tops = %u\n", lpm_profile_get_flash_tops(dev_id));
		seq_printf(seq, "   cur_tops        = %u\n", lpm_profile_get_cur_tops(dev_id));
	}
}

STATIC void lpm_profile_debugfs_set_tops(uint32_t *user_cfg, uint32_t param_num)
{
	int32_t ret;
	uint32_t dev_id;
	struct lpm_profile_dsmi_cfg_in in_put = {0};

	// inner function, no need check param_num
	(void)param_num;

	// trans user config to in put config
	// param not check here
	dev_id            = user_cfg[0x0];
	in_put.profile_id = user_cfg[0x1];
	in_put.save_flash = user_cfg[0x2];
	ret = lpm_profile_change_tops(dev_id, LPM_FLASH_OP_WAY_THREAD, &in_put);
	if (ret != 0) {
		lpm_log_err("debugfs write tops failed, dev_id=%u, profile_id=%u, save_flash=%u, ret=%d\n",
			dev_id, in_put.profile_id, in_put.save_flash, ret);
		return;
	}
}

STATIC ssize_t lpm_profile_debugfs_write(
	struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
	int32_t ret;
	uint32_t user_cfg[LPM_DEBUGFS_MAX_USER_INPUT_NUM] = {0};

	(void)file;
	(void)pos;

	// dev_id, profile_id, save_flash
	ret = lpm_debugfs_get_input(buf, len, user_cfg, 0x3);
	if (ret != 0) {
		lpm_log_err("debugfs write tops failed, ret=%d\n", ret);
		return -1;
	}

	lpm_profile_debugfs_set_tops(user_cfg, 0x3);
	return (ssize_t)len;
}

STATIC int32_t lpm_profile_debugfs_show(struct seq_file *seq, void *v)
{
	(void)v;
	lpm_profile_debug_get_tops_list(seq);
	return 0;
}

STATIC int32_t lpm_profile_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, lpm_profile_debugfs_show, inode->i_private);
}

STATIC int32_t lpm_profile_debugfs_init(void)
{
	int32_t ret;
	struct lpm_debugfs_init_cfg init_cfg = {
		.name = "profile",
		.fn_open = lpm_profile_debugfs_open,
		.fn_write = lpm_profile_debugfs_write
	};

	ret = lpm_debugfs_register_module(&init_cfg);
	if (ret != 0) {
		lpm_log_err("lpm profile register debugfs failed, ret=%d\n", ret);
		return ret;
	}
	return 0;
}
#endif
