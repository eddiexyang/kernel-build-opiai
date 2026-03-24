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

#ifndef QUEUE_WORK_AFFINITY_H
#define QUEUE_WORK_AFFINITY_H

#include <linux/workqueue.h>

#define QUEUE_WORK_BIND_CPU   (0)

/* queue_work_on inline function */
static inline void queue_work_affinity(struct workqueue_struct *wq, struct work_struct *work)
{
    queue_work_on(QUEUE_WORK_BIND_CPU, wq, work);
}
#endif
