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

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/mutex.h>
#include <linux/nsproxy.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
#include <linux/sched/task.h>
#endif
#include <linux/kref.h>

#include "securec.h"

#include "xsmem_cmd.h"
#include "xsmem_framework.h"
#include "xsmem_ns_adapt.h"

#define NS_MAX_SP_NUM            512U

static LIST_HEAD(ns_pool_list);
DEFINE_MUTEX(ns_mutex);

typedef enum ns_pool_num_ctrl_type {
    SP_NUM_CTRL_TYPE = 0,
    VMA_NUM_CTRL_TYPE,
    POOL_NUM_CTRL_TYPE_MAX,
} ns_pnum_ctrltype;

struct xsm_ns_algo_ctrl {
    u32 max_pool_num;
};

struct xsm_ns_mng {
    unsigned long mnt_ns;
    u32 pool_num[POOL_NUM_CTRL_TYPE_MAX];
    struct list_head ns_pool_node;
    spinlock_t lock;
    struct kref ref;
    DECLARE_BITMAP(xsmem_blockid, XSMEM_BLOCK_MAX_NUM);
};

static const struct xsm_ns_algo_ctrl g_ns_algo_ctrl[POOL_NUM_CTRL_TYPE_MAX] = {
    [SP_NUM_CTRL_TYPE] = {
        .max_pool_num = NS_MAX_SP_NUM /* For docker, we should check pool num */
    },
    [VMA_NUM_CTRL_TYPE] = {
        .max_pool_num = UINT_MAX
    },
};

/*
 * Check the process is running in the physical machine or in container
 */
static bool xsmem_is_host_system(unsigned long mnt_ns)
{
#ifndef EMU_ST
    if (mnt_ns == (unsigned long)init_task.nsproxy->mnt_ns) {
        return true;
    }
#endif
    return false;
}

static const int g_xsmem_ctrl_type[XSMEM_ALGO_MAX] = {
    [XSMEM_ALGO_VMA] = VMA_NUM_CTRL_TYPE,
    [XSMEM_ALGO_SP] = SP_NUM_CTRL_TYPE,
    [XSMEM_ALGO_CACHE_VMA] = VMA_NUM_CTRL_TYPE,
    [XSMEM_ALGO_CACHE_SP] = SP_NUM_CTRL_TYPE
};

static inline int xsmem_algo_to_ctrl(int algo)
{
    return g_xsmem_ctrl_type[algo];
}

static inline bool xsmem_ns_algo_is_valid(int algo)
{
    return (algo >= XSMEM_ALGO_VMA) && (algo < XSMEM_ALGO_MAX);
}

static inline void _xsmem_ns_mng_add(struct xsm_ns_mng *ns_mng)
{
    list_add_tail(&ns_mng->ns_pool_node, &ns_pool_list);
}

static inline void _xsmem_ns_mng_del(struct xsm_ns_mng *ns_mng)
{
    list_del(&ns_mng->ns_pool_node);
}

static struct xsm_ns_mng *_xsmem_ns_mng_create(unsigned long mnt_ns)
{
    struct xsm_ns_mng *ns_mng = kvzalloc(sizeof(struct xsm_ns_mng), GFP_KERNEL | __GFP_ACCOUNT);
    if (ns_mng != NULL) {
        ns_mng->mnt_ns = mnt_ns;
        _xsmem_ns_mng_add(ns_mng);
        kref_init(&ns_mng->ref);
        spin_lock_init(&ns_mng->lock);
    }
    return ns_mng;
}

static inline void _xsmem_ns_mng_destroy(struct xsm_ns_mng *ns_mng)
{
    _xsmem_ns_mng_del(ns_mng);
    kvfree(ns_mng);
}

static struct xsm_ns_mng *_xsmem_ns_mng_find(unsigned long mnt_ns)
{
    struct xsm_ns_mng *ns_mng = NULL;

    list_for_each_entry(ns_mng, &ns_pool_list, ns_pool_node) {
        if (ns_mng->mnt_ns == mnt_ns) {
            return ns_mng;
        }
    }
    return NULL;
}

static struct xsm_ns_mng *xsmem_ns_mng_get(unsigned long mnt_ns)
{
    struct xsm_ns_mng *ns_mng = NULL;

    mutex_lock(&ns_mutex);
    ns_mng = _xsmem_ns_mng_find(mnt_ns);
    if (ns_mng == NULL) {
        ns_mng = _xsmem_ns_mng_create(mnt_ns);
    }
    if (ns_mng != NULL) {
        if (kref_get_unless_zero(&ns_mng->ref) == 0) {
            mutex_unlock(&ns_mutex);
            return NULL;
        }
    }
    mutex_unlock(&ns_mutex);
    return ns_mng;
}

static void xsmem_ns_mng_release(struct kref *kref)
{
    struct xsm_ns_mng *ns_mng = container_of(kref, struct xsm_ns_mng, ref);
    mutex_lock(&ns_mutex);
    _xsmem_ns_mng_destroy(ns_mng);
    mutex_unlock(&ns_mutex);
}

static inline void xsmem_ns_mng_put(struct xsm_ns_mng *ns_mng)
{
    kref_put(&ns_mng->ref, xsmem_ns_mng_release);
}

