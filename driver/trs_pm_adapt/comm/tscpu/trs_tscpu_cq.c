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
* Create: 2022-8-15
*/
#include "trs_tscpu_cq.h"
#include "trs_ts_db.h"
#include "trs_chip_def_comm.h"

/**
* @ingroup
* @brief the struct define of report msg when task is completed
*/
typedef struct tagTsTaskReportMsg {
    uint16_t SOP : 1; /* start of packet, indicates this is the first 32bit return payload */
    uint16_t MOP : 1; /* middle of packet, indicates the payload is a continuation of previous task return payload */
    uint16_t EOP : 1; /* *<end of packet, indicates this is the last 32bit return payload.
                     SOP & EOP can appear in the same packet, MOP & EOP can also appear on the same packet. */
    uint16_t packageType : 3;
    uint16_t streamID : 10;
    uint16_t taskID;
    uint32_t payLoad;
    uint16_t SQ_id : 9;
    uint16_t reserved : 6;
    uint16_t phase : 1;
    uint16_t SQ_head : 14;
    uint16_t streamIDEx : 1; /* streamID high bit */
    uint16_t faultStreamIDEx : 1; /* fault streamID high bit */
} rtTaskReport_t; /* Non stars Cq report */

int trs_tscpu_cqe_get_streamid(struct trs_id_inst *inst, void *cqe, u32 *stream_id)
{
    *stream_id = ((rtTaskReport_t *)cqe)->streamID;
    return 0;
}

bool trs_tscpu_cqe_is_valid(struct trs_id_inst *inst, void *cqe, u32 loop)
{
    return (((rtTaskReport_t *)cqe)->phase != (loop & 0x1));
}

void trs_tscpu_cqe_get_sq_id(struct trs_id_inst *inst, void *cqe, u32 *sqid)
{
    *sqid = ((rtTaskReport_t *)cqe)->SQ_id;
}

void trs_tscpu_cqe_get_sq_head(struct trs_id_inst *inst, void *cqe, u32 *sq_head)
{
    *sq_head = ((rtTaskReport_t *)cqe)->SQ_head;
}

int trs_tscpu_cq_head_update(struct trs_id_inst *inst, u32 cqid, u32 head)
{
    return trs_ring_ts_db(inst, TRS_DB_TASK_CQ, cqid, head);
}

