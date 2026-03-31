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

#ifndef TIMER_AFFINITY_H
#define TIMER_AFFINITY_H

#include <linux/timer.h>

#define TIMER_BIND_CPU   (0)

/* add_timer_on inline function */
static inline void add_timer_affinity(struct timer_list *timer)
{
    add_timer_on(timer, TIMER_BIND_CPU);
}
#endif
