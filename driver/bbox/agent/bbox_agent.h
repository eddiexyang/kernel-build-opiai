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
#ifndef BBOX_AGENT_H
#define BBOX_AGENT_H
#include <linux/spinlock.h>
#include "device/bbox_pub.h"
#include "../common/bbox_xpc_common.h"

#ifndef UNUSED
#define UNUSED(x)   do {(void)(x);} while (0)
#endif


#define BBOX_MALLOC_MAX_SIZE 0x8000000  // 128M

struct bbox_agent_msg_list {
    struct list_head list;
    spinlock_t lock;
};

struct bbox_agent_msg_node {
    struct list_head list;
    struct bbox_report_info *msg;
};

s32 bbox_agent_msg_proc(const struct bbox_report_info *info);

#endif
