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
* Create: 2022-7-15
*/
#include <linux/idr.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

#include "securec.h"
#include "trs_id.h"
#include "soc_res.h"
#include "trs_res_id_def.h"

#include "chan_proc_fs.h"
#include "chan_rxtx.h"
#include "trs_chan.h"
#include "chan_init.h"

#define MAX_CHAN_NUM (64 * 2048)
#define TRS_SQCQ_SLOT_SIZE_MAX       SQE_CACHE_SIZE
#define TRS_SQCQ_DEPTH_MAX           65535
static DEFINE_MUTEX(chan_mutex);
static DEFINE_IDR(chan_idr);
static DEFINE_SPINLOCK(chan_idr_lock);

static void _trs_chan_get_sq_info(struct trs_chan *chan, struct trs_chan_sq_info *info)
{
    info->sqid = chan->sq.sqid;
    info->sq_phy_addr = chan->sq.sq_phy_addr;
    info->head_addr = chan->sq.head_addr;
    info->tail_addr = chan->sq.tail_addr;
    info->db_addr = chan->sq.db_addr;
    info->sq_para = chan->sq.para;
}

static void _trs_chan_get_cq_info(struct trs_chan *chan, struct trs_chan_cq_info *info)
{
    info->cqid = chan->cq.cqid;
    info->irq = (int)chan->irq;
    info->cq_phy_addr = chan->cq.cq_phy_addr;
    info->cq_para = chan->cq.para;
}

static struct trs_chan_irq_ctx *trs_chan_find_irq_ctx_with_least_chan(struct trs_chan_irq_ctx *irq_ctx, u32 irq_num)
{
    struct trs_chan_irq_ctx *irq_ctx_tmp = irq_ctx;
    struct trs_chan_irq_ctx *find = irq_ctx;
    u32 chan_num = 0xffff;
    u32 i;

    for (i = 0; i < irq_num; i++) {
        if (irq_ctx_tmp->chan_num < chan_num) {
            chan_num = irq_ctx_tmp->chan_num;
            find = irq_ctx_tmp;
        }
        irq_ctx_tmp++;
    }

    return find;
}

static void trs_chan_get_irq_ctx_base(struct trs_chan *chan, struct trs_chan_irq_ctx **irq_ctx, u32 *irq_num)
{
    if (chan->types.type == CHAN_TYPE_MAINT) {
        *irq_ctx = chan->ts_inst->maint_irq;
        *irq_num = chan->ts_inst->maint_irq_num;
    } else {
        *irq_ctx = chan->ts_inst->normal_irq;
        *irq_num = chan->ts_inst->normal_irq_num;
    }
}

static void trs_chan_add_to_irq_list(struct trs_chan *chan)
{
    struct trs_chan_irq_ctx *irq_ctx = NULL;
    u32 irq_num;

    trs_chan_get_irq_ctx_base(chan, &irq_ctx, &irq_num);
    if ((chan->types.type == CHAN_TYPE_HW) && (chan->ts_inst->ops.get_cq_affinity_irq != NULL)) {
        if (chan->ts_inst->hw_cq_ctx[chan->cq.cqid].irq_index == U32_MAX) {
            (void)chan->ts_inst->ops.get_cq_affinity_irq(&chan->inst, chan->cq.cqid,
                &chan->ts_inst->hw_cq_ctx[chan->cq.cqid].irq_index);
        }
        irq_ctx += chan->ts_inst->hw_cq_ctx[chan->cq.cqid].irq_index % irq_num;
    } else {
        irq_ctx = trs_chan_find_irq_ctx_with_least_chan(irq_ctx, irq_num);
    }

    spin_lock_bh(&irq_ctx->lock);
    irq_ctx->chan_num++;
    chan->irq = irq_ctx->irq;
    chan->irq_ctx = irq_ctx;
    list_add_tail(&chan->node, &irq_ctx->chan_list);
    spin_unlock_bh(&irq_ctx->lock);
}

