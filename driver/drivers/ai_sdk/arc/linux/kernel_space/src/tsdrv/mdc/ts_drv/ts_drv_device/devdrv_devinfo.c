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
 * Create: 2022-07-26
 */

#include "tsdrv_common.h"
#include "devdrv_common.h"

#ifdef CFG_SOC_PLATFORM_MDC_V51
# include "drv_notify.h"
#endif

#define KEY_CHIP_TYPE     (8)

int tsdrv_get_chip_type(void)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
#ifdef CFG_SOC_MDC_V51_LITE
    return CHIP_TYPE_MDC_BS9SX1A;
#else
    int chip_type = -1;
    if (centre_notify_get_val(KEY_CHIP_TYPE, &chip_type) != 0) {
#ifndef TSDRV_UT
        devdrv_drv_err("get chip type failed.\n");
        return -ENOSYS;
#endif
    }
    return chip_type;
#endif
#else
    return CHIP_TYPE_NOT_SET;
#endif
}
EXPORT_SYMBOL(tsdrv_get_chip_type);
