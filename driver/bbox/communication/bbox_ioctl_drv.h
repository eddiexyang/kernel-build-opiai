/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Create: 2022-08-13
 */

#ifndef BBOX_IOCTL_DRV_H
#define BBOX_IOCTL_DRV_H

#include <asm/ioctl.h>
#include <linux/fcntl.h>
#include "device/bbox_types.h"

struct ioctl_para {
    u32 magic;
    u32 devid;
    u32 type;
    char *buf;
    u32 len;
};

enum ioctl_event_type {
    BBOX_EPOLL_DATA_IN = 1U << 0, // for receive event
    BBOX_EPOLL_CONN_IN = 1U << 1, // for accept event
    BBOX_EPOLL_HANG_UP = 1U << 2  // for connection closed by peer event
};

enum ioctl_cmd_type {
    IOCTL_USER_REGISTER_DEV = 1,
    IOCTL_USER_QUERY_READ_INFO,
    IOCTL_USER_READ,
    IOCTL_USER_WRITE,
    IOCTL_USER_CMD_MAX
};

#define CHAR_DRIVER_NAME       "hisi_bbox"
#define CDEV_NAME_LEN          30
#define CDEV_OPEN_FLAG         (((u32)O_RDWR) | ((u32)O_SYNC))
#define BBOX_IOCTL_CMD_MAGIC   ((u32)_IOC(0x2UL, 0xA7UL, 0xFUL, 0x756UL))
#define BBOX_GEN_COMMU_ID      ((u32)(0xFFFFFFFFU & jiffies))

void bbox_dev_ioctl_exit(u32 num);
s32 bbox_dev_ioctl_init(u32 num);
void bbox_dev_ioctl_wakeup_poll(void);

#endif
