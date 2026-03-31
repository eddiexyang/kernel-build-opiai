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

#include <linux/uaccess.h>
#include <linux/io.h>

#include <securec.h>
#include "soc_res.h"
#include "chan_init.h"
#include "chan_proc_fs.h"
#include "chan_trace.h"

/* sqe/cqe address Domain, user address or kernel address */
#define CHAN_ADDR_DOMAIN_KERNEL 0
#define CHAN_ADDR_DOMAIN_USER 1

static inline bool trs_chan_mem_is_local_mem(u32 attr)
{
    return ((attr & (0x1 << CHAN_MEM_FLAG_LOCAL)) != 0);
}

static void trs_chan_sqe_flush(struct trs_chan *chan, u8 *sqe)
{
    if (chan->ts_inst->ops.flush_cache != NULL) {
        u64 pa = chan->sq.sq_phy_addr + (u64)(sqe - (u8 *)chan->sq.sq_addr);
        chan->ts_inst->ops.flush_cache(&chan->inst, &chan->types, sqe, pa, chan->sq.para.sqe_size);
    }
}

static void trs_chan_cqe_invalid_cache(struct trs_chan *chan, u8 *cqe)
{
    if (chan->ts_inst->ops.invalid_cache != NULL) {
        u64 pa = chan->cq.cq_phy_addr + (u64)(cqe - (u8 *)chan->cq.cq_addr);
        chan->ts_inst->ops.invalid_cache(&chan->inst, &chan->types, cqe, pa, chan->cq.para.cqe_size);
    }
}

static int trs_chan_sqe_update(struct trs_chan *chan, u8 *sqe)
{
    int ret = 0;
    if ((chan->types.type == CHAN_TYPE_HW) && (chan->types.sub_type == CHAN_SUB_TYPE_HW_RTS)) {
        if (chan->ts_inst->ops.sqe_update != NULL) {
            ret = chan->ts_inst->ops.sqe_update(&chan->inst, chan->pid, chan->sq.sqid, chan->sq.sq_tail, sqe);
        }
    }
    return ret;
}

static void trs_chan_cqe_update(struct trs_chan *chan, u8 *cqe)
{
    if ((chan->types.type == CHAN_TYPE_HW) && (chan->types.sub_type == CHAN_SUB_TYPE_HW_RTS)) {
        if (chan->ts_inst->ops.cqe_update != NULL) {
            (void)chan->ts_inst->ops.cqe_update(&chan->inst, chan->pid, cqe);
        }
    }
}

static bool trs_chan_sq_is_full(struct trs_chan_sq_ctx *sq)
{
    return (((sq->sq_tail + 1) % sq->para.sq_depth) == sq->sq_head);
}

static int trs_chan_submit_wait(struct trs_chan *chan, u32 timeout)
{
    struct trs_id_inst *inst = &chan->ts_inst->inst;
    struct trs_chan_sq_ctx *sq = &chan->sq;
    long ret;

    ret = wait_event_interruptible_timeout(sq->wait_queue, !trs_chan_sq_is_full(sq), msecs_to_jiffies((u32)timeout));
    if (ret == 0) {
        return -ETIMEDOUT;
    } else if (ret < 0) {
        trs_warn("Wait fail. (devid=%u; tsid=%u; sqid=%u; ret=%ld)\n", inst->devid, inst->tsid, sq->sqid, ret);
        return (int)ret;
    } else {
        /* do nothing */
    }

    return 0;
}

void trs_chan_submit_wakeup(struct trs_chan *chan)
{
    struct trs_chan_sq_ctx *sq = &chan->sq;

    if (waitqueue_active(&sq->wait_queue) != 0) {
        wake_up_interruptible(&sq->wait_queue);
    }
}

static int trs_chan_update_sq_head_from_hw(struct trs_chan *chan)
{
    if (chan->types.type == CHAN_TYPE_HW) {
        struct trs_id_inst *inst = &chan->ts_inst->inst;
        u64 sq_head;
        int ret = chan->ts_inst->ops.sqcq_query(inst, &chan->types, chan->sq.sqid, QUERY_CMD_SQ_HEAD, &sq_head);
        if (ret == 0) {
            chan->sq.sq_head = (u32)sq_head;
            return 0;
        }
    }

    return -1;
}

static bool trs_chan_is_full(struct trs_chan *chan)
{
    if (!trs_chan_sq_is_full(&chan->sq)) {
        return false;
    }
    (void)trs_chan_update_sq_head_from_hw(chan);
    return trs_chan_sq_is_full(&chan->sq);
}

