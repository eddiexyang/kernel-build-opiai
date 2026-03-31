/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * Create: 2022-8-15
 */
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/kref.h>

#include "securec.h"
#include "trs_ts_db.h"

static DEFINE_RWLOCK(db_lock);

#define MAX_TRS_DB_TYPE 10

struct trs_db_cfg_node {
    int type;

    void __iomem *vaddr;
    phys_addr_t paddr;
    size_t size;
    u32 stride;

    u32 start;
    u32 end;

    struct kref ref;
};

struct trs_db_cfg {
    u32 devid;
    u32 tsid;

    void __iomem *vaddr;
    phys_addr_t paddr;
    size_t size;
    u32 stride;
    u32 num;

    struct spinlock lock;
    struct kref ref;
    struct trs_db_cfg_node *db_cfg_node[MAX_TRS_DB_TYPE];
};

static struct trs_db_cfg *g_db_cfg[TRS_TS_INST_MAX_NUM];

static struct trs_db_cfg_node *trs_db_cfg_node_create(struct trs_db_cfg *db_cfg, int type, u32 start, u32 end)
{
    struct trs_db_cfg_node *cfg_node = NULL;

    cfg_node = kzalloc(sizeof(struct trs_db_cfg_node), GFP_KERNEL);
    if (cfg_node == NULL) {
        trs_err("Cfg node alloc fail. (devid=%u; tsid=%u; type=%d)\n", db_cfg->devid, db_cfg->tsid, type);
        return NULL;
    }

    cfg_node->stride = db_cfg->stride;
    cfg_node->vaddr = db_cfg->vaddr + start * db_cfg->stride;
    cfg_node->paddr = db_cfg->paddr + start * db_cfg->stride;
    cfg_node->size = (end - start) * cfg_node->stride;
    cfg_node->type = type;
    cfg_node->start = start;
    cfg_node->end = end;
    kref_init(&cfg_node->ref);

    return cfg_node;
}

static void trs_db_cfg_node_destroy(struct trs_db_cfg_node *cfg_node)
{
    if (cfg_node != NULL) {
        kfree(cfg_node);
    }
}

static int trs_db_cfg_node_add(struct trs_db_cfg *db_cfg, struct trs_db_cfg_node *cfg_node)
{
    spin_lock_bh(&db_cfg->lock);
    if (db_cfg->db_cfg_node[cfg_node->type] != NULL) {
        spin_unlock_bh(&db_cfg->lock);
        return -EEXIST;
    }
    db_cfg->db_cfg_node[cfg_node->type] = cfg_node;
    spin_unlock_bh(&db_cfg->lock);

    return 0;
}

void trs_db_cfg_node_release(struct kref *kref)
{
    struct trs_db_cfg_node *cfg_node = container_of(kref, struct trs_db_cfg_node, ref);

    trs_db_cfg_node_destroy(cfg_node);
}

static void trs_db_cfg_node_del(struct trs_db_cfg *db_cfg, int type)
{
    struct trs_db_cfg_node *cfg_node = NULL;

    spin_lock_bh(&db_cfg->lock);
    cfg_node = db_cfg->db_cfg_node[type];
    db_cfg->db_cfg_node[type] = NULL;
    spin_unlock_bh(&db_cfg->lock);

    if (cfg_node != NULL) {
        kref_put(&cfg_node->ref, trs_db_cfg_node_release);
    }
}

static struct trs_db_cfg_node *trs_db_cfg_node_get(struct trs_db_cfg *db_cfg, int type)
{
    struct trs_db_cfg_node *cfg_node = NULL;

    spin_lock_bh(&db_cfg->lock);
    cfg_node = db_cfg->db_cfg_node[type];
    if (cfg_node != NULL) {
        kref_get(&cfg_node->ref);
    }
    spin_unlock_bh(&db_cfg->lock);

    return cfg_node;
}

static void trs_db_cfg_node_put(struct trs_db_cfg_node *cfg_node)
{
    kref_put(&cfg_node->ref, trs_db_cfg_node_release);
}

static struct trs_db_cfg *trs_db_cfg_create(struct trs_id_inst *inst, phys_addr_t paddr, size_t size, u32 stride)
{
    struct trs_db_cfg *db_cfg = NULL;
    void __iomem *vaddr = NULL;