static void trs_chan_del_from_irq_list(struct trs_chan *chan)
{
    struct trs_chan_irq_ctx *irq_ctx = chan->irq_ctx;

    spin_lock_bh(&irq_ctx->lock);
    irq_ctx->chan_num--;
    list_del(&chan->node);
    spin_unlock_bh(&irq_ctx->lock);
}

static int trs_chan_get_sq_type(int hw_type, struct trs_chan_type *types)
{
    if (types->type == CHAN_TYPE_HW) {
        if ((types->sub_type == CHAN_SUB_TYPE_HW_RTS) || (types->sub_type == CHAN_SUB_TYPE_HW_DVPP) ||
            (types->sub_type == CHAN_SUB_TYPE_HW_TS)) {
            return TRS_HW_SQ_ID;
        } else {
            return TRS_RSV_HW_SQ_ID;
        }
    } else if (types->type == CHAN_TYPE_MAINT) {
        return TRS_MAINT_SQ_ID;
    } else {
        return (hw_type == TRS_HW_TYPE_STARS) ? TRS_SW_SQ_ID : TRS_HW_SQ_ID;
    }
}

static int trs_chan_get_cq_type(int hw_type, struct trs_chan_type *types)
{
    if (types->type == CHAN_TYPE_HW) {
        if ((types->sub_type == CHAN_SUB_TYPE_HW_RTS) || (types->sub_type == CHAN_SUB_TYPE_HW_DVPP) ||
            (types->sub_type == CHAN_SUB_TYPE_HW_TS)) {
            return TRS_HW_CQ_ID;
        } else {
            return TRS_RSV_HW_CQ_ID;
        }
    } else if (types->type == CHAN_TYPE_MAINT) {
        return TRS_MAINT_CQ_ID;
    } else if (types->type == CHAN_TYPE_TASK_SCHED) {
        return TRS_TASK_SCHED_CQ_ID;
    } else {
        return (hw_type == TRS_HW_TYPE_STARS) ? TRS_SW_CQ_ID : TRS_HW_CQ_ID;
    }
}

static int trs_chan_sq_mem_init(struct trs_chan_ts_inst *ts_inst, struct trs_chan *chan)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    struct trs_chan_sq_ctx *sq = &chan->sq;
    int ret;

    if ((sq->para.sq_depth == 0) || (sq->para.sq_depth > TRS_SQCQ_DEPTH_MAX) ||
        (sq->para.sqe_size == 0) || (sq->para.sqe_size > TRS_SQCQ_SLOT_SIZE_MAX) ||
        ((sq->para.sqe_size % SQE_ALIGN_SIZE) != 0)) {
        trs_err("Invalid alloc para. (devid=%u; tsid=%u; sq_depth=%u; sqe_size=%u)\n",
            inst->devid, inst->tsid, sq->para.sq_depth, sq->para.sqe_size);
        return -EINVAL;
    }

    ret = ts_inst->ops.sqcq_query(&chan->inst, &chan->types, sq->sqid, QUERY_CMD_SQ_HEAD_PADDR, &sq->head_addr);
    ret |= ts_inst->ops.sqcq_query(&chan->inst, &chan->types, sq->sqid, QUERY_CMD_SQ_TAIL_PADDR, &sq->tail_addr);
    ret |= ts_inst->ops.sqcq_query(&chan->inst, &chan->types, sq->sqid, QUERY_CMD_SQ_DB_PADDR, &sq->db_addr);
    if (ret != 0) {
        trs_err("Get sq paddr failed. (devid=%u; tsid=%u; sqid=%u; ret=%d)\n", inst->devid, inst->tsid, sq->sqid, ret);
        return ret;
    }

    sq->sq_addr = ts_inst->ops.sq_mem_alloc(inst, &chan->types, &sq->para, &sq->sq_phy_addr, &sq->attr);
    if (sq->sq_addr == NULL) {
        trs_err("Alloc sq mem failed. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -ENOMEM;
    }

    return 0;
}

