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
* Create: 2022-8-26
*/
#ifndef TSDRV_UT
#include <linux/time.h>

#include "tsdrv_common.h"
#include "devdrv_dfm.h"
#include "devdrv_interface.h"

#define KEY_CHIP_TYPE     (8)

int tsdrv_get_chip_type(void)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    int chip_type = -1;
    if (centre_notify_get_val(KEY_CHIP_TYPE, &chip_type) != 0) {
        devdrv_drv_err("get chip type failed.\n");
        return -ENOSYS;
    }
    return chip_type;
#elif defined(CFG_SOC_PLATFORM_MINIV3)
    return HISI_MINI_V3;
#else
    return CHIP_TYPE_NOT_SET;
#endif
}

int tsdrv_bbox_write(u32 devid, const char *msg)
{
#if defined(CFG_SOC_PLATFORM_MINI) && !defined(CFG_SOC_PLATFORM_MINIV2) && !defined(CFG_SOC_PLATFORM_MINIV3)
    return dfm_write_black_box(DFM_MODULE_ID_DRIVER, DFM_SUBMODULE_ID_DEVMNG, msg);
#else
    return dfm_write_black_box(devid, DFM_MODULE_ID_DRIVER, DFM_SUBMODULE_ID_DEVMNG, msg);
#endif
}

void tsdrv_bbox_system_err_time(u32 devid, u32 excep_id, struct timespec64 *ts, u32 arg)
{
    excep_time timestamp;

    timestamp.tv_sec = ts->tv_sec;
    timestamp.tv_usec = ts->tv_nsec / NSEC_PER_USEC;

#if defined(CFG_SOC_PLATFORM_MINI) && !defined(CFG_SOC_PLATFORM_MINIV2) && !defined(CFG_SOC_PLATFORM_MINIV3)
    /* miniv1 */
    mntn_system_error(excep_id, timestamp, 0);
#else
    bbox_system_error(devid, excep_id, &timestamp, arg);
#endif
}

void tsdrv_bbox_system_err(u32 devid, u32 excep_id, u32 arg)
{
    dfm_system_error_report_ex(devid, excep_id, arg);
}
#else
void adapt_devdrv_stub_test(void)
{
}
#endif
