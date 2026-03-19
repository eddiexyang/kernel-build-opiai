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

#define pr_fmt(fmt) "XSMEM: <%s:%d> " fmt, __func__, __LINE__

#include <linux/types.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/mutex.h>
#include <linux/fs.h>
#include <linux/hashtable.h>
#include <linux/uaccess.h>
#include <linux/nsproxy.h>
#include <linux/slab.h>
#include <linux/mman.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/rcupdate.h>
#include <linux/miscdevice.h>
#include <linux/jhash.h>
#include <linux/seq_file.h>
#include <linux/profile.h>
#include <linux/opiai_vendor_compat.h>

#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
#include <linux/sched/task.h>
#endif

#include "securec.h"
#include "ascend_kernel_hal.h"

#ifdef CFG_FEATURE_EXTERNAL_CDEV
#include "davinci_api.h"
#endif

#include "xsmem_algo_vma.h"
#include "xsmem_algo_cache_vma.h"

#ifdef CFG_FEATURE_SUPPORT_SP
#include "xsmem_algo_sp.h"
#include "xsmem_algo_cache_sp.h"
#endif

#include "xsmem_prop.h"
#include "xsmem_proc_fs.h"
#include "xsmem_res_dispatch.h"
#include "xsmem_ns_adapt.h"

#define XSHM_HLIST_TABLE_BIT 13
#define XSHM_HLIST_TABLE_MASK ((0x1 << XSHM_HLIST_TABLE_BIT) - 1)
#define TASK_HASH_TABLE_BIT 10
#define TASK_HASH_TABLE_MASK ((0x1UL << TASK_HASH_TABLE_BIT) - 1)

#define MAX_XP_NUM_OF_TASK  32

#define XSHM_KEY_AND_NAMESPACE_SIZE (XSHM_KEY_SIZE + 64)

/* Protect the lifetime for all POOL */
DEFINE_MUTEX(xsmem_mutex);
DEFINE_MUTEX(task_mutex);

static DEFINE_HASHTABLE(xsmem_key_list, XSHM_HLIST_TABLE_BIT);
static DEFINE_HASHTABLE(task_table, TASK_HASH_TABLE_BIT);

static u64 g_uni_process_id = 0;   // uniqueue id for each process

/* FIXME: use lock to portect the list_head */
static LIST_HEAD(registered_algo);

#ifdef CFG_FEATURE_SUPPORT_SP
extern void xsmem_recycle_callback(void);
#endif

/* Check user id */
unsigned int THREAD g_xsmem_authorized_user_id = XSMEM_NOT_CONFIRM_USER_ID;
module_param(g_xsmem_authorized_user_id, int, 0644);
MODULE_PARM_DESC(g_xsmem_authorized_user_id, "User ID of system user \"HwHiAiUser\"");

#ifdef EMU_ST
void xsmem_set_authorized_user_id(unsigned int value)
{
    g_xsmem_authorized_user_id = value;
}
#endif
int copy_from_user_safe(void *to, const void __user *from, unsigned long n)
{
    if ((from == NULL) || (n == 0)) {
        pr_err("The variable from is NULL or n is zero.\n");
        return -EINVAL;
    }

    if (copy_from_user(to, (void *)from, n) != 0) {
        pr_err("Failed to invoke the copy_from_user. (size=%lu)\n", n);
        return -EFAULT;
    }

    return 0;
}

static int copy_to_user_safe(void __user *to, const void *from, unsigned long n)
{
    if ((to == NULL) || (n == 0)) {
        pr_err("The variable to is NULL or n is zero.\n");
        return -EINVAL;
    }

    if (copy_to_user(to, (void *)from, n) != 0) {
        pr_err("Failed to invoke the copy_from_user. (size=%lu)\n", n);
        return -EFAULT;
    }

    return 0;
}

void xsmem_register_algo(struct xsm_pool_algo *algo)
{
    struct xsm_pool_algo *tmp = NULL;

    list_for_each_entry(tmp, &registered_algo, algo_node)
        if (algo->num == tmp->num) {
            pr_info("Algorithm with the same id, has been registered. (id=%d; name=%s)\n",
                tmp->num, tmp->name);
            return;
        }

    list_add_tail(&algo->algo_node, &registered_algo);

    pr_debug("Algo registered success. (num=%d; name=%s)\n", algo->num, algo->name);

    return;
}

static struct xsm_pool_algo *xsmem_find_algo(int algo)
{
    struct xsm_pool_algo *tmp = NULL;

    list_for_each_entry(tmp, &registered_algo, algo_node)
        if (tmp->num == algo) {
            return tmp;
        }

    return NULL;
}

/* The caller must hold xp->xp_block_mutex */
static struct xsm_block *xsmem_find_block(struct xsm_pool *xp, unsigned long offset)
{
    struct rb_node *block_rb_node = xp->block_root.rb_node;

    while (block_rb_node) {
        struct xsm_block *blk = rb_entry(block_rb_node, struct xsm_block, block_rb_node);

        if (offset >= (blk->offset + blk->alloc_size)) {
            block_rb_node = block_rb_node->rb_right;
        } else if (offset < blk->offset) {
            block_rb_node = block_rb_node->rb_left;
        } else {
            return blk;
        }
    }

    return NULL;
}

// link: new node
// parent: parent
static void xsmem_add_block(struct xsm_pool *xp, struct xsm_block *blk)
{
    struct rb_node *parent = NULL;
    struct rb_node **link = &xp->block_root.rb_node;
    while (*link) {
        struct xsm_block *tmp = NULL;

        parent = *link;
        tmp = rb_entry(*link, struct xsm_block, block_rb_node);

        if (blk->offset > tmp->offset) {
            link = &parent->rb_right;
        } else if (blk->offset < tmp->offset) {
            link = &parent->rb_left;
        } else {
            pr_warn("pool id %d alloc one block offset %pK more than once, the POOL's algo got broken\n",
                xp->pool_id, (void *)(uintptr_t)blk->offset);
            return;
        }
    }

    rb_link_node(&blk->block_rb_node, parent, link);
    rb_insert_color(&blk->block_rb_node, &xp->block_root);
}

static void xsmem_block_destroy(struct xsm_pool *xp, struct xsm_block *blk)
{
    xp->algo->xsm_block_free(xp, blk); // should not fail
    rb_erase(&blk->block_rb_node, &xp->block_root);
    kfree(blk);
}

static void xsmem_clear_block(struct xsm_pool *xp)
{
    struct rb_node *node = NULL;
    struct xsm_block *blk = NULL;
    int num = 0;

    mutex_lock(&xp->xp_block_mutex);
    node = rb_first(&xp->block_root);
    while (node != NULL) {
        blk = rb_entry(node, struct xsm_block, block_rb_node);
        xsmem_blockid_put(xp->mnt_ns, blk->id);
        node = rb_next(node);
        xsmem_block_destroy(xp, blk);
        num++;
    }
    mutex_unlock(&xp->xp_block_mutex);

    if (num > 0) {
        pr_info("pool %s id %d recycle blk num %d\n", xp->key, xp->pool_id, num);
    }
}

static struct xsm_task_block_node *xsmem_find_task_block_node(const struct xsm_task_pool_node *node,
    struct xsm_block *blk)
{
    struct xsm_task_block_node *task_blk_node = NULL;

    list_for_each_entry(task_blk_node, &blk->task_blk_head, blk_list) {
        if (task_blk_node->node == node) {
            return task_blk_node;
        }
    }

    return NULL;
}

static void xsmem_task_link_blk(struct xsm_task_pool_node *node, struct xsm_block *blk,
    struct xsm_task_block_node *task_blk_node)
{
    task_blk_node->blk = blk;
    task_blk_node->node = node;
    list_add_tail(&task_blk_node->node_list, &node->task_blk_head);
    list_add_tail(&task_blk_node->blk_list, &blk->task_blk_head);
}

static void xsmem_task_unlink_blk(struct xsm_task_block_node *task_blk_node)
{
    list_del(&task_blk_node->node_list);
    list_del(&task_blk_node->blk_list);
}

static void node_stat_add(struct xsm_task_pool_node *node, struct xsm_block *blk)
{
    node->alloc_size += blk->alloc_size;
    node->real_alloc_size += blk->real_size;
}

static void node_stat_sub(struct xsm_task_pool_node *node, struct xsm_block *blk)
{
    node->alloc_size -= blk->alloc_size;
    node->real_alloc_size -= blk->real_size;
}

static void xsmem_pool_stat_add(struct xsm_pool *xp, struct xsm_block *blk)
{
    xp->alloc_size += blk->alloc_size;
    xp->real_alloc_size += blk->real_size;
}

static void xsmem_pool_stat_sub(struct xsm_pool *xp, struct xsm_block *blk)
{
    xp->alloc_size -= blk->alloc_size;
    xp->real_alloc_size -= blk->real_size;
}

static bool xsmem_is_blk_need_auto_recycle(const struct xsm_block *blk)
{
    return (blk->flag & XSMEM_BLK_NOT_AUTO_RECYCLE) != XSMEM_BLK_NOT_AUTO_RECYCLE;
}

static void xsmem_task_clear_block(struct xsm_pool *xp, struct xsm_task_pool_node *node)
{
    struct xsm_task_block_node *task_blk_node = NULL, *tmp = NULL;

    mutex_lock(&xp->xp_block_mutex);
    list_for_each_entry_safe(task_blk_node, tmp, &node->task_blk_head, node_list) {
        struct xsm_block *blk = task_blk_node->blk;

        pr_info("pool %d task %d exit offset %pK alloced by task %d ref %d blk ref %ld flag %lx\n", xp->pool_id,
            node->task->pid, (void *)(uintptr_t)blk->offset, blk->pid, task_blk_node->refcnt, blk->refcnt, blk->flag);

        xsmem_task_unlink_blk(task_blk_node);
        blk->refcnt -= task_blk_node->refcnt;
        if ((blk->refcnt <= 0) && xsmem_is_blk_need_auto_recycle(blk)) {
            xsmem_pool_stat_sub(xp, blk);
            xsmem_blockid_put(xp->mnt_ns, blk->id);
            xsmem_block_destroy(xp, blk);
        }

        kfree(task_blk_node);
    }
    mutex_unlock(&xp->xp_block_mutex);
}

static struct xsm_pool *xsmem_pool_hnode_find(const char *name, unsigned int name_len)
{
    struct xsm_pool *xp = NULL;
    unsigned int key = jhash(name, name_len, 0) & XSHM_HLIST_TABLE_MASK;

    hash_for_each_possible(xsmem_key_list, xp, hnode, key)
        if ((name_len == xp->key_len) && (strncmp(name, xp->key, xp->key_len) == 0)) {
            return xp;
        }

