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

#ifndef TSDRV_LOG_H
#define TSDRV_LOG_H

#include <linux/sched.h>

#ifdef TSDRV_UT
#ifndef STATIC
#define STATIC
#endif

#include <stdio.h>
#include <stdarg.h>
#include <linux/printk.h>

#define TSDRV_PRINT_ERR(fmt, ...)                                           \
    printk(KERN_ERR "[ascend][ERROR][%s][%s %d]<%s:%d>" fmt, "tsdrv",       \
        __func__, __LINE__, current->comm, current->tgid, ##__VA_ARGS__)

#define TSDRV_PRINT_WARN(fmt, ...)                                          \
    printk(KERN_WARNING "[ascend][WARN][%s][%s %d]<%s:%d>" fmt, "tsdrv",    \
        __func__, __LINE__, current->comm, current->tgid, ##__VA_ARGS__)

#define TSDRV_PRINT_INFO(fmt, ...)                                          \
    printk(KERN_INFO "[ascend][INFO][%s][%s %d]<%s:%d>" fmt, "tsdrv",       \
        __func__, __LINE__, current->comm, current->tgid, ##__VA_ARGS__)

#define TSDRV_PRINT_EVENT(fmt, ...)                                         \
    printk(KERN_NOTICE "[ascend][EVENT][%s][%s %d]<%s:%d>" fmt, "tsdrv",    \
        __func__, __LINE__, current->comm, current->tgid, ##__VA_ARGS__)

#define TSDRV_PRINT_DEBUG(fmt, ...)                                         \
    printk(KERN_DEBUG "[ascend][DBG][%s][%s %d]<%s:%d>" fmt, "tsdrv",       \
        __func__, __LINE__, current->comm, current->tgid, ##__VA_ARGS__)

#define TSDRV_SPINLOCK_ERR(fmt...)    TSDRV_PRINT_ERR(fmt)
#define TSDRV_SPINLOCK_WARN(fmt...)   TSDRV_PRINT_WARN(fmt)
#define TSDRV_SPINLOCK_INFO(fmt...)   TSDRV_PRINT_INFO(fmt)
#define TSDRV_SPINLOCK_EVENT(fmt...)  TSDRV_PRINT_EVENT(fmt)
#define TSDRV_SPINLOCK_DEBUG(fmt...)  TSDRV_PRINT_DEBUG(fmt)

#define TSDRV_PRINT_FPGA(fmt...)      TSDRV_PRINT_DEBUG(fmt)
#define TSDRV_SAMPLE_BACK_DEBUG(fmt...)      TSDRV_PRINT_DEBUG(fmt)

#else /* !TSDRV_UT */
#include "drv_log.h"

#define module_tsdrv "tsdrv"

#ifndef STATIC
#define STATIC static
#endif

#define TSDRV_PRINT_ERR(fmt, ...) do { \
    drv_err(module_tsdrv, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
    share_log_err(TSDRV_SHARE_LOG_START, fmt, ##__VA_ARGS__); \
} while (0)

#define TSDRV_PRINT_WARN(fmt, ...)  drv_warn(module_tsdrv, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define TSDRV_PRINT_INFO(fmt, ...)  drv_info(module_tsdrv, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define TSDRV_PRINT_EVENT(fmt, ...) drv_event(module_tsdrv, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define TSDRV_PRINT_DEBUG(fmt, ...) drv_pr_debug(module_tsdrv, "<%s:%d> " fmt,     \
        current->comm, current->tgid, ##__VA_ARGS__)

#define TSDRV_SPINLOCK_ERR(fmt, ...)    \
        drv_err_spinlock(module_tsdrv, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define TSDRV_SPINLOCK_WARN(fmt, ...)   \
        drv_warn_spinlock(module_tsdrv, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define TSDRV_SPINLOCK_INFO(fmt, ...)   \
        drv_info_spinlock(module_tsdrv, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define TSDRV_SPINLOCK_DEBUG(fmt, ...)  \
        drv_debug_spinlock(module_tsdrv, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define TSDRV_SPINLOCK_EVENT(fmt, ...)  \
        drv_event_spinlock(module_tsdrv, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)

/* for fpga debug */
#define TSDRV_PRINT_FPGA(fmt, ...)

#ifdef CFG_SOC_SAMPLE_BACK_DEBUG
#  define TSDRV_SAMPLE_BACK_DEBUG TSDRV_PRINT_INFO
#else
#  define TSDRV_SAMPLE_BACK_DEBUG
#endif

#endif /* TSDRV_UT */

#endif /* TSDRV_LOG_H */
