/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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
#ifndef TS_DRV_INIT_H
#define TS_DRV_INIT_H

#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/cpumask.h>
#include <linux/sched.h>
#include <linux/io.h>

#include "hwts_drv_config.h"

#ifdef TSDRV_UT
#define STATIC

#define ts_drv_err(fmt, ...)
#define ts_drv_warn(fmt, ...)
#define ts_drv_info(fmt, ...)
#define ts_drv_debug(fmt, ...)

#else /* !TSDRV_UT */

#include "drv_log.h"

#define STATIC static

/* log */
#define module_hwts "tsdrv_aicpu"
#define ts_drv_err(fmt, ...) drv_err(module_hwts, "<%s:%d>" fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define ts_drv_warn(fmt, ...) drv_warn(module_hwts, "<%s:%d>" fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define ts_drv_info(fmt, ...) drv_info(module_hwts, "<%s:%d>" fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define ts_drv_debug(fmt, ...) drv_pr_debug(module_hwts, "<%s:%d>"fmt, current->comm, current->tgid, ##__VA_ARGS__)

#endif /* TSDRV_UT */

struct ts_context {
    int ts_id;
};

#define MINOR_DEV_COUNT    1
#define DEVNODE_PERMISSION 0600

struct drv_hwts_ctrl {
    int numa_node;
    struct mutex mutex;
    struct drv_hwts_ts_info ts_info[MAX_TS_NUM];
};

extern struct drv_hwts_ctrl *g_drv_ctrl_hwts[CHIP_NUM_MAX];

#endif
