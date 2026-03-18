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
* Create: 2022-10-15
*/
#include <linux/idr.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/rbtree.h>
#include <linux/err.h>

#include "id_pool.h"

#define MAX_ID_POOL_DEVID   4U
#define MAX_ID_POOL_SUB_ID  2U
#define MAX_ID_POOL_TYPE    TRS_ID_TYPE_MAX
#define MAX_ID_POOL_NUM     32U

struct id_pool_node {
    struct idr idr;
    struct rb_node node;

    bool node_tunable;   /* flag that can be moved to other type id pool. 0: cant move, 1: can move */
    u32 id_start;
    u32 id_avail_num;   /* Available id number */
    u32 id_total_num;
};

struct id_pool {
    u32 phy_devid;
    u32 sub_id;
    int type;

    struct rb_root root;
    struct mutex mutex;

    u16 pool_num;
    u32 id_total_num;
};

static struct mutex g_id_pool_lock;
static struct id_pool *g_id_pool[MAX_ID_POOL_DEVID][MAX_ID_POOL_SUB_ID][MAX_ID_POOL_TYPE];

size_t id_pool_get_size(void)
{
    return sizeof(struct id_pool);
}

size_t id_pool_node_get_size(void)
{
    return sizeof(struct id_pool_node);
}

static void id_pool_show(struct id_pool *pool)
{
    struct rb_node *node = NULL;
    int idx = 0;

    trs_info("Pool info. (phy_devid=%u; sub_id=%u; type=%d; pool_num=%u; id_total_num=%u)\n",
        pool->phy_devid, pool->sub_id, pool->type, pool->pool_num, pool->id_total_num);
    for (node = rb_first(&pool->root); node != NULL; node = rb_next(node)) {
        struct id_pool_node *pool_node = rb_entry(node, struct id_pool_node, node);

        trs_info("    (%d):id_start=%u; id_total_num=%u; id_avail_num=%u\n",
            idx++, pool_node->id_start, pool_node->id_total_num, pool_node->id_avail_num);
    }
    if (idx == 0) {
        trs_info("    None\n");
    }
}

static struct id_pool *id_pool_create(u32 phy_devid, u32 sub_id, int type)
{
    struct id_pool *pool = kzalloc(id_pool_get_size(), GFP_KERNEL | __GFP_ACCOUNT);

    if (pool == NULL) {
        return NULL;
    }
    pool->phy_devid = phy_devid;
    pool->sub_id = sub_id;
    pool->type = type;
    pool->root = RB_ROOT;
    pool->pool_num = 0;
    mutex_init(&pool->mutex);
    return pool;
}

static void id_pool_destroy(struct id_pool *pool)
{
    if (pool != NULL) {
        mutex_destroy(&pool->mutex);
        kfree(pool);
    }
}

static int id_pool_node_add(struct id_pool *pool, struct id_pool_node *pool_node)
{
    u32 id_total_num = pool->id_total_num;
    struct rb_node *parent = NULL;
    struct rb_node **node = NULL;

    if (pool->pool_num >= MAX_ID_POOL_NUM) {
        return -EBUSY;
    }

    id_total_num += pool_node->id_total_num;
    if (id_total_num < pool->id_total_num) {
        return -EFAULT;
    }

    node = &pool->root.rb_node;
    while (*node != NULL) {
        struct id_pool_node *entry = rb_entry(*node, struct id_pool_node, node);

        parent = *node;
        if ((pool_node->id_start + pool_node->id_total_num) <= entry->id_start) {
            node = &parent->rb_left;
        } else if (pool_node->id_start >= (entry->id_start + entry->id_total_num)) {
            node = &parent->rb_right;
        } else {
            return -ENODEV;
        }
    }
    rb_link_node(&pool_node->node, parent, node);
    rb_insert_color(&pool_node->node, &pool->root);
    pool->pool_num++;
    pool->id_total_num += pool_node->id_total_num;
    return 0;
}

static void id_pool_node_del(struct id_pool *pool, struct id_pool_node *pool_node)
{
    rb_erase(&pool_node->node, &pool->root);
    RB_CLEAR_NODE(&pool_node->node);
    pool->pool_num--;
    pool->id_total_num -= pool_node->id_total_num;
}

static void id_pool_node_destroy(struct id_pool_node *pool_node)
{
    u32 id;

    for (id = pool_node->id_start; id < (pool_node->id_start + pool_node->id_total_num); id++) {
        if (pool_node == (struct id_pool_node *)idr_find(&pool_node->idr, (unsigned long)id)) {
            idr_remove(&pool_node->idr, (unsigned long)id);
            pool_node->id_avail_num++;
        }
    }
    idr_destroy(&pool_node->idr);
    kfree(pool_node);
}

