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
#include "lpm_devmng_volt_current.h"

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"

STATIC int32_t lpm_volt_current_debugfs_init(void);
#endif

STATIC int32_t lpm_volt_current_get(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len,
	enum lpm_devmng_volt_current_type volt_current_type)
{
	unsigned long cpy_len;
	struct lpm_devmng_volt_current_info volt_current = {0};

	int32_t ret = lpm_check_dsmi_get_param(dev_id, in, in_len, (uint32_t)sizeof(volt_current), out_len);
	if (ret != 0) {
		lpm_log_err("Get volt_current info, check param failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}

	ret = lpm_get_pmu_volt_current_from_sharemem(dev_id, volt_current_type, &volt_current);
	if (ret != 0) {
		lpm_log_err("Read volt_current from sharemem failed. (dev_id=%u;ret=%d)\n", dev_id, ret);
		return ret;
	}

	cpy_len = copy_to_user(in, &volt_current, sizeof(volt_current));
	if (cpy_len != 0) {
		lpm_log_err("Get volt_current info, copy to user failed. (dev_id=%u; cpy_len=%lu)\n", dev_id, cpy_len);
		return -EIO;
	}

	*out_len = (uint32_t)sizeof(volt_current);
	return 0;
}

int32_t lpm_volt_current_get_aicore(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	return lpm_volt_current_get(dev_id, in, in_len, out_len, LPM_DEVMNG_VOLT_CURRENT_AICORE);
}

int32_t lpm_volt_current_get_taishan(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	return lpm_volt_current_get(dev_id, in, in_len, out_len, LPM_DEVMNG_VOLT_CURRENT_TAISHAN);
}

int32_t lpm_volt_current_get_hybrid(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
#ifdef LPM_VOLT_CURRENT_WITH_HYBRID
	return lpm_volt_current_get(dev_id, in, in_len, out_len, LPM_DEVMNG_VOLT_CURRENT_HYBRID);
#else
	(void)dev_id;
	(void)in;
	(void)in_len;
	(void)out_len;
	return -EOPNOTSUPP;
#endif
}

int32_t lpm_volt_current_get_ddr(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
#ifdef LPM_VOLT_CURRENT_WITH_DDR
	return lpm_volt_current_get(dev_id, in, in_len, out_len, LPM_DEVMNG_VOLT_CURRENT_DDR);
#else
	(void)dev_id;
	(void)in;
	(void)in_len;
	(void)out_len;
	return -EOPNOTSUPP;
#endif
}

int32_t lpm_volt_current_probe(uint64_t *param, uint32_t param_num)
{
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	int32_t ret = lpm_volt_current_debugfs_init();
	if (ret != 0) {
		lpm_log_err("lpm volt_current init debugfs failed, ret=%d\n", ret);
		return ret;
	}
#endif
	(void)param;
	(void)param_num;
	return 0;
}

int32_t lpm_volt_current_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)

STATIC int32_t lpm_volt_current_debugfs_show(struct seq_file *seq, void *v)
{
	int32_t ret;
	uint32_t dev_id;
	enum lpm_devmng_volt_current_type volt_current_type;
	struct lpm_devmng_volt_current_info volt_current = {0};
	uint32_t dev_num = lpm_common_get_dev_num();

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		for (volt_current_type = LPM_DEVMNG_VOLT_CURRENT_AICORE;
			 volt_current_type <= LPM_DEVMNG_VOLT_CURRENT_DDR; volt_current_type++) {
			ret = lpm_get_pmu_volt_current_from_sharemem(dev_id, volt_current_type, &volt_current);
			if (ret != 0) {
				seq_printf(seq, "Read volt_current from sharemem failed. (dev_id=%u;ret=%d)\n", dev_id, ret);
			} else {
				seq_printf(seq, "dev_id=%u type=%u volt=0x%x current=0x%x\n",
					dev_id, (uint32_t)volt_current_type, volt_current.volt, volt_current.curr);
			}
		}
	}

	(void)v;
	return 0;
}

STATIC int32_t lpm_volt_current_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, lpm_volt_current_debugfs_show, inode->i_private);
}

STATIC int32_t lpm_volt_current_debugfs_init(void)
{
	int32_t ret;
	struct lpm_debugfs_init_cfg init_cfg = {
		.name = "volt_current",
		.fn_open = lpm_volt_current_debugfs_open,
		.fn_write = NULL
	};

	ret = lpm_debugfs_register_module(&init_cfg);
	if (ret != 0) {
		lpm_log_err("lpm volt_current register debugfs failed, ret=%d\n", ret);
		return ret;
	}
	return 0;
}

#endif