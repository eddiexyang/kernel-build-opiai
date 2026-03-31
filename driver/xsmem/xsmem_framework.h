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

#ifndef XSMEM_FRAMEWORK_H
#define XSMEM_FRAMEWORK_H

#include <linux/types.h>
#include <linux/rbtree.h>
#include <linux/mm.h>
#include <linux/seq_file.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
#include <linux/sched/mm.h>
#endif
#include <linux/proc_fs.h>
#include <linux/wait.h>
#include <linux/gfp.h>

#include "xsmem_cmd.h"

#define XSMEM_DEVICE_NAME "xsmem_dev"

#define XSMEM_NOT_CONFIRM_USER_ID   0xffffffff
#define XSMEM_ROOT_USER_ID          0x0

#ifndef __GFP_ACCOUNT
#ifdef __GFP_KMEMCG
#define __GFP_ACCOUNT __GFP_KMEMCG /* for linux version 3.10 */
#endif

#ifdef __GFP_NOACCOUNT
#define __GFP_ACCOUNT 0 /* for linux version 4.1 */
#endif
#endif

#ifdef EMU_ST
#define STATIC
#define THREAD __thread
#else
#define STATIC static
#define THREAD
#endif

struct xsm_adding_task {
    int pid;
    GroupShareAttr attr;
    struct list_head    pool_node;
};

struct xsm_pool {
    int         pool_id;
    unsigned int priv_mbuf_flag;
    atomic_t        refcnt;
    int     task_id;
    struct mutex        mutex;

    struct list_head    node_list_head;
    struct list_head    register_task_list;
    struct hlist_node   hnode;

    struct xsm_pool_algo    *algo;

    /* Used to serialize the alloc and free operation on the POOL */
    struct mutex        xp_block_mutex;
    struct rb_root      block_root; /* for all alloced block */
    void            *private;
    struct proc_dir_entry *entry;

    unsigned long alloc_size;
    unsigned long real_alloc_size;
    int         create_pid;
    int         adding_id;
    unsigned int cache_type;
    wait_queue_head_t   adding_task_wq;
    struct list_head    adding_task_list_head;
    struct list_head    prop_list_head;
    unsigned long       mnt_ns;
    unsigned int         key_len;
    /* MUST be the last element */
    char            key[0];
};

struct xsm_task {
    pid_t           pid;
    pid_t           vpid; /* pid in container */
    int             attached_pool_count;
    u64             uid;    /* uniqueue id for each process */
    atomic_t        pool_num;
    struct hlist_node link; /* hash task link */
    struct list_head    node_list_head;
    struct list_head    register_xp_list_head;
    struct mutex        mutex;
    struct mm_struct *mm;
    struct proc_dir_entry *entry;
};

struct xsm_exit_task {
    pid_t           pid;
    u64             uid;
    struct list_head    node;
};

struct xsm_task_pool_node {
    struct list_head    task_node; /* list node in TASK */
    struct list_head    pool_node; /* list node in POOL */
    struct xsm_task *task;
    struct xsm_pool *pool;

    struct mutex        mutex;
    struct list_head    exit_task_head;

    GroupShareAttr attr;
    int task_id;

    u64     alloc_size;
    u64     real_alloc_size;
    struct list_head    task_blk_head;
    struct list_head    task_prop_head;
};

struct xsm_block {
    long refcnt;
    int pid;
    int id;
    unsigned long       offset;
    unsigned long       flag;
    unsigned long       alloc_size;
    unsigned long       real_size;

    struct rb_node      block_rb_node;
    void            *private;
    struct list_head    task_blk_head;
};

struct xsm_task_block_node {
    int refcnt;
    struct list_head    node_list; /* list node in TASK node */
    struct list_head    blk_list;  /* list node in BLOCK */
    struct xsm_block    *blk;
    struct xsm_task_pool_node *node;
};

#define ALGO_NAME_MAX 20
struct xsm_pool_algo {
    int num;
    char name[ALGO_NAME_MAX];
    struct list_head algo_node;
    int (*xsm_pool_init)(struct xsm_pool *xp, struct xsm_reg_arg *arg);
    int (*xsm_pool_free)(struct xsm_pool *xp);
    int (*xsm_pool_cache_create)(struct xsm_pool *xp, struct xsm_cache_create_arg *arg);
    int (*xsm_pool_cache_destroy)(struct xsm_pool *xp, struct xsm_cache_destroy_arg *arg);
    int (*xsm_pool_cache_query)(struct xsm_pool *xp, unsigned int devId,
        GrpQueryGroupAddrInfo *cache_buff, unsigned int *cache_cnt);
    int (*xsm_pool_perm_add)(struct xsm_pool *xp, int pid, unsigned long prop);
    int (*xsm_pool_va_check)(struct xsm_pool *xp, unsigned long va, int *result);
    void (*xsm_pool_show)(struct xsm_pool *xp, struct seq_file *seq);
    int (*xsm_block_alloc)(struct xsm_pool *xp, struct xsm_block *blk);
    int (*xsm_block_free)(struct xsm_pool *xp, struct xsm_block *blk);
};

void xsmem_register_algo(struct xsm_pool_algo *algo);

struct xsm_pool *xsmem_pool_get(int pool_id);
void xsmem_pool_put(struct xsm_pool *xp);

struct xsm_task_pool_node *task_pool_node_find(struct xsm_task *task, const struct xsm_pool *xp);

int copy_from_user_safe(void *to, const void __user *from, unsigned long n);
bool blk_is_alloced_from_os(struct xsm_block *blk);

#endif
