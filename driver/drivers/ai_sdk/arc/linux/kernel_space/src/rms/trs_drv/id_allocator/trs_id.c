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
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/kref.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/module.h>

#include "securec.h"

#include "trs_msg.h"
#include "trs_id.h"

#define MAX_TRS_ID_BATCH_NUM  64U
static struct mutex g_trs_id_mutex;

#ifdef TRS_ID_NODE_KMEM_CACHE
static struct kmem_cache *trs_id_cache;
#endif

struct trs_id_node {
    u32 id;
    struct list_head list; // free id list node
};

struct trs_id_pool {
    struct trs_id_inst inst;
    int type;

    struct trs_id_attr attr;
    u32 allocatable_num;    // Id numbers listed to free list (Allocatable id number)
    u32 alloc_num;          // Id numbers that already allocated
    struct list_head head;  // free id list head
    struct trs_id_ops ops;
    struct kref ref;
    struct mutex mutex;
};

static struct trs_id_pool *g_trs_id_pool[TRS_TS_INST_MAX_NUM][TRS_ID_TYPE_MAX];

size_t trs_id_get_node_size(void)
{
    return sizeof(struct trs_id_node);
}

size_t trs_id_get_pool_size(void)
{
    return sizeof(struct trs_id_pool);
}

static struct trs_id_node *trs_id_node_create(void)
{
    struct trs_id_node *node = NULL;

#ifdef TRS_ID_NODE_KMEM_CACHE
    node = kmem_cache_alloc(trs_id_cache, GFP_KERNEL | __GFP_ACCOUNT);
#else
    node = kmalloc(trs_id_get_node_size(), GFP_KERNEL | __GFP_ACCOUNT);
#endif
    return node;
}

static void trs_id_node_destroy(struct trs_id_node *node)
{
    if (node != NULL) {
#ifdef TRS_ID_NODE_KMEM_CACHE
        kmem_cache_free(trs_id_cache, node);
#else
        kfree(node);
#endif
    }
}

static int trs_id_pool_add_node(struct trs_id_pool *id_pool, u32 id)
{
    struct trs_id_node *node = trs_id_node_create();

    if (node == NULL) {
        trs_err("Alloc id node fail. (devid=%u; tsid=%u; type=%s; id=%u)\n",
            id_pool->inst.devid, id_pool->inst.tsid, trs_id_type_to_name(id_pool->type), id);
        return -ENOMEM;
    }
    node->id = id;
    if (node->id < id_pool->attr.split) {
        list_add(&node->list, &id_pool->head);
    } else {
        list_add_tail(&node->list, &id_pool->head);
    }
    id_pool->allocatable_num++;
    return 0;
}

static void trs_id_pool_del_node(struct trs_id_pool *id_pool, struct trs_id_node *node)
{
    list_del(&node->list);
    trs_id_node_destroy(node);
    id_pool->allocatable_num--;
}

static void trs_id_pool_del_node_all(struct trs_id_pool *id_pool)
{
    struct trs_id_node *node = NULL;
    struct trs_id_node *n = NULL;

    list_for_each_entry_safe(node, n, &id_pool->head, list) {
        trs_id_pool_del_node(id_pool, node);
    }
}

static int trs_id_node_all(struct trs_id_pool *id_pool)
{
    int ret;
    u32 id;

    for (id = id_pool->attr.id_start; id < id_pool->attr.id_end; id++) {
        ret = trs_id_pool_add_node(id_pool, id);
        if (ret != 0) {
            trs_id_pool_del_node_all(id_pool);
            return -EFAULT;
        }
    }
    return 0;
}

static struct trs_id_pool *trs_id_pool_create(struct trs_id_inst *inst,
    int type, struct trs_id_attr *attr, struct trs_id_ops *ops)
{
    struct trs_id_pool *id_pool = kzalloc(trs_id_get_pool_size(), GFP_KERNEL | __GFP_ACCOUNT);
    int ret;

    if (id_pool == NULL) {
        trs_err("Id pool no mem.\n");
        return NULL;
    }

    id_pool->inst = *inst;
    id_pool->type = type;
    id_pool->attr = *attr;
    id_pool->alloc_num = 0;
    INIT_LIST_HEAD(&id_pool->head);
    kref_init(&id_pool->ref);
    mutex_init(&id_pool->mutex);

    if (ops == NULL) {
        ret = trs_id_node_all(id_pool);
        if (ret != 0) {
            trs_err("Id node list init fail. (ret=%d)\n", ret);
            goto out;
        }
    } else {
        if ((attr->batch_num == 0) || (attr->batch_num > MAX_TRS_ID_BATCH_NUM)) {
            trs_err("Batch num invalid. (batch_num=%u)\n", attr->batch_num);
            goto out;
        }
        id_pool->ops = *ops;
    }
    return id_pool;
out:
    mutex_destroy(&id_pool->mutex);
    kfree(id_pool);
    return NULL;
}

