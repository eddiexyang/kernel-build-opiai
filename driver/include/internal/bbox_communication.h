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

#ifndef BBOX_COMMUNICATION_H
#define BBOX_COMMUNICATION_H

#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>

#include "bbox_platform.h"
#include "bbox_product.h"
#include "common/bbox_print.h"
#include "device/bbox_types.h"

#define NODE_MAX_NUM            100
#define NODE_RUNTIME_MAX_NUM    1000
#define MAX_NAK_TIMES           3

#ifdef DEBUG
#define WTINIT_GAP              100
#define WTSEND_GAP              100
#define WTRECV_GAP              100
#define WTRECV_TRY_GAP          100
#define WTPAUSE_GAP             100
#define MIN_TIMEOUT             100
#define WTRECV_TIMEOUT          500
#else
#define WTINIT_GAP              5000
#define WTSEND_GAP              1000
#define WTRECV_GAP              1000
#define WTRECV_TRY_GAP          200
#define WTPAUSE_GAP             1000
#define MIN_TIMEOUT             1000
#define WTRECV_TIMEOUT          5000
#endif

#define INVALID_SESSION_ID      ((u32)(-1))
#define MAX_TIMESTAMP_SEQ       1000U

#define BBOX_CHANNEL_CAPACITY   0x200000U // 2M
#define BBOX_CHANNEL_UPWARD     0x7ACD
#define BBOX_CHANNEL_DOWNWARD   0x5C9D
#define BBOX_CHANNEL_TYPE_STR(type) \
    (((type) == BBOX_CHANNEL_UPWARD) ? "up" : (((type) == BBOX_CHANNEL_DOWNWARD) ? "down" : "unknown"))

typedef s32 (*channel_init)(u32 num);
typedef s32 (*channel_exit)(void);
typedef u32 (*channel_capacity)(void);
typedef s32 (*channel_send)(u32 sid, const char *data, u32 data_len);
typedef s32 (*channel_recv)(u32 sid, char *buffer, u32 len);
typedef s32 (*server_create)(u32 devid, u32 type);
typedef s32 (*server_close)(u32 sid);
typedef s32 (*channel_is_connected)(u32 sid);

enum channel_status_type {
    CHANNEL_STATUS_INIT     = 0,
    CHANNEL_STATUS_UNNORMAL = 1,
    CHANNEL_STATUS_NORMAL   = 2,
    CHANNEL_STATUS_MAX
};

struct channel_ops {
    channel_init init;
    channel_exit exit;
    channel_capacity capacity;
    channel_send send;
    channel_recv recv;
    server_create create;
    server_close close;
    channel_is_connected is_connected;
};

struct msg_list {
    struct list_head list;
    spinlock_t lock;
    struct semaphore sem;               // msg semaphore
    u32 num;
};

struct buffer_info {
    char *buf;                          // send buffer
    u32 len;                            // send buffer length
};

struct send_info {
    u32 send_seq;
    struct excep_time tm;
};

struct channel_info {
    struct task_struct *task;                       // task thread
    struct msg_list msg_lst;                        // msg list
    struct buffer_info send;                        // send buffer
    struct buffer_info recv;                        // recv buffer
    u32 type;                                       // channel type: up or down
    u32 devid;                                      // device id
    u32 r_devid;                                    // remote device id
    u32 cid;                                        // channel id
    enum channel_status_type status;                // channel usable flag
    u32 sid;                                        // service id
    u32 send_seq;                                   // time stamp sequence
    u32 msg_seq;                                    // message sequence
    bbox_jiffies_t log_jiffies;
};

struct communication_management {
    struct channel_ops ops;             // channel ops
    struct channel_info *channel;
    size_t capacity;
    u32 channel_num;
};

static inline bool bbox_cid_is_upward(u32 cid)
{
    const u32 num = 2; // every device create 2 channel
    return (((cid % num) == 0U) ? true : false);
}

void bbox_netlink_register(struct channel_ops *ops);
void bbox_hdc_register(struct channel_ops *ops);
void bbox_ioctl_register(struct channel_ops *ops);

s32 bbox_communication_init(void);
s32 bbox_channel_buffer_init(struct channel_info *channel);
void bbox_communication_exit(void);
void bbox_channel_exit(struct channel_info *channel);
s32 bbox_channel_send(const struct channel_info *channel, const char *data, u32 data_len);
s32 bbox_channel_recv_try(const struct channel_info *channel, char *buffer, u32 len, s32 timeout);
s32 bbox_channel_recv(const struct channel_info *channel, char *buffer, u32 len);
u32 bbox_channel_get_capacity(void);
void bbox_channel_set_capacity(size_t capacity);
s32 bbox_channels_unnormal(void);
bool bbox_channels_established(u32 *connected);

#endif

