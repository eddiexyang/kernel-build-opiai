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


#ifndef URD_KV_H
#define URD_KV_H

#include <linux/types.h>
#include <linux/mutex.h>

#define KV_TABLE_BIT 10
#define KV_TABLE_SIZE (0x1 << KV_TABLE_BIT)
#define KV_TABLE_MASK (KV_TABLE_SIZE - 1)

#define KV_KEY_MAX_LEN 256
#define KV_VALUE_MAX_LEN 256

#define STATE_INIT 1U
#define STATE_ACTIVE 2U
#define STATE_STOP 3U

struct dms_kv_node {
    struct hlist_node hnode;
    char *key;
    int data_len;
    void *data;
};

struct dms_kv_table {
    struct hlist_head *table;
    struct mutex lock;
};
typedef ssize_t (*key_dump_handle)(void *node, char *buf, ssize_t *offset);
void dms_kv_dump(char *buf, ssize_t *offset, key_dump_handle handle);
int dms_kv_init(void);
int dms_kv_get(const char *key, void *value, int len);
int dms_kv_set(const char *key, void *value, int len);
int dms_kv_del(const char *key);
void dms_kv_uninit(void);

#endif