static void id_pool_node_destroy_all(struct id_pool *pool)
{
    if (pool != NULL) {
        struct rb_node *node = NULL;

        mutex_lock(&pool->mutex);
        while ((node = rb_first(&pool->root)) != NULL) {
            struct id_pool_node *pool_node = rb_entry(node, struct id_pool_node, node);
            id_pool_node_del(pool, pool_node);
            id_pool_node_destroy(pool_node);
        }
        mutex_unlock(&pool->mutex);
    }
}

static struct id_pool *id_pool_init(struct id_pool_inst *inst, int type)
{
    struct id_pool *pool = NULL;

    mutex_lock(&g_id_pool_lock);
    pool = g_id_pool[inst->phy_devid][inst->sub_id][type];
    if (pool == NULL) {
        pool = id_pool_create(inst->phy_devid, inst->sub_id, type);
        g_id_pool[inst->phy_devid][inst->sub_id][type] = pool;
    }
    mutex_unlock(&g_id_pool_lock);

    return pool;
}

static void id_pool_uninit(struct id_pool_inst *inst, int type)
{
    struct id_pool *pool = NULL;

    mutex_lock(&g_id_pool_lock);
    pool = g_id_pool[inst->phy_devid][inst->sub_id][type];
    g_id_pool[inst->phy_devid][inst->sub_id][type] = NULL;
    mutex_unlock(&g_id_pool_lock);

    id_pool_node_destroy_all(pool);
    id_pool_destroy(pool);
}

static struct id_pool *id_pool_get(struct id_pool_inst *inst, int type)
{
    struct id_pool *pool = NULL;

    mutex_lock(&g_id_pool_lock);
    pool = g_id_pool[inst->phy_devid][inst->sub_id][type];
    mutex_unlock(&g_id_pool_lock);
    return pool;
}

static struct id_pool_node *id_pool_node_create(struct id_pool_inst *inst, struct id_pool_attr *attr)
{
    struct id_pool_node *pool_node = kzalloc(id_pool_node_get_size(), GFP_KERNEL);

    if (pool_node == NULL) {
        return NULL;
    }
    pool_node->node_tunable = attr->node_tunable;
    pool_node->id_start = attr->id_start;
    pool_node->id_avail_num = attr->id_total_num;
    pool_node->id_total_num = attr->id_total_num;
    idr_init_base(&pool_node->idr, attr->id_start);
    return pool_node;
}

static struct id_pool_node *id_pool_node_find_by_id(struct id_pool *pool, u32 id)
{
    struct rb_node *node = pool->root.rb_node;

    while (node != NULL) {
        struct id_pool_node *pool_node = rb_entry(node, struct id_pool_node, node);

        if (id < pool_node->id_start) {
            node = pool_node->node.rb_left;
        } else if (id >= (pool_node->id_start + pool_node->id_total_num)) {
            node = pool_node->node.rb_right;
        } else {
            return pool_node;
        }
    }
    return NULL;
}

static struct id_pool_node *id_pool_node_find_by_range(struct id_pool *pool, u32 start, u32 total_num)
{
    struct id_pool_node *pool_node = NULL;
    struct rb_node *node = NULL;

    for (node = rb_first(&pool->root); node != NULL; node = rb_next(node)) {
        pool_node = rb_entry(node, struct id_pool_node, node);
        if ((pool_node->id_start == start) && (pool_node->id_total_num == total_num)) {
            return pool_node;
        }
    }
    return NULL;
}

static int id_pool_attr_check(struct id_pool_attr *attr)
{
    if (attr == NULL) {
        return -EINVAL;
    }

    if ((attr->id_total_num == 0) || ((attr->id_start + attr->id_total_num) < attr->id_start)) {
        return -ENODEV;
    }
    return 0;
}

static int id_pool_inst_check(struct id_pool_inst *inst, int type)
{
    if (inst == NULL) {
        return -EINVAL;
    }

    if ((inst->phy_devid >= MAX_ID_POOL_DEVID) || (inst->sub_id >= MAX_ID_POOL_SUB_ID) ||
        (type >= MAX_ID_POOL_TYPE) || (type < 0)) {
        return -ENODEV;
    }
    return 0;
}