static u32 _trs_id_pool_alloc_one_id(struct trs_id_pool *id_pool)
{
    struct trs_id_node *node = list_first_entry(&id_pool->head, struct trs_id_node, list);
    u32 id = node->id;

    trs_id_pool_del_node(id_pool, node);
    return id;
}

static int trs_id_pool_free_batch(struct trs_id_pool *id_pool)
{
    if (id_pool->ops.free_batch == NULL) {
        /* Local id do not need to free batch */
        return 0;
    }

    trs_debug("Free info. (type=%s; batch_num=%u; allocatable_num=%u)\n",
        trs_id_type_to_name(id_pool->type), id_pool->attr.batch_num, id_pool->allocatable_num);

    while (id_pool->allocatable_num > 0) {
        u32 batch_num = min_t(u32, id_pool->allocatable_num, id_pool->attr.batch_num);
        u32 id[MAX_TRS_ID_BATCH_NUM];
        u32 idx;
        int ret;

        for (idx = 0; idx < batch_num; idx++) {
            id[idx] = _trs_id_pool_alloc_one_id(id_pool);
        }
        ret = id_pool->ops.free_batch(&id_pool->inst, id_pool->type, id, batch_num);
        if (ret != 0) {
            trs_warn("Free batch fail. (type=%s; batch_num=%u; allocatable_num=%u; ret=%d)\n",
                trs_id_type_to_name(id_pool->type), batch_num, id_pool->allocatable_num, ret);
            return ret;
        }
    }

    return 0;
}

static void _trs_id_pool_destroy(struct trs_id_pool *id_pool)
{
    if (id_pool->ops.free_batch == NULL) {
        /* local id */
        trs_id_pool_del_node_all(id_pool);
    } else {
        (void)trs_id_pool_free_batch(id_pool);
    }
}

static void trs_id_pool_destroy(struct trs_id_pool *id_pool)
{
    if (id_pool != NULL) {
        trs_info("id pool destroy. (type=%s)\n", trs_id_type_to_name(id_pool->type));
        _trs_id_pool_destroy(id_pool);
        mutex_destroy(&id_pool->mutex);
        kfree(id_pool);
    }
}

static int trs_id_pool_add(int type, struct trs_id_pool *id_pool)
{
    u32 ts_inst = trs_id_inst_to_ts_inst(&id_pool->inst);

    mutex_lock(&g_trs_id_mutex);
    if (g_trs_id_pool[ts_inst][type] != NULL) {
        mutex_unlock(&g_trs_id_mutex);
        trs_err("[%s] id pool exists. (devid=%u; tsid=%u)\n",
            trs_id_type_to_name(type), id_pool->inst.devid, id_pool->inst.tsid);
        return -ENODEV;
    }
    g_trs_id_pool[ts_inst][type] = id_pool;
    mutex_unlock(&g_trs_id_mutex);
    return 0;
}

static void trs_id_pool_release(struct kref * kref)
{
    struct trs_id_pool *id_pool = container_of(kref, struct trs_id_pool, ref);

    trs_id_pool_destroy(id_pool);
}

static void trs_id_pool_del(struct trs_id_inst *inst, int type)
{
    u32 ts_inst = trs_id_inst_to_ts_inst(inst);
    struct trs_id_pool *id_pool = NULL;

    mutex_lock(&g_trs_id_mutex);
    id_pool = g_trs_id_pool[ts_inst][type];
    g_trs_id_pool[ts_inst][type] = NULL;
    mutex_unlock(&g_trs_id_mutex);

    if (id_pool != NULL) {
        kref_put(&id_pool->ref, trs_id_pool_release);
    }
}

static int trs_id_type_check(int type)
{
    if ((type < TRS_STREAM_ID) || (type >= TRS_ID_TYPE_MAX)) {
        trs_err("Unknown Trs id type. (type=%d)\n", type);
        return -EINVAL;
    }
    return 0;
}

static int trs_id_param_check(struct trs_id_inst *inst, int type)
{
    int ret;

    ret = trs_id_inst_check(inst);
    if (ret != 0) {
        return ret;
    }

    ret = trs_id_type_check(type);
    if (ret != 0) {
        return ret;
    }

    return 0;
}

