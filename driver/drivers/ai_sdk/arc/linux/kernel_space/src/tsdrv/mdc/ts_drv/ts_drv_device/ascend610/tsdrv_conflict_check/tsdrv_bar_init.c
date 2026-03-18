/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
#include "tsdrv_log.h"
#include "devdrv_driver_pm.h"
#include "tsdrv_set_runtime_conflict_check.h"
#include "tsdrv_bar_init.h"

int tsdrv_bar_space_init_conflict_check(u32 devid)
{
    // means it is dc
#if !defined(CFG_SOC_PLATFORM_MDC) && !defined(CFG_SOC_PLATFORM_MINIV3)
    int ret;

    ret = devdrv_set_runtime_runningplat(devid, DEVDRV_RUNTIME_AVAILABLE);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("init bar space runtime_runningplat failed, devid(%u)\n", devid);
        return ret;
#endif
    }
#endif /* CFG_SOC_PLATFORM_MDC */

    // means it is mdc
    return 0;
}

