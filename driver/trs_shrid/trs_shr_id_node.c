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
 * Create: 2022-07-28
 */
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/hashtable.h>
#include <linux/kref.h>

#include "ascend_hal_define.h"
#include "securec.h"
#include "trs_core.h"
#include "trs_res_id_def.h"
#include "trs_shr_id_node.h"

#define SHR_ID_HASH_TABLE_BIT           4

#define TRS_SHR_ID_NODE_MAX_OPEN_NUM    ((S32_MAX - SHR_ID_PID_MAX_NUM) / SHR_ID_PID_MAX_NUM)

struct shr_id_node_htable {
    DECLARE_HASHTABLE(htable, SHR_ID_HASH_TABLE_BIT);
    rwlock_t lock;
};

static struct shr_id_node_htable htable[SHR_ID_TYPE_MAX];

static u32 get_elfhash_key(const char *name, size_t len)
{
    u32 key = 0;
    u32 tmp_hash = 0;
    u32 tmp_len = 0;
    const u8 *shr_char = (u8 *)name;

    while ((*shr_char != 0) && (tmp_len < len)) {
        key = (key << 4) + *shr_char; /* hash shifts left 4 bits */
        tmp_hash = key & 0xF0000000;

        if (tmp_hash == 0) {
            key ^= tmp_hash >> 24; /* tmp_hash shifts right 24 bits */
            key ^= tmp_hash;
        }
        shr_char++;
        tmp_len += sizeof(u8);
    }

    return key;
}

struct shr_id_node_wlist {
    pid_t pid;
    u64 set_time;
    u32 open_num;
};

struct shr_id_node {
    struct trs_id_inst inst;
    u32 key;
    char name[SHR_ID_NSM_NAME_SIZE];
    u32 id;
    int type;       /* shr_id type */
    int res_type;   /* trs id type */
    pid_t pid; /* create pid */
    u32 flag;  /* remote flag */

    spinlock_t lock;
    struct hlist_node link;
    struct kref ref;
    struct shr_id_node_wlist wlist[SHR_ID_PID_MAX_NUM];
};

static void _shr_id_node_init(struct shr_id_node *node, const char *name, pid_t pid,
    struct shr_id_node_op_attr *attr)
{
    u32 i;

    for (i = 0; i < SHR_ID_NSM_NAME_SIZE; i++) {
        node->name[i] = name[i];
    }
    node->res_type = attr->res_type;
    node->inst = attr->inst;
    node->id = attr->id;
    node->type = attr->type;
    node->pid = pid;
    node->wlist[0].pid = pid;
    node->wlist[0].set_time = ktime_get_ns();
    node->key = get_elfhash_key(node->name, strlen(node->name) + 1);
    kref_init(&node->ref);
}

static struct shr_id_node *_shr_id_node_create(const char *name, pid_t pid, struct shr_id_node_op_attr *attr,
    u32 flag)
{
    struct shr_id_node *node = kzalloc(sizeof(struct shr_id_node), GFP_KERNEL | __GFP_ACCOUNT);

    if (node != NULL) {
        int ret;
        _shr_id_node_init(node, name, pid, attr);

        if ((flag & TSDRV_FLAG_REMOTE_ID) != 0) {
            ret = trs_res_id_check(&node->inst, attr->res_type, (u32)(attr->id));
            node->flag |= TSDRV_FLAG_REMOTE_ID;
        } else {
            ret = trs_res_id_get(&node->inst, attr->res_type, attr->id);
        }
        if (ret != 0) {
            kfree(node);
            trs_err("Check id failed. (devid=%u; tsid=%u; res_type=%d; id=%u; ret=%d; flag=%u)\n",
                attr->inst.devid, attr->inst.tsid, attr->res_type, attr->id, ret, flag);
            return NULL;
        }
    }
    return node;
}

static void _shr_id_node_destroy(struct shr_id_node *node)
{
    if ((node->flag & TSDRV_FLAG_REMOTE_ID) == 0) {
        int ret;

        ret = trs_res_id_put(&node->inst, node->res_type, node->id);
        if (ret != 0) {
            trs_err("Id put fail. (devid=%u; tsid=%u; name=%s; type=%d; id=%u; ret=%d)\n",
                node->inst.devid, node->inst.tsid, node->name, node->type, node->id, ret);
        }
    }
    kfree(node);
}

static struct shr_id_node *_shr_id_node_find(const char *name, int type)
{
    u32 key = get_elfhash_key(name, strlen(name) + 1);
    struct shr_id_node *node = NULL;

    hash_for_each_possible(htable[type].htable, node, link, key) {
        if (strcmp(name, node->name) == 0) {
            return node;
        }
    }
    return NULL;
}

static void _shr_id_node_add(struct shr_id_node *node)
{
    hash_add(htable[node->type].htable, &node->link, node->key);
}