    db_cfg = kzalloc(sizeof(struct trs_db_cfg), GFP_KERNEL);
    if (db_cfg == NULL) {
        return NULL;
    }

    vaddr = ioremap(paddr, size);
    if (vaddr == NULL) {
        kfree(db_cfg);
        return NULL;
    }
    db_cfg->vaddr = vaddr;
    db_cfg->paddr = paddr;
    db_cfg->size = size;
    db_cfg->stride = stride;
    db_cfg->num = (u32)(size / stride);
    db_cfg->devid = inst->devid;
    db_cfg->tsid = inst->tsid;
    spin_lock_init(&db_cfg->lock);
    kref_init(&db_cfg->ref);

    return db_cfg;
}

static void trs_db_cfg_destroy(struct trs_db_cfg *db_cfg)
{
    if (db_cfg != NULL) {
        if (db_cfg->vaddr != NULL) {
            iounmap(db_cfg->vaddr);
            db_cfg->vaddr = NULL;
        }
        kfree(db_cfg);
    }
}

static void trs_db_cfg_release(struct kref *kref)
{
    struct trs_db_cfg *db_cfg = container_of(kref, struct trs_db_cfg, ref);
    int type;

    spin_lock_bh(&db_cfg->lock);
    for (type = 0; type < MAX_TRS_DB_TYPE; type++) {
        struct trs_db_cfg_node *cfg_node = db_cfg->db_cfg_node[type];

        db_cfg->db_cfg_node[type] = NULL;
        if (cfg_node != NULL) {
            kref_put(&cfg_node->ref, trs_db_cfg_node_release);
        }
    }
    spin_unlock_bh(&db_cfg->lock);

    trs_db_cfg_destroy(db_cfg);
}

static int trs_ts_db_add(struct trs_id_inst *inst, struct trs_db_cfg *db_cfg)
{
    u32 ts_inst = trs_id_inst_to_ts_inst(inst);

    write_lock_bh(&db_lock);
    if (g_db_cfg[ts_inst] != NULL) {
        write_unlock_bh(&db_lock);
        return -ENODEV;
    }
    g_db_cfg[ts_inst] = db_cfg;
    write_unlock_bh(&db_lock);

    return 0;
}

static void trs_ts_db_del(struct trs_id_inst *inst)
{
    u32 ts_inst = trs_id_inst_to_ts_inst(inst);
    struct trs_db_cfg *db_cfg = NULL;

    write_lock_bh(&db_lock);
    db_cfg = g_db_cfg[ts_inst];
    g_db_cfg[ts_inst] = NULL;
    write_unlock_bh(&db_lock);

    if (db_cfg != NULL) {
        kref_put(&db_cfg->ref, trs_db_cfg_release);
    }
}

int trs_ts_db_cfg(struct trs_id_inst *inst, phys_addr_t paddr, size_t size, u32 stride)
{
    struct trs_db_cfg *db_cfg = NULL;
    int ret;

    db_cfg = trs_db_cfg_create(inst, paddr, size, stride);
    if (db_cfg == NULL) {
        return -ENOMEM;
    }

    ret = trs_ts_db_add(inst, db_cfg);
    if (ret != 0) {
        trs_db_cfg_destroy(db_cfg);
        return ret;
    }
    trs_debug("Ts doorbell config. (devid=%u; tsid=%u; db_size=0x%lx(B); db_stride=0x%x(B); db_num=%u)\n",
        inst->devid, inst->tsid, size, stride, (u32)(size / stride));
    return 0;
}
EXPORT_SYMBOL(trs_ts_db_cfg);

void trs_ts_db_decfg(struct trs_id_inst *inst)
{
    trs_ts_db_del(inst);
}
EXPORT_SYMBOL(trs_ts_db_decfg);

static struct trs_db_cfg *trs_ts_db_cfg_get(struct trs_id_inst *inst)
{
    u32 ts_inst = trs_id_inst_to_ts_inst(inst);
    struct trs_db_cfg *db_cfg = NULL;

    read_lock_bh(&db_lock);
    db_cfg = g_db_cfg[ts_inst];
    if (db_cfg != NULL) {
        kref_get(&db_cfg->ref);
    }
    read_unlock_bh(&db_lock);

    return db_cfg;
}

static void trs_ts_db_cfg_put(struct trs_db_cfg *db_cfg)
{
    kref_put(&db_cfg->ref, trs_db_cfg_release);
}