static int trs_chan_fill_sqe(struct trs_chan *chan, u8 *sqe, int timeout, int addr_domain)
{
    struct trs_id_inst *inst = &chan->ts_inst->inst;
    struct trs_chan_sq_ctx *sq = &chan->sq;
    u8 *dst_addr = (u8 *)sq->sq_addr + sq->para.sqe_size * sq->sq_tail;
    u8 sqe_tmp[SQE_CACHE_SIZE];
    u8 *sqe_addr = NULL;
    unsigned long ret_cpy;
    int ret;

    if (trs_chan_is_full(chan)) {
        chan->stat.tx_full++;
        if (timeout == 0) {
            trs_debug("Sq is full. (devid=%u; tsid=%u; sqid=%u)\n", inst->devid, inst->tsid, chan->sq.sqid);
            return -ENOSPC;
        }
        ret = trs_chan_submit_wait(chan, 1);
        if (ret < 0) {
            return ret;
        }
        if (trs_chan_is_full(chan)) {
            return -ETIMEDOUT;
        }
    }

    /* if using bar to r/w sqe, it should use stack value to store sqe to avoid waster time */
    sqe_addr = trs_chan_mem_is_local_mem(sq->attr) ? dst_addr : sqe_tmp;

    if (addr_domain == CHAN_ADDR_DOMAIN_KERNEL) {
        memcpy_s(sqe_addr, sq->para.sqe_size, sqe, sq->para.sqe_size);
    } else {
        ret_cpy = copy_from_user(sqe_addr, sqe, sq->para.sqe_size);
        if (ret_cpy != 0) {
            trs_err("Copy fail. (devid=%u; tsid=%u; sqid=%u; ret=%lu)\n", inst->devid, inst->tsid, sq->sqid, ret_cpy);
            return -EINVAL;
        }
    }

    ret = trs_chan_sqe_update(chan, sqe_addr);
    if (ret != 0) {
        trs_warn("Update fail. (devid=%u; tsid=%u; sqid=%u; ret=%d)\n", inst->devid, inst->tsid, sq->sqid, ret);
        return ret;
    }

    if (!trs_chan_mem_is_local_mem(sq->attr)) {
        memcpy_toio(dst_addr, sqe_addr, sq->para.sqe_size);
    }

    trs_chan_sqe_flush(chan, dst_addr);
    sq->sq_tail = (sq->sq_tail + 1) % sq->para.sq_depth;
    trs_chan_trace_sqe("Task Send", chan, sq, sqe_addr);
    return 0;
}

static int trs_chan_submit(struct trs_chan *chan, struct trs_chan_send_para *para, int addr_domain)
{
    struct trs_id_inst *inst = &chan->ts_inst->inst;
    struct trs_chan_sq_ctx *sq = &chan->sq;
    u32 sq_tail = sq->sq_tail;
    u32 send_sqe_num = 0;
    int ret;

    while (send_sqe_num < para->sqe_num) {
        ret = trs_chan_fill_sqe(chan, para->sqe + sq->para.sqe_size * send_sqe_num, para->timeout, addr_domain);
        if (ret == 0) {
            send_sqe_num++;
            continue;
        } else if (ret != -ETIMEDOUT) {
            return ret;
        } else {
            /* do nothing */
        }

        if ((para->timeout != -1)) {
            if (--para->timeout < 0) {
                trs_debug("Wait timeout. (devid=%u; tsid=%u; sqid=%u)\n", inst->devid, inst->tsid, sq->sqid);
                return -ENOSPC;
            }
        }
    }

    wmb();
    ret = chan->ts_inst->ops.sqcq_ctrl(inst, &chan->types, sq->sqid, CTRL_CMD_SQ_TAIL_UPDATE, sq->sq_tail);
    if (ret != 0) {
        sq->sq_tail = sq_tail;
        trs_err("Sq tail update fail. (devid=%u; tsid=%u; sqid=%u; ret=%d)\n", inst->devid, inst->tsid, sq->sqid, ret);
        return ret;
    }

    chan->stat.tx += para->sqe_num;

    return 0;
}

static int trs_chan_sem_down(struct trs_chan *chan, int timeout)
{
    int ret;

    if (timeout > 0) {
        ret = down_timeout(&chan->sq.sem, msecs_to_jiffies((u32)timeout));
    } else {
        ret = down_interruptible(&chan->sq.sem);
    }

    return ret;
}

static void trs_chan_sem_up(struct trs_chan *chan)
{
    up(&chan->sq.sem);
}