static struct trs_id_pool *trs_id_pool_get(struct trs_id_inst *inst, int type)
{
    struct trs_id_pool *id_pool = NULL;
    u32 ts_inst;
    int ret;

    ret = trs_id_param_check(inst, type);
    if (ret != 0) {
        return NULL;
    }

    ts_inst = trs_id_inst_to_ts_inst(inst);
    mutex_lock(&g_trs_id_mutex);
    id_pool = g_trs_id_pool[ts_inst][type];
    if (id_pool != NULL) {
        /* When id pool is obtained, the module reference counting of ops must be added. */
        if (try_module_get(id_pool->ops.owner)) {
            kref_get(&id_pool->ref);
        } else {
            id_pool = NULL;
        }
    }
    mutex_unlock(&g_trs_id_mutex);
    return id_pool;
}

static void trs_id_pool_put(struct trs_id_pool *id_pool)
{
    module_put(id_pool->ops.owner);
    kref_put(&id_pool->ref, trs_id_pool_release);
}

static int trs_id_pool_range_check(struct trs_id_pool *id_pool, u32 id)
{
    if ((id < id_pool->attr.id_start) || (id >= (id_pool->attr.id_end))) {
        trs_err("Trs id range check fail. (id_start=%u; id_end=%u; type=%d; id=%u)\n",
            id_pool->attr.id_start, id_pool->attr.id_end, id_pool->type, id);
        return -EBADR;
    }
    return 0;
}

static void trs_id_pool_alloc_batch(struct trs_id_pool *id_pool)
{
    u32 id[MAX_TRS_ID_BATCH_NUM];
    u32 real_id_num = 0, i;
    u32 remain_num, req_num;
    int ret;

    remain_num = id_pool->attr.id_num - id_pool->alloc_num;
    if (remain_num == 0) {
        /* Max alloc num, don't try to sync id */
        return;
    }

    if (id_pool->ops.alloc_batch == NULL) {
        return;
    }

    if (id_pool->attr.batch_num <= 1) {
        /* non-cache cannot goto cache id allocator branch. */
        return;
    }

    req_num = (remain_num > id_pool->attr.batch_num) ? id_pool->attr.batch_num : remain_num;
    ret = id_pool->ops.alloc_batch(&id_pool->inst, id_pool->type, id, req_num, &real_id_num);
    if ((ret != 0) || (real_id_num > id_pool->attr.batch_num)) {
        trs_err("Alloc batch fail. (devid=%u; tsid=%u; ret=%d; real_id_num=%u; batch_num=%u; type=%s)\n",
            id_pool->inst.devid, id_pool->inst.tsid, ret, real_id_num, id_pool->attr.batch_num,
            trs_id_type_to_name(id_pool->type));
        return;
    }

    for (i = 0; i < real_id_num; i++) {
        ret = trs_id_pool_range_check(id_pool, id[i]);
        if (ret == 0) {
            ret = trs_id_pool_add_node(id_pool, id[i]);
            if (ret != 0) {
                (void)id_pool->ops.free_batch(&id_pool->inst, id_pool->type, &id[i], real_id_num - i);
                break;
            }
        }
    }
}

static int trs_id_alloc_non_cache(struct trs_id_pool *id_pool, u32 *id)
{
    u32 real_id_num;
    int ret;

    ret = id_pool->ops.alloc_batch(&id_pool->inst, id_pool->type, id, 1, &real_id_num);
    if ((ret != 0) || (real_id_num > id_pool->attr.batch_num)) {
        trs_err("Alloc batch fail. (devid=%u; tsid=%u; real_id_num=%u; batch_num=%u; type=%s; ret=%d)\n",
            id_pool->inst.devid, id_pool->inst.tsid, real_id_num, id_pool->attr.batch_num,
            trs_id_type_to_name(id_pool->type), ret);
        return -ENOSPC;
    }

    id_pool->alloc_num++;
    return ret;
}

static int trs_id_alloc_cache(struct trs_id_pool *id_pool, u32 *id)
{
    if (id_pool->allocatable_num == 0) {
        trs_id_pool_alloc_batch(id_pool);
        if (id_pool->allocatable_num == 0) {
            return -ENOSPC;
        }
    }
    *id = _trs_id_pool_alloc_one_id(id_pool);
    id_pool->alloc_num++;

    return 0;
}

