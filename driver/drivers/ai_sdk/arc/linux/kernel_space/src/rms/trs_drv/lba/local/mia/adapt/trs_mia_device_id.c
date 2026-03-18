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
#include "id_pool.h"
#include "trs_id.h"
#include "trs_pub_def.h"
#include "trs_chip_def.h"
#include "trs_res_id_def.h"
#include "trs_mia_cfg.h"
#include "trs_pm_adapt.h"
#include "trs_mailbox_def.h"
#include "soc_res.h"

static int trs_mia_adapt_id_type_convert(int type)
{
    switch (type) {
        case TRS_EVENT_ID:
            return MIA_STARS_EVENT;
        case TRS_NOTIFY_ID:
            return MIA_STARS_NOTIFY;
        case TRS_CMO_ID:
            return MIA_STARS_CMO;
        case TRS_HW_SQ_ID:
        case TRS_HW_CQ_ID:
        case TRS_RSV_HW_SQ_ID:
        case TRS_RSV_HW_CQ_ID:
            return MIA_STARS_RTSQ;
        case TRS_CDQM_ID:
            return MIA_STARS_CDQ;
        default:
            return MIA_MAX_RES_TYPE;
    }
}

static u32 _trs_mia_adapt_get_id_bitmap(struct trs_mia_cfg *mia_cfg, u32 type)
{
    if (type == TRS_EVENT_ID) {
        return mia_cfg->event_bitmap;
    } else if (type == TRS_NOTIFY_ID) {
        return mia_cfg->notify_bitmap;
    } else {
        return mia_cfg->rtsq_bitmap;
    }
}

int trs_mia_adapt_trans_cqid(struct trs_mia_cfg *mia_cfg, u32 cqid, u32 *trans_cqid)
{
    u32 bitmap = _trs_mia_adapt_get_id_bitmap(mia_cfg, TRS_HW_CQ_ID);
    u32 cq_group = cqid / mia_cfg->cq_num_per_grp;

    *trans_cqid = cqid;
    return (((0x1 << cq_group) & bitmap) != 0) ? 0 : -EINVAL;
}

static int trs_mia_adapt_get_id_range_by_bit(struct trs_mia_cfg *mia_cfg, u32 type, u32 bit, u32 *start, u32 *end)
{
    u32 num_per_bit;
    int ret;

    if (trs_id_is_hw_divide_type(type)) {
        u64 bitmap;
        struct res_inst_info inst;
        inst.devid = mia_cfg->pm_inst.devid;
        inst.sub_type = TS_SUBSYS;
        inst.subid = mia_cfg->pm_inst.tsid;
        ret = soc_resmng_get_mia_res(&inst,
            trs_mia_adapt_id_type_convert(type), &bitmap, &num_per_bit);
        if (ret != 0) {
            return ret;
        }
        *start = bit * num_per_bit;
        *end = *start + num_per_bit;
    } else {
        ret = trs_device_get_id_range(&mia_cfg->pm_inst, type, start, end);
        if (ret != 0) {
            trs_err("Get id range failed. (devid=%u; type=%d)\n", mia_cfg->pm_inst.devid, type);
            return ret;
        }

        if (trs_id_is_local_type(type) == false) {
            num_per_bit = (*end - *start) / mia_cfg->sia_bitnum;
            *start = *start + bit * num_per_bit;
            *end = *start + num_per_bit;
        }
    }

    trs_debug("Show info. (devid=%u; tsid=%u; type=%s; start=%u; end=%u; num_per_bit=%u)\n",
        mia_cfg->inst.devid, mia_cfg->inst.tsid, trs_id_type_to_name(type), *start, *end, num_per_bit);

    return ret;
}

static int trs_mia_adapt_alloc_id_by_bit(struct trs_mia_cfg *mia_cfg, u32 type, u32 bit, u32 *id)
{
    struct id_pool_inst pool_inst;
    u32 start, end;
    int ret;

    ret = trs_mia_adapt_get_id_range_by_bit(mia_cfg, type, bit, &start, &end);
    if (ret != 0) {
        return ret;
    }

    id_pool_inst_pack(&pool_inst, mia_cfg->pm_inst.devid, mia_cfg->pm_inst.tsid);
    return id_pool_alloc_by_range(&pool_inst, type, start, end, id);
}