static int trs_chan_send_ex(struct trs_id_inst *inst, int chan_id, struct trs_chan_send_para *para, int addr_domain)
{
    struct trs_chan *chan = NULL;
    int ret;

    if ((para == NULL) || (para->sqe == NULL)) {
        trs_err("Null ptr. (chan_id=%d; domain=%d, inst=%pK; para=%pK)\n", chan_id, addr_domain, inst, para);
        return -EINVAL;
    }

    chan = trs_chan_get(inst, chan_id);
    if (chan == NULL) {
        trs_err("Invalid para. (chan_id=%d)\n", chan_id);
        return -EINVAL;
    }

    if (!trs_chan_has_sqcq_mem(chan)) {
        trs_chan_put(chan);
        trs_err("Unauthorized operation. (devid=%u; tsid=%u; chan_id=%d)\n", inst->devid, inst->tsid, chan_id);
        return -EPERM;
    }

    if (chan->sq.status == 0) {
        trs_chan_put(chan);
        return -EPERM;
    }

    if (in_softirq()) {
        spin_lock_bh(&chan->sq.lock);   /* submit task in tasklet */
        para->timeout = 0;  /* to avoid into wait event, if sq is full. */
    } else {
        ret = trs_chan_sem_down(chan, para->timeout);
        if (ret != 0) {
            trs_warn("Down fail. (devid=%u; tsid=%u; sqid=%u; ret=%d)\n",
                chan->ts_inst->inst.devid, chan->ts_inst->inst.tsid, chan->sq.sqid, ret);
            chan->stat.tx_timeout++;
            trs_chan_put(chan);
            return ret;
        }
    }

    para->first_pos = chan->sq.sq_tail;
    ret = trs_chan_submit(chan, para, addr_domain);

    if (in_softirq()) {
        spin_unlock_bh(&chan->sq.lock);
    } else {
        trs_chan_sem_up(chan);
    }
    trs_chan_put(chan);

    return ret;
}

int trs_chan_send(struct trs_id_inst *inst, int chan_id, struct trs_chan_send_para *para)
{
    return trs_chan_send_ex(inst, chan_id, para, CHAN_ADDR_DOMAIN_USER);
}
EXPORT_SYMBOL(trs_chan_send);

int hal_kernel_trs_chan_send(struct trs_id_inst *inst, int chan_id, struct trs_chan_send_para *para)
{
    return trs_chan_send_ex(inst, chan_id, para, CHAN_ADDR_DOMAIN_KERNEL);
}
EXPORT_SYMBOL(hal_kernel_trs_chan_send);

static int trs_chan_update_sq_head(struct trs_chan *chan, u32 sq_head)
{
    if (sq_head >= chan->sq.para.sq_depth) {
        trs_warn("Invalid head. (devid=%u; tsid=%u; sqid=%u; sq_head=%u; sq_depth=%u)\n",
            chan->ts_inst->inst.devid, chan->ts_inst->inst.tsid, chan->sq.sqid, sq_head, chan->sq.para.sq_depth);
        return -EINVAL;
    }

    chan->sq.sq_head = sq_head;
    smp_wmb();
    trs_chan_submit_wakeup(chan);

    return 0;
}

static int trs_chan_set_sq_head(struct trs_chan *chan, u32 sq_head)
{
    int ret;

    if (sq_head < chan->sq.para.sq_depth) {
        ret = chan->ts_inst->ops.sqcq_ctrl(&chan->ts_inst->inst, &chan->types, chan->sq.sqid,
            CTRL_CMD_SQ_HEAD_UPDATE, sq_head);
        if (ret != 0) {
            return ret;
        }
    }

    return trs_chan_update_sq_head(chan, sq_head);
}

static int trs_chan_set_sq_status(struct trs_chan *chan, u32 status)
{
    int ret = chan->ts_inst->ops.sqcq_ctrl(&chan->ts_inst->inst, &chan->types, chan->sq.sqid,
        CTRL_CMD_SQ_STATUS_SET, status);
    if (ret == 0) {
        chan->sq.status = status;
    }

    return ret;
}

static int trs_chan_sq_disable_to_enable(struct trs_chan *chan, u32 timeout)
{
    int ret = chan->ts_inst->ops.sqcq_ctrl(&chan->ts_inst->inst, &chan->types, chan->sq.sqid,
        CTRL_CMD_SQ_DISABLE_TO_ENABLE, timeout);
    if (ret == 0) {
        chan->sq.status = 1;
    }

    return ret;
}