static int trs_chan_sq_init(struct trs_chan_ts_inst *ts_inst, struct trs_chan *chan)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    int type = trs_chan_get_sq_type(ts_inst->hw_type, &chan->types);
    int ret;

    ret = trs_id_alloc(inst, type, &chan->sq.sqid);
    if (ret != 0) {
        trs_err("Alloc sq id failed. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    if (trs_chan_has_sqcq_mem(chan)) {
        ret = trs_chan_sq_mem_init(ts_inst, chan);
        if (ret != 0) {
            (void)trs_id_free(inst, type, chan->sq.sqid);
            trs_err("Failed to sq mem init. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
            return ret;
        }
    }

    init_waitqueue_head(&chan->sq.wait_queue);
    sema_init(&chan->sq.sem, 1); /* allow one send thread op sq */
    spin_lock_init(&chan->sq.lock);
    chan->sq.status = 1;

    if (chan->types.type == CHAN_TYPE_HW) { /* only hw need sq to chan trans */
        ts_inst->hw_sq_ctx[chan->sq.sqid].chan_id = chan->id;
    }

    trs_debug("Alloc sq. (devid=%u; tsid=%u; id=%d, sqid=%u)\n", inst->devid, inst->tsid, chan->id, chan->sq.sqid);

    return 0;
}

static void trs_chan_sq_uninit(struct trs_chan_ts_inst *ts_inst, struct trs_chan *chan)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    int type = trs_chan_get_sq_type(ts_inst->hw_type, &chan->types);
    if (chan->types.type == CHAN_TYPE_HW) {
        ts_inst->hw_sq_ctx[chan->sq.sqid].chan_id = -1;
    }

    if (chan->sq.sq_addr != NULL) {
        ts_inst->ops.sq_mem_free(inst, &chan->types, &chan->sq.para, chan->sq.sq_addr, chan->sq.sq_phy_addr);
        chan->sq.sq_addr = NULL;
    }
    (void)trs_id_free(inst, type, chan->sq.sqid);
}

static int trs_chan_cq_mem_init(struct trs_chan_ts_inst *ts_inst, struct trs_chan *chan)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    struct trs_chan_cq_ctx *cq = &chan->cq;

    if ((chan->cq.para.cq_depth == 0) || (chan->cq.para.cq_depth > TRS_SQCQ_DEPTH_MAX) ||
        (chan->cq.para.cqe_size == 0) || (chan->cq.para.cqe_size > TRS_SQCQ_SLOT_SIZE_MAX) ||
        ((chan->cq.para.cqe_size % CQE_ALIGN_SIZE) != 0)) {
        trs_err("Invalid alloc para. (devid=%u; tsid=%u; cq_depth=%u; cqe_size=%u)\n",
            inst->devid, inst->tsid, chan->cq.para.cq_depth, chan->cq.para.cqe_size);
        return -EINVAL;
    }

    chan->cq.cq_addr = ts_inst->ops.cq_mem_alloc(inst, &chan->types, &cq->para, &cq->cq_phy_addr, &cq->attr);
    if (chan->cq.cq_addr == NULL) {
        trs_err("Alloc cq mem failed. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -ENOMEM;
    }
    return 0;
}