static int trs_mia_adapt_get_id_num_per_bit(struct trs_mia_cfg *mia_cfg, u32 type, u32 bit, u32 *total_num)
{
    struct id_pool_inst pool_inst;
    u64 bitmap;
    u32 num_per_bit, start, end;
    int ret;
    struct res_inst_info inst;

    inst.devid = mia_cfg->pm_inst.devid;
    inst.sub_type = TS_SUBSYS;
    inst.subid = mia_cfg->pm_inst.tsid;
    ret = soc_resmng_get_mia_res(&inst,
        trs_mia_adapt_id_type_convert(type), &bitmap, &num_per_bit);
    if (ret != 0) {
        return ret;
    }

    start = num_per_bit * bit;
    end = start + num_per_bit;

    id_pool_inst_pack(&pool_inst, mia_cfg->pm_inst.devid, mia_cfg->pm_inst.tsid);
    ret = id_pool_get_total_num_by_range(&pool_inst, type, start, end, total_num);
    if (ret != 0) {
        trs_warn("Get total num by range result. (devid=%u; tsid=%u; type=%s; ret=%d\n",
            mia_cfg->inst.devid, mia_cfg->inst.tsid, trs_id_type_to_name(type), ret);
        return ret;
    }

    if (*total_num == 0) {
        trs_warn("Show info. (devid=%u; tsid=%u; type=%s; start=%u; end=%u; total_num=%u)\n",
            mia_cfg->inst.devid, mia_cfg->inst.tsid, trs_id_type_to_name(type), start, end, *total_num);
    }

    return ret;
}

static int _trs_mia_adapt_get_id_total_num(struct trs_mia_cfg *mia_cfg, int type, u32 *total_num)
{
    int ret = 0;

    if (trs_id_is_hw_divide_type(type)) {
        u32 bit, _total_num = 0;
        for (bit = 0; bit < mia_cfg->sia_bitnum; bit++) {
            if (trs_bitmap_bit_is_vaild(_trs_mia_adapt_get_id_bitmap(mia_cfg, type), bit)) {
                u32 num = 0;
                trs_mia_adapt_get_id_num_per_bit(mia_cfg, type, bit, &num);
                _total_num += num;
            }
        }
        *total_num = _total_num;
    } else {
        ret = trs_device_get_id_total_num(&mia_cfg->pm_inst, type, total_num);
        if (ret != 0) {
            return ret;
        }
        *total_num = (trs_id_is_local_type(type)) ? *total_num :
            ((*total_num / mia_cfg->sia_bitnum) * mia_cfg->mia_bitnum);
        *total_num = (trs_id_need_divide_type(type)) ? *total_num : 0;
    }

    trs_debug("Show info. (devid=%u; tsid=%u; type=%s; total_num=%u)\n",
        mia_cfg->inst.devid, mia_cfg->inst.tsid, trs_id_type_to_name(type), *total_num);

    return ret;
}

static int _trs_mia_adapt_alloc_id(struct trs_mia_cfg *mia_cfg, int type, u32 *id)
{
    u32 bitmap = _trs_mia_adapt_get_id_bitmap(mia_cfg, type);
    u32 bitnum = mia_cfg->sia_bitnum;
    int ret = -EBUSY;
    u32 bit;

    for (bit = 0; bit < bitnum; bit++) {
        if (trs_bitmap_bit_is_vaild(bitmap, bit)) {
            ret = trs_mia_adapt_alloc_id_by_bit(mia_cfg, type, bit, id);
            if (ret == 0) {
                break;
            }
        }
    }

    return ret;
}

static void _trs_mia_adapt_get_cq_group(struct trs_mia_cfg *mia_cfg,
    u32 group[], u32 group_num, u32 *valid_group_num)
{
    u32 bitmap = _trs_mia_adapt_get_id_bitmap(mia_cfg, TRS_HW_CQ_ID);
    u32 bitnum = mia_cfg->sia_bitnum;
    u32 bit;

    *valid_group_num = 0;
    for (bit = 0; bit < bitnum; bit++) {
        if (trs_bitmap_bit_is_vaild(bitmap, bit)) {
            if (*valid_group_num >= group_num) {
                return;
            }

            group[*valid_group_num] = bit;
            (*valid_group_num)++;
        }
    }
}

