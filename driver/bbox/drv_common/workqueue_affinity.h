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
*
* Description:
* Author: huawei
* Create: 2022-12-25
*/
#ifndef WORKQUEUE_AFFINITY_H
#define WORKQUEUE_AFFINITY_H

#include <linux/types.h>
#include <linux/workqueue.h>

/**
 * Setting the Affinity of the Workqueue Thread
 * @wq: unbound type workqueue
 * @flag: WQ_HIGHPRI or 0
 *
 * Bind the work queue thread to the control CPU.
 */
int set_workqueue_affinity(struct workqueue_struct *wq, u32 flag, const struct cpumask *wq_cpumask);

#endif