static int trs_id_pool_alloc(struct trs_id_pool *id_pool, u32 *id)
{
    if ((id_pool->ops.is_non_cache_type != NULL) && id_pool->ops.is_non_cache_type(id_pool->type)) {
        return trs_id_alloc_non_cache(id_pool, id);
    } else {
        return trs_id_alloc_cache(id_pool, id);
    }
}

static int trs_id_free_non_cache(struct trs_id_pool *id_pool, u32 id)
{
    int ret;

    ret = id_pool->ops.free_batch(&id_pool->inst, id_pool->type, &id, 1);
    if (ret != 0) {
#ifndef EMU_ST
        trs_warn("Free batch fail. (type=%s; batch_num=%u; allocatable_num=%u; ret=%d)\n",
                trs_id_type_to_name(id_pool->type), id_pool->attr.batch_num, id_pool->allocatable_num, ret);
#endif
        return ret;
    }

    id_pool->alloc_num--;
    return ret;
}

static int trs_id_free_cache(struct trs_id_pool *id_pool, u32 id)
{
    int ret;

    ret = trs_id_pool_add_node(id_pool, id);
    if (ret != 0) {
        if (!trs_id_is_local_type(id_pool->type)) {
            ret = trs_id_free_non_cache(id_pool, id);
        }
        return ret;
    }

    id_pool->alloc_num--;

    return ret;
}

static int trs_id_pool_free(struct trs_id_pool *id_pool, u32 id)
{
    int ret;

    ret = trs_id_pool_range_check(id_pool, id);
    if (ret != 0) {
        return ret;
    }

    if ((id_pool->ops.is_non_cache_type != NULL) && id_pool->ops.is_non_cache_type(id_pool->type)) {
        return trs_id_free_non_cache(id_pool, id);
    } else {
        return trs_id_free_cache(id_pool, id);
    }
}

static int trs_id_attr_check(struct trs_id_attr *attr)
{
    if (attr == NULL) {
        return -EINVAL;
    }

    if ((attr->id_num == 0) || ((attr->id_start + attr->id_num) < attr->id_start)) {
        trs_warn("Id para. (id_num=%u; id_start=%u; id_end=%u)\n", attr->id_num, attr->id_start, attr->id_end);
        return -EINVAL;
    }

    if (attr->batch_num > MAX_TRS_ID_BATCH_NUM) {
        trs_err("Invalid para. (batch_num=%u)\n", attr->batch_num);
        return -EINVAL;
    }

    return 0;
}

int trs_id_register(struct trs_id_inst *inst, int type, struct trs_id_attr *attr, struct trs_id_ops *ops)
{
    struct trs_id_pool *id_pool = NULL;
    int ret;

    if (trs_id_param_check(inst, type) != 0) {
        return -EINVAL;
    }

    if (trs_id_attr_check(attr) != 0) {
        return -EINVAL;
    }

    id_pool = trs_id_pool_create(inst, type, attr, ops);
    if (id_pool == NULL) {
        return -ENOMEM;
    }

    ret = trs_id_pool_add(type, id_pool);
    if (ret != 0) {
        trs_id_pool_destroy(id_pool);
        return ret;
    }
    trs_debug("Trs id init. (devid=%u; tsid=%u; type=%s; start=%u; end=%u; num=%u; split=%u)\n",
        inst->devid, inst->tsid, trs_id_type_to_name(type), attr->id_start, attr->id_end, attr->id_num,
        attr->split);
    return 0;
}
EXPORT_SYMBOL(trs_id_register);

int trs_id_unregister(struct trs_id_inst *inst, int type)
{
    if (trs_id_param_check(inst, type) != 0) {
        return -EINVAL;
    }

    trs_id_pool_del(inst, type);
    return 0;
}
EXPORT_SYMBOL(trs_id_unregister);

int trs_id_get_total_num(struct trs_id_inst *inst, int type, u32 *total_num)
{
    struct trs_id_pool *id_pool = trs_id_pool_get(inst, type);
    int ret = -EFAULT;

    if (id_pool != NULL) {
        mutex_lock(&id_pool->mutex);
        if (total_num != NULL) {
            *total_num  = id_pool->attr.id_num;
            ret = 0;
        }
        mutex_unlock(&id_pool->mutex);
        trs_id_pool_put(id_pool);
    }
    return ret;
}
EXPORT_SYMBOL(trs_id_get_total_num);