int trs_mia_adapt_alloc_id(struct trs_id_inst *inst, int type, u32 *id)
{
    struct trs_mia_cfg *mia_cfg = NULL;
    int ret = -EINVAL;

    mia_cfg = trs_mia_cfg_get(inst);
    if (mia_cfg != NULL) {
        ret = _trs_mia_adapt_alloc_id(mia_cfg, type, id);
        trs_mia_cfg_put(mia_cfg);
    }

    return ret;
}
EXPORT_SYMBOL(trs_mia_adapt_alloc_id);

void trs_mia_adapt_free_id(struct trs_id_inst *inst, int type, u32 id)
{
    struct trs_mia_cfg *mia_cfg = trs_mia_cfg_get(inst);
    struct id_pool_inst pool_inst;

    if (mia_cfg != NULL) {
        id_pool_inst_pack(&pool_inst, mia_cfg->pm_inst.devid, mia_cfg->pm_inst.tsid);
        id_pool_free(&pool_inst, type, id);
        trs_mia_cfg_put(mia_cfg);
    }
}
EXPORT_SYMBOL(trs_mia_adapt_free_id);

int trs_mia_adapt_get_id_range(struct trs_id_inst *inst, int type, u32 *start, u32 *end)
{
    struct trs_mia_cfg *mia_cfg = NULL;
    int ret = -EINVAL;

    mia_cfg = trs_mia_cfg_get(inst);
    if (mia_cfg != NULL) {
        ret = trs_device_get_id_range(&mia_cfg->pm_inst, type, start, end);
        trs_mia_cfg_put(mia_cfg);
    }

    return ret;
}
EXPORT_SYMBOL(trs_mia_adapt_get_id_range);

int trs_mia_adapt_get_id_total_num(struct trs_id_inst *inst, int type, u32 *total_num)
{
    struct trs_mia_cfg *mia_cfg = NULL;
    int ret = -EINVAL;

    mia_cfg = trs_mia_cfg_get(inst);
    if (mia_cfg != NULL) {
        ret = _trs_mia_adapt_get_id_total_num(mia_cfg, type, total_num);
        trs_mia_cfg_put(mia_cfg);
    }

    return ret;
}
EXPORT_SYMBOL(trs_mia_adapt_get_id_total_num);

int trs_mia_adapt_get_id_split(struct trs_id_inst *inst, int type, u32 *split)
{
    struct trs_mia_cfg *mia_cfg = NULL;
    int ret = -EINVAL;

    mia_cfg = trs_mia_cfg_get(inst);
    if (mia_cfg != NULL) {
        ret = trs_device_get_id_split(&mia_cfg->pm_inst, type, split);
        trs_mia_cfg_put(mia_cfg);
    }

    return ret;
}
EXPORT_SYMBOL(trs_mia_adapt_get_id_split);

int trs_mia_adapt_get_cq_group(struct trs_id_inst *inst,
    u32 group[], u32 group_num, u32 *valid_group_num)
{
    struct trs_mia_cfg *mia_cfg = NULL;

    mia_cfg = trs_mia_cfg_get(inst);
    if (mia_cfg == NULL) {
        return -EINVAL;
    }

    _trs_mia_adapt_get_cq_group(mia_cfg, group, group_num, valid_group_num);
    trs_mia_cfg_put(mia_cfg);
    return 0;
}
EXPORT_SYMBOL(trs_mia_adapt_get_cq_group);

int trs_mia_adapt_alloc_id_batch(struct trs_id_inst *inst, int type,
    u32 id[], u32 id_num, u32 *real_id_num)
{
    struct trs_adapt_notice_ops *notice_ops = trs_adapt_get_notice_ops();
    struct id_pool_inst pool_inst;
    int ret;

    *real_id_num = 0;
    /*
     * In the scenario of reserving IDs on the Device side,
     * if only a few IDs are reserved and the number of alloc_batches is relatively large,
     * it will cause waste.
     */
    id_pool_inst_pack(&pool_inst, inst->devid, inst->tsid);
    ret = trs_mia_adapt_alloc_id(inst, type, &id[0]);
    if (ret == -ENOSPC) {
        if (notice_ops->sync_id_proc != NULL) {
            ret = notice_ops->sync_id_proc(inst->devid, inst->tsid, type);
            if (ret != 0) {
                trs_err("Failed to sync ids from opposite. (devid=%u; tsid=%u; type=%u)\n",
                    inst->devid, inst->tsid, type);
                return ret;
            }
            ret = trs_mia_adapt_alloc_id(inst, type, &id[0]);
        } else {
            trs_info("Not support sync ids.\n");
        }
    }

    if (ret == 0) {
        *real_id_num = 1;
    }

    return ret;
}