static bool trs_chan_is_valid_cqe(struct trs_chan *chan, void *cqe, u32 loop)
{
    bool is_valid = (chan->ops.cqe_is_valid != NULL) ?
        chan->ops.cqe_is_valid(cqe, loop) : chan->ts_inst->ops.cqe_is_valid(&chan->ts_inst->inst, cqe, loop);

    rmb();

    return is_valid;
}

static void trs_chan_update_cq_head(struct trs_chan_cq_ctx *cq)
{
    cq->cq_head = (cq->cq_head + 1) % cq->para.cq_depth;
    if (cq->cq_head == 0) {
        cq->loop++;
    }
}

static void trs_chan_get_sq_head_in_cqe(struct trs_chan *chan, void *cqe, u32 *sq_head)
{
    if (chan->ops.get_sq_head_in_cqe != NULL) {
        chan->ops.get_sq_head_in_cqe(cqe, sq_head);
    } else {
        chan->ts_inst->ops.get_sq_head_in_cqe(&chan->ts_inst->inst, cqe, sq_head);
    }
}

static int trs_chan_fetch_wait(struct trs_chan *chan, u32 timeout)
{
    struct trs_id_inst *inst = &chan->ts_inst->inst;
    struct trs_chan_cq_ctx *cq = &chan->cq;
    long tm, ret;

    if (timeout == 0) {
        return -ETIMEDOUT;
    }

    tm = (timeout == -1) ? MAX_SCHEDULE_TIMEOUT : msecs_to_jiffies((u32)timeout);

    ret = wait_event_interruptible_timeout(cq->wait_queue, (cq->cqe_valid == 1), tm);
    if (ret == 0) {
        trs_warn("Wait timeout. (devid=%u; tsid=%u; type=%u; cqid=%u; timeout=%u)\n",
            inst->devid, inst->tsid, chan->types.type, cq->cqid, timeout);
        return -ETIMEDOUT;
    } else if (ret < 0) {
        trs_warn("Wait fail. (devid=%u; tsid=%u; cqid=%u; ret=%ld)\n", inst->devid, inst->tsid, cq->cqid, ret);
        return (int)ret;
    } else {
        /* do nothing */
    }

    cq->cqe_valid = 0;

    return 0;
}

void trs_chan_fetch_wakeup(struct trs_chan *chan)
{
    struct trs_chan_cq_ctx *cq = &chan->cq;

    cq->cqe_valid = 1;

    smp_wmb();

    if (waitqueue_active(&cq->wait_queue) != 0) {
        chan->stat.rx_wakeup++;
        wake_up_interruptible(&cq->wait_queue);
    }
}

static int trs_chan_fetch(struct trs_chan *chan, u8 *cqe, int addr_domain, u32 timeout)
{
    struct trs_chan_cq_ctx *cq = &chan->cq;
    u8 *cq_addr = NULL;
    int ret;

    mutex_lock(&cq->mutex);
    cq_addr = (u8 *)cq->cq_addr + cq->para.cqe_size * cq->cq_head;
    trs_chan_cqe_invalid_cache(chan, cq_addr);
    while (!trs_chan_is_valid_cqe(chan, cq_addr, cq->loop)) {
        chan->stat.rx_empty++;
        mutex_unlock(&cq->mutex);
        ret = trs_chan_fetch_wait(chan, timeout);
        if (ret != 0) {
            return ret;
        }
        mutex_lock(&cq->mutex);
        cq_addr = (u8 *)cq->cq_addr + cq->para.cqe_size * cq->cq_head;
        trs_chan_cqe_invalid_cache(chan, cq_addr);
    }

    trs_chan_cqe_update(chan, cq_addr);

    if (addr_domain == CHAN_ADDR_DOMAIN_KERNEL) {
        memcpy_fromio(cqe, cq_addr, cq->para.cqe_size);
    } else {
        if (((u64)cq_addr % sizeof(u8 *)) != 0) {
            /* copy_to_user need addr 8 byte align */
            ret = put_user(*(u32 *)cq_addr, (u32 *)cqe);
            ret |= copy_to_user(cqe + CQE_ALIGN_SIZE, (void *)(cq_addr + CQE_ALIGN_SIZE),
                cq->para.cqe_size - CQE_ALIGN_SIZE);
        } else {
            ret = copy_to_user(cqe, (void *)cq_addr, cq->para.cqe_size);
        }
        if (ret != 0) {
            mutex_unlock(&cq->mutex);
            trs_err("Copy fail. (devid=%u; tsid=%u; cqid=%u; ret=%d)\n",
                chan->ts_inst->inst.devid, chan->ts_inst->inst.tsid, cq->cqid, ret);
            return ret;
        }
    }

    trs_chan_update_cq_head(cq);
    trs_chan_trace_cqe("Task Recv", chan, cq, cq_addr);
    chan->stat.rx++;
    mutex_unlock(&cq->mutex);

    return 0;
}