    return NULL;
}

static void xsm_pool_hnode_add(struct xsm_pool *xp)
{
    unsigned int key = jhash(xp->key, xp->key_len, 0) & XSHM_HLIST_TABLE_MASK;

    hash_add(xsmem_key_list, &xp->hnode, key);
}

static void xsm_pool_hnode_del(struct xsm_pool *xp)
{
    hash_del(&xp->hnode);
}

STATIC struct xsm_task *xsm_task_find(int pid)
{
    struct xsm_task *task = NULL;
    unsigned int key = (u32)pid & TASK_HASH_TABLE_MASK;

    hash_for_each_possible(task_table, task, link, key)
        if (task->pid == pid) {
            return task;
        }

    return NULL;
}

static void xsm_task_add(struct xsm_task *task)
{
    int key = (int)((u32)task->pid & TASK_HASH_TABLE_MASK);

    hash_add(task_table, &task->link, (u32)key);
}

static void xsm_task_del(struct xsm_task *task)
{
    hash_del(&task->link);
}

static struct xsm_task_pool_node *xsmem_pool_task_node_find(struct xsm_pool *xp, int pid)
{
    struct xsm_task_pool_node *node = NULL;

    list_for_each_entry(node, &xp->node_list_head, pool_node) {
        if (node->task->pid == pid) {
            return node;
        }
    }

    return NULL;
}

struct xsm_task_pool_node *task_pool_node_find(struct xsm_task *task, const struct xsm_pool *xp)
{
    struct xsm_task_pool_node *node = NULL;

    list_for_each_entry(node, &task->node_list_head, task_node)
        if (node->pool == xp) {
            return node;
        }

    return NULL;
}

static struct xsm_task_pool_node *task_pool_node_add(struct xsm_task *task, struct xsm_pool *xp, GroupShareAttr attr)
{
    struct xsm_task_pool_node *node = NULL;

    node = task_pool_node_find(task, xp);
    if (node != NULL) {
        pr_err("The pool has allready attach to task. (pool=%d, task=%d)\n", xp->pool_id, task->pid);
        return ERR_PTR(-ESRCH);
    }

    node = kmalloc(sizeof(*node), GFP_KERNEL | __GFP_ACCOUNT);
    if (unlikely(node == NULL)) {
        pr_err("alloc xsm_task_pool_node failed\n");
        return ERR_PTR(-ENOMEM);
    }

    node->attr = attr;
    node->task = task;
    node->pool = xp;

    INIT_LIST_HEAD(&node->exit_task_head);
    INIT_LIST_HEAD(&node->task_blk_head);
    INIT_LIST_HEAD(&node->task_prop_head);
    node->real_alloc_size = 0;
    node->alloc_size = 0;

    list_add_tail(&node->task_node, &task->node_list_head);
    mutex_init(&node->mutex);

    /* Do not add node to xp->list_head until all its elements initilized */
    mutex_lock(&xp->mutex);
    list_add_tail(&node->pool_node, &xp->node_list_head);
    node->task_id = xp->task_id;
    xp->task_id = ((xp->task_id + 1) < 0) ? 0 : xp->task_id + 1;
    mutex_unlock(&xp->mutex);

    proc_fs_xsmem_pool_add_task(node);

    if (node->attr.alloc == 1) {
        pr_debug("pool %d task %d attr has alloc\n", xp->pool_id, node->task->pid);
    } else {
        pr_debug("pool %d task %d attr has no alloc\n", xp->pool_id, node->task->pid);
    }

    pr_debug("pool_id %d, node %pK, pid %d\n", xp->pool_id, node, node->task->pid);

    return node;
}

static void task_pool_node_del(struct xsm_task_pool_node *node)
{
    struct xsm_pool *xp = node->pool;
    struct xsm_exit_task *exit_task = NULL, *tmp = NULL;

    proc_fs_xsmem_pool_del_task(node);

    mutex_lock(&xp->mutex);
    list_del(&node->pool_node);
    mutex_unlock(&xp->mutex);

    mutex_lock(&node->mutex);
    list_for_each_entry_safe(exit_task, tmp, &node->exit_task_head, node) {
        pr_debug("task %d id %d exit uid %llu\n", node->task->pid, node->task_id, exit_task->uid);
        list_del(&exit_task->node);
        kfree(exit_task);
    }
    mutex_unlock(&node->mutex);

    list_del(&node->task_node);
    kfree(node);
}

static bool xsmem_is_task_has_admin(struct xsm_task *task, struct xsm_pool *xp)
{
    struct xsm_task_pool_node *node = NULL;

    node = task_pool_node_find(task, xp);
    if (node == NULL) {
        if (xp->create_pid == task->pid) {
            return true;
        }
    } else {
        if (node->attr.admin == 1) {
            return true;
        }
        pr_debug("admin %u, alloc %u, write %u, read %u", node->attr.admin,
            node->attr.alloc, node->attr.write, node->attr.read);
    }

    return false;
}

static void xsmem_pool_notice_other_task(struct xsm_pool *xp, struct xsm_task_pool_node *cur_node)
{
    struct xsm_task_pool_node *node = NULL;
    struct xsm_exit_task *exit_task = NULL;

    mutex_lock(&xp->mutex);

    list_for_each_entry(node, &xp->node_list_head, pool_node) {
        if (node == cur_node) {
            continue;
        }

        exit_task = kmalloc(sizeof(struct xsm_exit_task), GFP_KERNEL | __GFP_ACCOUNT);
        if (exit_task == NULL) {
            mutex_unlock(&xp->mutex);
            pr_warn("alloc mem %lx failed\n", sizeof(struct xsm_exit_task));
            return;
        }

        exit_task->pid = cur_node->task->pid;
        exit_task->uid = cur_node->task->uid;
        mutex_lock(&node->mutex);
        list_add_tail(&exit_task->node, &node->exit_task_head);
        mutex_unlock(&node->mutex);
        pr_debug("pool %d task proc %d uid %llu exit, notice task %d\n",
            xp->pool_id, cur_node->task->pid, cur_node->task->uid, node->task->pid);
    }

    mutex_unlock(&xp->mutex);
}

static struct xsm_adding_task *xsmem_pool_adding_task_find(struct xsm_pool *xp, int pid)
{
    struct xsm_adding_task *adding_task = NULL;

    list_for_each_entry(adding_task, &xp->adding_task_list_head, pool_node)
        if (adding_task->pid == pid) {
            return adding_task;
        }

    return NULL;
}

static int xsmem_pool_adding_task_add(struct xsm_pool *xp, int pid, GroupShareAttr attr)
{
    struct xsm_adding_task *adding_task = NULL;

    adding_task = kmalloc(sizeof(*adding_task), GFP_KERNEL | __GFP_ACCOUNT);
    if (unlikely(adding_task == NULL)) {
        pr_err("alloc adding_task node failed\n");
        return -ENOMEM;
    }

    adding_task->pid = pid;
    adding_task->attr = attr;
    list_add_tail(&adding_task->pool_node, &xp->adding_task_list_head);

    return 0;
}

static void xsmem_pool_adding_task_del(struct xsm_adding_task *adding_task)
{
    list_del(&adding_task->pool_node);
    kfree(adding_task);
}

static void xsmem_pool_adding_task_clear(struct xsm_pool *xp)
{
    struct xsm_adding_task *adding_task = NULL, *tmp = NULL;

    list_for_each_entry_safe(adding_task, tmp, &xp->adding_task_list_head, pool_node) {
        xsmem_pool_adding_task_del(adding_task);
    }
}

static int xsmem_pool_task_readd(const struct xsm_pool *xp, int pid, GroupShareAttr new_attr, GroupShareAttr cur_attr)
{
    if (*(unsigned int *)&new_attr == *(unsigned int *)&cur_attr) {
        return 0;
    } else {
        pr_warn("Don't add pid to pool repeatedly. (pool=%d, pid=%d)\n", xp->pool_id, pid);
        return -EEXIST;
    }
}

static int xsmem_pool_task_add(struct xsm_pool *xp, int pid, GroupShareAttr attr)
{
    int ret;
    struct xsm_task_pool_node *node = NULL;
    struct xsm_adding_task *adding_task = NULL;

    mutex_lock(&xp->mutex);
    adding_task = xsmem_pool_adding_task_find(xp, pid);
    if (adding_task != NULL) {
        ret = xsmem_pool_task_readd(xp, pid, attr, adding_task->attr);
        goto out;
    }

    node = xsmem_pool_task_node_find(xp, pid);
    if (node != NULL) {
        ret = xsmem_pool_task_readd(xp, pid, attr, node->attr);
        goto out;
    }

    ret = xsmem_pool_adding_task_add(xp, pid, attr);
    if (ret != 0) {
        goto out;
    }

    xp->adding_id++;
    wake_up_interruptible(&xp->adding_task_wq);

    pr_info("Xsmem pool task add. (comm=%s, tgid=%d, pid=%d, pool_id=%d, process=%d)\n",
        current->comm, current->tgid, current->pid, xp->pool_id, pid);
out:
    mutex_unlock(&xp->mutex);

    return ret;
}

static int xsmem_pool_task_del(struct xsm_pool *xp, int pid)
{
    struct xsm_adding_task *adding_task = NULL;

    mutex_lock(&xp->mutex);

    adding_task = xsmem_pool_adding_task_find(xp, pid);
    if (adding_task == NULL) {
        mutex_unlock(&xp->mutex);
        pr_err("pool_id %d has no task %d\n", xp->pool_id, pid);
        return -EINVAL;
    }

    xsmem_pool_adding_task_del(adding_task);
    mutex_unlock(&xp->mutex);

    pr_info("<%s:%d,%d> xsmem_pool_task_del, pool_id:%d, pid:%d\n",
        current->comm, current->tgid, current->pid, xp->pool_id, pid);

    return 0;
}

