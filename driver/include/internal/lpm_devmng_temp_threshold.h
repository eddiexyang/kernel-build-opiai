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
#ifndef LPM_DEVMNG_TEMP_THRESHOLD_H
#define LPM_DEVMNG_TEMP_THRESHOLD_H

#include <linux/types.h>
#include "lpm_devmng_common.h"

#define LPM_TEMP_THRESH_READ_WAIT_TIME LPM_DELAY_100_US
#define LPM_TEMP_THRESH_READ_RETRY_CNT 3

enum lpm_devmng_temp_threshold_type {
	LPM_DEVMNG_TEMP_DDR_GEAR,
	LPM_DEVMNG_TEMP_DDR_HIGH_THRESHOLD,
	LPM_DEVMNG_TEMP_SOC_HIGH_THRESHOLD,
	LPM_DEVMNG_TEMP_SOC_MIN_THRESHOLD,
	LPM_DEVMNG_TEMP_THRESHOLD_TYPE_MAX
};

struct lpm_temp_threshold_info {
	uint32_t offset;
	uint32_t in_len;
	uint32_t out_len;
	enum lpm_devmng_temp_threshold_type temp_type;
};

#ifdef LPM_TEMP_THRESHOLD
int32_t lpm_temp_threshold_get_ddr_gear(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len);
int32_t lpm_temp_threshold_get_ddr_high(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len);
int32_t lpm_temp_threshold_get_soc_high(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len);
int32_t lpm_temp_threshold_get_soc_min(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len);

int32_t lpm_temp_threshold_probe(uint64_t *param, uint32_t param_num);
int32_t lpm_temp_threshold_remove(uint64_t *param, uint32_t param_num);

#else
static inline int32_t lpm_temp_threshold_get_ddr_gear(
	uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	(void)dev_id;
	(void)in;
	(void)in_len;
	(void)out_len;
	// Operation not supported
	return -EOPNOTSUPP;
}

static inline int32_t lpm_temp_threshold_get_ddr_high(
	uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	(void)dev_id;
	(void)in;
	(void)in_len;
	(void)out_len;
	// Operation not supported
	return -EOPNOTSUPP;
}

static inline int32_t lpm_temp_threshold_get_soc_high(
	uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	(void)dev_id;
	(void)in;
	(void)in_len;
	(void)out_len;
	// Operation not supported
	return -EOPNOTSUPP;
}

static inline int32_t lpm_temp_threshold_get_soc_min(
	uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	(void)dev_id;
	(void)in;
	(void)in_len;
	(void)out_len;
	// Operation not supported
	return -EOPNOTSUPP;
}

static inline int32_t lpm_temp_threshold_probe(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}

static inline int32_t lpm_temp_threshold_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}

#endif

#endif