static int shr_id_node_add(struct shr_id_node *node)
{
    write_lock(&htable[node->type].lock);
    if (_shr_id_node_find(node->name, node->type) != NULL) {
        write_unlock(&htable[node->type].lock);
        trs_err("Add shr id node fail. (devid=%u; tsid=%u; name=%s; type=%d; id=%u)\n",
            node->inst.devid, node->inst.tsid, node->name, node->type, node->id);
        return -EACCES;
    }
    _shr_id_node_add(node);
    write_unlock(&htable[node->type].lock);

    return 0;
}

static void _shr_id_node_del(struct shr_id_node *node)
{
    hash_del(&node->link);
}

static void shr_id_node_del(struct shr_id_node *node)
{
    write_lock(&htable[node->type].lock);
    _shr_id_node_del(node);
    write_unlock(&htable[node->type].lock);
}

int shr_id_node_create(const char *name, pid_t pid, struct shr_id_node_op_attr *attr, u32 flag)
{
    struct shr_id_node *node = NULL;
    int ret;

    node = _shr_id_node_create(name, pid, attr, flag);
    if (node == NULL) {
        return -ENODEV;
    }

    ret = shr_id_node_add(node);
    if (ret != 0) {
        _shr_id_node_destroy(node);
    } else {
        trs_debug("Create succeed. (devid=%u; tsid=%u; name=%s; node.name=%s; type=%d; id=%u)\n",
            attr->inst.devid, attr->inst.tsid, name, node->name, attr->type, attr->id);
    }
    return ret;
}

static void shr_id_node_release(struct kref *kref)
{
    struct shr_id_node *node = container_of(kref, struct shr_id_node, ref);

    trs_debug("Release succeed. (devid=%u; tsid=%u; name=%s; type=%d; id=%u)\n",
        node->inst.devid, node->inst.tsid, node->name, node->type, node->id);
    shr_id_node_del(node);
    _shr_id_node_destroy(node);
}

static struct shr_id_node *shr_id_node_get(const char *name, int type)
{
    struct shr_id_node *node = NULL;
    size_t name_len;

    name_len = strnlen(name, SHR_ID_NSM_NAME_SIZE);
    if ((name_len == 0) || (name_len >= SHR_ID_NSM_NAME_SIZE)) {
        trs_err("Length out of range. (name_len=%lu; type=%d)\n", name_len, type);
        return NULL;
    }

    if (type >= SHR_ID_TYPE_MAX || type < 0) {
        trs_err("Unknown shr id type. (type=%d)\n", type);
        return NULL;
    }

    read_lock(&htable[type].lock);
    node = _shr_id_node_find(name, type);
    if (node != NULL) {
        if (kref_get_unless_zero(&node->ref) == 0) {
            read_unlock(&htable[type].lock);
            return NULL;
        }
    }
    read_unlock(&htable[type].lock);

    return node;
}

static void shr_id_node_put(struct shr_id_node *node)
{
    kref_put(&node->ref, shr_id_node_release);
}

static int _shr_id_node_find_wlist_index(struct shr_id_node *node, pid_t pid)
{
    int i;

    for (i = 0; i < SHR_ID_PID_MAX_NUM; i++) {
        if (node->wlist[i].pid == pid) {
            return i;
        }
    }

    return SHR_ID_PID_MAX_NUM;
}

static int _shr_id_node_get_idle_wlist_index(struct shr_id_node *node)
{
    int i;

    for (i = 0; i < SHR_ID_PID_MAX_NUM; i++) {
        if (node->wlist[i].pid == 0) {
            return i;
        }
    }
    return SHR_ID_PID_MAX_NUM;
}

static int _shr_id_node_set_pid(struct shr_id_node *node, pid_t pid)
{
    int idx = _shr_id_node_find_wlist_index(node, pid);
    if (idx == SHR_ID_PID_MAX_NUM) {
        idx = _shr_id_node_get_idle_wlist_index(node);
        if (idx == SHR_ID_PID_MAX_NUM) {
            return -ENODEV;
        }
    }
    node->wlist[idx].pid = pid;
    node->wlist[idx].set_time = ktime_get_ns();
    return 0;
}

static int _shr_id_node_set_pids(struct shr_id_node *node, pid_t create_pid, pid_t pid[], u32 pid_num)
{
    u32 i;

    if (node->pid != create_pid) {
        /* Only creator process have permission to add pid to wlist */
        return -EACCES;
    }

    for (i = 0; i < pid_num; i++) {
        int ret;
        if ((pid[i] == 0) || (node->pid == pid[i])) {
            /* empty pid and creator process do not add to wlist */
            continue;
        }
        ret = _shr_id_node_set_pid(node, pid[i]);
        if (ret != 0) {
            return ret;
        }
    }
    return 0;
}