int trs_mia_adapt_free_id_batch(struct trs_id_inst *inst, int type,
    u32 id[], u32 id_num)
{
    u32 i;

    for (i = 0; i < id_num; i++) {
        trs_mia_adapt_free_id(inst, type, id[i]);
    }

    return 0;
}

static int trs_mia_adapt_set_cq_num_per_grp(struct trs_id_inst *inst)
{
    struct trs_mia_cfg *mia_cfg = NULL;
    u32 num_per_bit = 0;
    u64 bitmap;
    int ret = -EINVAL;

    mia_cfg = trs_mia_cfg_get(inst);
    if (mia_cfg != NULL) {
        struct res_inst_info info;
        info.devid = mia_cfg->pm_inst.devid;
        info.sub_type = TS_SUBSYS;
        info.subid = mia_cfg->pm_inst.tsid;
        ret = soc_resmng_get_mia_res(&info, MIA_STARS_RTSQ, &bitmap, &num_per_bit);
        if (ret != 0) {
            trs_err("Get stars rtsq num per bit failed. (devid=%u; tsid=%u; ret=%u)\n",
                mia_cfg->pm_inst.devid, mia_cfg->pm_inst.tsid, ret);
        }
        mia_cfg->cq_num_per_grp = num_per_bit;
        trs_mia_cfg_put(mia_cfg);
    }

    return ret;
}

static int trs_mia_adapt_get_move_id_pool_range(struct trs_mia_cfg *mia_cfg, int type, u32 *start, u32 *end)
{
    struct id_pool_inst pool_inst;
    u32 num_per_bit;
    u64 bitmap;
    int ret;
    struct res_inst_info inst;
    inst.devid = mia_cfg->pm_inst.devid;
    inst.sub_type = TS_SUBSYS;
    inst.subid = mia_cfg->pm_inst.tsid;

    ret = soc_resmng_get_mia_res(&inst,
        trs_mia_adapt_id_type_convert(type), &bitmap, &num_per_bit);
    if (ret != 0) {
        return ret;
    }

    *start = (ffs(mia_cfg->rtsq_bitmap) - 1) * num_per_bit;
    *end = *start + num_per_bit;

    id_pool_inst_pack(&pool_inst, mia_cfg->pm_inst.devid, mia_cfg->pm_inst.tsid);
    ret = id_pool_get_tunable_range_by_range(&pool_inst, type, start, end);

    trs_debug("Show info. (devid=%u; tsid=%u; start=%u; end=%u, ret=%d)\n",
        mia_cfg->inst.devid, mia_cfg->inst.tsid, *start, *end, ret);

    return ret;
}

static int trs_mia_adapt_move_id_pool_node(struct trs_id_inst *inst, int dest_type, int src_type)
{
    struct trs_mia_cfg *mia_cfg = NULL;
    struct id_pool_inst pool_inst;
    struct id_pool_attr attr;
    u32 start, end;
    int ret = -EINVAL;

    mia_cfg = trs_mia_cfg_get(inst);
    if (mia_cfg != NULL) {
        ret = trs_mia_adapt_get_move_id_pool_range(mia_cfg, src_type, &start, &end);
        if (ret != 0) {
            trs_mia_cfg_put(mia_cfg);
            trs_err("Get move id pool range failed. (devid=%u; tsid=%u; src_type=%s; dest_type=%s; ret=%d)\n",
                inst->devid, inst->tsid, trs_id_type_to_name(src_type), trs_id_type_to_name(dest_type), ret);
            return ret;
        }

        attr.node_tunable = 1;
        attr.id_start = start;
        attr.id_total_num = end - start;
        id_pool_inst_pack(&pool_inst, mia_cfg->pm_inst.devid, mia_cfg->pm_inst.tsid);
        id_pool_unregister(&pool_inst, src_type, &attr);
        ret = id_pool_register(&pool_inst, dest_type, &attr);
        if (ret != 0) {
            trs_err("Move id pool node failed. (devid=%u; tsid=%u; src_type=%s; dest_type=%s; start=%u; end=%u)\n",
                inst->devid, inst->tsid, trs_id_type_to_name(src_type), trs_id_type_to_name(src_type), start, end);
        }

        trs_mia_cfg_put(mia_cfg);
    }

    return ret;
}

