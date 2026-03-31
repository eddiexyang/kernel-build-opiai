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
#include "trs_stars_cq.h"
#include "trs_stars_sq.h"
#include "trs_stars.h"
#include "trs_ts_db.h"
#include "trs_chip_def_comm.h"

int trs_chan_ops_get_valid_cq_list(struct trs_id_inst *inst, u32 group, u32 cqid[], u32 cq_id_num, u32 *valid_cq_num)
{
    return trs_stars_cq_get_valid_list(inst, group, cqid, cq_id_num, valid_cq_num);
}

void trs_chan_ops_intr_mask_config(struct trs_id_inst *inst, u32 group, u32 irq, int val)
{
    (void)trs_stars_set_cq_l1_mask(inst, (u32)val, group);
}

void trs_chan_ops_get_sq_head_in_cqe(struct trs_id_inst *inst, void *cqe, u32 *sq_head)
{
    trs_stars_cqe_get_sq_head(inst, cqe, sq_head);
}
EXPORT_SYMBOL(trs_chan_ops_get_sq_head_in_cqe);

bool trs_chan_ops_cqe_is_valid(struct trs_id_inst *inst, void *cqe, u32 loop)
{
    return trs_stars_cqe_is_valid(inst, cqe, loop);
}
EXPORT_SYMBOL(trs_chan_ops_cqe_is_valid);

static int trs_chan_ops_sq_sleep(unsigned long timeout_ms)
{
    int ret = 0;

    /* sleep time smaller than 10ms, use usleep_range */
    if (timeout_ms < 10) {
        usleep_range(timeout_ms * USEC_PER_MSEC, timeout_ms * USEC_PER_MSEC + 1);
    } else {
        if (msleep_interruptible(timeout_ms) != 0) {
            ret = -EINTR;
        }
    }
    return ret;
}

static int trs_chan_ops_sq_disable_to_enable(struct trs_id_inst *inst, u32 id, u32 para)
{
    unsigned long total_sleep_time_ms = 0;
    unsigned long sleep_time_ms = 1;
    const u32 adj_cnt = 10;
    u32 cnt = 0;
    int ret;

    while (true) {
        ret = trs_stars_sq_enable(inst, id);
        if (ret != -EAGAIN) {
            break;
        }
        /* Stars sq is already in enable state, sleep for a while and retry */

        /* adjust sleep time every 10 loop time */
        if (((++cnt) % adj_cnt) == 0) {
            sleep_time_ms *= adj_cnt;
        }

        ret = trs_chan_ops_sq_sleep(sleep_time_ms);
        if (ret != 0) {
            break;
        }

        total_sleep_time_ms += sleep_time_ms;
        if (time_after(total_sleep_time_ms, (unsigned long)para)) {
            ret = -ETIMEDOUT;
            break;
        }
    }

    return ret;
}


static int trs_chan_ops_get_sq_head(struct trs_id_inst *inst, u32 sqid, u32 *head)
{
    return trs_stars_get_sq_head(inst, sqid, head);
}

static int trs_chan_ops_get_sq_tail(struct trs_id_inst *inst, u32 sqid, u32 *tail)
{
    return trs_stars_get_sq_tail(inst, sqid, tail);
}

static int trs_chan_ops_get_cq_head(struct trs_id_inst *inst, u32 cqid, u32 *head)
{
    return trs_stars_get_cq_head(inst, cqid, head);
}

static int trs_chan_ops_get_cq_tail(struct trs_id_inst *inst, u32 cqid, u32 *tail)
{
    return trs_stars_get_cq_tail(inst, cqid, tail);
}

static int trs_chan_ops_get_sq_status(struct trs_id_inst *inst, u32 sqid, u32 *status)
{
    return trs_stars_get_sq_status(inst, sqid, status);
}

int trs_stars_chan_ops_ctrl_sqcq(struct trs_id_inst *inst, u32 id, u32 cmd, u32 para)
{
    int ret = -EINVAL;

    switch (cmd) {
        case CTRL_CMD_SQ_HEAD_UPDATE:
            ret = trs_stars_set_sq_head(inst, id, para);
            break;
        case CTRL_CMD_SQ_TAIL_UPDATE:
            ret = trs_stars_set_sq_tail(inst, id, para);
            break;
        case CTRL_CMD_CQ_HEAD_UPDATE:
            ret = trs_stars_cq_head_update(inst, id, para);
            break;
        case CTRL_CMD_SQ_STATUS_SET:
            ret = trs_stars_set_sq_status(inst, id, para);
            break;
        case CTRL_CMD_SQ_DISABLE_TO_ENABLE:
            ret = trs_chan_ops_sq_disable_to_enable(inst, id, para);
            break;
        default:
            break;
    }

    return ret;
}

int trs_stars_chan_ops_query_sqcq(struct trs_id_inst *inst, u32 id, u32 cmd, u64 *value)
{
    int ret = -EINVAL;

    switch (cmd) {
        case QUERY_CMD_SQ_HEAD:
            ret = trs_chan_ops_get_sq_head(inst, id, (u32 *)value);
            break;
        case QUERY_CMD_SQ_TAIL:
            ret = trs_chan_ops_get_sq_tail(inst, id, (u32 *)value);
            break;
        case QUERY_CMD_CQ_HEAD:
            ret = trs_chan_ops_get_cq_head(inst, id, (u32 *)value);
            break;
        case QUERY_CMD_CQ_TAIL:
            ret = trs_chan_ops_get_cq_tail(inst, id, (u32 *)value);
            break;
        case QUERY_CMD_SQ_STATUS:
            ret = trs_chan_ops_get_sq_status(inst, id, (u32 *)value);
            break;
        case QUERY_CMD_SQ_HEAD_PADDR:
            ret = trs_stars_get_sq_head_paddr(inst, id, value);
            break;
        case QUERY_CMD_SQ_TAIL_PADDR:
            ret = trs_stars_get_sq_tail_paddr(inst, id, value);
            break;
        case QUERY_CMD_SQ_DB_PADDR:
            ret = trs_get_ts_db_paddr(inst, TRS_DB_TRIGGER_SQ, 0, value);
            break;
        default:
            break;
    }

    return ret;
}