int id_pool_register(struct id_pool_inst *inst, int type, struct id_pool_attr *attr)
{
    struct id_pool_node *pool_node = NULL;
    struct id_pool *pool = NULL;
    int ret;

    ret = id_pool_inst_check(inst, type);
    if (ret != 0) {
        return ret;
    }

    ret = id_pool_attr_check(attr);
    if (ret != 0) {
        return ret;
    }

    pool = id_pool_init(inst, type);
    if (pool == NULL) {
        return -ENOMEM;
    }

    pool_node = id_pool_node_create(inst, attr);
    if (pool_node == NULL) {
        return -ENOMEM;
    }

    mutex_lock(&pool->mutex);
    ret = id_pool_node_add(pool, pool_node);
    if (ret != 0) {
        trs_err("Add fail.\n");
        id_pool_show(pool);
        id_pool_node_destroy(pool_node);
    }
    mutex_unlock(&pool->mutex);
    return ret;
}
EXPORT_SYMBOL(id_pool_register);

void id_pool_unregister(struct id_pool_inst *inst, int type, struct id_pool_attr *attr)
{
    struct id_pool_node *pool_node = NULL;
    struct id_pool *pool = NULL;
    int ret;

    ret = id_pool_inst_check(inst, type);
    if (ret != 0) {
        return;
    }

    ret = id_pool_attr_check(attr);
    if (ret != 0) {
        return;
    }

    pool = id_pool_get(inst, type);
    if (pool == NULL) {
        return;
    }
    mutex_lock(&pool->mutex);
    pool_node = id_pool_node_find_by_range(pool, attr->id_start, attr->id_total_num);
    if (pool_node != NULL) {
        id_pool_node_del(pool, pool_node);
        id_pool_node_destroy(pool_node);
    }
    mutex_unlock(&pool->mutex);
}
EXPORT_SYMBOL(id_pool_unregister);

static int _id_pool_alloc(struct id_pool *pool, u32 *id)
{
    struct rb_node *node = NULL;

    for (node = rb_first(&pool->root); node != NULL; node = rb_next(node)) {
        struct id_pool_node *pool_node = rb_entry(node, struct id_pool_node, node);
        int tmp_id;

        tmp_id = idr_alloc(&pool_node->idr, pool_node, (int)pool_node->id_start,
            (int)(pool_node->id_start + pool_node->id_total_num), GFP_KERNEL | __GFP_ACCOUNT);
        if (tmp_id < 0) {
            continue;
        }
        *id = (u32)tmp_id;
        pool_node->id_avail_num--;
        return 0;
    }

    return -ENOSPC;
}

static int _id_pool_free(struct id_pool *pool, u32 id)
{
    struct id_pool_node *pool_node = NULL;

    pool_node = id_pool_node_find_by_id(pool, id);
    if (pool_node == NULL) {
        trs_err("Invalid para. (id=%u)\n", id);
        id_pool_show(pool);
        return -ENODEV;
    }
    if (pool_node != idr_find(&pool_node->idr, (unsigned long)id)) {
        trs_err("Invalid para. (id=%u)\n", id);
        id_pool_show(pool);
        return -EFAULT;
    }
    idr_remove(&pool_node->idr, (unsigned long)id);
    pool_node->id_avail_num++;
    return 0;
}

static int _id_pool_alloc_range(struct id_pool *pool, u32 start, u32 end, u32 *id)
{
    struct rb_node *node = pool->root.rb_node;

    for (node = rb_first(&pool->root); node != NULL; node = rb_next(node)) {
        struct id_pool_node *pool_node = rb_entry(node, struct id_pool_node, node);
        u32 real_start, real_end;
        int tmp_id;

        if ((start >= (pool_node->id_start + pool_node->id_total_num)) || (end <= pool_node->id_start)) {
            continue;
        }
        real_start = max_t(u32, start, pool_node->id_start);
        real_end = min_t(u32, end, pool_node->id_start + pool_node->id_total_num);
        tmp_id = idr_alloc(&pool_node->idr, pool_node, (int)real_start, (int)real_end, GFP_KERNEL | __GFP_ACCOUNT);
        if (tmp_id < 0) {
            continue;
        }
        *id = (u32)tmp_id;
        pool_node->id_avail_num--;
        return 0;
    }

    return -ENOSPC;
}

static u32 _id_pool_get_avail_num(struct id_pool *pool)
{
    struct rb_node *node = NULL;
    u32 avail_num = 0;

    for (node = rb_first(&pool->root); node != NULL; node = rb_next(node)) {
        struct id_pool_node *pool_node = rb_entry(node, struct id_pool_node, node);

        avail_num += pool_node->id_avail_num;
    }
    return avail_num;
}

static u32 _id_pool_get_total_num(struct id_pool *pool)
{
    struct rb_node *node = NULL;
    u32 total_num = 0;

    for (node = rb_first(&pool->root); node != NULL; node = rb_next(node)) {
        struct id_pool_node *pool_node = rb_entry(node, struct id_pool_node, node);

        total_num += pool_node->id_total_num;
    }
    return total_num;
}

