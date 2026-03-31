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
#include <linux/securectype.h>
#include <linux/uio_driver.h>
#ifndef TSDRV_UT
#include "ts_drv_init.h"
#include "ts_drv_sysfs.h"

/* CONFIG_SYSFS */
#define TSDRV_ATTR_RO(_name) static struct kobj_attribute _name##_attr = __ATTR_RO(_name)

#define TSDRV_ATTR(_name) static struct kobj_attribute _name##_attr = __ATTR(_name, 0600, _name##_show, _name##_store)

STATIC ssize_t ts_drv_dfx_show_refill_buf(char *str, size_t buf_size, int node_id)
{
    char *refill_buf = str;
    int tlen;

    tlen = snprintf_s(str, buf_size, buf_size - 1, "-------node_id = %d -------\n", node_id);
    if (tlen < 0) {
        ts_drv_info("ts_drv_dfx_show_refill_buf: ts_drv_dfx sprintf_s failed.\n");
        return 0;
    }
    str += tlen;
    buf_size -= (size_t)tlen;

    tlen = snprintf_s(str, buf_size, buf_size - 1, "send irq to ts   :%lld\n",
                      atomic64_read(&g_aicpu_to_ts_cb[node_id].v_irq_tx));
    if (tlen < 0) {
        ts_drv_info("send irq to ts:%lld  sprintf_s failed.\n", atomic64_read(&g_aicpu_to_ts_cb[node_id].v_irq_tx));
        return 0;
    }
    str += tlen;
    buf_size -= (size_t)tlen;

    tlen = snprintf_s(str, buf_size, buf_size - 1, "recv irq from ts :%lld\n",
                      atomic64_read(&g_aicpu_to_ts_cb[node_id].v_irq_rx));
    if (tlen < 0) {
        ts_drv_info("recv irq from ts:%lld  sprintf_s failed.\n", atomic64_read(&g_aicpu_to_ts_cb[node_id].v_irq_rx));
        return 0;
    }
    str += tlen;
    buf_size -= (size_t)tlen;

    tlen = snprintf_s(str, buf_size, buf_size - 1, "send msg to ts   :%lld\n",
                      atomic64_read(&g_aicpu_to_ts_cb[node_id].v_sq_tx));
    if (tlen < 0) {
        ts_drv_info("send msg to ts:%lld sprintf_s failed.\n", atomic64_read(&g_aicpu_to_ts_cb[node_id].v_sq_tx));
        return 0;
    }
    str += tlen;
    buf_size -= (size_t)tlen;

    tlen = snprintf_s(str, buf_size, buf_size - 1, "recv msg from ts :%lld\n",
                      atomic64_read(&g_aicpu_to_ts_cb[node_id].v_cq_rx));
    if (tlen < 0) {
        ts_drv_info("recv msg from ts:%lld sprintf_s failed.\n", atomic64_read(&g_aicpu_to_ts_cb[node_id].v_cq_rx));
        return 0;
    }
    str += tlen;
    buf_size -= (size_t)tlen;

    tlen = snprintf_s(str, buf_size, buf_size - 1, "sq head :%u  tail :%u\n",
                      g_aicpu_to_ts_cb[node_id].sq.queue->head, g_aicpu_to_ts_cb[node_id].sq.queue->tail);
    if (tlen < 0) {
        ts_drv_info("sq head:%u  tail:%u sprintf_s failed.\n",
            (u32)g_aicpu_to_ts_cb[node_id].sq.queue->head, (u32)g_aicpu_to_ts_cb[node_id].sq.queue->tail);
        return 0;
    }
    str += tlen;
    buf_size -= (size_t)tlen;

    tlen = snprintf_s(str, buf_size, buf_size - 1, "cq head :%u  tail :%u\n",
                      g_aicpu_to_ts_cb[node_id].cq.queue->head, g_aicpu_to_ts_cb[node_id].cq.queue->tail);
    if (tlen < 0) {
        ts_drv_info("cq head:%u  tail:%u sprintf_s failed.\n",
            (u32)g_aicpu_to_ts_cb[node_id].cq.queue->head, (u32)g_aicpu_to_ts_cb[node_id].cq.queue->tail);
        return 0;
    }
    str += tlen;
    buf_size -= (size_t)tlen;

    return str - refill_buf;
}

STATIC ssize_t ts_drv_dfx_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    size_t buf_size = PAGE_SIZE;
    char *str = buf;
    int ret;
    int i;

    for (i = 0; i < CHIP_NUM_MAX; i++) {
        if ((g_aicpu_to_ts_cb[i].sq.queue != NULL) && (g_aicpu_to_ts_cb[i].cq.queue != NULL)) {
            ret = ts_drv_dfx_show_refill_buf(str, buf_size, i);
            if (ret == 0) {
                ts_drv_info("ts_drv_dfx_show sprintf_s failed.\n");
                return 0;
            }
            str += ret;
            buf_size -= (size_t)ret;
        }
    }

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

#else /* TSDRV_UT */
void ut_device_ts_drv_sysfs_test(void)
{
}
#endif /* TSDRV_UT */
