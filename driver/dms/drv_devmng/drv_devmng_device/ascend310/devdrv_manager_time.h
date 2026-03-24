/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* Description:
* Author: huawei
* Create: 2019-10-15
*/

#ifndef __DEVDRV_MANAGER_TIME_H
#define __DEVDRV_MANAGER_TIME_H

#include <linux/time.h>

#include "devdrv_manager_common.h"

#ifdef DEVMNG_UT
#define READ_SYSTEM_COUNTER_KER(cnt)
#else
#define READ_SYSTEM_COUNTER_KER(cnt) asm volatile("mrs %0, CNTVCT_EL0" : "=r"(cnt) :)
#endif

u64 devdrv_manager_get_cpu_tick(void);

#endif
