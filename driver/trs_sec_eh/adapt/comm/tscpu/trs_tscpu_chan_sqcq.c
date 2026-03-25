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
* Create: 2022-10-15
*/
#include <linux/delay.h>

#include "trs_chan.h"
#include "trs_tscpu_sq.h"
#include "trs_tscpu_cq.h"
#include "trs_ts_db.h"
#include "trs_chip_def_comm.h"
#include "trs_tscpu_chan_sqcq.h"

void trs_tscpu_chan_ops_get_sq_head_in_cqe(struct trs_id_inst *inst, void *cqe, u32 *sq_head)
{
    trs_tscpu_cqe_get_sq_head(inst, cqe, sq_head);
}

bool trs_tscpu_chan_ops_cqe_is_valid(struct trs_id_inst *inst, void *cqe, u32 loop)
{
    return trs_tscpu_cqe_is_valid(inst, cqe, loop);
}

static int trs_chan_ops_get_sq_db_paddr(struct trs_id_inst *inst, u32 sqid, u64 *paddr)
{
    return trs_tscpu_get_sq_db_paddr(inst, sqid, paddr);
}

static int trs_chan_ops_get_sq_tail(struct trs_id_inst *inst, u32 sqid, u32 *tail)
{
    return trs_tscpu_get_sq_tail(inst, sqid, tail);
}

int trs_tscpu_chan_ops_ctrl_sqcq(struct trs_id_inst *inst, u32 id, u32 cmd, u32 para)
{
    int ret = -EINVAL;

    switch (cmd) {
        case CTRL_CMD_SQ_HEAD_UPDATE:  /* no effect */
            ret = 0;
            break;
        case CTRL_CMD_SQ_TAIL_UPDATE:
            ret = trs_tscpu_sq_tail_update(inst, id, para);
            break;
        case CTRL_CMD_CQ_HEAD_UPDATE:
            ret = trs_tscpu_cq_head_update(inst, id, para);
            break;
        case CTRL_CMD_SQ_STATUS_SET: /* not surport */
            break;
        case CTRL_CMD_SQ_DISABLE_TO_ENABLE:  /* not surport */
            break;
        default:
            break;
    }

    return ret;
}

int trs_tscpu_chan_ops_query_sqcq(struct trs_id_inst *inst, u32 id, u32 cmd, u64 *value)
{
    int ret = -EINVAL;

    switch (cmd) {
        case QUERY_CMD_SQ_HEAD:
            break;
        case QUERY_CMD_SQ_TAIL:
            ret = trs_chan_ops_get_sq_tail(inst, id, (u32 *)value);
            break;
        case QUERY_CMD_CQ_HEAD:
            break;
        case QUERY_CMD_CQ_TAIL:
            break;
        case QUERY_CMD_SQ_STATUS:
            break;
        case QUERY_CMD_SQ_HEAD_PADDR:
        case QUERY_CMD_SQ_TAIL_PADDR:
            *value = 0; /* not has sq head tail addr */
            ret = 0;
            break;
        case QUERY_CMD_SQ_DB_PADDR:
            ret = trs_chan_ops_get_sq_db_paddr(inst, id, value);
            break;
        default:
            break;
    }

    return ret;
}

