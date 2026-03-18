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

#ifndef BBOX_ADAPTER_SNAPSHOT_H
#define BBOX_ADAPTER_SNAPSHOT_H

#include <linux/mm.h>
#include "bootcheck/hdr_public.h"
#include "common/bbox_common.h"
#include "device/bbox_pub.h"

enum EVENT_TYPE {
    EVENT_TYPE_PANIC = 0,   // panic 事件
    EVENT_TYPE_DIE   = 1,   // die 事件
    EVENT_TYPE_MAX
};

#define EVENT_FLAG_INVALID          (-1) // 无效事件
#define EVENT_FLAG_NONE             0    // 无并发事件
#define EVENT_FLAG_CALL             1    // 一个异常事件被另一个异常事件调用
#define EVENT_FLAG_NEST             2    // 异常嵌套

#define EVENT_NAME_DIE              "die"
#define EVENT_NAME_PANIC            "panic"
#define EVENT_NAME_COMM             "communication disconnect"

#define DIE_ERR_MASK                0xffffU

#define PANIC_REASON_UNKOW          "unknown panic reason!"
#define SNAPSHOT_DATE_LENGTH        30U
#define SNAPSHOT_MAGIC              0x1343B6F
#define SNAPSHOT_VERSION            0x100
#define SNAPSHOT_PRINT_LEN          0x100
#define SNAPSHOT_LOG_BUF_OFFSET     0x100U
#define SNAPSHOT_MAX_PRINT_MODULE   100
#define SNAPSHOT_MAX_LOG_NUM        20

#define TS_SEC2NSEC(sec)          ((sec) * 1000000000)

struct snapshot_key {
    u32 event_flag;
    u32 dump_flag;
    u32 err;
};

struct snapshot_buf_cb {
    char *buffer;
    u32 size;
    struct hdr_log_head *head;
    struct module_head *module;
    struct snapshot_key *key;
    char *log_buf;
    u32 log_start;
    u32 log_end;
    u32 log_buf_len;
    u32 err;
    u32 full;
    char printk_buf[SNAPSHOT_PRINT_LEN];
};

s32 bbox_snapshot_init(void);
void bbox_snapshot_exit(void);
s32 bbox_snapshot_dump_die(const void *args);
s32 bbox_snapshot_dump_panic(const void *args);
s32 bbox_snapshot_dump_comm(const struct bbox_exception_info *info, const struct bbox_time *tm);

#endif