static int _xsmem_ns_pool_num_inc(struct xsm_ns_mng *ns_mng, int algo)
{
    int ctrl_type = xsmem_algo_to_ctrl(algo);
    u32 max_pool_num;

    spin_lock(&ns_mng->lock);
    max_pool_num = xsmem_is_host_system(ns_mng->mnt_ns) ? UINT_MAX : g_ns_algo_ctrl[ctrl_type].max_pool_num;
    if (ns_mng->pool_num[ctrl_type] >= max_pool_num) {
        spin_unlock(&ns_mng->lock);
        return -EPERM;
    }
    ns_mng->pool_num[ctrl_type]++;
    spin_unlock(&ns_mng->lock);
    return 0;
}

int xsmem_ns_pool_num_inc(unsigned long mnt_ns, int algo)
{
    struct xsm_ns_mng *ns_mng = NULL;
    int ret = -ENOMEM;

    if (!xsmem_ns_algo_is_valid(algo)) {
        pr_err("Invalid algo. (algo=%d)\n", algo);
        return -EINVAL;
    }

    ns_mng = xsmem_ns_mng_get(mnt_ns);
    if (ns_mng != NULL) {
        ret = _xsmem_ns_pool_num_inc(ns_mng, algo);
        xsmem_ns_mng_put(ns_mng);
    }

    return ret;
}

static void _xsmem_ns_pool_num_dec(struct xsm_ns_mng *ns_mng, int algo)
{
    int ctrl_type = xsmem_algo_to_ctrl(algo);
    int i;

    spin_lock(&ns_mng->lock);
    if (ns_mng->pool_num[ctrl_type] > 0) {
        ns_mng->pool_num[ctrl_type]--;
    }
    for (i = SP_NUM_CTRL_TYPE; i < POOL_NUM_CTRL_TYPE_MAX; i++) {
        if (ns_mng->pool_num[i] != 0) {
            spin_unlock(&ns_mng->lock);
            return;
        }
    }
    spin_unlock(&ns_mng->lock);
    /* All ctrl type pool num is 0, put the xsmem_ns_mng and ready for destroy */
    xsmem_ns_mng_put(ns_mng);
}

void xsmem_ns_pool_num_dec(unsigned long mnt_ns, int algo)
{
    struct xsm_ns_mng *ns_mng = NULL;

    if (!xsmem_ns_algo_is_valid(algo)) {
        pr_err("Invalid algo. (algo=%d)\n", algo);
        return;
    }
    ns_mng = xsmem_ns_mng_get(mnt_ns);
    if (ns_mng != NULL) {
        _xsmem_ns_pool_num_dec(ns_mng, algo);
        xsmem_ns_mng_put(ns_mng);
    }
}

#ifndef EMU_ST
int xsmem_get_tgid_by_vpid(pid_t vpid, pid_t *tgid)
{
    struct task_struct *task = NULL;
    struct pid *pid = NULL;

    pid = find_get_pid(vpid);
    if (pid == NULL) {
        pr_err("Find_get_pid failed. (pid=%d)\n", vpid);
        return -EINVAL;
    }

    task = get_pid_task(pid, PIDTYPE_PID);
    if (task == NULL) {
        pr_err("Process is not exist. (pid=%d)\n", vpid);
        put_pid(pid);
        return -ESRCH;
    }

    *tgid = task->tgid;
    put_task_struct(task);
    put_pid(pid);

    pr_debug("Xsmem_get_tgid_by_vpid. (vpid=%d; tgid=%d)\n", vpid, *tgid);
    return 0;
}
#endif

int xsmem_strcat_with_ns(char *str_dest, unsigned int dest_max, const char *str_src)
{
    return sprintf_s(str_dest, dest_max, "%s_%lu", str_src, (unsigned long)current->nsproxy->mnt_ns);
}

static int xsmem_ns_blkid_alloc(struct xsm_ns_mng *ns_mng)
{
    u32 id;

    for (id = 0; id < XSMEM_BLOCK_MAX_NUM; id++) {
        id = (u32)find_next_zero_bit(ns_mng->xsmem_blockid, XSMEM_BLOCK_MAX_NUM, id);
        if (id >= XSMEM_BLOCK_MAX_NUM) {
            break;
        }
        if (!(bool)test_and_set_bit(id, ns_mng->xsmem_blockid)) {
            return (int)id;
        }
    }
    return -ENOSPC;
}

static void xsmem_ns_blkid_free(struct xsm_ns_mng *ns_mng, int id)
{
    clear_bit((u32)id, ns_mng->xsmem_blockid);
}

int xsmem_blockid_get(unsigned long mnt_ns)
{
    struct xsm_ns_mng *ns_mng = NULL;
    int id;

    ns_mng = xsmem_ns_mng_get(mnt_ns);
    if (unlikely(ns_mng == NULL)) {
        pr_err("Get ns mng fail. (mnt_ns=%lu)\n", mnt_ns);
        return -ENOMEM;
    }
    id = xsmem_ns_blkid_alloc(ns_mng);
    xsmem_ns_mng_put(ns_mng);
    return id;
}

void xsmem_blockid_put(unsigned long mnt_ns, int id)
{
    if (likely((id >= 0) && (id < XSMEM_BLOCK_MAX_NUM))) {
        struct xsm_ns_mng *ns_mng = xsmem_ns_mng_get(mnt_ns);
        if (likely(ns_mng != NULL)) {
            xsmem_ns_blkid_free(ns_mng, id);
            xsmem_ns_mng_put(ns_mng);
        }
    }
}

