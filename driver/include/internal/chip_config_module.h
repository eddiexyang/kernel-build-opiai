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
 * Create: 2022-05-24
 */
#ifndef CHIP_CONFIG_MODULE_H
#define CHIP_CONFIG_MODULE_H

#ifdef STATIC_SKIP
#define STATIC
#else
#define STATIC                     static
#endif

#include "drv_log.h"

#define module_ccfg "dbl_chip_config"

#define ccfg_err(fmt, ...) drv_err(module_ccfg, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define ccfg_warn(fmt, ...) drv_warn(module_ccfg, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define ccfg_info(fmt, ...) drv_info(module_ccfg, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define ccfg_event(fmt, ...) drv_event(module_ccfg, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define ccfg_debug(fmt, ...) drv_pr_debug(module_ccfg, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)

#endif /* CHIP_CONFIG_MODULE_H */
