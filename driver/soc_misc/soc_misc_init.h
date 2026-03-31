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

#ifndef __SOC_MISC_INIT_H
#define __SOC_MISC_INIT_H

#include <linux/types.h>
#include <linux/securec.h>
#include <linux/cdev.h>

#include "drv_log.h"

#define MODULE_SOC "soc_misc"
#ifdef UT_VCAST
#define soc_misc_drv_err(fmt, ...) drv_err(MODULE_SOC, fmt, ##__VA_ARGS__)
#define soc_misc_drv_warn(fmt, ...) drv_warn(MODULE_SOC, fmt, ##__VA_ARGS__)
#define soc_misc_drv_info(fmt, ...) drv_info(MODULE_SOC, fmt, ##__VA_ARGS__)
#define soc_misc_drv_event(fmt, ...) drv_event(MODULE_SOC, fmt, ##__VA_ARGS__)
#define soc_misc_drv_debug(fmt, ...) drv_debug(MODULE_SOC, fmt, ##__VA_ARGS__)
#else
#define soc_misc_drv_err(fmt, ...) drv_err(MODULE_SOC, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define soc_misc_drv_warn(fmt, ...) drv_warn(MODULE_SOC, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define soc_misc_drv_info(fmt, ...) drv_info(MODULE_SOC, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define soc_misc_drv_event(fmt, ...) drv_event(MODULE_SOC, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define soc_misc_drv_debug(fmt, ...) drv_debug(MODULE_SOC, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#endif
#ifndef DRV_SOC_MISC_UT
#define STATIC static
#else
#define STATIC
#endif

#endif /* __DRV_SOC_H */
