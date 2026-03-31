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
* Create: 2022-8-15
*/
#include <linux/errno.h>

#include "trs_pub_def.h"
#include "id_pool.h"
#include "trs_id.h"
#include "trs_mailbox_def.h"
#include "trs_pm_adapt.h"
#include "trs_msg.h"
#include "trs_chip_def.h"
#include "trs_device_notice.h"
#include "trs_device_mbox.h"
#include "trs_device_id.h"

struct trs_id_chip_def {
    u32 id_start;
    u32 id_end;
};

static const struct trs_id_chip_def id_chip_def[TRS_ID_TYPE_MAX] = {
    [TRS_LOGIC_CQ_ID] = {
        .id_start = TRS_LOGIC_CQ_ID_START,
        .id_end = TRS_LOGIC_CQ_ID_END
    },
    [TRS_CB_SQ_ID] = {
        .id_start = TRS_CB_SQ_ID_START,
        .id_end = TRS_CB_SQ_ID_END
    },
    [TRS_CB_CQ_ID] = {
        .id_start = TRS_CB_CQ_ID_START,
        .id_end = TRS_CB_CQ_ID_END
    },
};

static inline void trs_id_get_local_range(int type, u32 *start, u32 *end)
{
    *start = id_chip_def[type].id_start;
    *end = id_chip_def[type].id_end;
}

struct trs_device_id_policy {
    u32 split;              // id larger than spli will be added to tail, otherwise add to head
    struct trs_id_ops *ops; // Indicates whether the id need to be allocated from remote side
};

int trs_device_alloc_id_batch(struct trs_id_inst *inst, int type,
    u32 id[], u32 id_num, u32 *real_id_num)
{
    struct trs_adapt_notice_ops *notice_ops = trs_adapt_get_notice_ops();
    struct id_pool_inst pool_inst;
    u32 avail_num;
    int ret;

    *real_id_num = 0;
    id_pool_inst_pack(&pool_inst, inst->devid, inst->tsid);
    ret = id_pool_get_avail_num(&pool_inst, type, &avail_num);
    if (ret != 0) {
        return ret;
    }
    if ((avail_num == 0) && (notice_ops->sync_id_proc != NULL)) {
        trs_debug("There is no res in id_pool, please sync from opposite. (devid=%u; tsid=%u; type=%s)\n",
            inst->devid, inst->tsid, trs_id_type_to_name(type));
        ret = notice_ops->sync_id_proc(inst->devid, inst->tsid, type);
        if (ret != 0) {
            trs_err("Failed to sync ids from opposite. (devid=%u; tsid=%u; type=%s)\n",
                inst->devid, inst->tsid, trs_id_type_to_name(type));
            return ret;
        }
    }
    /*
     * In the scenario of reserving IDs on the Device side,
     * if only a few IDs are reserved and the number of alloc_batches is relatively large,
     * it will cause waste.
     */
    ret = id_pool_alloc(&pool_inst, type, &id[0]);
    if (ret == 0) {
        *real_id_num = 1;
    }
    return ret;
}

int trs_device_free_id_batch(struct trs_id_inst *inst, int type,
    u32 id[], u32 id_num)
{
    struct id_pool_inst pool_inst;
    u32 i;

    id_pool_inst_pack(&pool_inst, inst->devid, inst->tsid);
    for (i = 0; i < id_num; i++) {
        id_pool_free(&pool_inst, type, id[i]);
    }

    return 0;
}

int trs_device_get_id_range(struct trs_id_inst *inst, int type, u32 *start, u32 *end)
{
    int ret = 0;

    if (trs_id_is_local_type(type)) {
        trs_id_get_local_range(type, start, end);
    } else {
        struct id_pool_inst pool_inst;

        id_pool_inst_pack(&pool_inst, inst->devid, inst->tsid);
        ret = id_pool_get_range(&pool_inst, type, start, end);
    }

    return ret;
}
EXPORT_SYMBOL(trs_device_get_id_range);

