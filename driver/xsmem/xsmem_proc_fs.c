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

#include <linux/proc_fs.h>
#include "securec.h"
#include "xsmem_res_dispatch.h"
#include "xsmem_framework.h"
#include "xsmem_proc_fs.h"

#define PROC_FS_NAME_LEN 32
#define PROC_FS_MODE 0400

extern struct mutex xsmem_mutex;
extern struct idr xsmem_idr;

static struct proc_dir_entry *top_entry = NULL;
static struct proc_dir_entry *xp_entry = NULL;
static struct proc_dir_entry *task_entry = NULL;

static int task_node_show(struct seq_file *seq, void *offset)
{
    struct xsm_task_pool_node *node = (struct xsm_task_pool_node *)seq->private;
    struct xsm_pool *xp = node->pool;
    struct xsm_task_block_node *task_blk_node = NULL;
    int num = 0;

    seq_printf(seq, "task id %d pid %d\n", node->task_id, node->task->pid);
    seq_printf(seq, "block id: offset alloc_size real_size alloc_pid refcnt\n");

    mutex_lock(&xp->xp_block_mutex);
    list_for_each_entry(task_blk_node, &node->task_blk_head, node_list) {
        struct xsm_block *blk = task_blk_node->blk;
        seq_printf(seq, "    %-4d:%-8pK %-8lu %-8lu %-4d %-4ld\n", blk->id,
            (void *)(uintptr_t)blk->offset, blk->alloc_size, blk->real_size, blk->pid, blk->refcnt);
        num++;
    }
    mutex_unlock(&xp->xp_block_mutex);

    seq_printf(seq, "total block %d, total alloc size %llu, total real size %llu\n",
        num, node->alloc_size, node->real_alloc_size);

    return 0;
}

