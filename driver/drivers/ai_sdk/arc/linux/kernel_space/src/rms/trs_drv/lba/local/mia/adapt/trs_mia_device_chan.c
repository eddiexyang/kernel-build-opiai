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
#include <linux/slab.h>
#include <linux/vmalloc.h>

#include "soc_res.h"

#include "trs_pm_adapt.h"
#include "trs_chan.h"
#include "trs_mia_cfg.h"
#include "trs_mailbox_def.h"
#include "trs_mia_device_id.h"

static void *trs_mia_chan_ops_sq_mem_alloc(struct trs_id_inst *inst, struct trs_chan_type *types,
    struct trs_chan_sq_para *sq_para, u64 *phy_addr, u32 *attr)
{
    struct trs_mia_cfg *mia_cfg = NULL;
    void *ptr = NULL;

    mia_cfg = trs_mia_cfg_get(inst);
    if (mia_cfg != NULL) {
        ptr = trs_chan_ops_sq_mem_alloc(&mia_cfg->pm_inst, types, sq_para, phy_addr, attr);
        trs_mia_cfg_put(mia_cfg);
    }

    return ptr;
}

static void trs_mia_chan_ops_sq_mem_free(struct trs_id_inst *inst, struct trs_chan_type *types,
    struct trs_chan_sq_para *sq_para, void *sq_addr, u64 phy_addr)
{
    struct trs_mia_cfg *mia_cfg = trs_mia_cfg_get(inst);
    if (mia_cfg != NULL) {
        trs_chan_ops_sq_mem_free(&mia_cfg->pm_inst, types, sq_para, sq_addr, phy_addr);
        trs_mia_cfg_put(mia_cfg);
    }
}

static void *trs_mia_chan_ops_cq_mem_alloc(struct trs_id_inst *inst, struct trs_chan_type *types,
    struct trs_chan_cq_para *cq_para, u64 *phy_addr, u32 *attr)
{
    struct trs_mia_cfg *mia_cfg = NULL;
    void *ptr = NULL;

    mia_cfg = trs_mia_cfg_get(inst);
    if (mia_cfg != NULL) {
        ptr = trs_chan_ops_cq_mem_alloc(&mia_cfg->pm_inst, types, cq_para, phy_addr, attr);
        trs_mia_cfg_put(mia_cfg);
    }

    return ptr;
}

static void trs_mia_chan_ops_cq_mem_free(struct trs_id_inst *inst, struct trs_chan_type *types,
    struct trs_chan_cq_para *cq_para, void *cq_addr, u64 phy_addr)
{
    struct trs_mia_cfg *mia_cfg = trs_mia_cfg_get(inst);
    if (mia_cfg != NULL) {
        trs_chan_ops_cq_mem_free(&mia_cfg->pm_inst, types, cq_para, cq_addr, phy_addr);
        trs_mia_cfg_put(mia_cfg);
    }
}

static int trs_mia_chan_ops_ctrl_sqcq(struct trs_id_inst *inst,
    struct trs_chan_type *types, u32 id, u32 cmd, u32 para)
{
    struct trs_mia_cfg *mia_cfg = NULL;
    int ret = -EINVAL;

    mia_cfg = trs_mia_cfg_get(inst);
    if (mia_cfg != NULL) {
        ret = trs_chan_ops_ctrl_sqcq(&mia_cfg->pm_inst, types, id, cmd, para);
        trs_mia_cfg_put(mia_cfg);
    }

    return ret;
}

static int trs_mia_chan_ops_query_sqcq(struct trs_id_inst *inst,
    struct trs_chan_type *types, u32 id, u32 cmd, u64 *value)
{
    struct trs_mia_cfg *mia_cfg = NULL;
    int ret = -EINVAL;

    if (cmd == QUERY_CMD_SQ_DB_PADDR) {
        return trs_chan_ops_query_sqcq(inst, types, id, cmd, value);
    }

    mia_cfg = trs_mia_cfg_get(inst);
    if (mia_cfg != NULL) {
        ret = trs_chan_ops_query_sqcq(&mia_cfg->pm_inst, types, id, cmd, value);
        trs_mia_cfg_put(mia_cfg);
    }

    return ret;
}

