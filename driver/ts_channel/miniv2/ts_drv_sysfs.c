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
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/securec.h>
#ifndef AOS_LLVM_BUILD
#include <linux/securectype.h>
#include <linux/uio_driver.h>
#endif
#ifndef TSDRV_UT
#include "ts_drv_init.h"
#include "ts_drv_sysfs.h"

#ifndef AOS_LLVM_BUILD

/* CONFIG_SYSFS */
#define TSDRV_ATTR_RO(_name) static struct kobj_attribute _name##_attr = __ATTR_RO(_name)

#define TSDRV_ATTR(_name) static struct kobj_attribute _name##_attr = __ATTR(_name, 0600, _name##_show, _name##_store)

STATIC ssize_t ts_drv_dfx_show_refill_buf(char *str)
{
    int buf_size = PAGE_SIZE;
    char *refill_buf = str;
    int tlen;

    tlen = snprintf_s(str, buf_size, buf_size - 1, "ts_drv_dfx:-----------\n");
    if (tlen < 0) {
        ts_drv_info("ts_drv_dfx_show_refill_buf: ts_drv_dfx sprintf_s failed.\n");
        return 0;
    }
    str += tlen;
    buf_size -= tlen;

    return str - refill_buf;
}

STATIC ssize_t ts_drv_dfx_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    char *str = buf;
    int ret;

    ret = ts_drv_dfx_show_refill_buf(str);
    if (ret == 0) {
        ts_drv_info("ts_drv_dfx_show sprintf_s failed.\n");
        return 0;
    }
    str += ret;

    return str - buf;
}

TSDRV_ATTR_RO(ts_drv_dfx);

static struct attribute *ts_aisle_attrs[] = {
    &ts_drv_dfx_attr.attr,
    NULL,
};

STATIC struct attribute_group ts_aisle_attr_group = {
    .attrs = ts_aisle_attrs,
    .name = "ts_aisle_dfx",
};

struct attribute_group *ts_drv_get_aisle_attr_group(void)
{
    return &ts_aisle_attr_group;
}

/* CONFIG_SYSFS */
#endif
#else /* TSDRV_UT */
void ut_device_ts_drv_sysfs_test(void)
{
}
#endif /* TSDRV_UT */