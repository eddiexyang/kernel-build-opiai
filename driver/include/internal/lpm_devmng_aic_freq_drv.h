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

#ifndef LPM_DEVMNG_AIC_FREQ_DRV_H
#define LPM_DEVMNG_AIC_FREQ_DRV_H

#include <linux/types.h>
#include "lpm_devmng_common.h"
#include "lpm_devmng_sharemem_drv.h"

#ifdef LPM_AIC_FREQ_SYSCNT

struct lpm_aic_freq_syscnt_result {
	uint32_t count;
	uint32_t resv;
	struct lpm_aic_freq_syscnt_freq syscnt[LPM_AIC_FREQ_SYSCNT_MAC_NUM];
};

int32_t lpm_aic_freq_drv_probe(uint64_t *param, uint32_t param_num);
void lpm_aic_freq_drv_post_suspend(void);
int32_t lpm_aic_freq_drv_get_syscnt_info(uint32_t dev_id, void *out, uint32_t out_len, uint32_t *result_len);

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
void lpm_aic_freq_drv_debugfs_show(struct seq_file *seq);
ssize_t lpm_aic_freq_drv_debugfs_write(
	struct file *file, const char __user *buf, size_t len, loff_t *pos);
#endif

#else

static inline int32_t lpm_aic_freq_drv_probe(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}

static inline void lpm_aic_freq_drv_post_suspend(void)
{
}

static inline int32_t lpm_aic_freq_drv_get_syscnt_info(
	uint32_t dev_id, void *out, uint32_t out_len, uint32_t *result_len)
{
	(void)dev_id;
	(void)out;
	(void)out_len;
	(void)result_len;
	// not support
	return -EOPNOTSUPP;
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
static inline void lpm_aic_freq_drv_debugfs_show(struct seq_file *seq)
{
	(void)seq;
}
static inline ssize_t lpm_aic_freq_drv_debugfs_write(
	struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
	(void)file;
	(void)buf;
	(void)len;
	(void)pos;
	// not support
	return len;
}
#endif

#endif
#endif