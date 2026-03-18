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
* Create: 2022-11-1
*/

#include <linux/slab.h>
#include <linux/vmalloc.h>

#include "devdrv_interface.h"

#include "trs_pub_def.h"
#include "trs_id.h"
#include "id_pool.h"

#include "trs_cache_id_recycle.h"

struct trs_id_recycle_info {
    u32 id_end;
    u8 *used;
    int recycle_num;
};

struct trs_id_recycle {
    struct trs_id_inst pm_inst;

    struct trs_id_recycle_info info[TRS_ID_TYPE_MAX];
    struct mutex mutex;

    struct kref ref;
};

static DEFINE_SPINLOCK(id_recycle_lock);
static struct trs_id_recycle *g_id_recycle[TRS_TS_INST_MAX_NUM];

int trs_id_recycle_create(struct trs_id_inst *inst)
{
    u32 ts_inst_id = trs_id_inst_to_ts_inst(inst);
    struct trs_id_recycle *id_recycle = NULL;
    u32 pf_id, vf_id;
    int ret;

    id_recycle = (struct trs_id_recycle *)kzalloc(sizeof(struct trs_id_recycle), GFP_KERNEL);
    if (id_recycle == NULL) {
        return -ENOMEM;
    }

    ret = devdrv_get_pfvf_id_by_devid(inst->devid, &pf_id, &vf_id);
    if (ret != 0) {
        trs_err("Get pf vf id failed. (devid=%u)\n", inst->devid);
        kfree(id_recycle);
        return ret;
    }

    id_recycle->pm_inst.devid = pf_id;
    id_recycle->pm_inst.tsid = inst->tsid;

    kref_init(&id_recycle->ref);
    mutex_init(&id_recycle->mutex);

    spin_lock_bh(&id_recycle_lock);
    if (g_id_recycle[ts_inst_id] != NULL) {
        spin_unlock_bh(&id_recycle_lock);
        kfree(id_recycle);
        trs_err("Repeat create. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -EINVAL;
    }

    g_id_recycle[ts_inst_id] = id_recycle;
    spin_unlock_bh(&id_recycle_lock);

    return 0;
}

static void id_recycle_release(struct kref *kref)
{
    struct trs_id_recycle *id_recycle = container_of(kref, struct trs_id_recycle, ref);
    int type;

    for (type = 0; type < TRS_ID_TYPE_MAX; type++) {
        if (id_recycle->info[type].used != NULL) {
            vfree(id_recycle->info[type].used);
        }
    }
    kfree(id_recycle);
}

void trs_id_recycle_destroy(struct trs_id_inst *inst)
{
    u32 ts_inst_id = trs_id_inst_to_ts_inst(inst);
    struct trs_id_recycle *id_recycle = NULL;

    spin_lock_bh(&id_recycle_lock);
    id_recycle = g_id_recycle[ts_inst_id];
    g_id_recycle[ts_inst_id] = NULL;
    spin_unlock_bh(&id_recycle_lock);

    if (id_recycle != NULL) {
        kref_put(&id_recycle->ref, id_recycle_release);
    }
}

static struct trs_id_recycle *id_recycle_get(struct trs_id_inst *inst)
{
    u32 ts_inst_id = trs_id_inst_to_ts_inst(inst);
    struct trs_id_recycle *id_recycle = NULL;

    spin_lock_bh(&id_recycle_lock);
    id_recycle = g_id_recycle[ts_inst_id];
    if (id_recycle != NULL) {
        kref_get(&id_recycle->ref);
    }
    spin_unlock_bh(&id_recycle_lock);

    return id_recycle;
}

static void id_recycle_put(struct trs_id_recycle *id_recycle)
{
    kref_put(&id_recycle->ref, id_recycle_release);
}

static struct trs_id_recycle_info *get_id_recycle_info(struct trs_id_recycle *id_recycle, int id_type)
{
    struct id_pool_inst pool_inst;
    u32 start, end;
    int ret;

    if ((id_type >= TRS_ID_TYPE_MAX) || (id_type < 0)) {
        return NULL;
    }

    if (id_recycle->info[id_type].used != NULL) {
        return &id_recycle->info[id_type];
    }

    id_pool_inst_pack(&pool_inst, id_recycle->pm_inst.devid, id_recycle->pm_inst.tsid);
    ret = id_pool_get_range(&pool_inst, id_type, &start, &end);
    if (ret != 0) {
        return NULL;
    }

    id_recycle->info[id_type].used = (u8 *)vzalloc(sizeof(u8) * end);
    if (id_recycle->info[id_type].used == NULL) {
        return NULL;
    }
    id_recycle->info[id_type].id_end = end;

    return &id_recycle->info[id_type];
}

void trs_id_recycle_set(struct trs_id_inst *inst, int type, u32 id)
{
    struct trs_id_recycle *id_recycle = NULL;

    if ((type >= TRS_ID_TYPE_MAX) || (type < 0)) {
        return;
    }

    id_recycle = id_recycle_get(inst);
    if (id_recycle != NULL) {
        struct trs_id_recycle_info *info = get_id_recycle_info(id_recycle, type);
        if (info != NULL) {
            if (id < info->id_end) {
                info->used[id] = 1;
                info->recycle_num++;
            }
        } else {
            trs_warn("This is abnormal. (devid=%u; tsid=%u; type=%d)\n", inst->devid, inst->tsid, type);
        }
        id_recycle_put(id_recycle);
    } else {
        trs_warn("This is abnormal. (devid=%u; tsid=%u; type=%d)\n", inst->devid, inst->tsid, type);
    }
}

int trs_id_recycle_clear(struct trs_id_inst *inst, int type, u32 id)
{
    struct trs_id_recycle *id_recycle = NULL;
    int ret = -EINVAL;

    if ((type >= TRS_ID_TYPE_MAX) || (type < 0)) {
        return ret;
    }
    id_recycle = id_recycle_get(inst);
    if (id_recycle != NULL) {
        struct trs_id_recycle_info *info = get_id_recycle_info(id_recycle, type);
        if (info != NULL) {
            if ((id >= info->id_end) || (info->used[id] == 0)) {
                id_recycle_put(id_recycle);
                trs_err("Failed. (devid=%u; tsid=%u; type=%d; id=%u; id_end=%u; used=%u)\n",
                    inst->devid, inst->tsid, type, id, info->id_end, info->used[id]);
                return ret;
            }

            info->used[id] = 0;
            info->recycle_num--;
            ret = 0;
        }
        id_recycle_put(id_recycle);
    }

    return ret;
}

void trs_id_recycle_clear_all(struct trs_id_inst *inst, int type)
{
    struct trs_id_recycle *id_recycle = id_recycle_get(inst);
    u32 num = 0;

    if (id_recycle != NULL) {
        struct id_pool_inst pool_inst;
        u32 i;

        if (id_recycle->info[type].recycle_num > 0) {
            trs_info("Start cache id recycle. (devid=%u; tsid=%u; type=%d; recycle_num=%d)\n",
                inst->devid, inst->tsid, type, id_recycle->info[type].recycle_num);
        }

        if (id_recycle->info[type].used == NULL) {
            id_recycle_put(id_recycle);
            trs_warn("This is abnormal. (devid=%u; tsid=%u; type=%d)\n", inst->devid, inst->tsid, type);
            return;
        }

        id_pool_inst_pack(&pool_inst, id_recycle->pm_inst.devid, id_recycle->pm_inst.tsid);

        for (i = 0; i < id_recycle->info[type].id_end; i++) {
            if (id_recycle->info[type].used[i] == 1) {
                id_recycle->info[type].used[i] = 0;
                id_pool_free(&pool_inst, type, i);
                num++;
            }
        }
        id_recycle_put(id_recycle);

        if (num > 0) {
            trs_info("Cache id recycle. (devid=%u; tsid=%u; type=%d; num=%u)\n", inst->devid, inst->tsid, type, num);
        }
    } else {
        trs_debug("This is abnormal. (devid=%u; tsid=%u; type=%d)\n", inst->devid, inst->tsid, type);
    }
}

int trs_get_id_recycle_num(struct trs_id_inst *inst, int type)
{
    struct trs_id_recycle *id_recycle = NULL;
    int num = -EINVAL;

    if ((type >= TRS_ID_TYPE_MAX) || (type < 0)) {
        return num;
    }

    id_recycle = id_recycle_get(inst);
    if (id_recycle != NULL) {
        struct trs_id_recycle_info *info = get_id_recycle_info(id_recycle, type);
        if (info != NULL) {
            num = info->recycle_num;
        }
        id_recycle_put(id_recycle);
    } else {
        trs_warn("This is abnormal. (devid=%u; tsid=%u; type=%d)\n", inst->devid, inst->tsid, type);
    }
    return num;
}

