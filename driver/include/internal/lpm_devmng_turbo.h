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
#ifndef LPM_DEVMNG_TURBO_H
#define LPM_DEVMNG_TURBO_H

#include <linux/types.h>
#include "lpm_devmng_common.h"

struct lpm_turbo_dsmi_cfg_in {
	uint8_t enable; // 0: close, 1: open
	uint8_t reserved[15];
};

struct lpm_turbo_set_max_freq_send_ipc {
	uint16_t max_freq;  // unit:M
} __attribute__((packed));

struct lpm_turbo_set_max_freq_ack_ipc {
	uint8_t result; // 0:success, 1:fail
} __attribute__((packed));

#if defined(LPM_TURBO) && defined(LPM_BUILD_DEBUG)

struct lpm_turbo_ipc_send {
	uint8_t enable;
};

int32_t lpm_turbo_probe(uint64_t *param, uint32_t param_num);
int32_t lpm_turbo_remove(uint64_t *param, uint32_t param_num);
int32_t lpm_turbo_set_aic_max_freq(uint32_t dev_id, uint32_t max_freq);

#else

static inline int32_t lpm_turbo_probe(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}
static inline int32_t lpm_turbo_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}

static inline int32_t lpm_turbo_set_aic_max_freq(uint32_t dev_id, uint32_t max_freq)
{
	(void)dev_id;
	(void)max_freq;
	return 0;
}
#endif

#endif
