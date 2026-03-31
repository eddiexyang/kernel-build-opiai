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
* Create: 2022-08-27
*/
#ifndef TSDRV_SYS_PANIC_H
#define TSDRV_SYS_PANIC_H
#include <linux/notifier.h>

int tsdrv_ts_panic_notify(struct notifier_block *nb, unsigned long event, void *args);

#endif