int trs_device_get_id_total_num(struct trs_id_inst *inst, int type, u32 *total_num)
{
    u32 start, end;
    int ret = 0;

    if (trs_id_is_local_type(type)) {
        trs_id_get_local_range(type, &start, &end);
        *total_num = end - start;
    } else {
        struct id_pool_inst pool_inst;

        id_pool_inst_pack(&pool_inst, inst->devid, inst->tsid);
        ret = id_pool_get_total_num(&pool_inst, type, total_num);
    }
    return ret;
}
EXPORT_SYMBOL(trs_device_get_id_total_num);

int trs_device_get_id_split(struct trs_id_inst *inst, int type, u32 *split)
{
    u32 start, end;
    int ret;

    ret = trs_device_get_id_range(inst, type, &start, &end);
    if (ret == 0) {
        *split = start;
    }
    return ret;
}
EXPORT_SYMBOL(trs_device_get_id_split);

static int trs_device_res_avail_query(struct trs_id_inst *inst, int type, u32 *num)
{
    struct id_pool_inst pool_inst;

    id_pool_inst_pack(&pool_inst, inst->devid, inst->tsid);
    return id_pool_get_avail_num(&pool_inst, type, num);
}

static bool trs_device_id_is_non_cache_type(int type)
{
    /* In ep, dvpp may alloc hw sq/cq by kernel interface. */
    return ((type == TRS_RSV_HW_SQ_ID) || (type == TRS_RSV_HW_CQ_ID) ||
        (type == TRS_HW_SQ_ID) || (type == TRS_HW_CQ_ID) ||
        (type == TRS_TASK_SCHED_CQ_ID) || (type == TRS_CDQM_ID));
}

static int trs_device_id_init(struct trs_id_inst *inst, int type)
{
    struct trs_id_ops ops = {.owner = THIS_MODULE,
        .alloc_batch = trs_device_alloc_id_batch, .free_batch = trs_device_free_id_batch,
        .avail_query = trs_device_res_avail_query, .is_non_cache_type = trs_device_id_is_non_cache_type};
    struct trs_device_id_policy policy = {.split = 0, .ops = NULL};
    u32 start, end, total_num;
    struct trs_id_attr attr;
    int ret;

    ret = trs_device_get_id_range(inst, type, &start, &end);
    ret |= trs_device_get_id_total_num(inst, type, &total_num);
    ret |= trs_device_get_id_split(inst, type, &policy.split);
    if (ret != 0) {
        return ret;
    }
    if (!trs_id_is_local_type(type)) {
        policy.ops = &ops;
    }

    if (total_num == 0) { /* not need register */
        return 0;
    }

    attr.id_start = start;
    attr.id_end = end;
    attr.id_num = total_num;
    attr.batch_num = trs_device_id_is_non_cache_type(type) ? 1 : TRS_ID_CACHE_BATCH_NUM; /* 1: id non cache */
    attr.split = policy.split;
    ret = trs_id_register(inst, type, &attr, policy.ops);
    if (ret != 0) {
        trs_err("Id register fail. (devid=%u; tsid=%u; type=%d; ret=%d; start=%u; end=%u)\n",
            inst->devid, inst->tsid, type, ret, start, end);
    }
    return ret;
}

static void trs_device_id_uninit(struct trs_id_inst *inst, int type)
{
    trs_id_unregister(inst, type);
}

int trs_id_config(struct trs_id_inst *inst)
{
    int type, ret;

    for (type = TRS_STREAM_ID; type < TRS_ID_TYPE_MAX; type++) {
        ret = trs_device_id_init(inst, type);
        if (ret == 0) {
            trs_debug("Trs id init succeed. (devid=%u; tsid=%u; type=%s)\n",
                inst->devid, inst->tsid, trs_id_type_to_name(type));
        }
    }

    return 0;
}

void trs_id_deconfig(struct trs_id_inst *inst)
{
    int type;

    for (type = TRS_STREAM_ID; type < TRS_ID_TYPE_MAX; type++) {
        trs_device_id_uninit(inst, type);
    }
}