int trs_ring_ts_db(struct trs_id_inst *inst, int type, u32 offset, u32 val)
{
    struct trs_db_cfg_node *cfg_node = NULL;
    struct trs_db_cfg *db_cfg = NULL;

    db_cfg = trs_ts_db_cfg_get(inst);
    if (db_cfg == NULL) {
        return -ENODEV;
    }

    cfg_node = trs_db_cfg_node_get(db_cfg, type);
    if (cfg_node == NULL) {
        trs_ts_db_cfg_put(db_cfg);
        return -EFAULT;
    }

    if ((cfg_node->stride * offset) < cfg_node->size) {
        writel(val, cfg_node->vaddr + cfg_node->stride * offset);
    }

    trs_db_cfg_node_put(cfg_node);
    trs_ts_db_cfg_put(db_cfg);

    return 0;
}

int trs_get_ts_db_val(struct trs_id_inst *inst, int type, u32 offset, u32 *val)
{
    struct trs_db_cfg_node *cfg_node = NULL;
    struct trs_db_cfg *db_cfg = NULL;

    db_cfg = trs_ts_db_cfg_get(inst);
    if (db_cfg == NULL) {
        return -ENODEV;
    }

    cfg_node = trs_db_cfg_node_get(db_cfg, type);
    if (cfg_node == NULL) {
        trs_ts_db_cfg_put(db_cfg);
        return -EFAULT;
    }

    *val = 0;
    if ((cfg_node->stride * offset) < cfg_node->size) {
        *val = readl(cfg_node->vaddr + cfg_node->stride * offset);
    }

    trs_db_cfg_node_put(cfg_node);
    trs_ts_db_cfg_put(db_cfg);

    return 0;
}

int trs_get_ts_db_paddr(struct trs_id_inst *inst, int type, u32 offset, u64 *paddr)
{
    struct trs_db_cfg_node *cfg_node = NULL;
    struct trs_db_cfg *db_cfg = NULL;

    db_cfg = trs_ts_db_cfg_get(inst);
    if (db_cfg == NULL) {
        return -ENODEV;
    }

    cfg_node = trs_db_cfg_node_get(db_cfg, type);
    if (cfg_node == NULL) {
        trs_ts_db_cfg_put(db_cfg);
        return -EFAULT;
    }

    *paddr = 0;
    if ((cfg_node->stride * offset) < cfg_node->size) {
        *paddr = cfg_node->paddr + cfg_node->stride * offset;
    }

    trs_db_cfg_node_put(cfg_node);
    trs_ts_db_cfg_put(db_cfg);

    return 0;
}

int trs_ts_db_init(struct trs_id_inst *inst, int type, u32 start, u32 end)
{
    struct trs_db_cfg_node *cfg_node = NULL;
    struct trs_db_cfg *db_cfg = NULL;
    int ret;

    if (type >= MAX_TRS_DB_TYPE) {
        trs_err("Type invalid. (devid=%u; tsid=%u; type=%d)\n", inst->devid, inst->tsid, type);
        return -EINVAL;
    }

    db_cfg = trs_ts_db_cfg_get(inst);
    if (db_cfg == NULL) {
        return -ENODEV;
    }

    cfg_node = trs_db_cfg_node_create(db_cfg, type, start, end);
    if (cfg_node == NULL) {
        trs_ts_db_cfg_put(db_cfg);
        return -EFAULT;
    }

    ret = trs_db_cfg_node_add(db_cfg, cfg_node);
    if (ret != 0) {
        trs_db_cfg_node_destroy(cfg_node);
    }
    trs_ts_db_cfg_put(db_cfg);

    return ret;
}
EXPORT_SYMBOL(trs_ts_db_init);

void trs_ts_db_uninit(struct trs_id_inst *inst, int type)
{
    struct trs_db_cfg *db_cfg = NULL;

    if (type >= MAX_TRS_DB_TYPE) {
        trs_err("Invalid db type. (devid=%u; tsid=%u; type=%d)\n", inst->devid, inst->tsid, type);
        return;
    }

    db_cfg = trs_ts_db_cfg_get(inst);
    if (db_cfg == NULL) {
        return;
    }
    trs_db_cfg_node_del(db_cfg, type);
    trs_ts_db_cfg_put(db_cfg);
}
EXPORT_SYMBOL(trs_ts_db_uninit);
