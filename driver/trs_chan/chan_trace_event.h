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

#undef TRACE_SYSTEM
#define TRACE_SYSTEM trs_chan

#if !defined(CHAN_TRACE_EVENT_H) || defined(TRACE_HEADER_MULTI_READ)
#define CHAN_TRACE_EVENT_H

#include <linux/types.h>
#include <linux/time.h>
#include <linux/tracepoint.h>

#include "trs_pub_def.h"
#include "trs_chan.h"
#include "chan_trace.h"

TRACE_EVENT(sqe,
    TP_PROTO(const char *str, struct trs_id_inst *inst, struct trs_chan_sq_trace *sq_trace),
    TP_ARGS(str, inst, sq_trace),
    TP_STRUCT__entry(
        __string(           str,                        str ? str : "nop" )
        __field_struct(     struct trs_id_inst,         inst        )
        __field_struct(     struct trs_chan_sq_trace,   sq_trace    )
        __field_struct(     struct timespec64,          timestamp   )
    ),
    TP_fast_assign(
        __assign_str(str);
        __entry->inst = *inst;
        __entry->sq_trace = *sq_trace;
        ktime_get_ts64(&__entry->timestamp);
    ),
    TP_printk(
        "%s:\t"
        "devid=%u;"
        "tsid=%u;"
        "chan_id=%u;"
        "chan_name=%s;"
        "sqid=%u;"
        "sq_head=%u;"
        "sq_tail=%u;"
        "sqe_type=%u;"
        "task_id=%u;"
        "stream_id=%u;"
        "%lld.%06ld",
        __get_str(str),
        __entry->inst.devid,
        __entry->inst.tsid,
        __entry->sq_trace.chan_id,
        trs_chan_type_to_name(&__entry->sq_trace.types),
        __entry->sq_trace.sqid,
        __entry->sq_trace.sq_head,
        __entry->sq_trace.sq_tail,
        __entry->sq_trace.type,
        __entry->sq_trace.task_id,
        __entry->sq_trace.stream_id,
        /* tv_sec type is depends on linux version */
        (long long)__entry->timestamp.tv_sec, __entry->timestamp.tv_nsec / NSEC_PER_USEC
    )
);

TRACE_EVENT(cqe,
    TP_PROTO(const char *str, struct trs_id_inst *inst, struct trs_chan_cq_trace *cq_trace),
    TP_ARGS(str, inst, cq_trace),
    TP_STRUCT__entry(
        __string(           str,                            str ? str : "nop" )
        __field_struct(     struct trs_id_inst,             inst        )
        __field_struct(     struct trs_chan_cq_trace,       cq_trace    )
        __field_struct(     struct timespec64,              timestamp   )
    ),
    TP_fast_assign(
        __assign_str(str);
        __entry->inst = *inst;
        __entry->cq_trace = *cq_trace;
        ktime_get_ts64(&__entry->timestamp);
    ),
    TP_printk(
        "%s:\t"
        "devid=%u;"
        "tsid=%u;"
        "chan_id=%u;"
        "chan_name=%s;"
        "cqid=%u;"
        "cq_head=%u;"
        "round=%u;"
        "task_id=%u;"
        "stream_id=%u;"
        "sq_id=%u;"
        "sq_head=%u;"
        "%lld.%06ld",
        __get_str(str),
        __entry->inst.devid,
        __entry->inst.tsid,
        __entry->cq_trace.chan_id,
        trs_chan_type_to_name(&__entry->cq_trace.types),
        __entry->cq_trace.cqid,
        __entry->cq_trace.cq_head,
        __entry->cq_trace.round,
        __entry->cq_trace.task_id,
        __entry->cq_trace.stream_id,
        __entry->cq_trace.sq_id,
        __entry->cq_trace.sq_head,
        /* tv_sec type is depends on linux version */
        (long long)__entry->timestamp.tv_sec, __entry->timestamp.tv_nsec / NSEC_PER_USEC
    )
)

TRACE_EVENT(recv,
    TP_PROTO(const char *str, struct trs_id_inst *inst, struct trs_chan_recv_trace *recv_trace),
    TP_ARGS(str, inst, recv_trace),
    TP_STRUCT__entry(
        __string(           str,                            str ? str : "nop" )
        __field_struct(     struct trs_id_inst,             inst        )
        __field_struct(     struct trs_chan_recv_trace,     recv_trace  )
        __field_struct(     struct timespec64,              timestamp   )
    ),
    TP_fast_assign(
        __assign_str(str);
        __entry->inst = *inst;
        __entry->recv_trace = *recv_trace;
        ktime_get_ts64(&__entry->timestamp);
    ),
    TP_printk(
        "%s:\t"
        "devid=%u;"
        "tsid=%u;"
        "chan_id=%u;"
        "chan_name=%s;"
        "cqid=%u;"
        "cq_head=%u;"
        "cqe_num=%u;"
        "recv_cqe_num=%u;"
        "timeout=%d;"
        "%lld.%06ld",
        __get_str(str),
        __entry->inst.devid,
        __entry->inst.tsid,
        __entry->recv_trace.chan_id,
        trs_chan_type_to_name(&__entry->recv_trace.types),
        __entry->recv_trace.cqid,
        __entry->recv_trace.cq_head,
        __entry->recv_trace.cqe_num,
        __entry->recv_trace.recv_cqe_num,
        __entry->recv_trace.timeout,
        /* tv_sec type is depends on linux version */
        (long long)__entry->timestamp.tv_sec, __entry->timestamp.tv_nsec / NSEC_PER_USEC
    )
)
#endif /* CHAN_TRACE_EVENT_H */

/* This part must be outside protection */
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE chan_trace_event
#include <trace/define_trace.h>