int shr_id_node_set_pids(const char *name, int type, pid_t create_pid, pid_t pid[], u32 pid_num)
{
    struct shr_id_node *node = shr_id_node_get(name, type);
    int ret = -ENODEV;

    if (node != NULL) {
        spin_lock(&node->lock);
        ret = _shr_id_node_set_pids(node, create_pid, pid, pid_num);
        spin_unlock(&node->lock);
        shr_id_node_put(node);
    }
    return ret;
}

static int _shr_id_node_uninit(struct shr_id_node *node, pid_t pid)
{
    /* Only creator process have permission to destroy */
    if (node->pid != pid) {
        return -EACCES;
    }
    /* Set invalid creator pid to prevent multiple destroy */
    node->pid = 0;
    return 0;
}

int shr_id_node_destroy(const char *name, int type, pid_t pid)
{
    struct shr_id_node *node = NULL;
    int ret;

    node = shr_id_node_get(name, type);
    if (node == NULL) {
        /* in mc2 feature, node has been destory */
        return 0;
    }
    spin_lock(&node->lock);
    ret = _shr_id_node_uninit(node, pid);
    spin_unlock(&node->lock);
    if (ret == 0) {
        /* Decrease ref when shr_id_node creted. */
        shr_id_node_put(node);
        trs_debug("Destroy succeed. (devid=%u; tsid=%u; name=%s; type=%d; id=%u; ref=%u)\n",
            node->inst.devid, node->inst.tsid, node->name, node->type, node->id, kref_read(&node->ref));
    }
    shr_id_node_put(node);
    return ret;
}

static int _shr_id_node_open(struct shr_id_node *node, pid_t pid, unsigned long start_time,
    struct shr_id_node_op_attr *attr)
{
    int idx;

    idx = _shr_id_node_find_wlist_index(node, pid);
    if (idx == SHR_ID_PID_MAX_NUM) {
        return -EBUSY;
    }
    /*
     * Prevent scenario described below:
     * Exit the process that opened shr_id_node and pull up a new process with the same pid.
     * The new process can open shr_id_node without authorization.
     */
    if (time_after(start_time, (unsigned long)node->wlist[idx].set_time)) {
        return -ENODEV;
    }

    /* Repeated opening of a shr_id_node is allowed, but cannot exceed the maximum number of times */
    if (node->wlist[idx].open_num >= TRS_SHR_ID_NODE_MAX_OPEN_NUM) {
        return -EFAULT;
    }
    node->wlist[idx].open_num++;

    attr->inst = node->inst;
    attr->id = node->id;

    return 0;
}

int shr_id_node_open(const char *name, pid_t pid, unsigned long start_time, struct shr_id_node_op_attr *attr)
{
    struct shr_id_node *node = NULL;
    int ret;

    node = shr_id_node_get(name, attr->type);
    if (node == NULL) {
        trs_err("Get shr_id_node fail. (name=%s; type=%d)\n", name, attr->type);
        return -ENODEV;
    }
    spin_lock(&node->lock);
    ret = _shr_id_node_open(node, pid, start_time, attr);
    spin_unlock(&node->lock);
    if (ret != 0) {
        shr_id_node_put(node);
        trs_err("Open fail. (name=%s; type=%d; ret=%d)\n", name, attr->type, ret);
    } else {
        trs_debug("Open succeed. (devid=%u; tsid=%u; name=%s; type=%d; id=%u)\n",
            attr->inst.devid, attr->inst.tsid, name, attr->type, attr->id);
    }

    return ret;
}

static int _shr_id_node_close(struct shr_id_node *node, pid_t pid)
{
    int idx;

    idx = _shr_id_node_find_wlist_index(node, pid);
    if (idx == SHR_ID_PID_MAX_NUM) {
        return -EBUSY;
    }

    if (node->wlist[idx].open_num == 0) {
        return -EACCES;
    }

    node->wlist[idx].open_num--;

    return 0;
}

int shr_id_node_close(const char *name, int type, pid_t pid)
{
    struct shr_id_node *node = NULL;
    int ret = -ENODEV;

    node = shr_id_node_get(name, type);
    if (node != NULL) {
        spin_lock(&node->lock);
        ret = _shr_id_node_close(node, pid);
        spin_unlock(&node->lock);
        if (ret == 0) {
            /* Decrease ref when shr_id_node opened. */
            shr_id_node_put(node);
            trs_debug("Close succeed. (devid=%u; tsid=%u; name=%s; type=%d; id=%u)\n",
                node->inst.devid, node->inst.tsid, name, type, node->id);
        }
        shr_id_node_put(node);
    }
    return ret;
}

void shr_id_node_init(void)
{
    int type;

    for (type = 0; type < SHR_ID_TYPE_MAX; type++) {
        hash_init(htable[type].htable);
        rwlock_init(&htable[type].lock);
    }
}

