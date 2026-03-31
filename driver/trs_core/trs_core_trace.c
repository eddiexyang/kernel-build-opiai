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
#include "trs_ts_inst.h"
#include "trs_logic_cq.h"

#define CREATE_TRACE_POINTS
#include "trs_core_trace_event.h"
#include "trs_core_trace.h"

/* non stars cqe */
struct trs_logic_cqe_v1_st {
    u16 phase      : 1;
    u16 SOP        : 1; /* start of packet, indicates this is the first 32bit return payload */
    u16 MOP        : 1; /* middle of packet, indicates the payload is a continuation of previous task
                                      return payload */
    u16 EOP        : 1; /* end of packet, indicates this is the last 32bit return payload. SOP & EOP
                                      can appear in the same packet, MOP & EOP can also appear on the same packet. */
    u16 logic_cq_id  : 12;
    u16 stream_id ;
    u16 task_id;
    u8 error_type;
    u8 match_flag; /* ts set notice drv thread recv must match stream id and task id (sync task set) */
    u32 error_code;
    u32 reserved1;
};

static inline void trs_logic_cqe_trace_fill(struct trs_logic_cq *logic_cq, void *logic_cqe,
    struct trs_logic_cqe_trace *trace)
{
    if (logic_cq->cqe_verion == LOGIC_CQE_VERSION_V1) {
        struct trs_logic_cqe_v1_st *logic_cqe_ = (struct trs_logic_cqe_v1_st *)logic_cqe;
        trace->stream_id = logic_cqe_->stream_id;
        trace->task_id = logic_cqe_->task_id;
        trace->match_flag = logic_cqe_->match_flag;
        trace->errcode = logic_cqe_->error_code;
        trace->errtype = logic_cqe_->error_type;
    } else {
        struct trs_logic_cqe *logic_cqe_ = (struct trs_logic_cqe *)logic_cqe;
        trace->stream_id = logic_cqe_->stream_id;
        trace->task_id = logic_cqe_->task_id;
        trace->errcode = logic_cqe_->errorCode;
        trace->errtype = logic_cqe_->errorType;
        trace->sqe_type = logic_cqe_->sqeType;
        trace->sqid = logic_cqe_->sqId;
        trace->sq_head = logic_cqe_->sqHead;
        trace->sqe_type = logic_cqe_->sqeType;
        trace->match_flag = logic_cqe_->match_flag;
        trace->drop_flag = logic_cqe_->drop_flag;
        trace->timestamp = logic_cqe_->timeStamp;
    }
    trace->logic_cqid = logic_cq->cqid;
    trace->head = logic_cq->head;
    trace->tail = logic_cq->tail;
}

void trs_logic_cq_copy_trace(const char *str, struct trs_core_ts_inst *ts_inst,
    struct trs_logic_cq *logic_cq, u32 start, u32 num)
{
    if (trs_core_trace_is_enabled(ts_inst)) {
        u32 i;
        for (i = start; i < start + num; i++) {
            struct trs_logic_cqe_trace trace = {0};
            void *logic_cqe = logic_cq->addr + (unsigned long)i * logic_cq->cqe_size;
            trs_logic_cqe_trace_fill(logic_cq, logic_cqe, &trace);
            trace_logic_cqe(str, &ts_inst->inst, &trace);
        }
    }
}

void trs_logic_cq_recv_trace(const char *str, struct trs_core_ts_inst *ts_inst,
    struct halReportRecvInfo *para)
{
    if (trs_core_trace_is_enabled(ts_inst)) {
        struct trs_logic_cq_recv_trace_t recv_trace = {
            .logic_cqid = para->cqId,
            .report_cqe_num = para->report_cqe_num,
            .timeout = para->timeout
        };
        trace_logic_cq_recv(str, &ts_inst->inst, &recv_trace);
    }
}

void trs_logic_cq_enque_trace(struct trs_core_ts_inst *ts_inst, struct trs_logic_cq *logic_cq,
    u32 stream_id, u32 task_id, void *cqe)
{
    if (trs_core_trace_is_enabled(ts_inst)) {
        struct trs_logic_cqe_trace trace = {0};
        trs_logic_cqe_trace_fill(logic_cq, cqe, &trace);
        trace.stream_id = stream_id;
        trace.task_id = task_id;
        trace_logic_cqe("Logic Cq Enque", &ts_inst->inst, &trace);
    }
}

