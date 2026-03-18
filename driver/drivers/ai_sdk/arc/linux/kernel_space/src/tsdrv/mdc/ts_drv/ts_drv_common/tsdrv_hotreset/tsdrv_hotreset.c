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
 * Create: 2022-3-2
 */
#ifdef CFG_FEATURE_HOTRESET
#include <linux/errno.h>
#include "tsdrv_log.h"
#include "tsdrv_hotreset.h"
#include "tsdrv_device.h"

int tsdrv_device_pre_hotreset(u32 devid)
{
    struct tsdrv_dev_resource *dev_res = NULL;
    enum tsdrv_dev_status status;
    long ref_cnt;

    dev_res = tsdrv_get_dev_resource(devid, TSDRV_PM_FID);
    if (dev_res == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Device does not exist. (devid=%u)\n", devid);
        return -ENODEV;
#endif
    }

    ref_cnt = atomic64_read(&dev_res->dev_ref);
    if (ref_cnt != 0) {
        TSDRV_PRINT_ERR("The device is still in use, cannot do hotreset. (devid=%u; ref_cnt=%ld)\n",
            devid, ref_cnt);
        return -EBUSY;
    }

    status = (enum tsdrv_dev_status)atomic_cmpxchg(&dev_res->status,
        TSDRV_DEV_ACTIVE, TSDRV_DEV_HOTRESET);
    if (status != TSDRV_DEV_ACTIVE) {
        TSDRV_PRINT_ERR("Device does not active. (devid=%u; status=%u)\n",
            devid, status);
        return -EINVAL;
    }

    return 0;
}

int tsdrv_device_cancel_hotreset(u32 devid)
{
    enum tsdrv_dev_status status;
    struct tsdrv_dev_resource *dev_res = NULL;

    dev_res = tsdrv_get_dev_resource(devid, TSDRV_PM_FID);
    if (dev_res == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_WARN("Device does not exist. (devid=%u)\n", devid);
        return 0;
#endif
    }

    status = (enum tsdrv_dev_status)atomic_cmpxchg(&dev_res->status,
        TSDRV_DEV_HOTRESET, TSDRV_DEV_ACTIVE);
    if (status != TSDRV_DEV_HOTRESET) {
        TSDRV_PRINT_WARN("The device status is invalid. (devid=%u; status=%u)\n",
            devid, status);
        return 0;
    }

    return 0;
}
#endif
