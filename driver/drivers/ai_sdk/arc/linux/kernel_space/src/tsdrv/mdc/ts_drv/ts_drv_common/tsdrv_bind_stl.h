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
* Create: 2022-11-11
*/

#ifndef TSDRV_BIND_STL_H
#define TSDRV_BIND_STL_H

#include "tsdrv_ioctl.h"
#include "tsdrv_ctx.h"
int tsdrv_ioctl_bind_stl_operator(struct tsdrv_ctx *ctx, struct devdrv_ioctl_arg *arg);
#endif