static int xsmem_pool_task_attr_get(const struct xsm_task *task, struct xsm_pool *xp, int timeout,
    GroupShareAttr *attr)
{
    struct xsm_adding_task *adding_task = NULL;
    int ret, pid = (int)current->tgid, adding_id = xp->adding_id;
    unsigned long begin = jiffies, escaped, jf_timeout;
    unsigned long prop;
    long ret_wait_event;

    while (1) {
        mutex_lock(&xp->mutex);
        adding_task = xsmem_pool_adding_task_find(xp, pid);
        if (adding_task != NULL) {
            *attr = adding_task->attr;

            prop = ((attr->alloc != 0) || (attr->write != 0)) ? (PROT_WRITE | PROT_READ) : PROT_READ;
            if (xp->algo->xsm_pool_perm_add != NULL) {
                ret = xp->algo->xsm_pool_perm_add(xp, adding_task->pid, prop);
            } else {
                ret = 0;
            }

            pr_debug("Find adding task. "
                "(task_id=%d; pool_id=%d; timeout=%dms; admin=%u; alloc=%u; write=%u; adding_task=%d)\n",
                task->pid, xp->pool_id, timeout, attr->admin, attr->alloc, attr->write, adding_task->pid);

            xsmem_pool_adding_task_del(adding_task);
            mutex_unlock(&xp->mutex);
            return ret;
        }
        mutex_unlock(&xp->mutex);

        if (timeout == 0) {
            pr_err("the pool %d get task %d attr failed no wait\n", xp->pool_id, task->pid);
            return -EINVAL;
        }

        escaped = jiffies - begin;
        jf_timeout = msecs_to_jiffies((unsigned int)timeout);
        jf_timeout = (escaped >= jf_timeout) ? 1 : jf_timeout - escaped;

        pr_debug("<%s:%d,%d> the pool %d task %d begin wait add event adding_id %d\n",
            current->comm, current->tgid, current->pid, xp->pool_id, task->pid, adding_id);

        ret_wait_event =
            wait_event_interruptible_timeout(xp->adding_task_wq, (adding_id != xp->adding_id), (long)jf_timeout);
        if (ret_wait_event <= 0) {
            if (ret_wait_event == 0) {
                ret = -EAGAIN;
            } else {
                ret = (int)ret_wait_event;
            }
            break;
        }
        adding_id = xp->adding_id;
        pr_debug("the pool %d task %d retry adding_id %d\n", xp->pool_id, task->pid, adding_id);
    };

    return ret;
}

static unsigned int xsmem_pool_get_cache_type(struct xsm_pool_algo *algo)
{
    if (algo->xsm_pool_cache_create != NULL) {
        return BUFF_CACHE;
    }
    return BUFF_NOCACHE;
}

/*
 * The caller must hold xsmem_mutex and this function would unlock xsmem_mutex on failure.
 */
static struct xsm_pool *xsmem_pool_create(struct xsm_pool_algo *algo, struct xsm_reg_arg *arg)
{
    unsigned long ns = (unsigned long)current->nsproxy->mnt_ns;
    unsigned int key_len = arg->key_len;
    struct xsm_pool *xp = NULL;
    char *key = arg->key;
    int id, ret;

    ret = xsmem_ns_pool_num_inc(ns, algo->num);
    if (ret != 0) {
        pr_err("Namespace pool num inc failed. (ns=%lu; ret=%d)\n", ns, ret);
        return ERR_PTR(ret);
    }

    xp = kzalloc(sizeof(*xp) + key_len + 1, GFP_KERNEL | __GFP_ACCOUNT);
    if (unlikely(xp == NULL)) {
        xsmem_ns_pool_num_dec(ns, algo->num);
        pr_err("alloc pool memory failed\n");
        return ERR_PTR(-ENOMEM);
    }

    xp->algo = algo;
    ret = algo->xsm_pool_init(xp, arg);
    if (ret < 0) {
        kfree(xp);
        xsmem_ns_pool_num_dec(ns, algo->num);
        pr_err("init hook failed\n");
        return ERR_PTR(ret);
    }

    ret = xsmem_alloc_id(xp, &id);
    if (ret != 0) {
        algo->xsm_pool_free(xp);
        kfree(xp);
        xsmem_ns_pool_num_dec(ns, algo->num);
        return ERR_PTR(ret);
    }

    xp->pool_id = id;
    xp->create_pid = current->tgid;
    xp->priv_mbuf_flag = arg->priv_flag;
    xp->mnt_ns = ns;
    xp->cache_type = xsmem_pool_get_cache_type(algo);
    mutex_init(&xp->mutex);
    atomic_set(&xp->refcnt, 0);
    INIT_LIST_HEAD(&xp->node_list_head);
    INIT_LIST_HEAD(&xp->adding_task_list_head);
    init_waitqueue_head(&xp->adding_task_wq);
    INIT_LIST_HEAD(&xp->prop_list_head);

    mutex_init(&xp->xp_block_mutex);

    xp->block_root = RB_ROOT;

    xp->key_len = key_len;
    ret = strncpy_s(xp->key, key_len + 1, key, key_len);
    if (ret != 0) {
        pr_warn("pool id %d strncpy_s failed\n", id);
    }
    xp->key[key_len] = '\0';
    xsm_pool_hnode_add(xp);

    proc_fs_add_xsmem_pool(xp);

    pr_info("create pool id %d\n", id);

    return xp;
}

static void xsmem_pool_delete(struct xsm_pool *xp)
{
    pr_info("delete pool name %s pool id %d\n", xp->key, xp->pool_id);

    proc_fs_del_xsmem_pool(xp);

    xsmem_clear_block(xp);
    xsmem_pool_adding_task_clear(xp);
    xsmem_pool_prop_clear(xp);
    xsm_pool_hnode_del(xp);

    xsmem_delete_id(xp->pool_id);

    xp->algo->xsm_pool_free(xp);
    xsmem_ns_pool_num_dec(xp->mnt_ns, xp->algo->num);

    kfree(xp);
}

struct xsm_pool *xsmem_pool_get(int pool_id)
{
    struct xsm_pool *xp = NULL;

    mutex_lock(&xsmem_mutex);
    xp = xsmem_get_xp_by_id(pool_id);
    if (xp) {
        atomic_inc(&xp->refcnt);
    }
    mutex_unlock(&xsmem_mutex);

    return xp;
}

void xsmem_pool_put(struct xsm_pool *xp)
{
    mutex_lock(&xsmem_mutex);
    if (!atomic_dec_and_test(&xp->refcnt)) {
        mutex_unlock(&xsmem_mutex);
        return;
    }

    xsmem_pool_delete(xp);
    mutex_unlock(&xsmem_mutex);

    return;
}

/*
 * get the POOL specified by key, create one if it not exist.
 */
static struct xsm_pool *xsmem_pool_register(struct xsm_pool_algo *algo, struct xsm_reg_arg *arg)
{
    struct xsm_pool *xp = NULL;

    mutex_lock(&xsmem_mutex);
    xp = xsmem_pool_hnode_find((const char *)arg->key, arg->key_len);
    if (xp != NULL) {
        int create_pid = xp->create_pid;
        mutex_unlock(&xsmem_mutex);
        pr_err("pool name %s is exist create_pid %d\n", arg->key, create_pid);
        return ERR_PTR(-EEXIST);
    }

    xp = xsmem_pool_create(algo, arg);
    if (IS_ERR(xp)) {
        mutex_unlock(&xsmem_mutex);
        pr_err("pool name %s create failed\n", arg->key);
        return xp;
    }

    /*
     * Here increase the POOL's refcnt by one, one for the convenience of fallback in
     * error branch in ioctl_xsmem_pool_register(), another to promise that the POOL
     * cannot be delete in the later attach routine.
     */
    atomic_inc(&xp->refcnt);
    mutex_unlock(&xsmem_mutex);

    return xp;
}

static void xsmem_pool_unregister(struct xsm_pool *xp)
{
    pr_info("pool_id %d unregister, refcnt %d\n", xp->pool_id, atomic_read(&xp->refcnt));
    xsmem_pool_put(xp);
}

/*
 * The refcnt of the xp should be increased by one on success.
 * We don't require the xsmem_mutex here since the xp->refcnt has been increased already.
 */
static int xsmem_pool_attach(struct xsm_task *task, struct xsm_pool *xp, int timeout)
{
    struct xsm_task_pool_node *node = NULL;
    GroupShareAttr attr;
    int ret;

    mutex_lock(&task->mutex);
    node = task_pool_node_find(task, xp);
    mutex_unlock(&task->mutex);
    if (node != NULL) {
        pr_err("the pool %d has allready attach to task %d\n", xp->pool_id, task->pid);
        return -ESRCH;
    }

    ret = xsmem_pool_task_attr_get(task, xp, timeout, &attr);
    if (ret != 0) {
        pr_err("the pool %d add task %d get attr failed\n", xp->pool_id, task->pid);
        return ret;
    }

    if ((xp->create_pid != task->pid) && (atomic_inc_return(&task->pool_num) > MAX_XP_NUM_OF_TASK)) {
        atomic_dec(&task->pool_num);
        pr_err("Current task's pool_num has reached its limit. (task_pid=%d)\n", task->pid);
        return -ENOMEM;
    }

    mutex_lock(&task->mutex);
    node = task_pool_node_add(task, xp, attr);
    if (IS_ERR(node)) {
        mutex_unlock(&task->mutex);
        if (xp->create_pid != task->pid) {
            atomic_dec(&task->pool_num);
        }
        pr_err("the pool %d add task %d node failed\n", xp->pool_id, task->pid);
        return (int)PTR_ERR(node);
    }
    atomic_inc(&xp->refcnt);
    task->attached_pool_count++;
    mutex_unlock(&task->mutex);

    return 0;
}

static void xsmem_pool_task_detach(struct xsm_task_pool_node *node)
{
    struct xsm_pool *xp = node->pool;
    struct xsm_task *task = node->task;

    xsmem_proc_grp_prop_del(node);
    xsmem_pool_task_prop_clear(xp, task);
    xsmem_task_clear_block(xp, node);

    task->attached_pool_count--;
    task_pool_node_del(node);

    if (xp->create_pid != task->pid) {
        atomic_dec(&task->pool_num);
    }

    xsmem_pool_put(xp);
}

static int xsmem_pool_detach(struct xsm_task *task, struct xsm_pool *xp)
{
    struct xsm_task_pool_node *node = NULL;

    node = task_pool_node_find(task, xp);
    if (node == NULL) {
        pr_err("the pool %d has not task %d\n", xp->pool_id, task->pid);
        return -ESRCH;
    }

    xsmem_pool_task_detach(node);

    return 0;
}

static int xsmem_check_pool_name(char *name, unsigned int name_len)
{
    char *next = NULL;

    if (strnlen(name, XSHM_KEY_SIZE) != name_len) {
        return -EINVAL;
    }

    if ((name_len == 1) && (name[0] == '.')) {
        return -EINVAL;
    }
    if ((name_len == 2) && (name[0] == '.') && (name[1] == '.')) { /* 2: name can't be '..' */
        return -EINVAL;
    }

    /* name including '/' will cause proc_mkdir fail.
     * for example, name="aaa/bbb", aaa directory don't exist.
     */
    next = strchr(name, '/');
    if (next != NULL) {
        return -EINVAL;
    }
    return 0;
}

static bool xsmem_confirm_user(void)
{
    const struct cred *cred = current_cred();

    /* userid not init, means don't need confirm */
    if (g_xsmem_authorized_user_id == XSMEM_NOT_CONFIRM_USER_ID) {
        return true;
    }

    if (cred != NULL && (cred->uid.val == g_xsmem_authorized_user_id ||
        cred->uid.val == XSMEM_ROOT_USER_ID)) {
        return true;
    }

    return false;
}

