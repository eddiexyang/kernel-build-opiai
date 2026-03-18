/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * Create: 2022-08-13
 */

#ifndef __TSMNG_LOG_H__
#define __TSMNG_LOG_H__

#ifndef EMU_ST
#include "drv_log.h"
#else
#include <linux/sched.h>
#include "ut_log.h"
#endif

#define module_tsmng "tsmng"


#ifdef UT_VCAST
#define tsmng_drv_err(fmt, ...) drv_err(module_tsmng, fmt, ##__VA_ARGS__)
#define tsmng_drv_warn(fmt, ...) drv_warn(module_tsmng, fmt, ##__VA_ARGS__)
#define tsmng_drv_info(fmt, ...) drv_info(module_tsmng, fmt, ##__VA_ARGS__)
#define tsmng_drv_event(fmt, ...) drv_event(module_tsmng, fmt, ##__VA_ARGS__)
#define tsmng_drv_debug(fmt, ...) drv_pr_debug(module_tsmng, fmt, ##__VA_ARGS__)
#else
#ifdef CFG_FEATURE_SHARE_LOG
#define tsmng_drv_err(fmt, ...) do { \
    drv_err(module_tsmng, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
    share_log_err(DEVMNG_SHARE_LOG_START, fmt, ##__VA_ARGS__); \
} while (0)
#else
#define tsmng_drv_err(fmt, ...) do { \
    drv_err(module_tsmng, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
} while (0)
#endif

#define tsmng_drv_warn(fmt, ...) drv_warn(module_tsmng, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define tsmng_drv_info(fmt, ...) drv_info(module_tsmng, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define tsmng_drv_event(fmt, ...) drv_event(module_tsmng, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define tsmng_drv_debug(fmt, ...) drv_pr_debug(module_tsmng, "<%s:%d> " fmt,      \
        current->comm, current->tgid, ##__VA_ARGS__)
#endif /* UT_VCAST */

#define tsmng_log_adust(times, fmt, ...) do { \
                if (times >= IPC_RETRY_TIME) \
                    drv_err(module_tsmng, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__);      \
                else      \
                    drv_warn(module_tsmng, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__);      \
} while (0)


#endif  /* __TSMNG_LOG_H__ */
