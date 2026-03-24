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
* Create: 2022-7-2
*/

#ifndef SOC_RESMNG_LOG_H__
#define SOC_RESMNG_LOG_H__

#include <linux/sched.h>

#define module_soc_resmng "soc_resmng"

#ifndef EMU_ST
#include "drv_log.h"
#else
#include "ut_log.h"
#endif

#define soc_err(fmt, ...) do { \
    drv_err(module_soc_resmng, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
    share_log_err(TSDRV_SHARE_LOG_START, module_soc_resmng, "<%s:%d> " fmt, current->comm, \
        current->tgid, ##__VA_ARGS__); \
} while (0)

#define soc_warn(fmt, ...)  drv_warn(module_soc_resmng, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define soc_info(fmt, ...)  drv_info(module_soc_resmng, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define soc_event(fmt, ...) drv_event(module_soc_resmng, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define soc_debug(fmt, ...) drv_pr_debug(module_soc_resmng, "<%s:%d> " fmt, \
    current->comm, current->tgid, ##__VA_ARGS__)

#endif /* SOC_RESMNG_LOG_H__ */