static int trs_mia_device_chan_ops_mbox_send(struct trs_id_inst *inst, struct trs_chan_info *chan_info)
{
    struct trs_mia_cfg *mia_cfg = NULL;
    struct trs_chan_adapt_info adapt;
    int ret;

    mia_cfg = trs_mia_cfg_get(inst);
    if (mia_cfg == NULL) {
        return -EINVAL;
    }

    adapt.app_type = TRS_MBOX_SEND_FROM_DEVICE;
    adapt.sq_side = 0;
    adapt.cq_side = 0;
    adapt.fid = mia_cfg->vfid;

    trs_chan_update_ssid(&mia_cfg->pm_inst, chan_info);

    ret = trs_chan_mbox_send(&mia_cfg->pm_inst, chan_info, &adapt);
    trs_mia_cfg_put(mia_cfg);
    return ret;
}

static int trs_mia_chan_cqe_done_irq_init(struct trs_mia_cfg *mia_cfg, u32 irq_num)
{
    if (mia_cfg->cqe_done != NULL) {
        return 0;
    }

    mia_cfg->cqe_done = (struct trs_mia_cqe_done *)vzalloc(sizeof(struct trs_mia_cqe_done) * irq_num);
    if (mia_cfg->cqe_done == NULL) {
        trs_err("vzalloc failed. (devid=%u; irq_num=%u)\n", mia_cfg->inst.devid, irq_num);
        return -ENOMEM;
    }

    mia_cfg->cqe_done_irq_num = irq_num;

    return 0;
}

static int trs_mia_chan_get_irq(struct trs_id_inst *inst, u32 irq_type, u32 irq[], u32 irq_num, u32 *valid_irq_num)
{
    struct trs_mia_cfg *mia_cfg = NULL;
    int ret = -EINVAL;

    mia_cfg = trs_mia_cfg_get(inst);
    if (mia_cfg != NULL) {
        ret = trs_chan_get_irq(&mia_cfg->pm_inst, irq_type, irq, irq_num, valid_irq_num);
        if ((ret == 0) && (irq_type == TS_CQ_UPDATE_IRQ)) {
            mutex_lock(&mia_cfg->mutex);
            ret = trs_mia_chan_cqe_done_irq_init(mia_cfg, *valid_irq_num);
            mutex_unlock(&mia_cfg->mutex);
        }
        trs_mia_cfg_put(mia_cfg);
    }

    return ret;
}

static u32 trs_mia_chan_pick_own_cqid(struct trs_mia_cfg *mia_cfg, int irq_index, u32 cqid[], u32 cq_num)
{
    struct trs_mia_cqe_done *cqe_done = &mia_cfg->cqe_done[irq_index];
    u32 i, pick_num = 0;

    for (i = 0; i < cq_num; i++) {
        if (cqid[i] == U32_MAX) {   /* after mia pick owner cqid, it will set chan_irq's cqid equal U32_MAX */
            continue;
        }
        if (trs_mia_adapt_trans_cqid(mia_cfg, cqid[i], &cqe_done->cqid[pick_num]) == 0) {
            cqid[i] = U32_MAX;  /* pf & vf's irq node at one list, so vf's cq[] should set flag for pf to identify. */
            pick_num++;
        }
    }

    return pick_num;
}

static void trs_mia_chan_cqe_done_proc(int irq_type, int irq_index, void *para, u32 cqid[], u32 cq_num)
{
    struct trs_mia_cfg *mia_cfg = (struct trs_mia_cfg *)para;

    if ((irq_type == TS_CQ_UPDATE_IRQ) && ((u32)irq_index < mia_cfg->cqe_done_irq_num)) {
        struct trs_mia_cqe_done *cqe_done = &mia_cfg->cqe_done[irq_index];
        cq_num = trs_mia_chan_pick_own_cqid(mia_cfg, irq_index, cqid, cq_num);
        if ((cqe_done->cq_handler != NULL) && (cq_num > 0)) {
            cqe_done->cq_handler(irq_type, irq_index, cqe_done->para, cqe_done->cqid, cq_num);
        }
    }
}

static int _trs_mia_chan_ops_request_irq(struct trs_mia_cfg *mia_cfg, u32 irq_type, int irq_index, void *para,
    void(*handler)(int irq_type, int irq_index, void *para, u32 cqid[], u32 cq_num))
{
    if ((irq_type == TS_CQ_UPDATE_IRQ) && ((u32)irq_index < mia_cfg->cqe_done_irq_num)) {
        mia_cfg->cqe_done[irq_index].cq_handler = handler;
        mia_cfg->cqe_done[irq_index].para = para;

        return trs_chan_ops_request_irq(&mia_cfg->pm_inst, irq_type, irq_index, mia_cfg, trs_mia_chan_cqe_done_proc);
    }

    return trs_chan_ops_request_irq(&mia_cfg->pm_inst, irq_type, irq_index, para, handler);
}

