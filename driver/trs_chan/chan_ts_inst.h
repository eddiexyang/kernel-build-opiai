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
* Create: 2022-7-15
*/

#ifndef NVME_CHAN_H
#define NVME_CHAN_H

#include <linux/types.h>
#include <linux/kref.h>
#include <linux/idr.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>

#include "trs_chan.h"

struct trs_chan_ts_inst;

struct trs_chan_irq_ctx {
    struct trs_chan_ts_inst *ts_inst;
    u32 irq_type;
    u32 irq_index;
    u32 irq;
    u32 chan_num;
    spinlock_t lock;
    struct list_head chan_list;
    struct tasklet_struct task;
};

struct trs_chan_hw_sq_ctx {
    int chan_id;
};

struct trs_chan_hw_cq_ctx {
    int chan_id;
    u32 irq_index;
};

struct trs_chan_ts_inst {
    struct kref ref;
    spinlock_t lock;
    struct idr chan_idr;
    int hw_type;
    u32 chan_num;
    u32 maint_irq_num;
    u32 normal_irq_num;
    u32 sq_max_id;
    u32 cq_max_id;
    struct trs_id_inst inst;
    struct trs_chan_adapt_ops ops;
    struct trs_chan_irq_ctx *maint_irq;
    struct trs_chan_irq_ctx *normal_irq;
    struct trs_chan_hw_sq_ctx *hw_sq_ctx;
    struct trs_chan_hw_cq_ctx *hw_cq_ctx;
    struct proc_dir_entry *entry;
    bool trace_enable;
};

struct trs_chan_ts_inst *trs_chan_ts_inst_get(struct trs_id_inst *inst);
void trs_chan_ts_inst_put(struct trs_chan_ts_inst *ts_inst);

static inline int trs_chan_sq_to_chan_id(struct trs_chan_ts_inst *ts_inst, u32 sqid)
{
    return (sqid < ts_inst->sq_max_id) ? ts_inst->hw_sq_ctx[sqid].chan_id : -1;
}

static inline int trs_chan_cq_to_chan_id(struct trs_chan_ts_inst *ts_inst, u32 cqid)
{
    return (cqid < ts_inst->cq_max_id) ? ts_inst->hw_cq_ctx[cqid].chan_id : -1;
}

static inline bool trs_chan_trace_is_enabled(struct trs_chan_ts_inst *ts_inst)
{
    return ts_inst->trace_enable;
}

#endif

