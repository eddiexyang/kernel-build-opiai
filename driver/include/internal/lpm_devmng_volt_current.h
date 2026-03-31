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
#ifndef LPM_DEVMNG_VOLT_CURRENT_H
#define LPM_DEVMNG_VOLT_CURRENT_H

#include <linux/types.h>
#include "lpm_devmng_common.h"


#ifdef LPM_VOLT_CURRENT

int32_t lpm_volt_current_get_aicore(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len);
int32_t lpm_volt_current_get_taishan(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len);
int32_t lpm_volt_current_get_hybrid(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len);
int32_t lpm_volt_current_get_ddr(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len);

int32_t lpm_volt_current_probe(uint64_t *param, uint32_t param_num);
int32_t lpm_volt_current_remove(uint64_t *param, uint32_t param_num);

#else
static inline int32_t lpm_volt_current_get_aicore(
	uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	(void)dev_id;
	(void)in;
	(void)in_len;
	(void)out_len;
	// Operation not supported
	return -EOPNOTSUPP;
}

static inline int32_t lpm_volt_current_get_hybrid(
	uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	(void)dev_id;
	(void)in;
	(void)in_len;
	(void)out_len;
	// Operation not supported
	return -EOPNOTSUPP;
}

static inline int32_t lpm_volt_current_get_taishan(
	uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	(void)dev_id;
	(void)in;
	(void)in_len;
	(void)out_len;
	// Operation not supported
	return -EOPNOTSUPP;
}

static inline int32_t lpm_volt_current_get_ddr(
	uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	(void)dev_id;
	(void)in;
	(void)in_len;
	(void)out_len;
	// Operation not supported
	return -EOPNOTSUPP;
}

static inline int32_t lpm_volt_current_probe(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}
static inline int32_t lpm_volt_current_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}

#endif

#endif
