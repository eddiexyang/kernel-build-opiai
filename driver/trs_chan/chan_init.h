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

#ifndef CHAN_INIT_H
#define CHAN_INIT_H

#include <linux/workqueue.h>
#include <linux/spinlock.h>

#include "chan_ts_inst.h"

struct trs_chan_sq_ctx {
    u32 sqid;
    u32 status;
    u32 sq_head;
    u32 sq_tail;
    struct semaphore sem;
    u32 attr;   /* output para */
    void *sq_addr;
    u64 sq_phy_addr;
    u64 head_addr;
    u64 tail_addr;
    u64 db_addr;
    struct trs_chan_sq_para para;
    wait_queue_head_t wait_queue;
    spinlock_t lock;    /* submit task in tasklet */
};

struct trs_chan_cq_ctx {
    u32 cqid;
    u32 loop;
    u32 cq_head;
    u32 cqe_valid;
    u32 attr;   /* output para */
    void *cq_addr;
    u64 cq_phy_addr;
    struct trs_chan_cq_para para;
    struct mutex mutex;
    wait_queue_head_t wait_queue;
};

struct trs_chan_stat {
    u64 tx;
    u64 rx;
    u64 tx_full;
    u64 rx_empty;
    u64 tx_timeout;
    u64 rx_dispatch;
    u64 rx_wakeup;
    u64 hw_err;
    u64 rx_in;
};

struct trs_chan {
    int id;
    struct kref ref;
    u32 flag;
    u32 irq;
    int pid;
    int ssid;
    u32 msg[SQCQ_INFO_LENGTH];
    struct trs_chan_ts_inst *ts_inst;
    struct trs_chan_irq_ctx *irq_ctx;
    struct list_head node;
    struct trs_id_inst inst;
    struct trs_chan_type types;
    struct trs_chan_ops ops;
    struct trs_chan_sq_ctx sq;
    struct trs_chan_cq_ctx cq;
    struct trs_chan_stat stat;
    struct work_struct work;
    struct work_struct release_work;
    struct proc_dir_entry *entry;
    spinlock_t lock;    /* for cq dispatch */

    u32 retry_times;
};

static inline bool trs_chan_has_sq(struct trs_chan *chan)
{
    return ((chan->flag & (0x1 << CHAN_FLAG_ALLOC_SQ_BIT)) != 0);
}

static inline bool trs_chan_has_cq(struct trs_chan *chan)
{
    return ((chan->flag & (0x1 << CHAN_FLAG_ALLOC_CQ_BIT)) != 0);
}

static inline bool trs_chan_is_notice_ts(struct trs_chan *chan)
{
    return ((chan->flag & (0x1 << CHAN_FLAG_NOTICE_TS_BIT)) != 0);
}

static inline bool trs_chan_is_auto_update_sq_head(struct trs_chan *chan)
{
    return ((chan->flag & (0x1 << CHAN_FLAG_AUTO_UPDATE_SQ_HEAD_BIT)) != 0);
}

static inline bool trs_chan_is_recv_block(struct trs_chan *chan)
{
    return ((chan->flag & (0x1 << CHAN_FLAG_RECV_BLOCK_BIT)) != 0);
}

static inline void trs_chan_set_not_notice_ts(struct trs_chan *chan)
{
    chan->flag &= ~(0x1 << CHAN_FLAG_NOTICE_TS_BIT);
}

static inline bool trs_chan_has_sqcq_mem(struct trs_chan *chan)
{
    return ((chan->flag & (0x1 << CHAN_FLAG_NO_ALLOC_SQCQ_MEM_BIT)) == 0);
}

struct trs_chan *trs_chan_get(struct trs_id_inst *inst, u32 chan_id);
void trs_chan_put(struct trs_chan *chan);

#endif

