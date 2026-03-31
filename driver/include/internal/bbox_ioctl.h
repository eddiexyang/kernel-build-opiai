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

#ifndef BBOX_IOCTL_H
#define BBOX_IOCTL_H

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include "bbox_communication.h"

#ifdef DEBUG
#define IOCTL_WTRECV_GAP      100
#define IOCTL_WTSEND_GAP      300
#else
#define IOCTL_WTRECV_GAP      100
#define IOCTL_WTSEND_GAP      5000
#endif

#define IOCTL_PKT_UP          BBOX_CHANNEL_UPWARD
#define IOCTL_PKT_DOWN        BBOX_CHANNEL_DOWNWARD
#define IOCTL_SEGMENT         BBOX_CHANNEL_CAPACITY // 2M
#define IOCTL_CHANNEL_MAX_NUM 8U
#define IOCTL_DEVICE_MAX_NUM  (IOCTL_CHANNEL_MAX_NUM / 2U)
#define OPEN_CHAR_DEV_MAGIC   0x4EB8BB81

struct dev_info {
    struct cdev cdev;
    dev_t devno;
    struct file *accepted_filep;
};

struct ioctl_data_node {
    char *data;
    u32 len;
    bool clear_flag;
    struct semaphore sem;
    spinlock_t lock;
};

enum ioctl_cdev_status {
    UNINITIALISED,
    INITIALISED,
    FIRST_WAIT_CONN,
    WAIT_CONNECTION,
    CONNECTED,
    RECONNECTED
};

struct ioctl_channel_info {
    u32 magic;
    u32 devid;
    u32 channel_type;
    enum ioctl_cdev_status status;
    struct ioctl_data_node send_node;
    struct ioctl_data_node recv_node;
    struct dev_info *dev;
    struct mutex lock;
};

static inline bool bbox_multiples_of_two(u32 num)
{
    const u32 base = 2; // channel num is a multiples of 2
    return ((num % base) == 0);
}

struct ioctl_channel_info *bbox_ioctl_get_channel(u32 devid, u32 type, u32 *sid);

#endif