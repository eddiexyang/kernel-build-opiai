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
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/fs.h>
#include "securec.h"

#include "xsmem_framework.h"
#include "xsmem_prop.h"

DEFINE_MUTEX(prop_list_mutex);
static LIST_HEAD(prop_list_head);

#define PROP_NAME_MAX_LEN 128
struct xsm_prop {
    char name[PROP_NAME_MAX_LEN];
    unsigned int prop_len;
    int owner;
    int create_pid;
    int refcnt;
    unsigned long value;
    struct list_head    pool_node;
};

struct xsm_task_prop_node {
    struct list_head    task_node; /* list node in TASK pool node */
    struct xsm_prop *prop;
};

static struct xsm_task_prop_node *xsmem_task_prop_node_get(struct xsm_task_pool_node *node, const struct xsm_prop *prop)
{
    struct xsm_task_prop_node *prop_node = NULL;

    list_for_each_entry(prop_node, &node->task_prop_head, task_node) {
        if (prop_node->prop == prop) {
            return prop_node;
        }
    }
    return NULL;
}

static int xsmem_add_prop_to_task(struct xsm_task_pool_node *node, struct xsm_prop *prop)
{
    struct xsm_task_prop_node *prop_node = NULL;

    if (xsmem_task_prop_node_get(node, prop) != NULL) {
        return 0;
    }

    prop_node = kmalloc(sizeof(*prop_node), GFP_KERNEL | __GFP_ACCOUNT);
    if (prop_node == NULL) {
        pr_err("alloc prop node mem %lx failed\n", sizeof(*prop_node));
        return -ENOMEM;
    }

    prop->refcnt++;
    prop_node->prop = prop;
    list_add_tail(&prop_node->task_node, &node->task_prop_head);

    return 0;
}

static struct xsm_prop *xsmem_pool_prop_alloc(struct xsm_task *task, struct xsm_task_pool_node *node,
    struct xsm_prop_op_arg *prop_op, char *key)
{
    struct xsm_prop *prop = NULL;
    int ret;

    prop = kmalloc(sizeof(*prop), GFP_KERNEL | __GFP_ACCOUNT);
    if (prop == NULL) {
        pr_err("alloc prop mem %lx failed\n", sizeof(*prop));
        return ERR_PTR(-ENOMEM);
    }

    ret = strcpy_s(prop->name, PROP_NAME_MAX_LEN, key);
    if (ret != 0) {
        kfree(prop);
        pr_err("Pool add prop strcpy_s failed. (pool=%d; prop=%s; ret=%d)\n", prop_op->pool_id, key, ret);
        return ERR_PTR(-EFAULT);
    }

    prop->prop_len = prop_op->prop_len;
    prop->owner = prop_op->owner;
    prop->value = prop_op->value;
    prop->create_pid = task->pid;
    prop->refcnt = 0;

    if (prop->owner == XSMEM_PROP_OWNER_TASK_GRP) {
        ret = xsmem_add_prop_to_task(node, prop);
        if (ret != 0) {
            kfree(prop);
            return ERR_PTR(ret);
        }
    }

    pr_debug("Alloc. (task=%d; pool=%d; add prop=%s; owner=%d)\n",
        current->tgid, prop_op->pool_id, key, prop->owner);

    return prop;
}

static void xsmem_pool_prop_free(struct xsm_prop *prop)
{
    pr_debug("Free. (task=%d; create_pid=%d; del prop=%s)\n",
        current->tgid, prop->create_pid, prop->name);
    list_del(&prop->pool_node);
    kfree(prop);
}

static int xsmem_pool_prop_del(const struct xsm_task *task, struct xsm_task_pool_node *node, struct xsm_prop *prop)
{
    struct xsm_task_prop_node *prop_node = NULL;

    pr_debug("Delete. (task=%d; create_pid=%d; del prop=%s; owner=%d)\n",
        current->tgid, prop->create_pid, prop->name, prop->owner);
    if (prop->owner != XSMEM_PROP_OWNER_TASK_GRP) {
        if (prop->create_pid != task->pid) {
            return -EFAULT;
        }
        xsmem_pool_prop_free(prop);
    } else {
        prop_node = xsmem_task_prop_node_get(node, prop);
        if (prop_node == NULL) {
            return -EFAULT;
        }

        list_del(&prop_node->task_node);
        kfree(prop_node);
        prop->refcnt--;
        if (prop->refcnt <= 0) {
            xsmem_pool_prop_free(prop);
        }
    }
    return 0;
}

static int xsmem_pool_prop_op(struct xsm_task *task, struct xsm_task_pool_node *node,
    struct xsm_prop_op_arg *prop_op, struct xsm_prop_op_arg __user *arg)
{
    struct list_head *head = (node == NULL) ? &prop_list_head : &node->pool->prop_list_head;
    struct xsm_prop *prop = NULL, *tmp = NULL;
    char key[PROP_NAME_MAX_LEN];

    if (copy_from_user_safe(key, (char __user *)prop_op->prop, prop_op->prop_len) != 0) {
        pr_err("copy key from user failed, key_len %d\n", prop_op->prop_len);
        return -EFAULT;
    }
    key[prop_op->prop_len] = '\0';

    list_for_each_entry(tmp, head, pool_node) {
        if ((prop_op->prop_len == tmp->prop_len) && (strncmp(key, tmp->name, tmp->prop_len) == 0)) {
            prop = tmp;
            break;
        }
    }

