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

#ifndef TSDRV_PM_H
#define TSDRV_PM_H

#include <linux/types.h>
#include "devdrv_common.h"

int tsdrv_pm_suspend(u32 devid);
void tsdrv_ts_suspend_ready(u32 devid, u32 tsid, u32 ret);
int tsdrv_pm_resume(u32 devid);
void tsdrv_pm_shutdown(struct devdrv_info *dev_info);

void tsdrv_pm_init(void);
void tsdrv_pm_exit(void);

#endif
