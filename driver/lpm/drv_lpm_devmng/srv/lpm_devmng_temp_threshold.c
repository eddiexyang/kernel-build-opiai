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
#include "lpm_devmng_sharemem_drv.h"
#include "lpm_devmng_temp_threshold.h"

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"

STATIC int32_t lpm_temp_threshold_debugfs_init(void);
#endif

STATIC int32_t lpm_temp_threshold_get(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len,
	struct lpm_temp_threshold_info *threshold_info)
{
	unsigned long cpy_len;
	int32_t ret;
	uint8_t *data = NULL;
	struct lpm_temp_threshold_data temp_data = {0};

	ret = lpm_check_dsmi_get_param(dev_id, in, in_len, threshold_info->in_len, out_len);
	if (ret != 0) {
		lpm_log_err("Get temp_threshold info, check param failed. (dev_id=%u, temp_type=%u, ret=%d)\n",
			dev_id, (uint32_t)threshold_info->temp_type, ret);
		return ret;
	}

	ret = lpm_get_temp_threshold_from_sharemem(
		dev_id, LPM_TEMP_THRESH_READ_RETRY_CNT, LPM_TEMP_THRESH_READ_WAIT_TIME, &temp_data);
	if (ret != 0) {
		lpm_log_err("Get temp_threshold info from sharemem failed. (dev_id=%u, temp_type=%u, ret=%d)\n",
			dev_id, (uint32_t)threshold_info->temp_type, ret);
		return ret;
	}

	if (threshold_info->temp_type == LPM_DEVMNG_TEMP_DDR_GEAR) {
		if ((temp_data.ddr_gear.gear_num <= 0) ||
			(temp_data.ddr_gear.gear_num > (uint32_t)LPM_DEVMNG_DDR_TEMP_GEAR_MAX)) {
			lpm_log_err("invalid ddr gear_num=%u (dev_id=%u, temp_type=%u)\n",
				temp_data.ddr_gear.gear_num, dev_id, (uint32_t)threshold_info->temp_type);
			return -EINVAL;
		}

		threshold_info->offset += (uint32_t)offsetof(struct lpm_temp_threshold_ddr_gear, temp_gear);
		threshold_info->out_len = temp_data.ddr_gear.gear_num / (uint32_t)sizeof(uint8_t);
	}

	data = (uint8_t *)&temp_data;

	cpy_len = copy_to_user(in, &data[threshold_info->offset], (unsigned long)threshold_info->out_len);
	if (cpy_len != 0) {
		lpm_log_err("Get temp_threshold info, copy to user failed. "
			"(dev_id=%u, temp_type=%u, offset=%u, out_len=%u, cpy_len=%lu)\n",
			dev_id, (uint32_t)threshold_info->temp_type, threshold_info->offset, threshold_info->out_len, cpy_len);
		return -EIO;
	}

	*out_len = threshold_info->out_len;
	return 0;
}

int32_t lpm_temp_threshold_get_ddr_gear(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	struct lpm_temp_threshold_info threshold_info = {0};

	threshold_info.offset    = (uint32_t)offsetof(struct lpm_temp_threshold_data, ddr_gear);
	threshold_info.in_len    = (uint32_t)sizeof(uint8_t) * (uint32_t)LPM_DEVMNG_DDR_TEMP_GEAR_MAX;
	threshold_info.out_len   = 0; // read from sharemem
	threshold_info.temp_type = LPM_DEVMNG_TEMP_DDR_GEAR;

	return lpm_temp_threshold_get(dev_id, in, in_len, out_len, &threshold_info);
}

int32_t lpm_temp_threshold_get_ddr_high(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	struct lpm_temp_threshold_info threshold_info = {0};

	threshold_info.offset    = (uint32_t)offsetof(struct lpm_temp_threshold_data, ddr_high);
	threshold_info.in_len    = (uint32_t)sizeof(uint32_t);
	threshold_info.out_len   = (uint32_t)sizeof(uint32_t);
	threshold_info.temp_type = LPM_DEVMNG_TEMP_DDR_HIGH_THRESHOLD;

	return lpm_temp_threshold_get(dev_id, in, in_len, out_len, &threshold_info);
}

