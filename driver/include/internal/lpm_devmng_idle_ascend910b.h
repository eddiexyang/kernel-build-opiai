
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
#ifndef LPM_DEVMNG_IDLE_ASCEND910B_H
#define LPM_DEVMNG_IDLE_ASCEND910B_H

#include <linux/types.h>
#include "lpm_devmng_common.h"

#ifdef LPM_IDLE

struct lpm_idle_switch_ipc_send {
	uint8_t idle_switch;
};


#endif

#endif