static int trs_chan_recv_ex(struct trs_id_inst *inst, int chan_id, struct trs_chan_recv_para *para, int addr_domain)
{
    struct trs_chan *chan = NULL;
    u32 timeout;
    int ret = 0;

    if ((para == NULL) || (para->cqe == NULL)) {
        trs_err("Null ptr. (chan_id=%d; domain=%d, inst=%pK; para=%pK)\n", chan_id, addr_domain, inst, para);
        return -EINVAL;
    }
    chan = trs_chan_get(inst, chan_id);
    if (chan == NULL) {
        trs_err("Invalid para. (chan_id=%d)\n", chan_id);
        return -EINVAL;
    }

    if (!trs_chan_has_sqcq_mem(chan)) {
        trs_chan_put(chan);
        trs_err("Unauthorized operation. (devid=%u; tsid=%u; chan_id=%d)\n", inst->devid, inst->tsid, chan_id);
        return -EPERM;
    }

    para->recv_cqe_num = 0;
    timeout = para->timeout;
    trace_chan_trace_recv("Recv start", chan, para);
    while (para->recv_cqe_num < para->cqe_num) {
        ret = trs_chan_fetch(chan, para->cqe + chan->cq.para.cqe_size * para->recv_cqe_num, addr_domain, timeout);
        if (ret != 0) {
            break;
        }
        para->recv_cqe_num++;
        timeout = 0;
    }

    mb();

    if (para->recv_cqe_num > 0) {
        chan->ts_inst->ops.sqcq_ctrl(inst, &chan->types, chan->cq.cqid, CTRL_CMD_CQ_HEAD_UPDATE, chan->cq.cq_head);
        ret = 0;
    }
    trace_chan_trace_recv("Recv finish", chan, para);
    trs_chan_put(chan);

    return ret;
}

int trs_chan_recv(struct trs_id_inst *inst, int chan_id, struct trs_chan_recv_para *para)
{
    return trs_chan_recv_ex(inst, chan_id, para, CHAN_ADDR_DOMAIN_USER);
}
EXPORT_SYMBOL(trs_chan_recv);

int hal_kernel_trs_chan_recv(struct trs_id_inst *inst, int chan_id, struct trs_chan_recv_para *para)
{
    return trs_chan_recv_ex(inst, chan_id, para, CHAN_ADDR_DOMAIN_KERNEL);
}
EXPORT_SYMBOL(hal_kernel_trs_chan_recv);

static inline void trs_chan_try_to_update_sq_head(struct trs_chan *chan, u8 *cq_addr)
{
    u32 sq_head;

    if (trs_chan_has_sq(chan) && trs_chan_is_auto_update_sq_head(chan)) {
        if (trs_chan_update_sq_head_from_hw(chan) < 0) {
            trs_chan_get_sq_head_in_cqe(chan, cq_addr, &sq_head);
            (void)trs_chan_update_sq_head(chan, sq_head);
        }
    }
}

static void trs_chan_sched(struct trs_chan *chan)
{
    if (trs_chan_is_recv_block(chan)) {
        schedule_work(&chan->work);
    } else {
        tasklet_schedule(&chan->irq_ctx->task);
    }
}

