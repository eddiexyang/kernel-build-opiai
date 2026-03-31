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
#ifndef LPM_DEVMNG_HEALTH_STATUS_H
#define LPM_DEVMNG_HEALTH_STATUS_H

#include <linux/types.h>

struct lpm_health_status {
	uint32_t status;
	uint64_t status_info;
};

#ifdef LPM_HEALTH_STATUS

int32_t lpm_get_health_status(char *in, uint32_t in_len, char *out, uint32_t out_len);
int32_t lpm_health_status_probe(uint64_t *param, uint32_t param_num);
int32_t lpm_health_status_remove(uint64_t *param, uint32_t param_num);

#else
static inline int32_t lpm_health_status_probe(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}
static inline int32_t lpm_health_status_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}

static inline int32_t lpm_get_health_status(char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	(void)in;
	(void)in_len;
	(void)out;
	(void)out_len;
	// Operation not supported
	return -EOPNOTSUPP;
}
#endif

#endif
