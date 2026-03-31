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
#ifndef _LOG_XPC_SERVER_H
#define _LOG_XPC_SERVER_H
#include <linux/xpc_kernel_inter.h>
#include "log_drv_sqcq.h"
#include "log_xpc.h"

int log_xpc_send_cmd(u32 device_id, u32 tsid, u32 sqcq_idx, struct log_sq_scheduler *sq_info);
int log_xpc_cqsq_init(struct log_device_ctx *device_ctx, u32 tsid);
void log_xpc_cqsq_uninit(struct log_device_ctx *device_ctx, u32 tsid);
void log_xpc_parse_cq_data(unsigned char *buf, int len, int type);
#endif
