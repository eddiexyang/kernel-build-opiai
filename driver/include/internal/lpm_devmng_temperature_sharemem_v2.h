
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
#ifndef LPM_DEVMNG_TEMPERATURE_SHAREMEM_V2_H
#define LPM_DEVMNG_TEMPERATURE_SHAREMEM_V2_H

#include <linux/types.h>
#include "lpm_devmng_common.h"
#include "lpm_devmng_sharemem_drv.h"

#ifdef LPM_TEMPERATURE

struct lpm_temperature_drv_support_cfg {
	enum lpm_devmng_core_id core_id;
	enum lpm_devmng_temperature_type temp_type;
};

struct lpm_temperature_drv_query_list {
	// supported temperature types for queries
	uint32_t support_num;
	struct lpm_temperature_drv_support_cfg *support_cfg;
};

struct lpm_temperature_drv_sharemem_priv {
	// supported temperature types for queries
	struct lpm_temperature_drv_query_list query_list;
};

#endif

#endif
