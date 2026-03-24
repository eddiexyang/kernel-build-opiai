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
*
* Description:
* Author: huawei
* Create: 2023-01-06
*/

#ifndef TSDRV_CPUMASK_H
#define TSDRV_CPUMASK_H
#include <linux/cpumask.h>
#include <linux/sched.h>

void tsdrv_get_ctrlcpu_mask(cpumask_t *cpumask);
void tsdrv_init_ctrlcpu_mask(void);
void tsdrv_bind_cpu(struct task_struct *task);
#endif