static int trs_chan_cq_init(struct trs_chan_ts_inst *ts_inst, struct trs_chan *chan)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    int type = trs_chan_get_cq_type(ts_inst->hw_type, &chan->types);
    int ret;

    ret = trs_id_alloc(inst, type, &chan->cq.cqid);
    if ((ret != 0) || ((chan->types.type == CHAN_TYPE_HW) && (chan->cq.cqid >= ts_inst->cq_max_id))) {
        trs_err("Alloc cq id failed. (devid=%u; tsid=%u; type=%u; cqid=%u; ret=%d)\n",
            inst->devid, inst->tsid, chan->types.type, chan->cq.cqid, ret);
        return -EFAULT;
    }

    if (trs_chan_has_sqcq_mem(chan)) {
        ret = trs_chan_cq_mem_init(ts_inst, chan);
        if (ret != 0) {
            (void)trs_id_free(inst, type, chan->cq.cqid);
            trs_err("Failed to cq mem init. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
            return ret;
        }
    }

    init_waitqueue_head(&chan->cq.wait_queue);
    mutex_init(&chan->cq.mutex);
    INIT_WORK(&chan->work, trs_chan_work);

    trs_chan_add_to_irq_list(chan);
    if (chan->types.type == CHAN_TYPE_HW) { /* only hw need cq to chan trans */
        ts_inst->hw_cq_ctx[chan->cq.cqid].chan_id = chan->id;
    }

    trs_debug("Alloc cq. (devid=%u; tsid=%u; id=%d, cqid=%u)\n", inst->devid, inst->tsid, chan->id, chan->cq.cqid);

    return 0;
}

static void trs_chan_cq_uninit(struct trs_chan_ts_inst *ts_inst, struct trs_chan *chan)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    int type = trs_chan_get_cq_type(ts_inst->hw_type, &chan->types);

    trs_chan_del_from_irq_list(chan);
    (void)cancel_work_sync(&chan->work);
    if (chan->cq.cq_addr != NULL) {
        ts_inst->ops.cq_mem_free(inst, &chan->types, &chan->cq.para, chan->cq.cq_addr, chan->cq.cq_phy_addr);
        chan->cq.cq_addr = NULL;
    }
    (void)trs_id_free(inst, type, chan->cq.cqid);
}

static int trs_chan_notice_ts(struct trs_chan *chan, u32 op)
{
    struct trs_id_inst *inst = &chan->ts_inst->inst;
    struct trs_chan_info info = { 0 };
    int ret;

    info.op = op;
    info.ssid = chan->ssid;
    info.types = chan->types;
    info.pid = current->tgid;
    info.irq_type = (chan->types.type == CHAN_TYPE_MAINT) ? TS_FUNC_CQ_IRQ : TS_CQ_UPDATE_IRQ;
    _trs_chan_get_sq_info(chan, &info.sq_info);
    _trs_chan_get_cq_info(chan, &info.cq_info);

    ret = memcpy_s(info.msg, sizeof(info.msg), chan->msg, sizeof(chan->msg));
    if (ret != 0) {
        trs_err("Memcopy failed. (dest_len=%lx; src_len=%lx)\n", sizeof(info.msg), sizeof(chan->msg));
        return ret;
    }

    if ((chan->flag & (0x1 << CHAN_FLAG_USE_MASTER_PID_BIT)) != 0) {
        info.master_pid_flag = 1;
    }

    trs_debug("Info. (pid=%d; master_pid=%d; flag=%u)\n", current->tgid, info.msg[SQCQ_INFO_LENGTH - 1], chan->flag);

    return chan->ts_inst->ops.notice_ts(inst, &info);
}

static void trs_chan_inst_uninit(struct trs_chan_ts_inst *ts_inst, struct trs_chan *chan)
{
    if (trs_chan_is_notice_ts(chan)) {
        (void)trs_chan_notice_ts(chan, 0);
    }

    if (trs_chan_has_sq(chan)) {
        trs_chan_sq_uninit(ts_inst, chan);
    }

    if (trs_chan_has_cq(chan)) {
        trs_chan_cq_uninit(ts_inst, chan);
    }
}

static void _trs_chan_inst_release(struct trs_chan *chan)
{
    struct trs_id_inst *inst = &chan->ts_inst->inst;

    trs_debug("Release success. (devid=%u; tsid=%u; id=%d; flag=%x)\n", inst->devid, inst->tsid, chan->id, chan->flag);

    chan_proc_fs_del_chan(chan);
    trs_chan_inst_uninit(chan->ts_inst, chan);
    trs_chan_ts_inst_put(chan->ts_inst);
    kfree(chan);
}

static void trs_chan_inst_release_work(struct work_struct *p_work)
{
    struct trs_chan *chan = container_of(p_work, struct trs_chan, release_work);

    _trs_chan_inst_release(chan);
}

