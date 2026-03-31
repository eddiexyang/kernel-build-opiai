
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
#ifndef LPM_DEVMNG_TEMPERATURE_ASCEND610_H
#define LPM_DEVMNG_TEMPERATURE_ASCEND610_H

#include <linux/types.h>
#include "lpm_devmng_common.h"
#include "lpm_devmng_ipc.h"

#ifdef LPM_TEMPERATURE

struct lpm_query_temp_ipc {
	uint32_t result; // 0: success, 1: fail
	uint32_t soc_max_temp;
};

#endif

#endif
