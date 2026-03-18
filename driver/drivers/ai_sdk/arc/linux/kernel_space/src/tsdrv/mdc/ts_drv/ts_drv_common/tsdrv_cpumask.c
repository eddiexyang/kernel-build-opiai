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

#include <linux/errno.h>
#include <linux/cpu.h>
#include <linux/kthread.h>
#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/securec.h>
#include <linux/fs.h>
#include <linux/kallsyms.h>
#include <linux/cpumask.h>

#include "drv_cpu_type.h"
#include "tsdrv_cpumask.h"
#include "tsdrv_log.h"

/**
 * in mdc enviroment, kthread should be bind to ctrlcpu, to avoid the threads
 * run in aicpu
 */
static cpumask_t g_ctrl_cpumask;

void tsdrv_init_ctrlcpu_mask(void)
{
    int ret;

    cpumask_clear(&g_ctrl_cpumask);
    ret = drv_get_ctrlcpu_mask(&g_ctrl_cpumask);
    if (ret) {
        TSDRV_PRINT_ERR("get ctrl cpumask info failed. (ret=%d)\n", ret);
    }
    return;
}

void tsdrv_get_ctrlcpu_mask(cpumask_t *cpumask)
{
    cpumask_copy(cpumask, &g_ctrl_cpumask);
}

void tsdrv_bind_cpu(struct task_struct *task)
{
#ifndef TSDRV_UT
    if (task == NULL) {
        TSDRV_PRINT_ERR("task is null.\n");
        return;
    }

    if (cpumask_empty(&g_ctrl_cpumask)) {
        TSDRV_PRINT_ERR("cpumask is empty.\n");
        return;
    }

    if (cpumask_available(&g_ctrl_cpumask)) {
        set_cpus_allowed_ptr(task, &g_ctrl_cpumask);
    }
#endif
    return;
}
EXPORT_SYMBOL(tsdrv_bind_cpu);