static int ioctl_xsmem_pool_register(struct xsm_task *task, unsigned int cmd, unsigned long arg)
{
    struct xsm_pool *xp = NULL;
    unsigned int key_len;
    char key[XSHM_KEY_SIZE];
    char key_tmp[XSHM_KEY_AND_NAMESPACE_SIZE] = {0};
    struct xsm_reg_arg reg_arg;
    struct xsm_pool_algo *algo = NULL;
    int ret;

    if (xsmem_confirm_user() == false) {
        pr_err("The user is not allowed to register pool.\n");
        return -EPERM;
    }

    if (copy_from_user_safe(&reg_arg, (struct xsm_reg_arg __user *)arg, sizeof(reg_arg)) != 0) {
        pr_err("register: copy_from_user failed\n");
        return -EFAULT;
    }

    algo = xsmem_find_algo(reg_arg.algo);
    if (unlikely(algo == NULL)) {
        pr_err("unsupported algorithm %d\n", reg_arg.algo);
        return -EINVAL;
    }

    key_len = reg_arg.key_len;
    if ((reg_arg.key == NULL) || (key_len == 0) || (key_len >= XSHM_KEY_SIZE)) {
        pr_err("Key or key_len is invalid. (key_len=%u)\n", key_len);
        return -EINVAL;
    }

    if (copy_from_user_safe(key, (char __user *)reg_arg.key, key_len) != 0) {
        pr_err("copy key from user failed, key_len %d\n", key_len);
        return -EFAULT;
    }

    key[key_len] = '\0';
    ret = xsmem_check_pool_name(key, key_len);
    if (ret != 0) {
        pr_err("Pool name is invalid. (key_len=%u)\n", key_len);
        return ret;
    }

    if (atomic_inc_return(&task->pool_num) > MAX_XP_NUM_OF_TASK) {
        atomic_dec(&task->pool_num);
        pr_err("Current task's pool_num has reached its limit. (task_pid=%d)\n", task->pid);
        return -ENOMEM;
    }

    ret = xsmem_strcat_with_ns(key_tmp, XSHM_KEY_AND_NAMESPACE_SIZE, key);
    if (ret < 0) {
        atomic_dec(&task->pool_num);
        pr_err("Sprintf_s error. (ret=%d)\n", ret);
        return -EINVAL;
    }

    reg_arg.key = key_tmp;
    reg_arg.key_len = (unsigned int)strlen(key_tmp);
    xp = xsmem_pool_register(algo, &reg_arg);
    if (IS_ERR(xp)) {
        atomic_dec(&task->pool_num);
        pr_err("register pool %s failed algo %d size %lx\n", key_tmp, reg_arg.algo, reg_arg.pool_size);
        return (int)PTR_ERR(xp);
    }
    mutex_lock(&task->mutex);
    list_add_tail(&xp->register_task_list, &task->register_xp_list_head);
    mutex_unlock(&task->mutex);

    pr_info("Xsmem pool register success. (pool_name=%s; id=%d; algo=%d; size=%lx)\n",
        key_tmp, xp->pool_id, reg_arg.algo, reg_arg.pool_size);

    return xp->pool_id;
}

static void xsmem_task_unregister_pool(struct xsm_task *task, struct xsm_pool *xp)
{
    atomic_dec(&task->pool_num);
    xsmem_pool_unregister(xp);
}

static int ioctl_xsmem_pool_unregister(struct xsm_task *task, unsigned int cmd, unsigned long arg)
{
    struct xsm_task_pool_node *node = NULL;
    struct xsm_pool *xp = NULL, *xp_tmp = NULL;
    int pool_id = (int)arg;

    mutex_lock(&task->mutex);
    list_for_each_entry(xp_tmp, &task->register_xp_list_head, register_task_list) {
        if (xp_tmp->pool_id == pool_id) {
            xp = xp_tmp;
            break;
        }
    }

    if (xp == NULL) {
        mutex_unlock(&task->mutex);
        pr_err("pool_id %d couldn't find the pool\n", pool_id);
        return -ENODEV;
    }

    node = task_pool_node_find(task, xp);
    if (node != NULL) {
        (void)xsmem_pool_detach(task, xp);
    }

    pr_info("Xsmem pool unregister success. (pool_name=%s; id=%d)\n", xp->key, xp->pool_id);

    list_del(&xp->register_task_list);
    mutex_unlock(&task->mutex);

    xsmem_task_unregister_pool(task, xp);

    return 0;
}

static int ioctl_xsmem_cache_create(struct xsm_task *task, unsigned int cmd, unsigned long arg)
{
    struct xsm_cache_create_arg cache_arg;
    struct xsm_pool *xp = NULL;
    int ret;

    if (copy_from_user_safe(&cache_arg, (struct xsm_cache_create_arg __user *)arg, sizeof(cache_arg)) != 0) {
        pr_err("Copy_from_user failed.\n");
        return -EFAULT;
    }

    xp = xsmem_pool_get(cache_arg.poolId);
    if (xp == NULL) {
        pr_err("Invalid pool id. (pool_id=%d)\n", cache_arg.poolId);
        return -EINVAL;
    }

    if (xp->create_pid != current->tgid) {
        xsmem_pool_put(xp);
        pr_err("Task has no cache alloc prop. (proc=%s; tgid=%d; pid=%d; pool_id=%d)\n",
            current->comm, current->tgid, current->pid, cache_arg.poolId);
        return -EPERM;
    }

    if (xp->algo->xsm_pool_cache_create == NULL) {
        pr_err("Not support cache. (name=%s)\n", xp->algo->name);
        xsmem_pool_put(xp);
        return -EOPNOTSUPP;
    }

    ret = xp->algo->xsm_pool_cache_create(xp, &cache_arg);
    if (ret != 0) {
        xsmem_pool_put(xp);
        pr_err("Xsm_pool_cache_create fail. (pool_id=%d)\n", cache_arg.poolId);
        return ret;
    }

    xsmem_pool_put(xp);
    return 0;
}

static unsigned int _xsmem_show_alloced_mem_info(struct xsm_block *blk,
    unsigned int max_show_cnt)
{
    struct xsm_task_block_node *task_blk_node = NULL;
    unsigned int show_alloced_mem_cnt = 0;

    list_for_each_entry(task_blk_node, &blk->task_blk_head, blk_list) {
        if (show_alloced_mem_cnt < max_show_cnt) {
            show_alloced_mem_cnt++;
            pr_warn("va=0x%lx; flag=%lx; pid=%d, ref=%d\n",
                blk->offset, blk->flag, task_blk_node->node->task->pid, task_blk_node->refcnt);
        } else {
            break;
        }
    }

    return show_alloced_mem_cnt;
}

bool blk_is_alloced_from_os(struct xsm_block *blk)
{
    return ((blk->flag & XSMEM_BLK_ALLOC_FROM_OS) == XSMEM_BLK_ALLOC_FROM_OS);
}

#define XSMEM_SHOW_ALLOCED_MEM_MAX_CNT 20
static void xsmem_show_alloced_mem_info(struct xsm_pool *xp)
{
    struct rb_node *node = NULL;
    struct xsm_block *blk = NULL;
    unsigned int show_cnt = 0;
    unsigned int max_show_cnt = XSMEM_SHOW_ALLOCED_MEM_MAX_CNT;
    unsigned long os_mem = 0;
    unsigned long cache_mem = 0;

    mutex_lock(&xp->xp_block_mutex);
    pr_warn("Show alloced mem srart. (pool_id=%d)\n", xp->pool_id);

    node = rb_first(&xp->block_root);
    while (node != NULL) {
        blk = rb_entry(node, struct xsm_block, block_rb_node);
        node = rb_next(node);

        if (blk_is_alloced_from_os(blk)) {
            os_mem += blk->real_size;
        } else {
            show_cnt += _xsmem_show_alloced_mem_info(blk, max_show_cnt);
            max_show_cnt = XSMEM_SHOW_ALLOCED_MEM_MAX_CNT - show_cnt;
            cache_mem += blk->real_size;
        }
    }

    pr_warn("Show alloced mem end. (pool_id=%d; os_mem(Bytes)=%lu; cache_mem(Bytes)=%lu; show_cnt=%u)\n",
        xp->pool_id, os_mem, cache_mem, show_cnt);
    mutex_unlock(&xp->xp_block_mutex);
}

static int ioctl_xsmem_cache_destroy(struct xsm_task *task, unsigned int cmd, unsigned long arg)
{
    struct xsm_cache_destroy_arg cache_arg;
    struct xsm_pool *xp = NULL;
    int ret;

    if (copy_from_user_safe(&cache_arg, (struct xsm_cache_destroy_arg __user *)arg, sizeof(cache_arg)) != 0) {
        pr_err("Copy_from_user failed.\n");
        return -EFAULT;
    }

    xp = xsmem_pool_get(cache_arg.poolId);
    if (xp == NULL) {
        pr_err("Invalid pool id. (pool_id=%d)\n", cache_arg.poolId);
        return -EINVAL;
    }

    if (xp->create_pid != current->tgid) {
        xsmem_pool_put(xp);
        pr_err("Task has no cache free prop. (proc=%s; tgid=%d; pid=%d; pool_id=%d)\n",
            current->comm, current->tgid, current->pid, cache_arg.poolId);
        return -EPERM;
    }

    if (xp->algo->xsm_pool_cache_destroy == NULL) {
        xsmem_pool_put(xp);
        return -EOPNOTSUPP;
    }

    ret = xp->algo->xsm_pool_cache_destroy(xp, &cache_arg);
    if (ret != 0) {
        if (ret == -EBUSY) {
            xsmem_show_alloced_mem_info(xp);
        } else {
            pr_err("Xsm_pool_cache_destroy fail. (pool_id=%d; ret=%d)\n", cache_arg.poolId, ret);
        }
        xsmem_pool_put(xp);
        return ret;
    }

    xsmem_pool_put(xp);
    return 0;
}