int32_t lpm_temp_threshold_get_soc_high(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	struct lpm_temp_threshold_info threshold_info = {0};

	threshold_info.offset    = (uint32_t)offsetof(struct lpm_temp_threshold_data, soc_high);
	threshold_info.in_len    = (uint32_t)sizeof(uint32_t);
	threshold_info.out_len   = (uint32_t)sizeof(uint32_t);
	threshold_info.temp_type = LPM_DEVMNG_TEMP_SOC_HIGH_THRESHOLD;

	return lpm_temp_threshold_get(dev_id, in, in_len, out_len, &threshold_info);
}

int32_t lpm_temp_threshold_get_soc_min(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	struct lpm_temp_threshold_info threshold_info = {0};

	threshold_info.offset    = (uint32_t)offsetof(struct lpm_temp_threshold_data, soc_min);
	threshold_info.in_len    = (uint32_t)sizeof(int32_t);
	threshold_info.out_len   = (uint32_t)sizeof(int32_t);
	threshold_info.temp_type = LPM_DEVMNG_TEMP_SOC_MIN_THRESHOLD;

	return lpm_temp_threshold_get(dev_id, in, in_len, out_len, &threshold_info);
}

int32_t lpm_temp_threshold_probe(uint64_t *param, uint32_t param_num)
{
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	int32_t ret = lpm_temp_threshold_debugfs_init();
	if (ret != 0) {
		lpm_log_err("lpm temp_threshold init debugfs failed, ret=%d\n", ret);
		return ret;
	}
#endif
	(void)param;
	(void)param_num;
	lpm_log_info("lpm temp_threshold probe success\n");
	return 0;
}

int32_t lpm_temp_threshold_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	lpm_log_info("lpm temp_threshold remove success\n");
	return 0;
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)

STATIC void lpm_debugfs_show_temp_info(struct seq_file *seq, const uint8_t *data, uint32_t data_len)
{
	uint32_t i;
	int32_t num = 0;
	uint32_t num_len = (uint32_t)sizeof(num);
	uint32_t byte_per_line = 4; // show 4 byte in each line

	for (i = 0; i < data_len; i++) {
		if ((i % byte_per_line) == 0) {
			seq_printf(seq, "\n    [0x%02x]:", i);
			if ((i + num_len) <= data_len) {
				num = *(int32_t *)&data[i];
				seq_printf(seq, " (int_value of the %u byte: %10d)", byte_per_line, num);
			}
		}
		seq_printf(seq, " 0x%02x", data[i]);
	}
	seq_printf(seq, "\n");
}

STATIC int32_t lpm_temp_threshold_debugfs_show(struct seq_file *seq, void *v)
{
	int32_t ret;
	uint32_t dev_id;
	uint32_t dev_num = lpm_common_get_dev_num();
	struct lpm_temp_threshold_data temp_data = {0};

	seq_printf(seq, "Get temp_threshold info:\n");

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		seq_printf(seq, " [dev_id=%u]\n", dev_id);

		ret = lpm_get_temp_threshold_from_sharemem(
			dev_id, LPM_TEMP_THRESH_READ_RETRY_CNT, LPM_TEMP_THRESH_READ_WAIT_TIME, &temp_data);
		if (ret != 0) {
			seq_printf(seq, "   Read temp_threshold info from sharemem failed, ret=%d\n", ret);
		} else {
			seq_printf(seq, "   Read temp_threshold info from sharemem:");
		}
		lpm_debugfs_show_temp_info(seq, (uint8_t *)&temp_data, (uint32_t)sizeof(temp_data));
	}

	(void)v;
	return 0;
}

STATIC int32_t lpm_temp_threshold_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, lpm_temp_threshold_debugfs_show, inode->i_private);
}

STATIC int32_t lpm_temp_threshold_debugfs_init(void)
{
	int32_t ret;
	struct lpm_debugfs_init_cfg init_cfg = {
		.name = "temp_threshold",
		.fn_open = lpm_temp_threshold_debugfs_open,
		.fn_write = NULL
	};

	ret = lpm_debugfs_register_module(&init_cfg);
	if (ret != 0) {
		lpm_log_err("lpm temp_threshold register debugfs failed, ret=%d\n", ret);
		return ret;
	}
	return 0;
}

#endif