static int trs_chan_inst_init(struct trs_chan_ts_inst *ts_inst, struct trs_chan_para *para, struct trs_chan *chan)
{
    int ret;

    chan->ts_inst = ts_inst;
    chan->inst = ts_inst->inst;
    chan->types = para->types;
    chan->ops = para->ops;
    chan->sq.para = para->sq_para;
    chan->cq.para = para->cq_para;
    chan->ssid = para->ssid;
    chan->pid = current->tgid;
    kref_init(&chan->ref);
    INIT_WORK(&chan->release_work, trs_chan_inst_release_work);
    spin_lock_init(&chan->lock);

    ret = memcpy_s(chan->msg, sizeof(chan->msg), para->msg, sizeof(para->msg));
    if (ret != 0) {
        trs_err("Memcopy failed. (dest_len=%lx; src_len=%lx)\n", sizeof(chan->msg), sizeof(para->msg));
        return -EFAULT;
    }

    if ((para->flag & (0x1 << CHAN_FLAG_USE_MASTER_PID_BIT)) != 0) {
        chan->flag |= (0x1 << CHAN_FLAG_USE_MASTER_PID_BIT);
    }

    if ((para->flag & (0x1 << CHAN_FLAG_NO_ALLOC_SQCQ_MEM_BIT)) != 0) {
        chan->flag |= (0x1 << CHAN_FLAG_NO_ALLOC_SQCQ_MEM_BIT);
    }

    if ((para->flag & (0x1 << CHAN_FLAG_ALLOC_SQ_BIT)) != 0) {
        ret = trs_chan_sq_init(ts_inst, chan);
        if (ret != 0) {
            return ret;
        }
        chan->flag |= (0x1 << CHAN_FLAG_ALLOC_SQ_BIT);
    }

    if ((para->flag & (0x1 << CHAN_FLAG_ALLOC_CQ_BIT)) != 0) {
        ret = trs_chan_cq_init(ts_inst, chan);
        if (ret != 0) {
            trs_chan_inst_uninit(ts_inst, chan);
            return ret;
        }
        chan->flag |= (0x1 << CHAN_FLAG_ALLOC_CQ_BIT);
        if ((para->flag & (0x1 << CHAN_FLAG_RECV_BLOCK_BIT)) != 0) {
            chan->flag |= (0x1 << CHAN_FLAG_RECV_BLOCK_BIT);
        }
    }

    if ((para->flag & (0x1 << CHAN_FLAG_NOTICE_TS_BIT)) != 0) {
        ret = trs_chan_notice_ts(chan, 1);
        if (ret != 0) {
            trs_chan_inst_uninit(ts_inst, chan);
            return ret;
        }
        chan->flag |= (0x1 << CHAN_FLAG_NOTICE_TS_BIT);
    }

    if ((para->flag & (0x1 << CHAN_FLAG_AUTO_UPDATE_SQ_HEAD_BIT)) != 0) {
        chan->flag |= (0x1 << CHAN_FLAG_AUTO_UPDATE_SQ_HEAD_BIT);
    }

    return 0;
}

static void trs_chan_id_alloc(struct trs_chan *chan)
{
    spin_lock_bh(&chan_idr_lock);
    chan->id = idr_alloc_cyclic(&chan_idr, chan, 0, MAX_CHAN_NUM, GFP_ATOMIC);
    spin_unlock_bh(&chan_idr_lock);
}

static void trs_chan_id_free(struct trs_chan *chan)
{
    spin_lock_bh(&chan_idr_lock);
    (void)idr_remove(&chan_idr, chan->id);
    spin_unlock_bh(&chan_idr_lock);
}

