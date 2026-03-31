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
 * Create: 2022-6-27
 */
#ifndef BOARD_CONFIG_MODULE_H
#define BOARD_CONFIG_MODULE_H

#include "drv_log.h"

#define module_bdcfg "dbl_board_config"

#define bdcfg_err(fmt, ...) drv_err(module_bdcfg, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define bdcfg_warn(fmt, ...) drv_warn(module_bdcfg, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define bdcfg_info(fmt, ...) drv_info(module_bdcfg, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define bdcfg_event(fmt, ...) drv_event(module_bdcfg, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define bdcfg_debug(fmt, ...) drv_pr_debug(module_bdcfg, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)

#endif /* BOARD_CONFIG_MODULE_H */
