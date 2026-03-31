/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
#ifndef LPM_DEVMNG_AIC_FREQ_H
#define LPM_DEVMNG_AIC_FREQ_H

#include <linux/types.h>
#include <linux/mutex.h>
#include "lpm_devmng_common.h"

#ifdef LPM_AIC_FREQ
#define LPM_ADJ_REDUCE_READ_WAIT_TIME LPM_DELAY_100_US
#define LPM_ADJ_REDUCE_READ_RETRY_CNT 3

int32_t lpm_aic_freq_probe(uint64_t *param, uint32_t param_num);
int32_t lpm_aic_freq_remove(uint64_t *param, uint32_t param_num);
void lpm_aic_freq_post_suspend(void);
int32_t lpm_aic_freq_get_adj_cause(
	uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len);

int32_t lpm_aic_freq_get_syscnt_info(uint32_t dev_id, void *out, uint32_t out_len, uint32_t *result_len);

#else

static inline int32_t lpm_aic_freq_probe(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}

static inline int32_t lpm_aic_freq_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}

static inline void lpm_aic_freq_post_suspend(void)
{
}

static inline int32_t lpm_aic_freq_get_adj_cause(
	uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	(void)dev_id;
	(void)in;
	(void)in_len;
	(void)out_len;
	// Operation not supported
	return -EOPNOTSUPP;
}

#endif

#endif
