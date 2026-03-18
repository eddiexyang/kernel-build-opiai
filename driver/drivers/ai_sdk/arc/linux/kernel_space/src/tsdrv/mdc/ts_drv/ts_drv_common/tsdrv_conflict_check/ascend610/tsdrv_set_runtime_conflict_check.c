/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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
#include "tsdrv_set_runtime_conflict_check.h"
#include "devdrv_driver_pm.h"
#include "devdrv_common.h"
#include "tsdrv_device.h"

int tsdrv_set_runtime_run_conflict_check(u32 devid)
{
    // means it is mdc
#if defined(CFG_SOC_PLATFORM_MDC) || defined(CFG_SOC_PLATFORM_HELPER) || defined(CFG_SOC_PLATFORM_MINIV3)
    TSDRV_PRINT_INFO("enter func.");
    return 0;
    // means it is dc
#else
    u64 get_status = 0;
    u64 local_status;

    local_status = (tsdrv_get_env_type() == TSDRV_ENV_ONLINE) ? DEVDRV_RUNTIME_ON_HOST : DEVDRV_RUNTIME_ON_DEVICE;

    if (devdrv_get_runtime_runningplat(devid, &get_status)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("dev(%u) get runtime status faild,local_status=0x%llx, get status:0x%llx.\n",
                        devid, local_status, get_status);
        return -ENODEV;
#endif
    }

    if (get_status == local_status) {
        return 0;
    } else if (get_status == DEVDRV_RUNTIME_AVAILABLE) {
#ifndef TSDRV_UT
        if (devdrv_set_runtime_runningplat(devid, local_status)) {
            TSDRV_PRINT_ERR("dev(%u) set runtime status faild, plat_type:%d, local status:0x%llx.\n",
                            devid, (int)tsdrv_get_env_type(), local_status);
            return -ENODEV;
        }

        return 0;
#endif
    } else {
#ifndef TSDRV_UT
        TSDRV_PRINT_WARN("runtime running on the other side... \nplease check your process... \n"
                         "local_status=0x%llx, get_status=0x%llx\n", local_status, get_status);
        return -EAGAIN;
#endif
    }

#endif /* CFG_SOC_PLATFORM_MDC */
}

int tsdrv_set_runtime_available_conflict_check(u32 devid)
{
    // means it is mdc
#if defined(CFG_SOC_PLATFORM_MDC) || defined(CFG_SOC_PLATFORM_HELPER) || defined(CFG_SOC_PLATFORM_MINIV3)
    return 0;

    // means it is dc
#else
    u32 ctx_sum_num = 0;
    u32 i;

    for (i = 0; i < TSDRV_MAX_FID_NUM; i++) {
        ctx_sum_num += tsdrv_dev_get_ctx_num(devid, i);
    }

    // there is runtime process running in the system, just return ok, do nothing.
    if (ctx_sum_num != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_INFO("dev(%u) ctx_num(%u).\n", devid, ctx_sum_num);
        return 0;
#endif
    }

    // no runtime process running in the system, so set running plat to available
    if (devdrv_set_runtime_runningplat(devid, DEVDRV_RUNTIME_AVAILABLE)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("dev(%u) devdrv_set_runtime_runningplat() faild.\n", devid);
        return -ENODEV;
#endif
    }
    return 0;

#endif /* CFG_SOC_PLATFORM_MDC */
}

