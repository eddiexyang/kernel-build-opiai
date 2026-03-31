/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
 * Create: 2019-10-15
 */
#ifndef DEVDRV_NSM_H
#define DEVDRV_NSM_H

#ifdef CFG_FEATURE_IPC_NOTIFY
#include "devdrv_user_common.h"
#include "tsdrv_kernel_common.h"
#define RANDOM_LENGTH          20

struct ipc_open_info {
    pid_t pid;
    u64 set_time;
    atomic_t ref;
    struct ipc_notify_node *node;
    struct list_head open_list_node;
};

struct ipc_notify_node {
    u32 delete_flag; /* 1 means it start to deleting */
    u32 dev_id;
    u32 tsid;
    u32 notify_id;
    u32 type;

    u32 tag;
    char name[DEVDRV_IPC_NAME_SIZE];
    struct ipc_open_info open_info[DEVDRV_PID_MAX_NUM];
    struct hlist_node link;
    struct list_head create_list_node;
    struct mutex mutex;
};

enum devdrv_ipc_type {
    DEVDRV_IPC_NOTIFY = 0,
    DEVDRV_IPC_TYPE_MAX
};

enum devdrv_ipc_ref_status {
    DEVDRV_IPC_REF_INIT = 1,
    DEVDRV_IPC_REF_MAX
};

int devdrv_ipc_create(char *ipc_name, void **ipc_node, enum devdrv_ipc_type ipc_type);
int devdrv_ipc_del(const char *ipc_name, u32 max_len, enum devdrv_ipc_type ipc_type);
int devdrv_ipc_find(const char *ipc_name, void **ipc_node, enum devdrv_ipc_type ipc_type);
#else
enum devdrv_ipc_type {
    DEVDRV_IPC_NOTIFY = 0,
    DEVDRV_IPC_TYPE_MAX
};
static inline int devdrv_ipc_create(char *ipc_name, void **ipc_node, enum devdrv_ipc_type ipc_type)
{
    return 0;
}
static inline int devdrv_ipc_del(const char *ipc_name, u32 max_len, enum devdrv_ipc_type ipc_type)
{
    return 0;
}
static inline int devdrv_ipc_find(const char *ipc_name, void **ipc_node, enum devdrv_ipc_type ipc_type)
{
    return 0;
}
#endif /* CFG_FEATURE_IPC_NOTIFY */
int devdrv_get_random(char *sign, u32 len);

#endif /* DEVDRV_NSM_H */

