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
#ifndef LPM_DEVMNG_FREQUENCY_SHAREMEM_V2_H
#define LPM_DEVMNG_FREQUENCY_SHAREMEM_V2_H

#include <linux/types.h>
#include "lpm_devmng_common.h"
#include "lpm_devmng_sharemem_drv.h"

#ifdef LPM_FREQUENCY

struct lpm_frequency_sharemem_cfg {
	enum lpm_devmng_core_id core_id;
	enum lpm_devmng_frequency_type freq_type;
};

struct lpm_frequency_query_sharemem_list {
	uint32_t support_num;
	struct lpm_frequency_sharemem_cfg *sharemem_cfg;
};

struct lpm_frequency_drv_sharemem_priv {
	// supported frequency types for queries from sharemem
	struct lpm_frequency_query_sharemem_list query_list;
};

#endif

#endif
