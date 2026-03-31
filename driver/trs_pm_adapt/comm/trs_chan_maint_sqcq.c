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
* Create: 2022-12-25
*/
#include "trs_chan.h"

#include "trs_ts_db.h"
#include "trs_chip_def_comm.h"

static int trs_maint_set_sq_tail(struct trs_id_inst *inst, u32 sqid, u32 tail)
{
    return trs_ring_ts_db(inst, TRS_DB_MAINT_SQ, sqid, tail);
}

static int trs_maint_cq_update_head(struct trs_id_inst *inst, u32 cqid, u32 head)
{
    return trs_ring_ts_db(inst, TRS_DB_MAINT_CQ, cqid, head);
}

int trs_chan_ops_ctrl_maint_sqcq(struct trs_id_inst *inst, u32 id, u32 cmd, u32 para)
{
    int ret = -EINVAL;

    switch (cmd) {
        case CTRL_CMD_SQ_TAIL_UPDATE:
            ret = trs_maint_set_sq_tail(inst, id, para);
            break;
        case CTRL_CMD_CQ_HEAD_UPDATE:
            ret = trs_maint_cq_update_head(inst, id, para);
            break;
        default:
            break;
    }

    return ret;
}

int trs_chan_ops_query_maint_sqcq(struct trs_id_inst *inst, u32 id, u32 cmd, u64 *value)
{
    int ret = -EINVAL;

    switch (cmd) {
        case QUERY_CMD_SQ_HEAD_PADDR:
        case QUERY_CMD_SQ_TAIL_PADDR:
        case QUERY_CMD_SQ_DB_PADDR:
            *value = 0; /* not has db */
            ret = 0;
            break;
        default:
            break;
    }

    return ret;
}