int trs_id_get_range(struct trs_id_inst *inst, int type, u32 *start, u32 *end)
{
    struct trs_id_pool *id_pool = trs_id_pool_get(inst, type);
    int ret = -EFAULT;

    if (id_pool != NULL) {
        mutex_lock(&id_pool->mutex);
        if ((start != NULL) && (end != NULL)) {
            *start = id_pool->attr.id_start;
            *end = id_pool->attr.id_end;
            ret = 0;
        }
        mutex_unlock(&id_pool->mutex);
        trs_id_pool_put(id_pool);
    }

    return ret;
}
EXPORT_SYMBOL(trs_id_get_range);

int trs_id_get_max_id(struct trs_id_inst *inst, int type, u32 *max_id)
{
    struct trs_id_pool *id_pool = trs_id_pool_get(inst, type);
    int ret = -ENODEV;

    if (id_pool != NULL) {
        mutex_lock(&id_pool->mutex);
        if (max_id != NULL) {
            *max_id  = id_pool->attr.id_end;
            ret = 0;
        }
        mutex_unlock(&id_pool->mutex);

        trs_id_pool_put(id_pool);
    }

    return ret;
}
EXPORT_SYMBOL(trs_id_get_max_id);

int trs_id_get_split(struct trs_id_inst *inst, int type, u32 *split)
{
    struct trs_id_pool *id_pool = trs_id_pool_get(inst, type);
    int ret = -ENODEV;

    if (id_pool != NULL) {
        mutex_lock(&id_pool->mutex);
        if (split != NULL) {
            *split = id_pool->attr.split;
            ret = 0;
        }
        mutex_unlock(&id_pool->mutex);
        trs_id_pool_put(id_pool);
    }
    return ret;
}
EXPORT_SYMBOL(trs_id_get_split);

int trs_id_get_avail_num(struct trs_id_inst *inst, int type, u32 *avail_num)
{
    struct trs_id_pool *id_pool = trs_id_pool_get(inst, type);
    int ret = -ENODEV;

    if (id_pool != NULL) {
        mutex_lock(&id_pool->mutex);
        if (avail_num != NULL) {
            *avail_num = id_pool->attr.id_num - id_pool->alloc_num;
            ret = 0;
        }
        mutex_unlock(&id_pool->mutex);
        trs_id_pool_put(id_pool);
    }
    return ret;
}
EXPORT_SYMBOL(trs_id_get_avail_num);

int trs_id_get_avail_num_in_pool(struct trs_id_inst *inst, int type, u32 *avail_num)
{
    struct trs_id_pool *id_pool = trs_id_pool_get(inst, type);
    int ret = -ENODEV;

    if (id_pool != NULL) {
        mutex_lock(&id_pool->mutex);
        if (id_pool->ops.avail_query != NULL) {
            ret = id_pool->ops.avail_query(inst, type, avail_num);
        }
        mutex_unlock(&id_pool->mutex);
        trs_id_pool_put(id_pool);
    }
    return ret;
}
EXPORT_SYMBOL(trs_id_get_avail_num_in_pool);

int trs_id_get_used_num(struct trs_id_inst *inst, int type, u32 *used_num)
{
    struct trs_id_pool *id_pool = trs_id_pool_get(inst, type);
    int ret = -ENODEV;

    if (id_pool != NULL) {
        mutex_lock(&id_pool->mutex);
        if (used_num != NULL) {
            *used_num = id_pool->alloc_num;
            ret = 0;
        }
        mutex_unlock(&id_pool->mutex);
        trs_id_pool_put(id_pool);
    }
    return ret;
}
EXPORT_SYMBOL(trs_id_get_used_num);

int trs_id_get_stat(struct trs_id_inst *inst, int type, struct trs_id_stat *stat)
{
    struct trs_id_pool *id_pool = trs_id_pool_get(inst, type);
    int ret = -ENODEV;

    if (id_pool != NULL) {
        mutex_lock(&id_pool->mutex);
        stat->alloc = id_pool->alloc_num;
        stat->allocatable = id_pool->allocatable_num;
        ret = 0;
        mutex_unlock(&id_pool->mutex);
        trs_id_pool_put(id_pool);
    }
    return ret;
}
EXPORT_SYMBOL(trs_id_get_stat);

int trs_id_alloc(struct trs_id_inst *inst, int type, u32 *id)
{
    struct trs_id_pool *id_pool = trs_id_pool_get(inst, type);
    int ret = -EFAULT;

    if (id_pool != NULL) {
        mutex_lock(&id_pool->mutex);
        if (id != NULL) {
            ret = trs_id_pool_alloc(id_pool, id);
        }
        mutex_unlock(&id_pool->mutex);
        trs_id_pool_put(id_pool);
    }

    if ((ret == 0) && (id != NULL)) {
        trs_debug("Alloc success. (devid=%u; type=%s; id=%u)\n", inst->devid, trs_id_type_to_name(type), *id);
    }

    return ret;
}
EXPORT_SYMBOL(trs_id_alloc);

