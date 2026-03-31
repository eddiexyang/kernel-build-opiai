
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
#ifndef LPM_DEVMNG_AIC_FREQ_SHAREMEM_V2_H
#define LPM_DEVMNG_AIC_FREQ_SHAREMEM_V2_H

#include <linux/types.h>
#include <linux/mutex.h>
#include "lpm_devmng_common.h"
#include "lpm_devmng_sharemem_drv.h"

#ifdef LPM_AIC_FREQ_SYSCNT

#define LPM_AIC_FREQ_SYSCNT_WAIT_TIME LPM_DELAY_100_US
#define LPM_AIC_FREQ_SYSCNT_RETRY_CNT 3

struct lpm_aic_freq_last_syscnt {
	uint16_t next_index;
	uint16_t rsv[3];
	struct lpm_aic_freq_syscnt_freq syscnt_freq;
};

struct lpm_aic_freq_drv_syscnt_priv {
	struct lpm_aic_freq_last_syscnt last_syscnt;
	struct mutex syscnt_mutex;
};

struct lpm_aic_freq_drv_dev_priv {
	struct lpm_aic_freq_drv_syscnt_priv syscnt_info;
};

struct lpm_aic_freq_drv_priv {
	struct lpm_aic_freq_drv_dev_priv dev_data[LPM_DEVMNG_DEV_MAX_NUM];
};

#endif

#endif