static int trs_chan_inst_create(struct trs_chan_ts_inst *ts_inst, struct trs_chan_para *para, int *chan_id)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    struct trs_chan *chan = NULL;
    int ret;

    chan = kzalloc(sizeof(struct trs_chan), GFP_KERNEL | __GFP_ACCOUNT);
    if (chan == NULL) {
        trs_err("Mem alloc failed. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -ENOMEM;
    }

    trs_chan_id_alloc(chan);
    if (chan->id < 0) {
        kfree(chan);
        trs_err("Alloc chan id failed. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -EFAULT;
    }

    ret = trs_chan_inst_init(ts_inst, para, chan);
    if (ret != 0) {
        trs_chan_id_free(chan);
        kfree(chan);
        return ret;
    }

    *chan_id = chan->id;
    chan_proc_fs_add_chan(chan);

    trs_debug("Create success. (devid=%u; tsid=%u; id=%d; type=%u; sub_type=%u; flag=%x)\n",
        inst->devid, inst->tsid, chan->id, chan->types.type, chan->types.sub_type, chan->flag);

    return 0;
}

static void trs_chan_inst_release(struct kref *kref)
{
    struct trs_chan *chan = container_of(kref, struct trs_chan, ref);

    if (in_softirq()) {
        trs_info("Release in soft irq. (chanId=%u; sqid=%u; cqid=%u)\n", chan->id, chan->sq.sqid, chan->cq.cqid);
        schedule_work(&chan->release_work);
        return;
    }

    _trs_chan_inst_release(chan);
}

static void trs_chan_inst_destroy(struct trs_chan *chan)
{
    struct trs_chan_ts_inst *ts_inst = chan->ts_inst;
    struct trs_id_inst *inst = &ts_inst->inst;

    trs_chan_id_free(chan);

    if (trs_chan_has_sq(chan)) {
        trs_chan_submit_wakeup(chan);
    }

    if (trs_chan_has_cq(chan)) {
        trs_chan_fetch_wakeup(chan);
    }

    trs_debug("Destroy success. (devid=%u; tsid=%u; id=%d; flag=%x)\n", inst->devid, inst->tsid, chan->id, chan->flag);
    kref_put(&chan->ref, trs_chan_inst_release);
}

struct trs_chan *trs_chan_get(struct trs_id_inst *inst, u32 chan_id)
{
    struct trs_chan *chan = NULL;

    if (trs_id_inst_check(inst) != 0) {
        return NULL;
    }

    spin_lock_bh(&chan_idr_lock);
    chan = (struct trs_chan *)idr_find(&chan_idr, chan_id);
    if (chan != NULL) {
        kref_get(&chan->ref);
    }
    spin_unlock_bh(&chan_idr_lock);

    return chan;
}

void trs_chan_put(struct trs_chan *chan)
{
    kref_put(&chan->ref, trs_chan_inst_release);
}

int hal_kernel_trs_chan_create(struct trs_id_inst *inst, struct trs_chan_para *para, int *chan_id)
{
    struct trs_chan_ts_inst *ts_inst = NULL;
    int ret;
    if ((para == NULL) || (chan_id == NULL)) {
        trs_err("Invalid para. (para=%pK; chan_id=%pK)\n", para, chan_id);
        return -EINVAL;
    }

    ret = trs_id_inst_check(inst);
    if (ret != 0) {
        return ret;
    }

    ts_inst = trs_chan_ts_inst_get(inst);
    if (ts_inst == NULL) {
        /* will be called by kworker */
        trs_debug("Invalid para. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -EINVAL;
    }

    mutex_lock(&chan_mutex);
    ret = trs_chan_inst_create(ts_inst, para, chan_id);
    mutex_unlock(&chan_mutex);
    if (ret != 0) { /* if success, put 'ts inst' in destroy */
        trs_chan_ts_inst_put(ts_inst);
    }

    return ret;
}
EXPORT_SYMBOL(hal_kernel_trs_chan_create);

static bool trs_chan_is_need_show(struct trs_chan *chan)
{
    return (chan->stat.hw_err != 0);
}

static void trs_chan_detail_show(struct trs_id_inst *inst, struct trs_chan *chan)
{
    char buff[512];  /* 512 Bytes */

    trs_info("Chan stat show. (devid=%u; tsid=%u; id=%u)\n", inst->devid, inst->tsid, chan->id);
    if (trs_chan_to_string(inst, chan->id, buff, 512) > 0) {  /* 512 Bytes */
        trs_info("%s\n", buff);
    }
}

void hal_kernel_trs_chan_destroy(struct trs_id_inst *inst, int chan_id)
{
    struct trs_chan_ts_inst *ts_inst = NULL;
    struct trs_chan *chan = NULL;

    mutex_lock(&chan_mutex);
    chan = trs_chan_get(inst, (u32)chan_id);
    if (chan == NULL) {
        mutex_unlock(&chan_mutex);
        trs_err("Invalid para. (chan_id=%d)\n", chan_id);
        return;
    }

    ts_inst = chan->ts_inst;
    if (trs_chan_is_need_show(chan)) {
        trs_chan_detail_show(inst, chan);
    }
    trs_chan_inst_destroy(chan);
    trs_chan_put(chan);
    mutex_unlock(&chan_mutex);
}
EXPORT_SYMBOL(hal_kernel_trs_chan_destroy);

int trs_chan_get_sq_info(struct trs_id_inst *inst, int chan_id, struct trs_chan_sq_info *info)
{
    struct trs_chan *chan = NULL;

    if (info == NULL) {
        trs_err("Null ptr. (chan_id=%d)\n", chan_id);
        return -EINVAL;
    }

    chan = trs_chan_get(inst, chan_id);
    if (chan == NULL) {
        trs_err("Invalid para. (chan_id=%d)\n", chan_id);
        return -EINVAL;
    }

    if (!trs_chan_has_sq(chan)) {
        trs_chan_put(chan);
        trs_err("Chan not has sq\n");
        return -EINVAL;
    }

    _trs_chan_get_sq_info(chan, info);
    trs_chan_put(chan);

    return 0;
}
EXPORT_SYMBOL(trs_chan_get_sq_info);

int trs_chan_get_chan_id(struct trs_id_inst *inst, int res_type, u32 res_id, int *chan_id)
{
    struct trs_chan_ts_inst *ts_inst = NULL;

    if ((chan_id == NULL) || ((res_type != TRS_HW_SQ) && (res_type != TRS_HW_CQ))) {
        trs_err("Null ptr. (res_type=%d; res_id=%u)\n", res_type, res_id);
        return -EINVAL;
    }

    ts_inst = trs_chan_ts_inst_get(inst);
    if (ts_inst == NULL) {
        trs_err("Invalid para. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -EINVAL;
    }

    if (res_type == TRS_HW_SQ) {
        *chan_id = trs_chan_sq_to_chan_id(ts_inst, res_id);
    } else {
        *chan_id = trs_chan_cq_to_chan_id(ts_inst, res_id);
    }
    if (*chan_id == -1) {
        trs_err("Invalid para. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        trs_chan_ts_inst_put(ts_inst);
        return -EINVAL;
    }
    trs_chan_ts_inst_put(ts_inst);

    return 0;
}
EXPORT_SYMBOL(trs_chan_get_chan_id);

int trs_chan_get_cq_info(struct trs_id_inst *inst, int chan_id, struct trs_chan_cq_info *info)
{
    struct trs_chan *chan = NULL;

    if (info == NULL) {
        trs_err("Null ptr. (chan_id=%d)\n", chan_id);
        return -EINVAL;
    }

    chan = trs_chan_get(inst, chan_id);
    if (chan == NULL) {
        trs_err("Invalid para. (chan_id=%d)\n", chan_id);
        return -EINVAL;
    }

    if (!trs_chan_has_cq(chan)) {
        trs_chan_put(chan);
        trs_err("Chan not has cq\n");
        return -EINVAL;
    }

    _trs_chan_get_cq_info(chan, info);
    trs_chan_put(chan);

    return 0;
}
EXPORT_SYMBOL(trs_chan_get_cq_info);
