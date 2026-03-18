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

#include <linux/of.h>
#include <linux/io.h>
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"
#endif
#include "lpm_devmng_common.h"
#include "lpm_devmng_aic_freq_drv.h"
#include "lpm_devmng_aic_freq_sharemem_v2.h"

STATIC struct lpm_aic_freq_drv_priv *lpm_aic_freq_get_drv_priv_info(void)
{
	static struct lpm_aic_freq_drv_priv lpm_aic_freq_drv_priv_info = {0};
	return &lpm_aic_freq_drv_priv_info;
}

STATIC void lpm_aic_freq_drv_clr_last_syscnt(uint32_t dev_id)
{
	struct lpm_aic_freq_drv_priv *aic_freq_priv = lpm_aic_freq_get_drv_priv_info();

	(void)memset_s(&aic_freq_priv->dev_data[dev_id].syscnt_info.last_syscnt,
		sizeof(struct lpm_aic_freq_last_syscnt), 0, sizeof(struct lpm_aic_freq_last_syscnt));
	aic_freq_priv->dev_data[dev_id].syscnt_info.last_syscnt.next_index = LPM_AIC_FREQ_SYSCNT_INVALID_INDEX;
}

STATIC void lpm_aic_freq_drv_init_data(void)
{
	struct lpm_aic_freq_drv_priv *aic_freq_priv = lpm_aic_freq_get_drv_priv_info();
	uint32_t dev_id;

	for (dev_id = 0; dev_id < LPM_DEVMNG_DEV_MAX_NUM; dev_id++) {
		lpm_aic_freq_drv_clr_last_syscnt(dev_id);
		mutex_init(&aic_freq_priv->dev_data[dev_id].syscnt_info.syscnt_mutex);
	}
}

STATIC bool lpm_aic_freq_drv_check_syscnt_param(
	uint32_t dev_id, void *out, uint32_t out_len, uint32_t *result_len)
{
	if (!lpm_common_check_dev_id(dev_id)) {
		lpm_log_err("check get syscnt failed, dev_id=%u is out of range\n", dev_id);
		return false;
	}

	if (out == NULL) {
		lpm_log_err("dev_id=%u check get syscnt failed, out is null\n", dev_id);
		return false;
	}

	if (result_len == NULL) {
		lpm_log_err("dev_id=%u check get syscnt failed, result_len is null\n", dev_id);
		return false;
	}

	if (out_len < (uint32_t)sizeof(struct lpm_aic_freq_syscnt_result)) {
		lpm_log_err("dev_id=%u check get aic freq reduce cause failed, out_len=%u not enough\n",
			dev_id, out_len);
		return false;
	}

	return true;
}

STATIC void lpm_aic_freq_drv_get_syscnt_data(
	uint32_t dev_id, struct lpm_aic_freq_syscnt_info *syscnt_info,
	struct lpm_aic_freq_syscnt_result *result, uint32_t *result_len)
{
	uint32_t next_index;
	uint32_t last_index;
	uint32_t j;
	struct lpm_aic_freq_drv_priv *aic_freq_priv = lpm_aic_freq_get_drv_priv_info();
	struct lpm_aic_freq_last_syscnt *last_syscnt =
		&aic_freq_priv->dev_data[dev_id].syscnt_info.last_syscnt;

	// no valid data
	if (syscnt_info->freq_index == LPM_AIC_FREQ_SYSCNT_INVALID_INDEX) {
		*result_len = 0;
		return;
	}

	last_index = (syscnt_info->freq_index + LPM_AIC_FREQ_SYSCNT_MAC_NUM - 1) % LPM_AIC_FREQ_SYSCNT_MAC_NUM;

	// no new data was added between the two queries
	if ((syscnt_info->freq_index == last_syscnt->next_index) &&
		(syscnt_info->syscnt[last_index].syscnt == last_syscnt->syscnt_freq.syscnt)) {
		*result_len = 0;
		return;
	}

	// calculate the starting freq_index for read data
	// if next_index equal to LPM_AIC_FREQ_SYSCNT_INVALID_INDEX, it means never read data before
	// should read from freq_index 0
	next_index =
		(last_syscnt->next_index == LPM_AIC_FREQ_SYSCNT_INVALID_INDEX) ? 0 : last_syscnt->next_index;

	*result_len = (uint32_t)sizeof(struct lpm_aic_freq_syscnt_result);

	// if next_index equal to syscnt_info->freq_index, it means result count is LPM_AIC_FREQ_SYSCNT_MAC_NUM
	result->count = (syscnt_info->freq_index > next_index) ?
		(syscnt_info->freq_index - next_index) :
		(LPM_AIC_FREQ_SYSCNT_MAC_NUM - next_index + syscnt_info->freq_index);

	last_index = next_index;
	for (j = 0; j < result->count; j++) {
		result->syscnt[j].freq   = syscnt_info->syscnt[next_index].freq;
		result->syscnt[j].syscnt = syscnt_info->syscnt[next_index].syscnt;
		result->syscnt[j].resv   = syscnt_info->syscnt[next_index].resv;
		last_index = next_index;
		next_index = (next_index + 0x1) % LPM_AIC_FREQ_SYSCNT_MAC_NUM;
	}

	// updata last syscnt_freq info
	last_syscnt->next_index = next_index;
	last_syscnt->syscnt_freq.freq   = syscnt_info->syscnt[last_index].freq;
	last_syscnt->syscnt_freq.syscnt = syscnt_info->syscnt[last_index].syscnt;
	last_syscnt->syscnt_freq.resv   = syscnt_info->syscnt[last_index].resv;
}