static int trs_mia_chan_ops_request_irq(struct trs_id_inst *inst, u32 irq_type, int irq_index, void *para,
    void(*handler)(int irq_type, int irq_index, void *para, u32 cqid[], u32 cq_num))
{
    struct trs_mia_cfg *mia_cfg = NULL;
    int ret = -EINVAL;

    mia_cfg = trs_mia_cfg_get(inst);
    if (mia_cfg != NULL) {
        mutex_lock(&mia_cfg->mutex);
        ret = _trs_mia_chan_ops_request_irq(mia_cfg, irq_type, irq_index, para, handler);
        if (ret != 0) {
            trs_mia_cfg_put(mia_cfg);
        }
        mutex_unlock(&mia_cfg->mutex);
    }

    return ret;
}

static int _trs_mia_chan_free_irq(struct trs_mia_cfg *mia_cfg, int irq_type, int irq_index, void *para)
{
    if ((irq_type == TS_CQ_UPDATE_IRQ) && ((u32)irq_index < mia_cfg->cqe_done_irq_num)) {
        return trs_chan_free_irq(&mia_cfg->pm_inst, irq_type, irq_index, mia_cfg);
    }

    return trs_chan_free_irq(&mia_cfg->pm_inst, irq_type, irq_index, para);
}
static int trs_mia_chan_free_irq(struct trs_id_inst *inst, int irq_type, int irq_index, void *para)
{
    struct trs_mia_cfg *mia_cfg = NULL;
    int ret = -EINVAL;

    mia_cfg = trs_mia_cfg_get(inst);
    if (mia_cfg != NULL) {
        mutex_lock(&mia_cfg->mutex);
        ret = _trs_mia_chan_free_irq(mia_cfg, irq_type, irq_index, para);
        mutex_unlock(&mia_cfg->mutex);
        trs_mia_cfg_put(mia_cfg);
        trs_mia_cfg_put(mia_cfg); /* for trs_mia_chan_ops_request_irq */
    }

    return ret;
}

static int trs_mia_chan_sqe_update(struct trs_id_inst *inst, int pid, u32 sqid, u32 sqeid, void *sqe)
{
    return trs_chan_ops_sqe_update(inst, pid, sqe);
}

struct trs_chan_adapt_ops trs_mia_device_chan_ops = {
    .owner = THIS_MODULE,
    .sq_mem_alloc = trs_mia_chan_ops_sq_mem_alloc,          /* rsv mem need pm_inst */
    .sq_mem_free = trs_mia_chan_ops_sq_mem_free,            /* rsv mem need pm_inst */
    .cq_mem_alloc = trs_mia_chan_ops_cq_mem_alloc,          /* rsv mem need pm_inst */
    .cq_mem_free = trs_mia_chan_ops_cq_mem_free,            /* rsv mem need pm_inst */
    .flush_cache = trs_chan_ops_flush_sqe_cache,            /* related to type */
    .invalid_cache = trs_chan_ops_invalid_cqe_cache,        /* related to type */
    .cqe_is_valid = trs_chan_ops_cqe_is_valid,              /* parse cqe, no need pm_inst */
    .get_sq_head_in_cqe = trs_chan_ops_get_sq_head_in_cqe,  /* parse cqe, no need pm_inst */
    .sqe_update = trs_mia_chan_sqe_update,                  /* for tsagent */
    .cqe_update = trs_chan_ops_cqe_update,                  /* for tsagent */
    .sqcq_ctrl = trs_mia_chan_ops_ctrl_sqcq,                /* operate stars, need pm_inst */
    .sqcq_query = trs_mia_chan_ops_query_sqcq,              /* operate stars, need pm_inst */
    .notice_ts = trs_mia_device_chan_ops_mbox_send,         /* use pm mailbox */
    .get_irq = trs_mia_chan_get_irq,                        /* operate stars, need pm_inst */
    .request_irq = trs_mia_chan_ops_request_irq,            /* operate stars, need pm_inst */
    .free_irq = trs_mia_chan_free_irq,                      /* operate soc_resmng, need pm_inst */
};

int trs_mia_device_chan_config(struct trs_id_inst *inst)
{
    return trs_chan_ts_inst_register(inst, TRS_HW_TYPE_STARS, &trs_mia_device_chan_ops);
}

void trs_mia_device_chan_deconfig(struct trs_id_inst *inst)
{
    trs_chan_ts_inst_unregister(inst);
}