int trs_id_free(struct trs_id_inst *inst, int type, u32 id)
{
    struct trs_id_pool *id_pool = trs_id_pool_get(inst, type);
    int ret = -ENODEV;

    trs_debug("Free id. (type=%s; id=%u)\n", trs_id_type_to_name(type), id);

    if (id_pool != NULL) {
        mutex_lock(&id_pool->mutex);
        ret = trs_id_pool_free(id_pool, id);
        mutex_unlock(&id_pool->mutex);
        trs_id_pool_put(id_pool);
    }
    return ret;
}
EXPORT_SYMBOL(trs_id_free);

int trs_id_free_batch_specific(struct trs_id_inst *inst, int type)
{
    struct trs_id_pool *id_pool = trs_id_pool_get(inst, type);
    int ret;

    if (id_pool == NULL) {
        return 0;
    }

    mutex_lock(&id_pool->mutex);
    ret = trs_id_pool_free_batch(id_pool);
    mutex_unlock(&id_pool->mutex);
    trs_id_pool_put(id_pool);
    return ret;
}
EXPORT_SYMBOL(trs_id_free_batch_specific);

int trs_id_flush_to_pool(struct trs_id_inst *inst)
{
    int type, ret = 0;

    for (type = TRS_STREAM_ID; type < TRS_ID_TYPE_MAX; type++) {
        if (trs_id_free_batch_specific(inst, type) != 0) {
            ret = -ENODEV;
            trs_err("Id fush to pool fail. (devid=%u; tsid=%u; type=%s)\n",
                inst->devid, inst->tsid, trs_id_type_to_name(type));
        }
    }

    return ret;
}
EXPORT_SYMBOL(trs_id_flush_to_pool);

int trs_id_to_string(struct trs_id_inst *inst, int type, u32 id, char *msg, u32 msg_len)
{
    struct trs_id_pool *id_pool = trs_id_pool_get(inst, type);
    int ret = -ENODEV;

    if (id_pool != NULL) {
        ret = 0;
        if (id_pool->ops.trans != NULL) {
            u32 phy_id;
            ret = id_pool->ops.trans(inst, type, id, &phy_id);
            if ((ret == 0) && (id != phy_id)) {
                ret = sprintf_s(msg, msg_len, "type(%s),id(%u),phy id(%u).\n", trs_id_type_to_name(type), id, phy_id);
            }
        }

        trs_id_pool_put(id_pool);
    }

    return ret;
}
EXPORT_SYMBOL(trs_id_to_string);

int __init init_trs_id(void)
{
    int trs_inst, type;

    for (trs_inst = 0; trs_inst < TRS_TS_INST_MAX_NUM; trs_inst++) {
        for (type = TRS_STREAM_ID; type < TRS_ID_TYPE_MAX; type++) {
            g_trs_id_pool[trs_inst][type] = NULL;
        }
    }
#ifdef TRS_ID_NODE_KMEM_CACHE
    trs_id_cache = kmem_cache_create("trs_id_cache", sizeof(struct trs_id_node), 0, 0, NULL);
    if (trs_id_cache == NULL) {
        trs_err("kmem cache create fail\n");
        return -ENOMEM;
    }
#endif
    mutex_init(&g_trs_id_mutex);
    return 0;
}

void __exit exit_trs_id(void)
{
    int trs_inst, type;

    trs_info("Exit trs id\n");
#ifdef TRS_ID_NODE_KMEM_CACHE
    if (trs_id_cache != NULL) {
        kmem_cache_destroy(trs_id_cache);
        trs_id_cache = NULL;
        trs_debug("Trs id cache destroy\n");
    }
#endif

    for (trs_inst = 0; trs_inst < TRS_TS_INST_MAX_NUM; trs_inst++) {
        for (type = TRS_STREAM_ID; type < TRS_ID_TYPE_MAX; type++) {
            trs_id_pool_destroy(g_trs_id_pool[trs_inst][type]);
            g_trs_id_pool[trs_inst][type] = NULL;
        }
    }
    mutex_destroy(&g_trs_id_mutex);
}

module_init(init_trs_id);
module_exit(exit_trs_id);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("TRS ID ALLOCATOR");