static int ioctl_xsmem_cache_query(struct xsm_task *task, unsigned int cmd, unsigned long arg)
{
    struct xsm_query_cache_arg *usr_arg = (struct xsm_query_cache_arg __user *)arg;
    struct xsm_query_cache_arg query_arg;
    struct xsm_task_pool_node *node = NULL;
    GrpQueryGroupAddrInfo *cache_buff = NULL;
    unsigned int cache_cnt;
    struct xsm_pool *xp = NULL;
    int ret;

    if (copy_from_user_safe(&query_arg, usr_arg, sizeof(query_arg)) != 0) {
        pr_err("Copy_from_user failed.\n");
        return -EFAULT;
    }

    xp = xsmem_pool_get(query_arg.poolId);
    if (xp == NULL) {
        pr_err("Invalid pool id. (pool_id=%d)\n", query_arg.poolId);
        return -EINVAL;
    }

    // for safety, va can't be queried when not in the same pool.
    node = task_pool_node_find(task, xp);
    if (node == NULL) {
        pr_err("Task is not in the pool. (pool_id=%d; task_pid=%d)\n", xp->pool_id, task->pid);
        xsmem_pool_put(xp);
        return -ESRCH;
    }

    if (xp->algo->xsm_pool_cache_query == NULL) {
        pr_err("Not support cache. (name=%s)\n", xp->algo->name);
        xsmem_pool_put(xp);
        return -EOPNOTSUPP;
    }

    cache_cnt = query_arg.cache_cnt;
    if (cache_cnt == 0 || cache_cnt > BUFF_GROUP_ADDR_MAX_NUM) {
        cache_cnt = BUFF_GROUP_ADDR_MAX_NUM;
    }
    cache_buff = kmalloc(sizeof(GrpQueryGroupAddrInfo) * cache_cnt, GFP_KERNEL | __GFP_ACCOUNT);
    if (cache_buff == NULL) {
        pr_err("Alloc memory for GrpQueryCacheInfo failed.\n");
        xsmem_pool_put(xp);
        return -ENOMEM;
    }

    ret = xp->algo->xsm_pool_cache_query(xp, query_arg.devId, cache_buff, &cache_cnt);
    if (ret != 0) {
        kfree(cache_buff);
        xsmem_pool_put(xp);
        pr_err("Cache query fail. (pool_id=%d)\n", query_arg.poolId);
        return ret;
    }

    if (cache_cnt != 0) {
        ret = (int)copy_to_user_safe(query_arg.cache_buff, cache_buff,
            cache_cnt * sizeof(GrpQueryGroupAddrInfo));
    }
    ret += (int)put_user(cache_cnt, &usr_arg->cache_cnt);

    kfree(cache_buff);
    xsmem_pool_put(xp);
    return ret;
}

static int ioctl_xsmem_priv_flag_query(struct xsm_task *task, unsigned int cmd, unsigned long arg)
{
    struct xsm_query_pool_flag_arg *usr_arg = (struct xsm_query_pool_flag_arg __user *)arg;
    struct xsm_query_pool_flag_arg query_arg;
    struct xsm_task_pool_node *node = NULL;
    struct xsm_pool *xp = NULL;
    int ret;

    if (copy_from_user_safe(&query_arg, usr_arg, sizeof(query_arg)) != 0) {
        pr_err("Copy_from_user failed.\n");
        return -EFAULT;
    }

    xp = xsmem_pool_get(query_arg.pool_id);
    if (xp == NULL) {
        pr_err("Invalid pool id. (pool_id=%d)\n", query_arg.pool_id);
        return -EINVAL;
    }

    /* for safety, flag can't be queried when not in the same pool. */
    node = task_pool_node_find(task, xp);
    if (node == NULL) {
        pr_err("Task is not in the pool. (pool_id=%d; task_pid=%d)\n", xp->pool_id, task->pid);
        xsmem_pool_put(xp);
        return -ESRCH;
    }

    ret = (int)put_user(xp->priv_mbuf_flag, &usr_arg->priv_flag);

    xsmem_pool_put(xp);
    return ret;
}

static int ioctl_xsmem_pool_task_common(struct xsm_task *task, unsigned int cmd, unsigned long arg)
{
    struct xsm_task_arg proc_arg;
    struct xsm_pool *xp = NULL;
    int ret;

    if (copy_from_user_safe(&proc_arg, (struct xsm_task_arg __user *)arg, sizeof(proc_arg)) != 0) {
        pr_err("copy_from_user failed\n");
        return -EFAULT;
    }

    xp = xsmem_pool_get(proc_arg.pool_id);
    if (xp == NULL) {
        pr_err("invalid pool_id %d\n", proc_arg.pool_id);
        return -EINVAL;
    }

    pr_debug("create_pid:%d pid:%d cmd %d op pid %d\n", xp->create_pid, task->pid, cmd, proc_arg.pid);

    mutex_lock(&task->mutex);
    if (xsmem_is_task_has_admin(task, xp) == false) {
        mutex_unlock(&task->mutex);
        xsmem_pool_put(xp);
        pr_err("task %d has no admin attr in pool %d(creat_pid %d).\n",
            task->pid, proc_arg.pool_id, xp->create_pid);
        return -EINVAL;
    }
    mutex_unlock(&task->mutex);

    ret = xsmem_get_tgid_by_vpid(proc_arg.pid, &proc_arg.pid);
    if (ret != 0) {
        xsmem_pool_put(xp);
        return ret;
    }
    if (cmd == XSMEM_POOL_TASK_ADD) {
        ret = xsmem_pool_task_add(xp, proc_arg.pid, proc_arg.attr);
    } else {
        ret = xsmem_pool_task_del(xp, proc_arg.pid);
    }

    xsmem_pool_put(xp);

    return ret;
}

static int ioctl_xsmem_pool_attach(struct xsm_task *task, unsigned int cmd, unsigned long arg)
{
    struct xsm_task_attach_arg *usr_arg = (struct xsm_task_attach_arg __user *)arg;
    struct xsm_pool *xp = NULL;
    struct xsm_task_attach_arg attach;
    int pool_id, ret;

    if (copy_from_user_safe(&attach, usr_arg, sizeof(attach)) != 0) {
        pr_err("copy_from_user failed\n");
        return -EFAULT;
    }

    pool_id = attach.pool_id;

    xp = xsmem_pool_get(pool_id);
    if (xp == NULL) {
        pr_err("pool_id %d couldn't find the pool\n", pool_id);
        return -ENODEV;
    }

    ret = xsmem_pool_attach(task, xp, attach.timeout);
    if (ret != 0) {
        xsmem_pool_put(xp);
        return ret;
    }

    attach.uid = task->uid;
    attach.cache_type = xp->cache_type;
    pr_info("<%s:%d,%d> pool_id %d tid %d uid %llu attach success, refcnt %d\n", current->comm,
        current->tgid, current->pid, pool_id, current->tgid, attach.uid, atomic_read(&xp->refcnt));
    xsmem_pool_put(xp);

    ret = (int)put_user(attach.uid, &usr_arg->uid);
    ret += (int)put_user(attach.cache_type, &usr_arg->cache_type);
    return ret;
}

static int ioctl_xsmem_pool_detach(struct xsm_task *task, unsigned int cmd, unsigned long arg)
{
    struct xsm_pool *xp = NULL;
    int pool_id = (int)arg;
    int ret;

    xp = xsmem_pool_get(pool_id);
    if (xp == NULL) {
        pr_err("pool_id %d couldn't find the pool\n", pool_id);
        return -ENODEV;
    }
    mutex_lock(&task->mutex);
    ret = xsmem_pool_detach(task, xp);
    if (ret == 0) {
        pr_info("pool_id %d task %d detach success,  refcnt %d\n", pool_id, current->tgid, atomic_read(&xp->refcnt));
    }
    mutex_unlock(&task->mutex);

    xsmem_pool_put(xp);

    return ret;
}

static int ioctl_xsmem_block_alloc(struct xsm_pool *xp, struct xsm_task_pool_node *node, struct xsm_block_arg *arg)
{
    struct xsm_task_block_node *task_blk_node = NULL;
    struct xsm_block *blk = NULL;
    int blkid;
    int ret;

    if (node->attr.alloc == 0) {
        pr_err("Task has no alloc prop. (proc=%s; tgid=%d; pid=%d; pool_id=%d)\n",
            current->comm, current->tgid, current->pid, xp->pool_id);
        return -EINVAL;
    }

    blk = kmalloc(sizeof(*blk), GFP_KERNEL | __GFP_ACCOUNT);
    if (unlikely(blk == NULL)) {
        pr_err("Alloc xsm_block memory failed.\n");
        return -ENOMEM;
    }

    task_blk_node = kmalloc(sizeof(*task_blk_node), GFP_KERNEL | __GFP_ACCOUNT);
    if (unlikely(task_blk_node == NULL)) {
        pr_err("Alloc xshm_task_block_cnt memory failed.\n");
        kfree(blk);
        return -ENOMEM;
    }

    blkid = xsmem_blockid_get(xp->mnt_ns);
    if (blkid < 0) {
        pr_err("Alloc blkid fail. (blkid=%d)\n", blkid);
        kfree(blk);
        kfree(task_blk_node);
        return -ENOSPC;
    }

    INIT_LIST_HEAD(&blk->task_blk_head);
    blk->alloc_size = arg->alloc.size;
    blk->flag = arg->alloc.flag;
    ret = xp->algo->xsm_block_alloc(xp, blk);
    if (ret < 0) {
        pr_err("Pool block alloc failed. (pool_id=%d; size=0x%lx; flag=0x%lx; ret=%d)\n",
            xp->pool_id, blk->alloc_size, blk->flag, ret);
        xsmem_blockid_put(xp->mnt_ns, blkid);
        kfree(blk);
        kfree(task_blk_node);
    } else {
        arg->alloc.offset = blk->offset;
        arg->alloc.blkid = (u32)blkid;
        blk->id = blkid;
        blk->pid = (int)current->tgid;
        task_blk_node->refcnt = 1;
        blk->refcnt = 1;
        mutex_lock(&xp->xp_block_mutex);
        xsmem_add_block(xp, blk);
        xsmem_task_link_blk(node, blk, task_blk_node);
        node_stat_add(node, blk);
        xsmem_pool_stat_add(xp, blk);

        pr_debug("Block alloc success. (pool=%d; task=%d; alloc_size=0x%lx; offset=0x%lx; real_size=0x%lx; flag=0x%lx; "
            "pool_alloc_size=0x%lx; pool_real_size=0x%lx; task_alloc_size=0x%llx; task_real_size=0x%llx)\n",
            xp->pool_id, node->task->pid, blk->alloc_size, blk->offset, blk->real_size, blk->flag,
            xp->alloc_size, xp->real_alloc_size, node->alloc_size, node->real_alloc_size);
        mutex_unlock(&xp->xp_block_mutex);
    }

    return ret;
}

