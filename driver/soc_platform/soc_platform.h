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
 * Create: 2022-09-15
 */
#ifndef SOC_PLATFORM_H__
#define SOC_PLATFORM_H__

#include <linux/types.h>
#include <linux/sched.h>
#include <linux/smp.h>

#ifndef EMU_ST
#include "drv_log.h"
#else
#include "ut_log.h"
#endif

#define module_log_name "soc_platform"

#define soc_err(fmt, ...) do { \
    drv_err(module_log_name, "<%s:%d:%d:%d> " fmt, \
        current->comm, current->tgid, current->pid, smp_processor_id(), ##__VA_ARGS__); \
} while (0)
#define soc_warn(fmt, ...) do { \
    drv_warn(module_log_name, "<%s:%d:%d:%d> " fmt, \
        current->comm, current->tgid, current->pid, smp_processor_id(), ##__VA_ARGS__); \
} while (0)
#define soc_info(fmt, ...) do { \
    drv_info(module_log_name, "<%s:%d:%d:%d> " fmt, \
        current->comm, current->tgid, current->pid, smp_processor_id(), ##__VA_ARGS__); \
} while (0)
#define soc_debug(fmt, ...) do { \
    drv_pr_debug(module_log_name, "<%s:%d:%d:%d> " fmt, \
        current->comm, current->tgid, current->pid, smp_processor_id(), ##__VA_ARGS__); \
} while (0)

#endif
