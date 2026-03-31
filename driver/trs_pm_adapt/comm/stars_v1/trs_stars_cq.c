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
#include "trs_stars.h"
#include "trs_timestamp.h"
#include "trs_stars_cq.h"

typedef struct {
    uint16_t task_id;
    uint16_t stream_id : 12; /* 0~4096 for stream_id */
    uint16_t result : 1;
    uint16_t rsv : 3;
} rtStarsModelExecuteError_t;

typedef union {
    rtStarsModelExecuteError_t model_exec;
    uint32_t value;
} rtStarsCqeSwStatus_t;

typedef struct tagStarsCqeErrorInfo {
    uint8_t error_code;
    uint8_t drop_flag : 1;
    uint8_t acc_error : 1;
    uint8_t sqe_type : 6;
    uint16_t sqe_index;
    rtStarsCqeSwStatus_t sq_sw_status;
} rtStarsCqeErrorInfo_t;

/**
* @ingroup
* @brief the struct define of cqe when task is completed
*/
typedef struct tagStarsCqeSysCnt {
    uint32_t syscnt_low;
    uint32_t syscnt_high;
} rtStarsCqeSysCnt_t;

typedef union tagStarsCqeStatus {
    rtStarsCqeSysCnt_t sysCnt;
    rtStarsCqeErrorInfo_t errorInfo;
} rtStarsCqeStatus_t;

typedef struct tagStarsCqe {
    uint16_t phase : 1;
    uint16_t warn : 1;          /* process warning */
    uint16_t evt : 1;           /* event record flag */
    uint16_t place_hold : 1;
    uint16_t SQ_id : 11;
    uint16_t error_bit : 1;
    uint16_t SQ_head;

    uint16_t streamID;
    uint16_t taskID;

    rtStarsCqeStatus_t cqe_status;
} rtStarsCqe_t;

#define LOGIC_CQE_STREAM_ID_MASK 0xfff

int trs_stars_cqe_get_streamid(struct trs_id_inst *inst, void *cqe, u32 *stream_id)
{
    *stream_id = ((rtStarsCqe_t *)cqe)->streamID & LOGIC_CQE_STREAM_ID_MASK;
    return 0;
}

bool trs_stars_cqe_is_valid(struct trs_id_inst *inst, void *cqe, u32 loop)
{
    return (((rtStarsCqe_t *)cqe)->phase != (loop & 0x1));
}

void trs_stars_trace_cqe_fill(struct trs_id_inst *inst, struct trs_chan_cq_trace *cq_trace, void *cqe)
{
    rtStarsCqe_t *cqe_ = (rtStarsCqe_t *)cqe;
    cq_trace->task_id = cqe_->taskID;
    cq_trace->stream_id = cqe_->streamID;
    cq_trace->sq_id = cqe_->SQ_id;
    cq_trace->sq_head = cqe_->SQ_head;
}

void trs_stars_cqe_get_sqid(struct trs_id_inst *inst, void *cqe, u32 *sqid)
{
    *sqid = ((rtStarsCqe_t *)cqe)->SQ_id;
}

void trs_stars_cqe_get_sq_head(struct trs_id_inst *inst, void *cqe, u32 *sq_head)
{
    *sq_head = ((rtStarsCqe_t *)cqe)->SQ_head;
}

int trs_stars_cq_head_update(struct trs_id_inst *inst, u32 cqid, u32 head)
{
    return trs_stars_set_cq_head(inst, cqid, head);
}

int trs_stars_cq_get_valid_list(struct trs_id_inst *inst, u32 group, u32 cqid[], u32 num, u32 *valid_num)
{
    return trs_stars_get_valid_cq_list(inst, group, cqid, num, valid_num);
}

