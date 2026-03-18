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


#include <linux/fs.h>
#include <linux/jiffies.h>

#include "devdrv_manager.h"
#include "devdrv_manager_rand.h"

int devdrv_get_random(char *random, u32 len)
{
    struct file *file = NULL;
    unsigned long time;
    u32 tmp_len = len;
    u32 out_size = 0;
    loff_t pos;
    int ret;

    if (random == NULL || len == 0) {
        devdrv_drv_err("Random is null or invalid len. (len=%u)\n", len);
        return -EINVAL;
    }

    file = filp_open("/dev/urandom", O_RDONLY, 0);
    if (IS_ERR_OR_NULL(file)) {
        devdrv_drv_err("Open file:/dev/urandom failed. (file=%pK; errcode=%ld)\n", file, PTR_ERR(file));
        return -EINVAL;
    }

    time = jiffies;
    do {
        pos = 0;
        tmp_len = len - out_size;

        ret = kernel_read(file, random + out_size, (ssize_t)tmp_len, &pos);
        if (ret == -ERESTARTSYS || ret == -EAGAIN) {
            devdrv_drv_warn("Kernel read continue. (ret=%d)\n", ret);
            continue;
        }
        if (ret < 0) {
            devdrv_drv_err("Kernel read failed. (ret=%d)\n", ret);
            filp_close(file, NULL);
            file = NULL;
            return -EINVAL;
        }
        out_size += ret;
    } while (out_size < len);

    if (time_is_before_jiffies(time + HZ / 10)) {
        devdrv_drv_warn("Spend time. (time=%ums)\n", jiffies_to_msecs(jiffies - time));
    }

    filp_close(file, NULL);
    file = NULL;

    return 0;
}
EXPORT_SYMBOL(devdrv_get_random);
