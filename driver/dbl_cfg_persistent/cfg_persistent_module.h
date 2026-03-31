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
#ifndef CFG_PERSISTENT_MODULE_H
#define CFG_PERSISTENT_MODULE_H

#include "drv_log.h"

#define module_cfgpt "dbl_cfg_persistent"

#define cfgpt_err(fmt, ...) drv_err(module_cfgpt, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define cfgpt_warn(fmt, ...) drv_warn(module_cfgpt, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define cfgpt_info(fmt, ...) drv_info(module_cfgpt, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define cfgpt_event(fmt, ...) drv_event(module_cfgpt, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define cfgpt_debug(fmt, ...) drv_pr_debug(module_cfgpt, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)

#endif /* CFG_PERSISTENT_MODULE_H */

