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
#ifndef LPM_DEVMNG_IDLE_H
#define LPM_DEVMNG_IDLE_H

#include <linux/types.h>
#include "lpm_devmng_common.h"
#include "lpm_devmng_idle_drv.h"

#ifdef LPM_IDLE

int32_t lpm_idle_probe(uint64_t *param, uint32_t param_num);
int32_t lpm_idle_remove(uint64_t *param, uint32_t param_num);
int32_t lpm_idle_suspend_prepare(void);
int32_t lpm_idle_post_suspend(void);
int32_t lpm_idle_get_status(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len);
int32_t lpm_idle_set_switch(uint32_t dev_id, const char __user *in, uint32_t in_len);
#else

static inline int32_t lpm_idle_probe(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}

static inline int32_t lpm_idle_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}

static inline int32_t lpm_idle_suspend_prepare(void)
{
	return 0;
}

static inline int32_t lpm_idle_post_suspend(void)
{
	return 0;
}

static inline int32_t lpm_idle_get_status(
	uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	// not support
	(void)dev_id;
	(void)in;
	(void)in_len;
	(void)out_len;
	// Operation not supported
	return -EOPNOTSUPP;
}

static inline int32_t lpm_idle_set_switch(uint32_t dev_id, const char __user *in, uint32_t in_len)
{
	// not support
	(void)dev_id;
	(void)in;
	(void)in_len;
	// Operation not supported
	return -EOPNOTSUPP;
}
#endif
#endif