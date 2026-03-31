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
#define TRACE_SYSTEM trs_core

#if !defined(TRS_CORE_TRACE_EVENT_H) || defined(TRACE_HEADER_MULTI_READ)
#define TRS_CORE_TRACE_EVENT_H

#include <linux/types.h>
#include <linux/time.h>

#include <linux/tracepoint.h>
#include "trs_pub_def.h"
#include "trs_core.h"
#include "trs_core_trace.h"

TRACE_EVENT(logic_cqe,
    TP_PROTO(const char *str, struct trs_id_inst *inst, struct trs_logic_cqe_trace *cq_trace),
    TP_ARGS(str, inst, cq_trace),
    TP_STRUCT__entry(
        __string(           str,                            str ? str : "nop" )
        __field_struct(     struct trs_id_inst,             inst        )
        __field_struct(     struct trs_logic_cqe_trace,     cq_trace    )
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
        "logic_cqid=%u;"
        "head=%u;"
        "tail=%u;"
        "task_id=%u;"
        "stream_id=%u;"
        "sqid=%u;"
        "sq_head=%u;"
        "sqe_type=%u;"
        "match_flag=%u;"
        "%lld.%06ld",
        __get_str(str),
        __entry->inst.devid,
        __entry->inst.tsid,
        __entry->cq_trace.logic_cqid,
        __entry->cq_trace.head,
        __entry->cq_trace.tail,
        __entry->cq_trace.task_id,
        __entry->cq_trace.stream_id,
        __entry->cq_trace.sqid,
        __entry->cq_trace.sq_head,
        __entry->cq_trace.sqe_type,
        __entry->cq_trace.match_flag,
        /* tv_sec type is depends on linux version */
        (long long)__entry->timestamp.tv_sec, __entry->timestamp.tv_nsec / NSEC_PER_USEC
    )
);

TRACE_EVENT(logic_cq_recv,
    TP_PROTO(const char *str, struct trs_id_inst *inst, struct trs_logic_cq_recv_trace_t *recv_trace),
    TP_ARGS(str, inst, recv_trace),
    TP_STRUCT__entry(
        __string(           str,                                str ? str : "nop" )
        __field_struct(     struct trs_id_inst,                 inst        )
        __field_struct(     struct trs_logic_cq_recv_trace_t,   recv_trace  )
        __field_struct(     struct timespec64,                  timestamp   )
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
        "logic_cqid=%u;"
        "report_cqe_num=%u;"
        "timeout=%d;"
        "%lld.%06ld",
        __get_str(str),
        __entry->inst.devid,
        __entry->inst.tsid,
        __entry->recv_trace.logic_cqid,
        __entry->recv_trace.report_cqe_num,
        __entry->recv_trace.timeout,
        /* tv_sec type is depends on linux version */
        (long long)__entry->timestamp.tv_sec, __entry->timestamp.tv_nsec / NSEC_PER_USEC
    )
);
#endif /* TRS_CORE_TRACE_EVENT_H */

/* This part must be outside protection */
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE trs_core_trace_event
#include <trace/define_trace.h>