static void trs_chan_cq_dispatch(struct trs_chan *chan, u32 dispatch_num)
{
    struct trs_chan_cq_ctx *cq = &chan->cq;
    const int max_resched_times = 100; /* max is 100 */
    int recv_num = 0;
    u32 proc_num = 0;
    u8 *cq_addr = NULL;
    u8 report[CQE_CACHE_SIZE];
    int ret;

    chan->stat.rx_in++;

    if (!trs_chan_has_sqcq_mem(chan)) {
        trs_debug("Unauthorized operation. (devid=%u; tsid=%u; cq_id=%d)\n",
            chan->ts_inst->inst.devid, chan->ts_inst->inst.tsid, chan->cq.cqid);
        return;
    }

    while (1) {
        cq_addr = (u8 *)cq->cq_addr + cq->para.cqe_size * cq->cq_head;
        trs_chan_cqe_invalid_cache(chan, cq_addr);

        if (trs_chan_mem_is_local_mem(cq->attr) == false) { /* using bar to r/w cqe waster time, so add a stack value */
            ret = memcpy_s(report, sizeof(report), cq_addr, cq->para.cqe_size);
            if (ret != EOK) {
                trs_err("Failed to invoke function [memcpy_s]. (devid=%u; tsid=%u; cq_id=%d)\n",
                    chan->ts_inst->inst.devid, chan->ts_inst->inst.tsid, chan->cq.cqid);
                break;
            }
            cq_addr = report;
        }

        if (!trs_chan_is_valid_cqe(chan, cq_addr, cq->loop)) {
#ifndef EMU_ST  /* if delete, the emu st will into and sched tasklet, which causes dispatch grab the spin_lock */
            if (chan->types.type == CHAN_TYPE_HW) {
                struct trs_chan_adapt_ops *ops = &chan->ts_inst->ops;
                u64 cq_tail;
                ops->sqcq_query(&chan->inst, &chan->types, chan->cq.cqid, QUERY_CMD_CQ_TAIL, &cq_tail);
                if ((cq_tail != chan->cq.cq_head) && (chan->retry_times < max_resched_times)) {
                    trs_chan_sched(chan);
                    chan->retry_times++;
                    if (chan->retry_times == max_resched_times) {
                        chan->stat.hw_err++;
                    }
                }
            }
#endif
            break;
        }
        chan->retry_times = 0;

        proc_num++;
        if (proc_num == dispatch_num) {
            schedule_work(&chan->work);
            break;
        }

        trs_chan_cqe_update(chan, cq_addr);

        chan->stat.rx_dispatch++;

        trs_chan_try_to_update_sq_head(chan, cq_addr);

        trs_chan_trace_cqe("Cqe Dispatch", chan, cq, cq_addr);
        ret = CQ_RECV_FINISH;
        if (chan->ops.cq_recv != NULL) {
            ret = chan->ops.cq_recv(&chan->ts_inst->inst, cq->cqid, cq_addr);
        }
        if (ret == CQ_RECV_FINISH) {
            trs_chan_update_cq_head(cq);
            recv_num++;
        } else {
            trs_chan_fetch_wakeup(chan);
            break; /* stop traversal, wait for thread to fetch */
        }
    }

    mb();

    if (recv_num > 0) {
        chan->ts_inst->ops.sqcq_ctrl(&chan->ts_inst->inst, &chan->types, chan->cq.cqid,
            CTRL_CMD_CQ_HEAD_UPDATE, cq->cq_head);
    }
}

static void trs_chan_cq_dispatch_non_block(struct trs_chan *chan)
{
    if (spin_trylock_bh(&chan->lock) == 0) {
        return;
    }

    trs_chan_cq_dispatch(chan, 64); /* 64 for tasklet reduce time consuming */

    spin_unlock_bh(&chan->lock);
}

static void trs_chan_cq_dispatch_block(struct trs_chan *chan)
{
    trs_chan_cq_dispatch(chan, chan->cq.para.cq_depth + 1);
}

static void trs_chan_irq_proc_cqs(struct trs_chan_ts_inst *ts_inst, u32 cqid[], u32 cq_num)
{
    u32 i;
    unsigned long timeout = jiffies + (3 * HZ);

    for (i = 0; i < cq_num; i++) {
        struct trs_chan *chan = NULL;
        int chan_id;

        if (cqid[i] == U32_MAX) {
            continue;   /* in mia. if cqid equals U32_MAX, it means the cqid is belong to vf. */
        }
        chan_id = trs_chan_cq_to_chan_id(ts_inst, cqid[i]);
        chan = trs_chan_get(&ts_inst->inst, (u32)chan_id);
        if (chan != NULL) {
            if (time_after(jiffies, timeout)) {
                schedule_work(&chan->work);
            } else {
                trs_chan_cq_dispatch_non_block(chan);
            }
            trs_chan_put(chan);
        } else {
            trs_debug("Chan failed. (devid=%u; tsid=%u; cqid=%u; chan_id=%d)\n",
                ts_inst->inst.devid, ts_inst->inst.tsid, cqid[i], chan_id);
        }
    }
}

void trs_chan_work(struct work_struct *p_work)
{
    struct trs_chan *chan = container_of(p_work, struct trs_chan, work);

    if (!trs_chan_is_recv_block(chan)) {
        trs_chan_cq_dispatch_non_block(chan);
    } else {
        trs_chan_cq_dispatch_block(chan);
    }
}

