/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-7-29
*/

#ifndef TRS_SHR_ID_FOPS_H
#define TRS_SHR_ID_FOPS_H

#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/hashtable.h>
#include <linux/kref.h>

struct shr_id_proc_ctx {
    int pid;
    u64 start_time;
    u32 open_node_num;
    u32 create_node_num;
    struct list_head create_list_head;
    struct list_head open_list_head;
    rwlock_t lock;

    struct hlist_node link;
    struct kref ref;
    struct mutex mutex;
};

#endif

