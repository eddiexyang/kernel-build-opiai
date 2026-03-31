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

#ifndef _CAN_DRV_LOG__
#define _CAN_DRV_LOG__

#include "drv_log.h"
#ifdef RUN_IN_AOS
#include "aos_log.h"
#endif

#ifdef LOG_PRINT_PID
#define module_mttcan "mttcan"
#define mttcan_err(fmt, ...) do { \
    drv_err("mttcan", "<%s:%d:%d> " fmt, \
    current->comm, current->tgid, current->pid, ##__VA_ARGS__); \
} while (0)
#define mttcan_warn(fmt, ...) do { \
    drv_warn("mttcan", "<%s:%d:%d> " fmt, \
    current->comm, current->tgid, current->pid, ##__VA_ARGS__); \
} while (0)
#define mttcan_info(fmt, ...) do { \
    drv_info("mttcan", "<%s:%d:%d> " fmt, \
    current->comm, current->tgid, current->pid, ##__VA_ARGS__); \
} while (0)
#define mttcan_debug(fmt, ...)
#else
#define mttcan_err(fmt, ...)        drv_err("mttcan", fmt, ##__VA_ARGS__)
#define mttcan_warn(fmt, ...)       drv_warn("mttcan", fmt, ##__VA_ARGS__)
#define mttcan_info(fmt, ...)       drv_info("mttcan", fmt, ##__VA_ARGS__)
#define mttcan_debug(fmt, ...)      drv_debug("mttcan", fmt, ##__VA_ARGS__)
#endif

#endif /* _CAN_DRV_LOG__ */
