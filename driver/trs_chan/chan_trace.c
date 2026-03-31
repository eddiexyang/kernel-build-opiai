/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
* Create: 2023-10-24
*/

#include "chan_init.h"
#include "chan_ts_inst.h"
#define CREATE_TRACE_POINTS
#include "chan_trace_event.h"
#include "chan_trace.h"

static inline void trs_chan_sq_to_trace(struct trs_chan *chan, struct trs_chan_sq_ctx *sq,
    struct trs_chan_sq_trace *sq_trace)
{
    sq_trace->sqid = sq->sqid;
    sq_trace->sq_head = sq->sq_head;
    sq_trace->sq_tail = sq->sq_tail;
    sq_trace->chan_id = (u32)chan->id;
    sq_trace->types = chan->types;
}

static inline void tra_chan_cq_to_trace(struct trs_chan *chan, struct trs_chan_cq_ctx *cq,
    struct trs_chan_cq_trace *cq_trace)
{
    cq_trace->cqid = cq->cqid;
    cq_trace->cq_head = cq->cq_head;
    cq_trace->round = cq->loop;
    cq_trace->chan_id = (u32)chan->id;
    cq_trace->types = chan->types;
}

void trs_chan_trace_cqe(const char *str, struct trs_chan *chan, struct trs_chan_cq_ctx *cq, void *cqe)
{
    if (trs_chan_trace_is_enabled(chan->ts_inst) && (chan->ops.trace_cqe_fill != NULL)) {
        struct trs_chan_cq_trace cq_trace = {0};
        tra_chan_cq_to_trace(chan, cq, &cq_trace);
        chan->ops.trace_cqe_fill(&chan->inst, &cq_trace, cqe);
        trace_cqe(str, &chan->inst, &cq_trace);
    }
}

void trs_chan_trace_sqe(const char *str, struct trs_chan *chan, struct trs_chan_sq_ctx *sq, void *sqe)
{
    if (trs_chan_trace_is_enabled(chan->ts_inst) && (chan->ops.trace_sqe_fill != NULL)) {
        struct trs_chan_sq_trace sq_trace = {0};
        trs_chan_sq_to_trace(chan, sq, &sq_trace);
        chan->ops.trace_sqe_fill(&chan->inst, &sq_trace, sqe);
        trace_sqe(str, &chan->inst, &sq_trace);
    }
}

void trace_chan_trace_recv(const char *str, struct trs_chan *chan, struct trs_chan_recv_para *para)
{
    if (trs_chan_trace_is_enabled(chan->ts_inst) && (chan->ops.trace_cqe_fill != NULL)) {
        struct trs_chan_recv_trace recv_trace = {
            .chan_id = chan->id,
            .types = chan->types,
            .cqid = chan->cq.cqid,
            .cq_head = chan->cq.cq_head,
            .cqe_num = para->cqe_num,
            .recv_cqe_num = para->recv_cqe_num,
            .timeout = para->timeout
        };
        trace_recv(str, &chan->inst, &recv_trace);
    }
}