static int ioctl_xsmem_block_free(struct xsm_pool *xp, struct xsm_task_pool_node *node, unsigned long offset)
{
    struct xsm_task_block_node *task_blk_node = NULL;
    struct xsm_block *blk = NULL;

    mutex_lock(&xp->xp_block_mutex);
    blk = xsmem_find_block(xp, offset);
    if (blk == NULL) {
        mutex_unlock(&xp->xp_block_mutex);
        pr_err("Free unalloced block. (pool_id=%d; offset=0x%lx)\n", xp->pool_id, offset);
        return -ENODEV;
    }

    task_blk_node = xsmem_find_task_block_node(node, blk);
    if (task_blk_node == NULL) {
        mutex_unlock(&xp->xp_block_mutex);
        pr_err("Free other task block. (pool_id=%d; offset=0x%lx)\n", xp->pool_id, offset);
        return -ENODEV;
    }

    pr_debug("Block free. (pool=%d; task=%d; offset=0x%lx; alloced_task=%d; ref=%d; blk_ref=%ld)\n",
        xp->pool_id, node->task->pid, blk->offset, blk->pid, task_blk_node->refcnt, blk->refcnt);

    task_blk_node->refcnt--;
    if (task_blk_node->refcnt <= 0) {
        if (blk->pid == node->task->pid) {
            node_stat_sub(node, blk);
        }
        xsmem_task_unlink_blk(task_blk_node);
        kfree(task_blk_node);
    }

    blk->refcnt--;
    if (blk->refcnt <= 0) {
        xsmem_pool_stat_sub(xp, blk);
        xsmem_blockid_put(xp->mnt_ns, blk->id);
        pr_debug("Block free success. (pool=%d; task=%d; alloc_size=0x%lx; offset=0x%lx; real_size=0x%lx; pid=%d; "
            "pool_alloc_size=0x%lx; pool_real_size=0x%lx; task_alloc_size=0x%llx; task_real_size=0x%llx)\n",
            xp->pool_id, node->task->pid, blk->alloc_size, blk->offset, blk->real_size, blk->pid,
            xp->alloc_size, xp->real_alloc_size, node->alloc_size, node->real_alloc_size);
        xsmem_block_destroy(xp, blk);
    }

    mutex_unlock(&xp->xp_block_mutex);

    return 0;
}

static int xsmem_copy_blk_alloc_info_to_user(struct xsm_block_arg *block_arg, struct xsm_block_arg *usr_arg)
{
    int ret;

    ret = (int)put_user(block_arg->alloc.offset, &usr_arg->alloc.offset);
    if (ret != 0) {
        pr_err("Xsmem put alloc offset to user fail.\n");
        return ret;
    }

    ret = (int)put_user(block_arg->alloc.blkid, &usr_arg->alloc.blkid);
    if (ret != 0) {
        pr_err("Xsmem put alloc blkid to user fail.\n");
        return ret;
    }

    return 0;
}

static int ioctl_xsmem_block_common(struct xsm_task *task, unsigned int cmd, unsigned long arg)
{
    struct xsm_block_arg *usr_arg = (struct xsm_block_arg __user *)arg;
    struct xsm_task_pool_node *node = NULL;
    struct xsm_block_arg block_arg;
    struct xsm_pool *xp = NULL;
    int ret = 0;

    if (copy_from_user_safe(&block_arg, usr_arg, sizeof(block_arg)) != 0) {
        pr_err("copy_from_user failed\n");
        return -EFAULT;
    }

    xp = xsmem_pool_get(block_arg.pool_id);
    if (xp == NULL) {
        pr_err("invalid pool_id\n");
        return -EINVAL;
    }

    mutex_lock(&task->mutex);
    node = task_pool_node_find(task, xp);
    if (node == NULL) {
        mutex_unlock(&task->mutex);
        xsmem_pool_put(xp);
        pr_err("Task is not attached to pool. (proc=%s, tgid=%d, pid=%d, pool_id=%d)\n",
            current->comm, current->tgid, current->pid, block_arg.pool_id);
        return -EINVAL;
    }

    if (cmd == XSMEM_BLOCK_ALLOC) {
        ret = ioctl_xsmem_block_alloc(xp, node, &block_arg);
        if (ret == 0) {
            ret = xsmem_copy_blk_alloc_info_to_user(&block_arg, usr_arg);
        }
    } else {
        ret = ioctl_xsmem_block_free(xp, node, block_arg.free.offset);
    }
    mutex_unlock(&task->mutex);

    xsmem_pool_put(xp);

    return ret;
}

static int xsmem_pool_block_get(struct xsm_pool *xp, struct xsm_task_pool_node *node, struct xsm_block_arg *arg)
{
    struct xsm_task_block_node *task_blk_node = NULL;
    struct xsm_block *blk = NULL;
    unsigned long offset = arg->get.offset;

    mutex_lock(&xp->xp_block_mutex);
    blk = xsmem_find_block(xp, offset);
    if (blk == NULL) {
        mutex_unlock(&xp->xp_block_mutex);
        return -ENODEV;
    }

    task_blk_node = xsmem_find_task_block_node(node, blk);
    if (task_blk_node == NULL) {
        task_blk_node = kmalloc(sizeof(*task_blk_node), GFP_KERNEL | __GFP_ACCOUNT);
        if (unlikely(task_blk_node == NULL)) {
            mutex_unlock(&xp->xp_block_mutex);
            pr_err("alloc task_blk_node memory failed\n");
            return -ENOMEM;
        }

        xsmem_task_link_blk(node, blk, task_blk_node);
        task_blk_node->refcnt = 0;
    } else {
        if (task_blk_node->refcnt >= TASK_BLK_MAX_GET_NUM) {
            mutex_unlock(&xp->xp_block_mutex);
            pr_err("Task get blk too many times\n");
            return -EINVAL;
        }
    }

    blk->refcnt++;
    task_blk_node->refcnt++;

    arg->get.alloc_size = blk->alloc_size;
    arg->get.alloc_offset = blk->offset;
    arg->get.blkid = (u32)blk->id;

    pr_debug("pool %d task %d get offset %pK, blk offset %pK size %lx alloced by task %d ref %d blk ref %ld\n",
        xp->pool_id, node->task->pid, (void *)(uintptr_t)offset, (void *)(uintptr_t)blk->offset, blk->alloc_size,
        blk->pid, task_blk_node->refcnt, blk->refcnt);

    mutex_unlock(&xp->xp_block_mutex);

    return 0;
}

static int ioctl_xsmem_block_get(struct xsm_task *task, unsigned int cmd, unsigned long arg)
{
    struct xsm_block_arg *usr_arg = (struct xsm_block_arg __user *)arg;
    struct xsm_task_pool_node *node = NULL;
    struct xsm_block_arg block_arg;

    if (copy_from_user_safe(&block_arg, usr_arg, sizeof(block_arg)) != 0) {
        pr_err("copy_from_user failed\n");
        return -EFAULT;
    }
    mutex_lock(&task->mutex);
    list_for_each_entry(node, &task->node_list_head, task_node) {
        int ret = xsmem_pool_block_get(node->pool, node, &block_arg);
        if (ret == 0) {
            int pool_id = node->pool->pool_id;
            mutex_unlock(&task->mutex);
            ret = (int)put_user(block_arg.get.alloc_offset, &usr_arg->get.alloc_offset);
            ret += (int)put_user(block_arg.get.alloc_size, &usr_arg->get.alloc_size);
            ret += (int)put_user(block_arg.get.blkid, &usr_arg->get.blkid);
            ret += (int)put_user(pool_id, &usr_arg->pool_id);
            return ret;
        }
    }
    mutex_unlock(&task->mutex);
    pr_warn("offset %pK not find\n", (void *)(uintptr_t)block_arg.get.offset);
    return -ENODEV;
}

static int ioctl_xsmem_pool_id_query(struct xsm_task *task, unsigned int cmd, unsigned long arg)
{
    struct xsm_pool_query_arg *usr_arg = (struct xsm_pool_query_arg __user *)arg;
    struct xsm_pool_query_arg query_arg;
    struct xsm_pool *xp = NULL;
    char key_tmp[XSHM_KEY_AND_NAMESPACE_SIZE] = {0};
    char key[XSHM_KEY_SIZE];
    int ret;

    if (copy_from_user_safe(&query_arg, usr_arg, sizeof(query_arg)) != 0) {
        pr_err("copy_from_user failed\n");
        return -EFAULT;
    }

    if ((query_arg.key == NULL) || (query_arg.key_len == 0) || (query_arg.key_len >= XSHM_KEY_SIZE)) {
        pr_err("Key or key_len is invalid. (key_len=%u)\n", query_arg.key_len);
        return -EINVAL;
    }

    if (copy_from_user_safe(key, (char __user *)query_arg.key, query_arg.key_len) != 0) {
        pr_err("copy key from user failed, key_len %d\n", query_arg.key_len);
        return -EFAULT;
    }
    key[query_arg.key_len] = '\0';

    ret = xsmem_strcat_with_ns(key_tmp, XSHM_KEY_AND_NAMESPACE_SIZE, key);
    if (ret < 0) {
        pr_err("Sprintf_s error. (ret=%d)\n", ret);
        return -EINVAL;
    }

    query_arg.key_len = (u32)strlen(key_tmp);
    pr_debug("Attempt to find pool name. (pool name=%s)\n", key_tmp);
    mutex_lock(&xsmem_mutex);
    xp = xsmem_pool_hnode_find((const char *)key_tmp, query_arg.key_len);
    if (xp == NULL) {
        mutex_unlock(&xsmem_mutex);
        return -EINVAL;
    }

    query_arg.pool_id = xp->pool_id;
    mutex_unlock(&xsmem_mutex);

    return (int)put_user(query_arg.pool_id, &usr_arg->pool_id);
}

static unsigned int xsmem_get_user_pool_name_len(const char *name)
{
    char *name_tmp = NULL;
    unsigned int name_len;

    name_tmp = strrchr(name, '_');
    if (name_tmp == NULL) {
        pr_err("Strrchr failed.\n");
        return 0;
    }

    name_len = (unsigned int)(strlen(name) - strlen(name_tmp));
    return name_len;
}

static int ioctl_xsmem_pool_name_query(struct xsm_task *task, unsigned int cmd, unsigned long arg)
{
    struct xsm_pool_query_arg query_arg;
    struct xsm_pool *xp = NULL;
    unsigned int key_len;
    int ret = -EINVAL;

    if (copy_from_user_safe(&query_arg, (struct xsm_pool_query_arg __user *)arg, sizeof(query_arg)) != 0) {
        pr_err("copy_from_user failed\n");
        return -EFAULT;
    }

    xp = xsmem_pool_get(query_arg.pool_id);
    if (xp == NULL) {
        return -EINVAL;
    }

    key_len = xsmem_get_user_pool_name_len(xp->key);

    pr_debug("Key info. (key=%s; key_len=%u; query_key_len=%u)\n", xp->key, key_len, query_arg.key_len);
    if ((key_len > 0) && (key_len < query_arg.key_len)) {
        ret = (int)copy_to_user_safe(query_arg.key, xp->key, key_len);
    }

    xsmem_pool_put(xp);

    return ret;
}