    if (prop_op->op == XSMEM_PROP_OP_SET) {
        if (prop != NULL) {
            return -EEXIST;
        }

        prop = xsmem_pool_prop_alloc(task, node, prop_op, key);
        if (IS_ERR(prop)) {
            return (int)PTR_ERR(prop);
        }

        list_add_tail(&prop->pool_node, head);
        return 0;
    } else if (prop_op->op == XSMEM_PROP_OP_GET) {
        if (prop == NULL) {
            return -ENOENT;
        }

        if (prop->owner == XSMEM_PROP_OWNER_TASK_GRP) {
            int ret = xsmem_add_prop_to_task(node, prop);
            if (ret != 0) {
                return ret;
            }
        }
        return (int)put_user(prop->value, &arg->value);
    } else {
        if (prop == NULL) {
            return -ENOENT;
        }
        return xsmem_pool_prop_del(task, node, prop);
    }
}

void xsmem_proc_grp_prop_del(struct xsm_task_pool_node *node)
{
    struct xsm_task_prop_node *prop_node = NULL, *tmp = NULL;
    struct xsm_prop *prop = NULL;

    mutex_lock(&prop_list_mutex);
    list_for_each_entry_safe(prop_node, tmp, &node->task_prop_head, task_node) {
        prop = prop_node->prop;
        list_del(&prop_node->task_node);
        kfree(prop_node);

        prop->refcnt--;
        if (prop->refcnt <= 0) {
            xsmem_pool_prop_free(prop);
        }
    }
    mutex_unlock(&prop_list_mutex);
}

void xsmem_task_prop_del(int pid)
{
    struct xsm_prop *prop = NULL, *tmp = NULL;

    mutex_lock(&prop_list_mutex);
    list_for_each_entry_safe(prop, tmp, &prop_list_head, pool_node) {
        if ((prop->create_pid == pid) && (prop->owner == XSMEM_PROP_OWNER_TASK)) {
            xsmem_pool_prop_free(prop);
        }
    }
    mutex_unlock(&prop_list_mutex);
}

void xsmem_pool_prop_clear(struct xsm_pool *xp)
{
    struct xsm_prop *prop = NULL, *tmp = NULL;

    mutex_lock(&prop_list_mutex);
    list_for_each_entry_safe(prop, tmp, &xp->prop_list_head, pool_node) {
        xsmem_pool_prop_free(prop);
    }
    mutex_unlock(&prop_list_mutex);
}

void xsmem_pool_task_prop_clear(struct xsm_pool *xp, const struct xsm_task *task)
{
    struct xsm_prop *prop = NULL, *tmp = NULL;

    mutex_lock(&prop_list_mutex);
    list_for_each_entry_safe(prop, tmp, &xp->prop_list_head, pool_node) {
        if ((prop->create_pid == task->pid) && (prop->owner == XSMEM_PROP_OWNER_TASK)) {
            xsmem_pool_prop_free(prop);
        }
    }
    mutex_unlock(&prop_list_mutex);
}

int ioctl_xsmem_pool_prop_op(struct xsm_task *task, unsigned int cmd, unsigned long arg)
{
    struct xsm_task_pool_node *node = NULL;
    struct xsm_prop_op_arg prop_op;
    struct xsm_pool *xp = NULL;
    int pool_id, ret;

    if (copy_from_user_safe(&prop_op, (struct xsm_prop_op_arg __user *)arg, sizeof(prop_op)) != 0) {
        pr_err("copy_from_user failed\n");
        return -EFAULT;
    }

    if ((prop_op.op == XSMEM_PROP_OP_SET) &&
        (prop_op.owner != XSMEM_PROP_OWNER_TASK) && (prop_op.owner != XSMEM_PROP_OWNER_TASK_GRP)) {
        pr_err("Owner is invalid. (owner=%d)\n", prop_op.owner);
        return -EINVAL;
    }

    if ((prop_op.prop == NULL) || (prop_op.prop_len == 0) || (prop_op.prop_len >= PROP_NAME_MAX_LEN)) {
        pr_err("Prop or prop_len is invalid. (prop_len=%u)\n", prop_op.prop_len);
        return -EINVAL;
    }

    pool_id = prop_op.pool_id;

    mutex_lock(&task->mutex);
    if (pool_id >= 0) {
        xp = xsmem_pool_get(pool_id);
        if (xp == NULL) {
            mutex_unlock(&task->mutex);
            pr_err("pool_id %d couldn't find the pool\n", pool_id);
            return -ENODEV;
        }

        node = task_pool_node_find(task, xp);
        if (node == NULL) {
            mutex_unlock(&task->mutex);
            xsmem_pool_put(xp);
            pr_err("Task is not in pool. (pid=%d; pool_id=%d)\n", task->pid, pool_id);
            return -EINVAL;
        }
    } else {
        if ((prop_op.op == XSMEM_PROP_OP_SET) && (prop_op.owner == XSMEM_PROP_OWNER_TASK_GRP)) {
            mutex_unlock(&task->mutex);
            pr_err("Prop owner is invalid, can not set pool id.\n");
            return -EINVAL;
        }
    }

    mutex_lock(&prop_list_mutex);
    ret = xsmem_pool_prop_op(task, node, &prop_op, (struct xsm_prop_op_arg __user *)arg);
    mutex_unlock(&prop_list_mutex);

    mutex_unlock(&task->mutex);

    if (pool_id >= 0) {
        xsmem_pool_put(xp);
    }

    return ret;
}
