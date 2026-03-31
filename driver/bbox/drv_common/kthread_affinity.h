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

#ifndef KTHREAD_AFFINITY_H
#define KTHREAD_AFFINITY_H

#include <linux/kthread.h>
#include <linux/cpumask.h>
#include <linux/sched.h>

#include "drv_cpu_type.h"

/* kthread_create security risk elimination */
static inline void kthread_bind_to_ctrl_cpu(struct task_struct *thread)
{
    static cpumask_t ctrl_cpumask;

    drv_get_ctrlcpu_mask(&ctrl_cpumask);
    if (cpumask_available(&ctrl_cpumask)) {
        set_cpus_allowed_ptr(thread, &ctrl_cpumask);
    }
}
#endif