static int ioctl_xsmem_pool_task_query(struct xsm_task *task, unsigned int cmd, unsigned long arg)
{
    struct xsm_query_pool_task_arg *usr_arg = (struct xsm_query_pool_task_arg __user *)arg;
    struct xsm_query_pool_task_arg query_arg;
    struct xsm_task_pool_node *node = NULL;
    struct xsm_pool *xp = NULL;
    int task_num = 0, ret = 0, task_pid;

    if (copy_from_user_safe(&query_arg, usr_arg, sizeof(query_arg)) != 0) {
        pr_err("copy_from_user failed\n");
        return -EFAULT;
    }

    xp = xsmem_pool_get(query_arg.pool_id);
    if (xp == NULL) {
        pr_err("invalid pool_id %d\n", query_arg.pool_id);
        return -EINVAL;
    }

    mutex_lock(&xp->mutex);

    list_for_each_entry(node, &xp->node_list_head, pool_node) {
        if (task_num < query_arg.pid_num) {
            task_pid = (int)node->task->vpid;
            ret += (int)put_user(task_pid, query_arg.pid + task_num);
            task_num++;
        }
    }

    mutex_unlock(&xp->mutex);

    xsmem_pool_put(xp);

    if (ret != 0) {
        pr_err("pool_id %d task num %d put_user failed ret %d\n", query_arg.pool_id, task_num, ret);
        return -EFAULT;
    }

    return (int)put_user(task_num, &usr_arg->pid_num);
}

static int ioctl_xsmem_pool_task_attr_query(struct xsm_task *task, unsigned int cmd, unsigned long arg)
{
    struct xsm_task_arg *usr_arg = (struct xsm_task_arg __user *)arg;
    struct xsm_adding_task *adding_task = NULL;
    struct xsm_task_pool_node *node = NULL;
    struct xsm_task_arg query_arg;
    struct xsm_pool *xp = NULL;
    int ret;

    if (copy_from_user_safe(&query_arg, usr_arg, sizeof(query_arg)) != 0) {
        pr_err("copy_from_user failed\n");
        return -EFAULT;
    }

    xp = xsmem_pool_get(query_arg.pool_id);
    if (xp == NULL) {
        pr_err("pool_id %d couldn't find the pool\n", query_arg.pool_id);
        return -ENODEV;
    }

    ret = xsmem_get_tgid_by_vpid(query_arg.pid, &query_arg.pid);
    if (ret != 0) {
        xsmem_pool_put(xp);
        return ret;
    }

    mutex_lock(&xp->mutex);
    node = xsmem_pool_task_node_find(xp, query_arg.pid);
    if (node != NULL) {
        query_arg.attr = node->attr;
        ret = 0;
    } else {
        adding_task = xsmem_pool_adding_task_find(xp, query_arg.pid);
        if (adding_task != NULL) {
            query_arg.attr = adding_task->attr;
            ret = 0;
        } else {
            ret = -EINVAL;
        }
    }

    mutex_unlock(&xp->mutex);
    xsmem_pool_put(xp);

    if (ret == 0) {
        if (copy_to_user_safe(usr_arg, &query_arg, sizeof(query_arg)) != 0) {
            pr_err("copy_to_user failed\n");
            return -EFAULT;
        }
    }

    return ret;
}

static int xsmem_task_in_pool_query(struct xsm_query_task_pool_arg *query_arg)
{
    struct xsm_task_pool_node *node = NULL;
    struct xsm_task *task = NULL;
    int pool_num = 0, ret = 0;

    mutex_lock(&task_mutex);
    task = xsm_task_find(query_arg->pid);
    if (task == NULL) {
        mutex_unlock(&task_mutex);
        return -EFAULT;
    }

    mutex_lock(&task->mutex);
    list_for_each_entry(node, &task->node_list_head, task_node) {
        if (pool_num >= query_arg->pool_num) {
            pr_info("<%s:%d,%d> task in pool query, query pid: %d, pool num: %d\n",
                current->comm, current->tgid, current->pid, query_arg->pid, query_arg->pool_num);
            break;
        }

        ret += (int)put_user(node->pool->pool_id, query_arg->pool_id + pool_num);
        pool_num++;
    }
    mutex_unlock(&task->mutex);

    mutex_unlock(&task_mutex);

    if (ret != 0) {
        pr_err("task %d pool num %d put_user failed ret %d\n", query_arg->pid, pool_num, ret);
        return -EFAULT;
    }

    query_arg->query_num = pool_num;

    return 0;
}

static int xsmem_adding_task_search(int id, void *p, void *data)
{
    struct xsm_pool *xp = p;
    struct xsm_adding_task *adding_task = NULL;
    struct xsm_query_task_pool_arg *query_arg = data;
    int ret = 0;

    if (query_arg->query_num >= query_arg->pool_num) {
        pr_info("<%s:%d,%d> adding task search, query pid: %d, query num: %d, pool num: %d\n",
            current->comm, current->tgid, current->pid, query_arg->pid, query_arg->query_num, query_arg->pool_num);
        return ret;
    }

    mutex_lock(&xp->mutex);
    adding_task = xsmem_pool_adding_task_find(xp, query_arg->pid);
    mutex_unlock(&xp->mutex);

    if (adding_task != NULL) {
        ret = (int)put_user(xp->pool_id, query_arg->pool_id + query_arg->query_num);
        query_arg->query_num++;
    }

    return ret;
}

static int xsmem_task_adding_to_pool_query(struct xsm_query_task_pool_arg *query_arg)
{
    int ret;

    query_arg->query_num = 0;

    mutex_lock(&xsmem_mutex);
    ret = xsmem_id_for_each(xsmem_adding_task_search, query_arg);
    mutex_unlock(&xsmem_mutex);

    return ret;
}

static int xsmem_adding_task_del(int id, void *p, void *data)
{
    struct xsm_adding_task *adding_task = NULL;
    struct xsm_pool *xp = p;
    int pid = *((int *)data);

    mutex_lock(&xp->mutex);
    adding_task = xsmem_pool_adding_task_find(xp, pid);
    if (adding_task != NULL) {
        xsmem_pool_adding_task_del(adding_task);
    }
    mutex_unlock(&xp->mutex);

    return 0;
}

STATIC int xsmem_task_adding_to_pool_del(int pid)
{
    int ret;

    mutex_lock(&xsmem_mutex);
    ret = xsmem_id_for_each(xsmem_adding_task_del, &pid);
    mutex_unlock(&xsmem_mutex);

    return ret;
}

static int ioctl_xsmem_task_pool_query(struct xsm_task *task, unsigned int cmd, unsigned long arg)
{
    struct xsm_query_task_pool_arg *usr_arg = (struct xsm_query_task_pool_arg __user *)arg;
    struct xsm_query_task_pool_arg query_arg;
    int ret;

    if (copy_from_user_safe(&query_arg, usr_arg, sizeof(query_arg)) != 0) {
        pr_err("copy_from_user failed\n");
        return -EFAULT;
    }

    ret = xsmem_get_tgid_by_vpid(query_arg.pid, &query_arg.pid);
    if (ret != 0) {
        return ret;
    }

    if (query_arg.type == XSMEM_TASK_IN_POOL) {
        ret = xsmem_task_in_pool_query(&query_arg);
    } else {
        ret = xsmem_task_adding_to_pool_query(&query_arg);
    }

    return (ret != 0) ? ret : (int)put_user(query_arg.query_num, &usr_arg->pool_num);
}

static int ioctl_xsmem_poll_exit_task(struct xsm_task *task, unsigned int cmd, unsigned long arg)
{
    struct xsm_poll_exit_task_arg *usr_arg = (struct xsm_poll_exit_task_arg __user *)arg;
    struct xsm_poll_exit_task_arg query_arg;
    struct xsm_task_pool_node *node = NULL;
    struct xsm_pool *xp = NULL;
    struct xsm_exit_task *exit_task = NULL;
    int ret = -1;

    if (copy_from_user_safe(&query_arg, usr_arg, sizeof(query_arg)) != 0) {
        pr_err("copy_from_user failed\n");
        return -EFAULT;
    }

    xp = xsmem_pool_get(query_arg.pool_id);
    if (xp == NULL) {
        pr_err("pool_id %d couldn't find the pool\n", query_arg.pool_id);
        return -ENODEV;
    }
    mutex_lock(&task->mutex);
    node = task_pool_node_find(task, xp);
    if (node != NULL) {
        mutex_lock(&node->mutex);
        if ((list_empty_careful(&node->exit_task_head)) == 0) {
            exit_task = list_first_entry(&node->exit_task_head, struct xsm_exit_task, node);
            pr_info("pool %d task %d poll task pid %d uid %llu exit\n",
                xp->pool_id, task->pid, exit_task->pid, exit_task->uid);
            query_arg.uid = exit_task->uid;
            ret = (int)put_user(query_arg.uid, &usr_arg->uid);
            list_del(&exit_task->node);
            kfree(exit_task);
        }
        mutex_unlock(&node->mutex);
    }
    mutex_unlock(&task->mutex);

    xsmem_pool_put(xp);

    return ret;
}

static int ioctl_xsmem_pool_va_check(struct xsm_task *task, unsigned int cmd, unsigned long arg)
{
    struct xsm_check_va_arg *usr_arg = (struct xsm_check_va_arg __user *)arg;
    struct xsm_check_va_arg query_arg;
    struct xsm_pool *xp = NULL;
    int ret;

    if (copy_from_user_safe(&query_arg, usr_arg, sizeof(query_arg)) != 0) {
        pr_err("copy_from_user failed\n");
        return -EFAULT;
    }

    xp = xsmem_pool_get(query_arg.pool_id);
    if (xp == NULL) {
        pr_err("Invalid pool id. (pool_id=%d)\n", query_arg.pool_id);
        return -EINVAL;
    }

    if (xp->algo->xsm_pool_va_check == NULL) {
        pr_err("Not support va check. (name=%s)\n", xp->algo->name);
        xsmem_pool_put(xp);
        return -EOPNOTSUPP;
    }

    ret = xp->algo->xsm_pool_va_check(xp, query_arg.va, &query_arg.result);
    if (ret != 0) {
        pr_err("Check va failed. (pool_id=%d)\n", query_arg.pool_id);
    }
    xsmem_pool_put(xp);

    return (ret != 0) ? ret : (int)put_user(query_arg.result, &usr_arg->result);
}