STATIC int task_node_open(struct inode *inode, struct file *file)
{
    return single_open(file, task_node_show, PDE_DATA(inode));
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 5, 0)
static const struct file_operations task_node_ops = {
    .owner = THIS_MODULE,
    .open    = task_node_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
#else
static const struct proc_ops task_node_ops = {
    .proc_open    = task_node_open,
    .proc_read    = seq_read,
    .proc_lseek  = seq_lseek,
    .proc_release = single_release,
};
#endif


static void proc_fs_format_task_dir_name(const struct xsm_task *task, char *name, int len)
{
    if (sprintf_s(name, (u32)len, "%d", task->pid) <= 0) {
        pr_warn("pid %d sprintf_s failed\n", task->pid);
    }
}

static struct proc_dir_entry *proc_fs_mk_task_dir(struct xsm_task *task, struct proc_dir_entry *parent)
{
    char name[PROC_FS_NAME_LEN];

    proc_fs_format_task_dir_name(task, name, PROC_FS_NAME_LEN);
    return proc_mkdir((const char *)name, parent);
}

static void proc_fs_rm_task_dir(struct xsm_task *task, struct proc_dir_entry *parent)
{
    char name[PROC_FS_NAME_LEN];

    proc_fs_format_task_dir_name(task, name, PROC_FS_NAME_LEN);
    (void)remove_proc_subtree((const char *)name, parent);
}

void proc_fs_xsmem_pool_add_task(struct xsm_task_pool_node *node)
{
    struct proc_dir_entry *entry = NULL;
    struct xsm_pool *xp = node->pool;
    struct xsm_task *task = node->task;

    if (xp->entry != NULL) {
        entry = proc_fs_mk_task_dir(task, xp->entry);
        if (entry != NULL) {
            (void)proc_create_data("block", PROC_FS_MODE, entry, &task_node_ops, node);
        }
    }

    if (task->entry != NULL) {
        entry = proc_mkdir(xp->key, task->entry);
        if (entry != NULL) {
            (void)proc_create_data("block", PROC_FS_MODE, entry, &task_node_ops, node);
        }
    }
}

void proc_fs_xsmem_pool_del_task(struct xsm_task_pool_node *node)
{
    struct xsm_pool *xp = node->pool;
    struct xsm_task *task = node->task;

    if (xp->entry != NULL) {
        proc_fs_rm_task_dir(task, xp->entry);
    }
    if (task->entry != NULL) {
        (void)remove_proc_subtree(xp->key, task->entry);
    }
}

void proc_fs_add_xsmem_pool(struct xsm_pool *xp)
{
    if (xp_entry != NULL) {
        xp->entry = proc_mkdir(xp->key, xp_entry);
    }
}

void proc_fs_del_xsmem_pool(struct xsm_pool *xp)
{
    if (xp_entry != NULL) {
        (void)remove_proc_subtree(xp->key, xp_entry);
    }
}

static void per_task_info_show(struct seq_file *seq, struct xsm_task *task)
{
    struct xsm_task_pool_node *node = NULL;
    u64 alloc_size = 0, real_alloc_size = 0;

    seq_printf(seq, "pool id(name): alloc_size real_size\n");

    list_for_each_entry(node, &task->node_list_head, task_node) {
        seq_printf(seq, "%d(%s) %-8llu %-8llu\n", node->pool->pool_id, node->pool->key,
                node->alloc_size, node->real_alloc_size);
        alloc_size += node->alloc_size;
        real_alloc_size += node->real_alloc_size;
    }

    seq_printf(seq, "summary: %-8llu %-8llu\n", alloc_size, real_alloc_size);
}

static int task_info_show(struct seq_file *seq, void *offset)
{
    struct xsm_task *task = (struct xsm_task *)seq->private;

    mutex_lock(&task->mutex);
    seq_printf(seq, "task pid %d pool cnt %d\n", task->pid, task->attached_pool_count);
    per_task_info_show(seq, task);
    mutex_unlock(&task->mutex);

    return 0;
}

STATIC int task_sum_open(struct inode *inode, struct file *file)
{
    return single_open(file, task_info_show, PDE_DATA(inode));
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 5, 0)
static const struct file_operations task_sum_ops = {
    .owner = THIS_MODULE,
    .open    = task_sum_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
#else
static const struct proc_ops task_sum_ops = {
    .proc_open    = task_sum_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};
#endif

void proc_fs_add_task(struct xsm_task *task)
{
    if (task_entry == NULL) {
        return;
    }

    task->entry = proc_fs_mk_task_dir(task, task_entry);
    if (task->entry != NULL) {
        (void)proc_create_data("summary", PROC_FS_MODE, task->entry, &task_sum_ops, task);
    }
}

void proc_fs_del_task(struct xsm_task *task)
{
    if (task_entry == NULL) {
        return;
    }

    proc_fs_rm_task_dir(task, task_entry);
}

static int per_pool_info_show(int id, void *p, void *data)
{
    struct xsm_pool *xp = p;
    struct seq_file *seq = data;

    seq_printf(seq, "id:%d\n    key:%s, create_pid %d, refcnt:%d, algo:%s, alloc_size %lu, real_alloc_size %lu\n",
            id, xp->key, xp->create_pid, atomic_read(&xp->refcnt), xp->algo->name, xp->alloc_size, xp->real_alloc_size);
    if (xp->algo->xsm_pool_show) {
        mutex_lock(&xp->xp_block_mutex);
        xp->algo->xsm_pool_show(xp, seq);
        mutex_unlock(&xp->xp_block_mutex);
    }

    return 0;
}

static int pool_info_show(struct seq_file *seq, void *offset)
{
    int ret;

    seq_printf(seq, "pool list:\n");

    mutex_lock(&xsmem_mutex);
    ret = idr_for_each(&xsmem_idr, per_pool_info_show, seq);
    mutex_unlock(&xsmem_mutex);

    return ret;
}

STATIC int xsmem_pool_sum_open(struct inode *inode, struct file *file)
{
    return single_open(file, pool_info_show, PDE_DATA(inode));
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 5, 0)
static const struct file_operations pool_sum_ops = {
    .owner = THIS_MODULE,
    .open    = xsmem_pool_sum_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
#else
static const struct proc_ops pool_sum_ops = {
    .proc_open    = xsmem_pool_sum_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};
#endif

void xsmem_proc_fs_init(void)
{
    top_entry = proc_mkdir("xsmem", NULL);
    if (top_entry != NULL) {
        xp_entry = proc_mkdir("pool", top_entry);
        if (xp_entry != NULL) {
            (void)proc_create_data("summary", PROC_FS_MODE, xp_entry, &pool_sum_ops, NULL);
        }
        task_entry = proc_mkdir("task", top_entry);
    }
}

void xsmem_proc_fs_uninit(void)
{
    if (top_entry != NULL) {
        (void)remove_proc_subtree("xsmem", NULL);
    }
}
