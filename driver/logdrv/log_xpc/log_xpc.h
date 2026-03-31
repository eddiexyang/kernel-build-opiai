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
#ifndef _LOG_XPC_H
#define _LOG_XPC_H
#include <linux/xpc_kernel_inter.h>
#include "log_drv_sqcq.h"
#define INVALID_XPC_CHL_ID (-1)

enum log_xpc_channel_type {
    XPC_CHANNEL_TYPE_CMD = 0,
    XPC_CHANNEL_TYPE_INIT,
    XPC_CHANNEL_TYPE_INIT_REPLY,
    XPC_CHANNEL_TYPE_CQ0,
    XPC_CHANNEL_TYPE_CQ1,
    XPC_CHANNEL_TYPE_MAX,
};

enum log_xpc_init_type {
    LOG_CQSQ_INIT,
    LOG_CQSQ_UNINIT,
};

struct log_xpc_sq_cmd {
    u32 device_id;
    u32 tsid;
    u32 sqcq_idx;
    struct log_sq_scheduler sq_info;
};

struct log_xpc_cq_head {
    int ret;
    u32 device_id;
    u32 tsid;
};
struct log_xpc_cq_st {
    struct log_xpc_cq_head head;
    struct log_cq_scheduler data;
};

struct log_xpc_cqcq_init_arg {
    u32 device_id;
    u32 tsid;
    int type;
    u32 sq_0_index;
    u32 cq_0_index;
    u32 cq_1_index;
};

struct log_xpc_cqcq_init_reult {
    u32 device_id;
    u32 tsid;
    u32 sq_0_index;
    u32 cq_0_index;
    u32 cq_1_index;
    int ret;
};

int log_xpc_get_chl_id(int type);
void log_xpc_set_chl_id(int type, int chl_id);
int log_xpc_open_all_channel(void);
void log_xpc_close_all_channel(void);
void log_xpc_parse_cq_data(unsigned char *buf, int len, int type);
int log_xpc_send_cmd(u32 device_id, u32 tsid, u32 sqcq_idx, struct log_sq_scheduler *sq_info);
int log_xpc_cqsq_init(struct log_device_ctx *device_ctx, u32 tsid);
#endif
