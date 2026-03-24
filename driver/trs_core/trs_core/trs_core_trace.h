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

#ifndef TRS_CORE_TRACE_H
#define TRS_CORE_TRACE_H
#include "drv_type.h"
#include "ascend_hal_define.h"
#include "trs_logic_cq.h"
#include "trs_ts_inst.h"

struct trs_logic_cqe_trace {
    u32 logic_cqid;
    u32 head;
    u32 tail;
    u32 task_id;
    u32 stream_id;
    u32 sqid;
    u32 sq_head;
    u32 sqe_type;
    u32 match_flag;
    u32 errcode;
    u32 errtype;
    u32 drop_flag;
    u64 timestamp;
};

struct trs_logic_cq_recv_trace_t {
    u32 logic_cqid;
    u32 report_cqe_num;
    int timeout;
};

#ifdef CFG_BUILD_DEBUG
void trs_logic_cq_copy_trace(const char *str, struct trs_core_ts_inst *ts_inst,
    struct trs_logic_cq *logic_cq, u32 start, u32 num);
void trs_logic_cq_recv_trace(const char *str, struct trs_core_ts_inst *ts_inst,
    struct halReportRecvInfo *para);
void trs_logic_cq_enque_trace(struct trs_core_ts_inst *ts_inst, struct trs_logic_cq *logic_cq,
    u32 stream_id, u32 task_id, void *cqe);
#else
static inline void trs_logic_cq_copy_trace(const char *str, struct trs_core_ts_inst *ts_inst,
    struct trs_logic_cq *logic_cq, u32 start, u32 num)
{
}
static inline void trs_logic_cq_recv_trace(const char *str, struct trs_core_ts_inst *ts_inst,
    struct halReportRecvInfo *para)
{
}
static inline void trs_logic_cq_enque_trace(struct trs_core_ts_inst *ts_inst, struct trs_logic_cq *logic_cq,
    u32 stream_id, u32 task_id, void *cqe)
{
}
#endif

#endif /* TRS_CORE_TRACE_H */
