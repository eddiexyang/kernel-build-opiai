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
#ifndef _LOG_XPC_AGENT_H
#define _LOG_XPC_AGENT_H
#include <linux/xpc_kernel_inter.h>
#include "log_drv_sqcq.h"
#include "log_xpc.h"

void log_agent_send_cq(unsigned int type, struct log_xpc_cq_head *head, const u8 *cq_buf);
void log_agent_parse_cmd(unsigned char *data, unsigned int len);
void log_agent_parse_init_cmd(unsigned char *data, unsigned int len);
#endif