static int (*const xsmem_ioctl_handles[XSMEM_MAX_CMD])(struct xsm_task *task, unsigned int cmd, unsigned long arg) = {
    [_IOC_NR(XSMEM_POOL_REGISTER)] = ioctl_xsmem_pool_register,
    [_IOC_NR(XSMEM_POOL_UNREGISTER)] = ioctl_xsmem_pool_unregister,
    [_IOC_NR(XSMEM_POOL_TASK_ADD)] = ioctl_xsmem_pool_task_common,
    [_IOC_NR(XSMEM_POOL_TASK_DEL)] = ioctl_xsmem_pool_task_common,
    [_IOC_NR(XSMEM_POOL_ATTACH)] = ioctl_xsmem_pool_attach,
    [_IOC_NR(XSMEM_POOL_DETACH)] = ioctl_xsmem_pool_detach,
    [_IOC_NR(XSMEM_BLOCK_ALLOC)] = ioctl_xsmem_block_common,
    [_IOC_NR(XSMEM_BLOCK_FREE)] = ioctl_xsmem_block_common,
    [_IOC_NR(XSMEM_BLOCK_GET)] = ioctl_xsmem_block_get,
    [_IOC_NR(XSMEM_BLOCK_PUT)] = ioctl_xsmem_block_common,
    [_IOC_NR(XSMEM_POOL_ID_QUERY)] = ioctl_xsmem_pool_id_query,
    [_IOC_NR(XSMEM_POOL_NAME_QUERY)] = ioctl_xsmem_pool_name_query,
    [_IOC_NR(XSMEM_POOL_TASK_QUERY)] = ioctl_xsmem_pool_task_query,
    [_IOC_NR(XSMEM_POOL_TASK_ATTR_QUERY)] = ioctl_xsmem_pool_task_attr_query,
    [_IOC_NR(XSMEM_TASK_POOL_QUERY)] = ioctl_xsmem_task_pool_query,
    [_IOC_NR(XSMEM_PROP_OP)] = ioctl_xsmem_pool_prop_op,
    [_IOC_NR(XSMEM_POLL_EXIT_TASK)] = ioctl_xsmem_poll_exit_task,
    [_IOC_NR(XSMEM_CACHE_CREATE)] = ioctl_xsmem_cache_create,
    [_IOC_NR(XSMEM_CACHE_DESTROY)] = ioctl_xsmem_cache_destroy,
    [_IOC_NR(XSMEM_CACHE_QUERY)] = ioctl_xsmem_cache_query,
    [_IOC_NR(XSMEM_POOL_FLAG_QUERY)] = ioctl_xsmem_priv_flag_query,
    [_IOC_NR(XSMEM_VADDR_CHECK)] = ioctl_xsmem_pool_va_check,
};

STATIC long xsmem_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct xsm_task *task = file->private_data;
    int cmd_nr = _IOC_NR(cmd);
    if ((cmd_nr < 0) || (cmd_nr >= XSMEM_MAX_CMD) || (xsmem_ioctl_handles[cmd_nr] == NULL)) {
        pr_err("unsupported command %x\n", cmd);
        return -EINVAL;
    }
    if (task == NULL) {
#ifndef EMU_ST
        pr_err("The filep private_data is NULL.\n");
        return -ESRCH;
#endif
    }

    return xsmem_ioctl_handles[cmd_nr](task, cmd, arg);
}

#ifndef EMU_ST
static pid_t xsmem_get_vpid(struct task_struct *tsk)
{
    return task_pid_vnr(tsk);
}
#endif
STATIC int xsmem_open(struct inode *inode, struct file *file)
{
    struct xsm_task *task = NULL;
    mutex_lock(&task_mutex);

    task = xsm_task_find((int)current->tgid);
    if (task != NULL) {
        mutex_unlock(&task_mutex);
        pr_err("pid %d has been opened\n", task->pid);
        return -ENOMEM;
    }

    task = kmalloc(sizeof(*task), GFP_KERNEL | __GFP_ACCOUNT);
    if (unlikely(task == NULL)) {
        mutex_unlock(&task_mutex);
        pr_err("kmalloc failed\n");
        return -ENOMEM;
    }

    file->private_data = task;

    /* mmget in open avoid share pool group exit in mm_exit before fput release */
    task->mm = current->mm;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
    mmget(task->mm);
#else
    atomic_inc(&task->mm->mm_users);
#endif

    task->pid = current->tgid;
    task->vpid = xsmem_get_vpid(current);
    task->attached_pool_count = 0;
    task->uid = ++g_uni_process_id;
    atomic_set(&task->pool_num, 0);
    INIT_LIST_HEAD(&task->node_list_head);
    INIT_LIST_HEAD(&task->register_xp_list_head);

    mutex_init(&task->mutex);
    xsm_task_add(task);
    mutex_unlock(&task_mutex);

    proc_fs_add_task(task);

    pr_info("Xsmem open. (task_name=%s; tgid=%d; pid=%d; task_pid=%d; task_vpid=%d)\n",
        current->comm, current->tgid, current->pid, task->pid, task->vpid);

    return 0;
}

STATIC int xsmem_release(struct inode *inode, struct file *file)
{
    struct xsm_task_pool_node *node = NULL, *node_tmp = NULL;
    struct xsm_pool *xp = NULL, *xp_tmp = NULL;
    struct xsm_task *task = file->private_data;

    if (task == NULL) {
#ifndef EMU_ST
        pr_err("The filep private_data is NULL.\n");
        return -ESRCH;
#endif
    }

    pr_info("<%s:%d,%d> task %d release\n",
        current->comm, current->tgid, current->pid, task->pid);

    mutex_lock(&task->mutex);
    list_for_each_entry_safe(node, node_tmp, &task->node_list_head, task_node) {
        xp = node->pool;
        pr_info("pid %d pool_id %d task_id %d release, refcnt %d\n",
            task->pid, xp->pool_id, node->task_id, atomic_read(&xp->refcnt));

        xsmem_pool_notice_other_task(xp, node);
        xsmem_pool_task_detach(node);
    }

    list_for_each_entry_safe(xp, xp_tmp, &task->register_xp_list_head, register_task_list) {
        list_del(&xp->register_task_list);
        xsmem_task_unregister_pool(task, xp);
    }

    mutex_unlock(&task->mutex);

    proc_fs_del_task(task);
    xsmem_task_prop_del(task->pid);
    mmput(task->mm);

    mutex_lock(&task_mutex);
    xsm_task_del(task);
    mutex_unlock(&task_mutex);

    kfree(task);

    return 0;
}

static struct file_operations xsmem_fops = {
    .owner          = THIS_MODULE,
    .open           = xsmem_open,
    .release        = xsmem_release,
    .unlocked_ioctl = xsmem_ioctl,
};

static int task_exit_notify(struct notifier_block *self, unsigned long val, void *data)
{
    struct task_struct *task_proc = NULL;

    task_proc = (struct task_struct*)data;
    if (task_proc == NULL) {
        return 0;
    }
    if (task_proc->tgid == task_proc->pid) {
        (void)xsmem_task_adding_to_pool_del(task_proc->tgid);
    }
    return 0;
}


static struct notifier_block task_exit_nb = {
    .notifier_call = task_exit_notify,
};

static void xsmem_task_exit_reg(void)
{
    int ret;
    ret = profile_event_register(PROFILE_TASK_EXIT, &task_exit_nb);
    pr_info("task exit reg, %d\n", ret);
}

static void xsmem_task_exit_unreg(void)
{
    int ret;
    ret = profile_event_unregister(PROFILE_TASK_EXIT, &task_exit_nb);
    pr_info("task exit unreg, %d\n", ret);
}
#ifndef CFG_FEATURE_EXTERNAL_CDEV
static struct miscdevice xsmem_dev;
static int xsmem_dev_reg(void)
{
    xsmem_dev.minor = MISC_DYNAMIC_MINOR;
    xsmem_dev.name  = XSMEM_DEVICE_NAME;
    xsmem_dev.fops  = &xsmem_fops;
    return misc_register(&xsmem_dev);
}

static void xsmem_dev_unreg(void)
{
    xsmem_task_exit_unreg();
    misc_deregister(&xsmem_dev);
}
#else
static int xsmem_notifier_release(struct file *file, unsigned long mode)
{
    return xsmem_release(NULL, file);
}

static const struct notifier_operations xsmem_notifier_ops = {
    .notifier_call =  xsmem_notifier_release,
};

static int xsmem_dev_reg(void)
{
    int ret;

    /* davinci_recycle can't call mg_sp_free, because davinci_recycle is kernel thread, which is not in the sp group */
    xsmem_fops.release = NULL;
    ret = drv_davinci_register_sub_module(DAVINCI_XSMEM_SUB_MODULE_NAME, &xsmem_fops);
    if (ret != 0) {
        pr_err("Register sub module fail. (ret=%d)\n", ret);
        return ret;
    }

    ret = drv_ascend_register_notify(DAVINCI_XSMEM_SUB_MODULE_NAME, &xsmem_notifier_ops);
    if (ret != 0) {
        (void)drv_ascend_unregister_sub_module(DAVINCI_XSMEM_SUB_MODULE_NAME);
        pr_err("Register notify fail. (ret=%d)\n", ret);
        return ret;
    }
    return ret;
}

static void xsmem_dev_unreg(void)
{
    xsmem_task_exit_unreg();
    (void)drv_ascend_unregister_sub_module(DAVINCI_XSMEM_SUB_MODULE_NAME);
}
#endif

static void xsmem_algo_register(void)
{
#ifdef CFG_FEATURE_SUPPORT_VMA
    xsmem_register_algo(xsm_get_vma_algo());
#endif
#ifdef CFG_FEATURE_SUPPORT_SP
    xsmem_register_algo(xsm_get_sp_algo());
#endif

#ifdef CFG_FEATURE_SUPPORT_CACHE
#ifdef CFG_FEATURE_SUPPORT_VMA
    xsmem_register_algo(xsm_get_cache_vma_algo());
#endif
#ifdef CFG_FEATURE_SUPPORT_SP
    xsmem_register_algo(xsm_get_cache_sp_algo());
    /* vma algo for cache memory assignment */
    xsmem_register_algo(xsm_get_vma_algo());
#endif
#endif
}

STATIC int __init xsmem_init(void)
{
    int ret;

    ret = xsmem_dev_reg();
    if (ret != 0) {
        pr_err("misc_register failed, %d\n", ret);
        return ret;
    }
    xsmem_algo_register();

    xsmem_id_switch_init();
    xsmem_proc_fs_init();
    xsmem_task_exit_reg(); // must after id init

    pr_info("Module init success.\n");

    return 0;
};
module_init(xsmem_init);

STATIC void __exit xsmem_exit(void)
{
    xsmem_proc_fs_uninit();
    xsmem_dev_unreg();
    pr_info("Module exit.\n");
}
module_exit(xsmem_exit);

MODULE_LICENSE("GPL v2");