static int trs_mia_adapt_id_pool_adjust(struct trs_id_inst *inst)
{
    int ret;

    ret = trs_mia_adapt_move_id_pool_node(inst, TRS_RSV_HW_SQ_ID, TRS_HW_SQ_ID);
    if (ret != 0) {
        return ret;
    }

    ret = trs_mia_adapt_move_id_pool_node(inst, TRS_RSV_HW_CQ_ID, TRS_HW_CQ_ID);
    if (ret != 0) {
        (void)trs_mia_adapt_move_id_pool_node(inst, TRS_HW_SQ_ID, TRS_RSV_HW_SQ_ID);
        return ret;
    }

    return 0;
}

static void trs_mia_adapt_id_pool_reset(struct trs_id_inst *inst)
{
    (void)trs_mia_adapt_move_id_pool_node(inst, TRS_HW_SQ_ID, TRS_RSV_HW_SQ_ID);
    (void)trs_mia_adapt_move_id_pool_node(inst, TRS_HW_CQ_ID, TRS_RSV_HW_CQ_ID);
}

static bool trs_mia_adapt_id_is_non_cache_type(int type)
{
    /* In ep, dvpp may alloc hw sq/cq by kernel interface. */
    return ((type == TRS_RSV_HW_SQ_ID) || (type == TRS_RSV_HW_CQ_ID) ||
        (type == TRS_HW_SQ_ID) || (type == TRS_HW_CQ_ID) ||
        (type == TRS_TASK_SCHED_CQ_ID) || (type == TRS_CDQM_ID));
}

static int trs_mia_adapt_id_init(struct trs_id_inst *inst, int type)
{
    struct trs_id_ops ops = {.owner = THIS_MODULE,
        .alloc_batch = trs_mia_adapt_alloc_id_batch, .free_batch = trs_mia_adapt_free_id_batch,
        .is_non_cache_type = trs_mia_adapt_id_is_non_cache_type};
    struct trs_id_ops *tmp_ops = NULL;
    u32 start, end, total_num, split;
    struct trs_id_attr attr;
    int ret;

    ret = trs_mia_adapt_get_id_range(inst, type, &start, &end);
    ret |= trs_mia_adapt_get_id_total_num(inst, type, &total_num);
    ret |= trs_mia_adapt_get_id_split(inst, type, &split);
    if (ret != 0) {
        return ret;
    }
    if (!trs_id_is_local_type(type)) {
        tmp_ops = &ops;
    }
    attr.id_start = start;
    attr.id_end = end;
    attr.id_num = total_num;
    attr.batch_num = trs_mia_adapt_id_is_non_cache_type(type) ? 1 : TRS_ID_CACHE_BATCH_NUM; /* 1: id non cache */
    attr.split = split;
    ret = trs_id_register(inst, type, &attr, tmp_ops);
    if (ret != 0) {
        trs_debug("Id register fail. (devid=%u; tsid=%u; type=%s; ret=%d; num=%u; start=%u; end=%u)\n",
            inst->devid, inst->tsid, trs_id_type_to_name(type), ret, total_num, start, end);
    }

    if (type == TRS_HW_CQ_ID) {
        ret = trs_mia_adapt_set_cq_num_per_grp(inst);
    }
    return ret;
}

static void trs_mia_adapt_id_uninit(struct trs_id_inst *inst, int type)
{
    trs_id_unregister(inst, type);
}

void trs_mia_device_id_deconfig(struct trs_id_inst *inst)
{
    int type;

    for (type = TRS_STREAM_ID; type < TRS_ID_TYPE_MAX; type++) {
        trs_mia_adapt_id_uninit(inst, type);
    }
    trs_mia_adapt_id_pool_reset(inst);
}

int trs_mia_device_id_config(struct trs_id_inst *inst)
{
    int type, ret;

    ret = trs_mia_adapt_id_pool_adjust(inst);
    if (ret != 0) {
        return ret;
    }

    for (type = TRS_STREAM_ID; type < TRS_ID_TYPE_MAX; type++) {
        ret = trs_mia_adapt_id_init(inst, type);
        if (ret == 0) {
            trs_info("Trs mia id init succeed. (devid=%u; tsid=%u; type=%s)\n",
                inst->devid, inst->tsid, trs_id_type_to_name(type));
        }
    }

    return 0;
}