static void _id_pool_get_range(struct id_pool *pool, u32 *start, u32 *end)
{
    struct rb_node *node = NULL;
    u32 tmp_start = UINT_MAX;
    u32 tmp_end = 0;

    for (node = rb_first(&pool->root); node != NULL; node = rb_next(node)) {
        struct id_pool_node *pool_node = rb_entry(node, struct id_pool_node, node);

        tmp_start = min_t(u32, tmp_start, pool_node->id_start);
        tmp_end = max_t(u32, tmp_end, pool_node->id_start + pool_node->id_total_num);
    }

    if (start != NULL) {
        *start = tmp_start;
    }

    if (end != NULL) {
        *end = tmp_end;
    }
}

static u32 _id_pool_get_total_num_by_range(struct id_pool *pool, u32 start, u32 end)
{
    struct rb_node *node = NULL;
    u32 total_num = 0;

    for (node = rb_first(&pool->root); node != NULL; node = rb_next(node)) {
        struct id_pool_node *pool_node = rb_entry(node, struct id_pool_node, node);
        u32 id_start = pool_node->id_start;
        u32 id_end = pool_node->id_start + pool_node->id_total_num;
        if (start < id_end && end > id_start) {
            id_start = max_t(u32, start, id_start);
            id_end = min_t(u32, end, id_end);
            total_num += (id_end - id_start);
        }
    }

    return total_num;
}

static void _id_pool_get_tunable_range_by_range(struct id_pool *pool, u32 *start, u32 *end)
{
    struct rb_node *node = NULL;

    for (node = rb_first(&pool->root); node != NULL; node = rb_next(node)) {
        struct id_pool_node *pool_node = rb_entry(node, struct id_pool_node, node);
        if ((pool_node->id_start >= *start) && ((pool_node->id_start + pool_node->id_total_num) <= *end)) {
            if (pool_node->node_tunable == 1) {
                *start = pool_node->id_start;
                *end = pool_node->id_start + pool_node->id_total_num;
                break;
            }
        }
    }
}

int id_pool_alloc_by_range(struct id_pool_inst *inst, int type, u32 start, u32 end, u32 *id)
{
    struct id_pool *pool = NULL;
    int ret;

    if (id == NULL) {
        trs_err("Fail. (devid=%u; tsid=%u; type=%d; start=%u; end=%d)\n",
            inst->phy_devid, inst->sub_id, type, start, end);
        return -EINVAL;
    }

    if (start >= end) {
        trs_err("Fail. (devid=%u; tsid=%u; type=%d; start=%u; end=%d)\n",
            inst->phy_devid, inst->sub_id, type, start, end);
        return -EFAULT;
    }

    ret = id_pool_inst_check(inst, type);
    if (ret != 0) {
        return ret;
    }

    pool = id_pool_get(inst, type);
    if (pool == NULL) {
        return -ENXIO;
    }

    mutex_lock(&pool->mutex);
    ret = _id_pool_alloc_range(pool, start, end, id);
    mutex_unlock(&pool->mutex);
    return ret;
}
EXPORT_SYMBOL(id_pool_alloc_by_range);

int id_pool_alloc(struct id_pool_inst *inst, int type, u32 *id)
{
    struct id_pool *pool = NULL;
    int ret;

    if (id == NULL) {
        return -EINVAL;
    }

    ret = id_pool_inst_check(inst, type);
    if (ret != 0) {
        return ret;
    }

    pool = id_pool_get(inst, type);
    if (pool == NULL) {
        return -ENXIO;
    }

    mutex_lock(&pool->mutex);
    ret = _id_pool_alloc(pool, id);
    mutex_unlock(&pool->mutex);
    return ret;
}
EXPORT_SYMBOL(id_pool_alloc);

void id_pool_free(struct id_pool_inst *inst, int type, u32 id)
{
    struct id_pool *pool = NULL;
    int ret;

    ret = id_pool_inst_check(inst, type);
    if (ret != 0) {
        trs_err("Invalid para. (type=%d; id=%u)\n", type, id);
        return;
    }

    pool = id_pool_get(inst, type);
    if (pool != NULL) {
        mutex_lock(&pool->mutex);
        (void)_id_pool_free(pool, id);
        mutex_unlock(&pool->mutex);
    }
}
EXPORT_SYMBOL(id_pool_free);