static void trs_chan_irq_proc_list(struct trs_chan_irq_ctx *irq_ctx)
{
    struct trs_chan *chan = NULL;
    unsigned long timeout = jiffies + (3 * HZ);

    spin_lock_bh(&irq_ctx->lock);
    list_for_each_entry(chan, &irq_ctx->chan_list, node) {
        if (!trs_chan_is_recv_block(chan) && time_before(jiffies, timeout)) {
            trs_chan_cq_dispatch_non_block(chan);
        } else {
            schedule_work(&chan->work);
        }
    }
    spin_unlock_bh(&irq_ctx->lock);
}

void trs_chan_irq_proc(int irq_type, int irq_index, void *para, u32 cqid[], u32 cq_num)
{
    struct trs_chan_irq_ctx *irq_ctx = (struct trs_chan_irq_ctx *)para;

    if ((cqid != NULL) && (cq_num > 0)) {
        trs_chan_irq_proc_cqs(irq_ctx->ts_inst, cqid, cq_num);
    } else {
        trs_chan_irq_proc_list(irq_ctx);
    }
}

void trs_chan_tasklet(unsigned long data)
{
    struct trs_chan_irq_ctx *irq_ctx = (struct trs_chan_irq_ctx *)data;
    trs_chan_irq_proc_list(irq_ctx);
}

int trs_chan_ctrl(struct trs_id_inst *inst, int chan_id, u32 cmd, u32 para)
{
    struct trs_chan *chan = trs_chan_get(inst, chan_id);
    int ret;

    if (chan == NULL) {
        trs_err("Invalid para. (chan_id=%d; cmd=%u)\n", chan_id, cmd);
        return -EINVAL;
    }

    if (((cmd == CHAN_CTRL_CMD_CQ_SCHED) && !trs_chan_has_cq(chan)) ||
        ((cmd != CHAN_CTRL_CMD_CQ_SCHED) && !trs_chan_has_sq(chan))) {
        trs_chan_put(chan);
        trs_err("Chan no sq or cq. (devid=%u; tsid=%u, chan_id=%d; cmd=%u)\n", inst->devid, inst->tsid, chan_id, cmd);
        return -EINVAL;
    }

    switch (cmd) {
        case CHAN_CTRL_CMD_SQ_HEAD_UPDATE:
            ret = trs_chan_update_sq_head(chan, para);
            break;
        case CHAN_CTRL_CMD_SQ_HEAD_SET:
            ret = trs_chan_set_sq_head(chan, para);
            break;
        case CHAN_CTRL_CMD_SQ_STATUS_SET:
            ret = trs_chan_set_sq_status(chan, para);
            break;
        case CHAN_CTRL_CMD_SQ_DISABLE_TO_ENABLE:
            ret = trs_chan_sq_disable_to_enable(chan, para);
            break;
        case CHAN_CTRL_CMD_CQ_SCHED:
            trs_chan_sched(chan);
            ret = 0;
            break;
        case CHAN_CTRL_CMD_NOT_NOTICE_TS:
            trs_chan_set_not_notice_ts(chan);
            ret = 0;
            break;
        default:
            trs_err("Invalid cmd. (devid=%u; tsid=%u, chan_id=%d; cmd=%u)\n", inst->devid, inst->tsid, chan_id, cmd);
            ret = -EINVAL;
            break;
    }

    trs_chan_put(chan);

    return ret;
}
EXPORT_SYMBOL(trs_chan_ctrl);

int trs_chan_query(struct trs_id_inst *inst, int chan_id, u32 cmd, u32 *value)
{
    struct trs_chan *chan = trs_chan_get(inst, chan_id);
    u64 tmp;
    int ret = 0;

    if (chan == NULL) {
        trs_err("Invalid para. (chan_id=%d; cmd=%u)\n", chan_id, cmd);
        return -EINVAL;
    }

    if (!trs_chan_has_sq(chan)) {
        trs_chan_put(chan);
        trs_err("Chan no sq. (devid=%u; tsid=%u, chan_id=%d; cmd=%u)\n", inst->devid, inst->tsid, chan_id, cmd);
        return -EINVAL;
    }

    switch (cmd) {
        case CHAN_QUERY_CMD_SQ_STATUS:
            ret = chan->ts_inst->ops.sqcq_query(inst, &chan->types, chan->sq.sqid, QUERY_CMD_SQ_STATUS, &tmp);
            *value = (u32)tmp;
            break;
        case CHAN_QUERY_CMD_SQ_HEAD:
            (void)trs_chan_update_sq_head_from_hw(chan);
            *value = chan->sq.sq_head;
            break;
        case CHAN_QUERY_CMD_SQ_TAIL:
            *value = chan->sq.sq_tail;
            break;
        default:
            trs_err("Invalid cmd. (devid=%u; tsid=%u, chan_id=%d; cmd=%u)\n", inst->devid, inst->tsid, chan_id, cmd);
            ret = -EINVAL;
            break;
    }

    trs_chan_put(chan);

    return ret;
}
EXPORT_SYMBOL(trs_chan_query);

