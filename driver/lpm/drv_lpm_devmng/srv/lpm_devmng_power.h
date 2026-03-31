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
#ifndef LPM_DEVMNG_POWER_H
#define LPM_DEVMNG_POWER_H

#include <linux/types.h>
#include <linux/mutex.h>
#include "lpm_devmng_common.h"

#ifdef LPM_POWER

#define POWER_INFO_RESERVED_BYTES 32
struct lpm_power_info {
	uint32_t soc_rated_power;
	uint8_t reserved[POWER_INFO_RESERVED_BYTES];
} __attribute__((packed));

int32_t lpm_power_get_info(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len);

#else
static inline int32_t lpm_power_get_info(
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