static inline u64 trs_stars_cq_get_timestamp(rtStarsCqe_t *cqe)
{
    u64 timestamp;

    timestamp = (u64)(((u64)cqe->cqe_status.sysCnt.syscnt_high) <<
        (sizeof(cqe->cqe_status.sysCnt.syscnt_low) * BITS_PER_BYTE));
    timestamp |= (u64)cqe->cqe_status.sysCnt.syscnt_low;

    return timestamp;
}

static inline u8 trs_stars_cq_get_sqe_type(rtStarsCqe_t *cqe)
{
    return cqe->cqe_status.errorInfo.sqe_type;
}

static inline u8 trs_stars_cq_get_warn_type(rtStarsCqe_t *cqe)
{
    return (u8)(cqe->warn << 6); /* cqe warn offset: 6 */
}

static inline u8 trs_stars_cq_get_err_type(rtStarsCqe_t *cqe)
{
    return (u8)((cqe->cqe_status.sysCnt.syscnt_low & 0x3F) | trs_stars_cq_get_warn_type(cqe));
}

static inline u8 trs_stars_cq_get_drop_flag(rtStarsCqe_t *cqe)
{
    return cqe->cqe_status.errorInfo.drop_flag;
}

static inline u32 trs_stars_cq_get_err_code(rtStarsCqe_t *cqe)
{
    return cqe->cqe_status.sysCnt.syscnt_high;
}

static inline u16 trs_stars_cq_get_acc_error(rtStarsCqe_t *cqe)
{
    return cqe->cqe_status.errorInfo.acc_error;
}

static inline u16 trs_stars_cq_get_sqe_index(rtStarsCqe_t *cqe)
{
    return cqe->cqe_status.errorInfo.sqe_index;
}

static inline bool trs_stars_cq_has_syscnt(rtStarsCqe_t *cqe)
{
    return (((cqe->evt == 1) || (cqe->place_hold == 1)) && (cqe->error_bit == 0));
}

u16 trs_stars_cqe_get_match_flag(rtStarsCqe_t *cqe)
{
    /* In the bit12 ~ bit15, only the bit15 is 1 means match_type cqe */
    return (cqe->streamID & 0xf000) == 0x8000 ? 1 : 0; /* 0xf000, 0x8000 */
}

void trs_stars_cqe_to_logic_cqe(void *hw_cqe, struct trs_logic_cqe *logic_cqe)
{
    rtStarsCqe_t *cqe = (rtStarsCqe_t *)hw_cqe;
    logic_cqe->drop_flag = 0;

    if (trs_stars_cq_has_syscnt(cqe)) {
        logic_cqe->timeStamp = trs_stars_cq_get_timestamp(cqe);
        logic_cqe->sqeType = (cqe->evt != 0) ? TRS_STARS_CQE_RECORED : TRS_STARS_CQE_HOLDER;
        logic_cqe->errorCode = 0;
        /* No error, get warning */
        logic_cqe->errorType = trs_stars_cq_get_warn_type(cqe);
    } else {
        logic_cqe->sqe_index = trs_stars_cq_get_sqe_index(cqe);
        logic_cqe->sqeType = trs_stars_cq_get_sqe_type(cqe);
        logic_cqe->errorCode = trs_stars_cq_get_err_code(cqe);
        logic_cqe->errorType = trs_stars_cq_get_err_type(cqe);
        logic_cqe->drop_flag = trs_stars_cq_get_drop_flag(cqe);
    }
    logic_cqe->stream_id = cqe->streamID & LOGIC_CQE_STREAM_ID_MASK;
    logic_cqe->match_flag = trs_stars_cqe_get_match_flag(cqe);
    logic_cqe->task_id = cqe->taskID;
    logic_cqe->sqId = cqe->SQ_id;
    logic_cqe->sqHead = cqe->SQ_head;
    logic_cqe->error_bit = cqe->error_bit;
    logic_cqe->acc_error = trs_stars_cq_get_acc_error(cqe);
    if (logic_cqe->match_flag != 0) {
        logic_cqe->enque_timestamp = trs_get_s_timestamp();
    }
}