int _trs_chan_to_string(struct trs_chan *chan, char *buff, u32 buff_len)
{
    struct trs_chan_adapt_ops *ops = &chan->ts_inst->ops;
    int len;

    len = sprintf_s(buff, buff_len, "    chan details:\n");

    if (trs_chan_has_sq(chan)) {
        len += sprintf_s(buff + len, buff_len - len,
            "    sq: id(%u),status(%u),head(%u),tail(%u),sqe_size(%u),sq_depth(%u)\n"
            "      stat: tx(%llu),tx_full(%llu),tx_timeout(%llu)\n",
            chan->sq.sqid, chan->sq.status, chan->sq.sq_head, chan->sq.sq_tail, chan->sq.para.sqe_size,
            chan->sq.para.sq_depth, chan->stat.tx, chan->stat.tx_full, chan->stat.tx_timeout);
    }

    if (trs_chan_has_cq(chan)) {
        len += sprintf_s(buff + len, buff_len - len,
            "    cq: id(%u),head(%u),loop(%u),cqe_size(%u),cq_depth(%u)\n"
            "      stat: rx(%llu),rx_empty(%llu),rx_dispatch(%llu),rx_wakeup(%llu),hw_err(%llu), rx_in(%llu)\n",
            chan->cq.cqid, chan->cq.cq_head, chan->cq.loop, chan->cq.para.cqe_size, chan->cq.para.cq_depth,
            chan->stat.rx, chan->stat.rx_empty, chan->stat.rx_dispatch, chan->stat.rx_wakeup, chan->stat.hw_err,
            chan->stat.rx_in);
    }

    if (chan->types.type == CHAN_TYPE_HW) {
        u64 sq_head, sq_tail, cq_head, cq_tail, sq_status;
        int ret;

        len += sprintf_s(buff + len, buff_len - len, "    hw info: ");

        ret = ops->sqcq_query(&chan->inst, &chan->types, chan->sq.sqid, QUERY_CMD_SQ_STATUS, &sq_status);
        if (ret == 0) {
            len += sprintf_s(buff + len, buff_len - len, "sq status %llu ", sq_status);
        }

        ret = ops->sqcq_query(&chan->inst, &chan->types, chan->sq.sqid, QUERY_CMD_SQ_HEAD, &sq_head);
        if (ret == 0) {
            len += sprintf_s(buff + len, buff_len - len, "sq head %llu ", sq_head);
        }

        ret = ops->sqcq_query(&chan->inst, &chan->types, chan->sq.sqid, QUERY_CMD_SQ_TAIL, &sq_tail);
        if (ret == 0) {
            len += sprintf_s(buff + len, buff_len - len, "sq tail %llu ", sq_tail);
        }

        ret = ops->sqcq_query(&chan->inst, &chan->types, chan->cq.cqid, QUERY_CMD_CQ_HEAD, &cq_head);
        if (ret == 0) {
            len += sprintf_s(buff + len, buff_len - len, "cq head %llu ", cq_head);
        }

        ret = ops->sqcq_query(&chan->inst, &chan->types, chan->cq.cqid, QUERY_CMD_CQ_TAIL, &cq_tail);
        if (ret == 0) {
            len += sprintf_s(buff + len, buff_len - len, "cq tail %llu ", cq_tail);
        }

        len += sprintf_s(buff + len, buff_len - len, "\n");
    }

    return len;
}

int trs_chan_to_string(struct trs_id_inst *inst, int chan_id, char *buff, u32 buff_len)
{
    struct trs_chan *chan = trs_chan_get(inst, chan_id);
    int len;

    if (chan == NULL) {
        trs_err("Invalid para. (chan_id=%d)\n", chan_id);
        return -EINVAL;
    }

    len = _trs_chan_to_string(chan, buff, buff_len);

    trs_chan_put(chan);

    return len;
}
EXPORT_SYMBOL(trs_chan_to_string);
