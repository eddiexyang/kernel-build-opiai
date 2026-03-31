/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#ifndef ESCHED_FOPS_H
#define ESCHED_FOPS_H

#include <linux/cdev.h>

#include "event_sched_inner.h"
#include "esched.h"

/* used for c-dev */
struct sched_char_dev {
    struct device *device;
    struct class *dev_class;
    struct cdev cdev;
    dev_t devno;
};

#define MINOR_DEV_COUNT 1
#if defined(CFG_SOC_PLATFORM_MDC_V51) || defined(CFG_SOC_PLATFORM_MDC_V11)
#define DEVNODE_PERMISSION 0600
#else
#define DEVNODE_PERMISSION 0640
#endif


int32_t copy_from_user_safe(void *to, const void __user *from, unsigned long n);
int32_t copy_to_user_safe(void __user *to, const void *from, unsigned long n);

int32_t sched_publish_event_para_check(struct sched_published_event_info *event_info);
u32 sched_ioctl_devid(u32 open_devid, u32 cmd_devid);
void esched_register_ioctl_cmd_func(int nr, int32_t (*fn)(u32 devid, unsigned long arg));
int32_t sched_submit_event_to_thread(uint32_t chip_id, struct sched_published_event *event);
#endif