int32_t lpm_aic_freq_drv_get_syscnt_info(uint32_t dev_id, void *out, uint32_t out_len, uint32_t *result_len)
{
	int32_t ret;
	struct lpm_aic_freq_syscnt_info syscnt_info = {0};
	struct lpm_aic_freq_drv_priv *aic_freq_priv = lpm_aic_freq_get_drv_priv_info();

	if (!lpm_aic_freq_drv_check_syscnt_param(dev_id, out, out_len, result_len)) {
		lpm_log_err("check get syscnt param failed, dev_id=%u, out_len=%u\n", dev_id, out_len);
		return -EINVAL;
	}

	(void)memset_s(out, out_len, 0, out_len);

	mutex_lock(&aic_freq_priv->dev_data[dev_id].syscnt_info.syscnt_mutex);
	ret = lpm_read_aic_freq_syscnt_from_sharemem(
		dev_id, LPM_AIC_FREQ_SYSCNT_RETRY_CNT, LPM_AIC_FREQ_SYSCNT_WAIT_TIME, &syscnt_info);
	if (ret != 0) {
		*result_len = 0;
		mutex_unlock(&aic_freq_priv->dev_data[dev_id].syscnt_info.syscnt_mutex);
		lpm_log_err("query aic freq syscnt failed, dev_id=%u, out_len=%u, ret=%d\n",
			dev_id, out_len, ret);
		return ret;
	}

	lpm_aic_freq_drv_get_syscnt_data(dev_id, &syscnt_info, (struct lpm_aic_freq_syscnt_result *)out, result_len);

	mutex_unlock(&aic_freq_priv->dev_data[dev_id].syscnt_info.syscnt_mutex);

	return 0;
}

int32_t lpm_aic_freq_drv_probe(uint64_t *param, uint32_t param_num)
{
	lpm_aic_freq_drv_init_data();

	(void)param;
	(void)param_num;
	lpm_log_info("lpm aic freq drv probe success\n");
	return 0;
}

void lpm_aic_freq_drv_post_suspend(void)
{
	uint32_t dev_id;

	for (dev_id = 0; dev_id < LPM_DEVMNG_DEV_MAX_NUM; dev_id++) {
		// syscnt info may reset after resume
		// causes the timestamp go backwards after resume
		// so lp core will reset syscnt info after resume
		lpm_aic_freq_drv_clr_last_syscnt(dev_id);
	}
	lpm_log_info("lpm aic freq drv resume success\n");
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)

ssize_t lpm_aic_freq_drv_debugfs_write(
	struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
	int32_t ret;
	uint32_t user_cfg[LPM_DEBUGFS_MAX_USER_INPUT_NUM] = {0};
	struct lpm_aic_freq_syscnt_result result = {0};
	uint32_t result_len = 0;
	uint32_t i;

	(void)file;
	(void)pos;

	// magic_num(0x5aa5), dev_id
	ret = lpm_debugfs_get_input(buf, len, user_cfg, 0x2);
	if (ret != 0) {
		lpm_log_err("debugfs write idle failed, ret=%d\n", ret);
		return -1;
	}

	if (user_cfg[0] != 0x5aa5) {
		// do nothing
		goto write_end;
	}

	ret = lpm_aic_freq_drv_get_syscnt_info(
		user_cfg[1], (void *)&result, (uint32_t)sizeof(result), &result_len);
	if (ret != 0) {
		lpm_log_err("debugfs write get syscnt info failed, ret=%d\n", ret);
		goto write_end;
	}

	lpm_log_info("result_len=%u, result.count=%u\n", result_len, result.count);
	if (result.count == 0) {
		// no need print result
		goto write_end;
	}

	for (i = 0; i < result.count; i++) {
		lpm_log_info("[%2u] syscnt=%llu, freq=%u\n", i, result.syscnt[i].syscnt, result.syscnt[i].freq);
	}

write_end:
	return (ssize_t)len;
}

void lpm_aic_freq_drv_debugfs_show(struct seq_file *seq)
{
	struct lpm_aic_freq_drv_priv *aic_freq_priv = lpm_aic_freq_get_drv_priv_info();
	struct lpm_aic_freq_syscnt_info syscnt_info = {0};
	struct lpm_aic_freq_last_syscnt *last_syscnt = NULL;
	uint32_t dev_num = lpm_common_get_dev_num();
	uint32_t dev_id;
	int32_t ret;
	uint32_t i;

	seq_printf(seq, "[aic freq syscnt info]\n");

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		last_syscnt = &aic_freq_priv->dev_data[dev_id].syscnt_info.last_syscnt;

		seq_printf(seq, " dev_id=%u, next_index=0x%x, last syscnt_freq info: syscnt=%llu, freq=%u\n",
			dev_id, last_syscnt->next_index,
			last_syscnt->syscnt_freq.syscnt, last_syscnt->syscnt_freq.freq);

		ret = lpm_read_aic_freq_syscnt_from_sharemem(
			dev_id, LPM_AIC_FREQ_SYSCNT_RETRY_CNT, LPM_AIC_FREQ_SYSCNT_WAIT_TIME, &syscnt_info);
		if (ret != 0) {
			seq_printf(seq, "  dev_id=%u get syscnt from sharemem failed, ret=%d\n", dev_id, ret);
		} else {
			seq_printf(seq, "  sharemem info: freq_index=%u\n", syscnt_info.freq_index);

			for (i = 0; i < LPM_AIC_FREQ_SYSCNT_MAC_NUM; i++) {
				seq_printf(seq, "    [%2u] syscnt=%llu, freq=%u\n",
					i, syscnt_info.syscnt[i].syscnt, syscnt_info.syscnt[i].freq);
			}
		}
	}
}
#endif