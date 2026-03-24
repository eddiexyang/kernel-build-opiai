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

#ifndef FPDC_INCLUDE_FPDC_DEFINE_H
#define FPDC_INCLUDE_FPDC_DEFINE_H

#include "drv_log.h"


#define MODULE_FPDC "fpdc"
#define fpdc_err(fmt, ...) drv_err(MODULE_FPDC, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define fpdc_warn(fmt, ...) drv_warn(MODULE_FPDC, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define fpdc_info(fmt, ...) drv_info(MODULE_FPDC, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define fpdc_event(fmt, ...) drv_event(MODULE_FPDC, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define fpdc_debug(fmt, ...) drv_debug(MODULE_FPDC, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)


#ifdef STATIC_SKIP
#define STATIC
#else
#define STATIC static
#endif

#ifdef CFG_BUILD_DEBUG
#define EXPORT_SYMBOL_UNRELEASE(symbol) EXPORT_SYMBOL(symbol)
#else
#define EXPORT_SYMBOL_UNRELEASE(symbol)
#endif


#endif