int id_pool_get_total_num(struct id_pool_inst *inst, int type, u32 *id_total_num)
{
    struct id_pool *pool = NULL;
    int ret;

    if (id_total_num == NULL) {
        return -EINVAL;
    }

    ret = id_pool_inst_check(inst, type);
    if (ret != 0) {
        return -EFAULT;
    }

    pool = id_pool_get(inst, type);
    if (pool == NULL) {
        return -ENXIO;
    }
    mutex_lock(&pool->mutex);
    *id_total_num = _id_pool_get_total_num(pool);
    mutex_unlock(&pool->mutex);
    return 0;
}
EXPORT_SYMBOL(id_pool_get_total_num);

int id_pool_get_range(struct id_pool_inst *inst, int type, u32 *start, u32 *end)
{
    struct id_pool *pool = NULL;
    int ret;

    ret = id_pool_inst_check(inst, type);
    if (ret != 0) {
        return -EFAULT;
    }

    pool = id_pool_get(inst, type);
    if (pool == NULL) {
        return -ENXIO;
    }
    mutex_lock(&pool->mutex);
    _id_pool_get_range(pool, start, end);
    mutex_unlock(&pool->mutex);
    return 0;
}
EXPORT_SYMBOL(id_pool_get_range);

int id_pool_get_total_num_by_range(struct id_pool_inst *inst, int type, u32 start, u32 end, u32 *total_num)
{
    struct id_pool *pool = NULL;
    int ret;

    ret = id_pool_inst_check(inst, type);
    if (ret != 0) {
        return -EFAULT;
    }

    pool = id_pool_get(inst, type);
    if (pool == NULL) {
        return -ENXIO;
    }
    mutex_lock(&pool->mutex);
    if (total_num != NULL) {
        *total_num = _id_pool_get_total_num_by_range(pool, start, end);
    }
    mutex_unlock(&pool->mutex);
    return 0;
}
EXPORT_SYMBOL(id_pool_get_total_num_by_range);

int id_pool_get_tunable_range_by_range(struct id_pool_inst *inst, int type, u32 *start, u32 *end)
{
    struct id_pool *pool = NULL;
    int ret;

    ret = id_pool_inst_check(inst, type);
    if (ret != 0) {
        return -EFAULT;
    }

    pool = id_pool_get(inst, type);
    if (pool == NULL) {
        return -ENXIO;
    }
    mutex_lock(&pool->mutex);
    _id_pool_get_tunable_range_by_range(pool, start, end);
    mutex_unlock(&pool->mutex);
    return 0;
}
EXPORT_SYMBOL(id_pool_get_tunable_range_by_range);

int id_pool_get_avail_num(struct id_pool_inst *inst, int type, u32 *avail_num)
{
    struct id_pool *pool = NULL;
    int ret;

    if (avail_num == NULL) {
        return -EINVAL;
    }

    ret = id_pool_inst_check(inst, type);
    if (ret != 0) {
        return -EFAULT;
    }

    pool = id_pool_get(inst, type);
    if (pool == NULL) {
        return -ENXIO;
    }
    mutex_lock(&pool->mutex);
    *avail_num = _id_pool_get_avail_num(pool);
    mutex_unlock(&pool->mutex);
    return 0;
}
EXPORT_SYMBOL(id_pool_get_avail_num);

static void id_pool_exit_all(void)
{
    u32 phy_devid, sub_id;
    int type;

    for (phy_devid = 0; phy_devid < MAX_ID_POOL_DEVID; phy_devid++) {
        for (sub_id = 0; sub_id < MAX_ID_POOL_SUB_ID; sub_id++) {
            for (type = 0; type < MAX_ID_POOL_TYPE; type++) {
                struct id_pool_inst inst;
                id_pool_inst_pack(&inst, phy_devid, sub_id);
                id_pool_uninit(&inst, type);
            }
        }
    }
}

static void id_pool_init_all(void)
{
    u32 phy_devid, sub_id;
    int type;

    for (phy_devid = 0; phy_devid < MAX_ID_POOL_DEVID; phy_devid++) {
        for (sub_id = 0; sub_id < MAX_ID_POOL_SUB_ID; sub_id++) {
            for (type = 0; type < MAX_ID_POOL_TYPE; type++) {
                mutex_lock(&g_id_pool_lock);
                g_id_pool[phy_devid][sub_id][type] = NULL;
                mutex_unlock(&g_id_pool_lock);
            }
        }
    }
}

int __init init_id_pool(void)
{
    mutex_init(&g_id_pool_lock);
    id_pool_init_all();
    return 0;
}

void __exit exit_id_pool(void)
{
    trs_info("exit_id_pool\n");
    id_pool_exit_all();
    mutex_destroy(&g_id_pool_lock);
}

module_init(init_id_pool);
module_exit(exit_id_pool);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("ID POOL DRIVER");
