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
* Create: 2022-11-16
*/
#include "trs_sec_eh_mbox.h"
#include "trs_sec_eh_sq.h"
#include "trs_sec_eh_id.h"

typedef int (*mb_update_func)(struct trs_sec_eh_ts_inst *sec_eh_cfg, void *mb_data);

static int trs_sec_eh_create_sqcq_mb_update(struct trs_sec_eh_ts_inst *sec_eh_cfg, void *data)
{
    struct trs_normal_cqsq_mailbox *mb_data = (struct trs_normal_cqsq_mailbox *)data;
    struct trs_sec_eh_sq_ctx_info ctx;
    int ret = -EINVAL;

    if ((!trs_sec_eh_id_is_belong_to_vf(&sec_eh_cfg->id_info[TRS_HW_SQ], mb_data->sq_index)) ||
        (!trs_sec_eh_id_is_belong_to_vf(&sec_eh_cfg->id_info[TRS_HW_CQ], mb_data->cq0_index))) {
        trs_debug("Invalid. (devid=%u; sq=%u; cq=%u)\n",
            sec_eh_cfg->inst.devid, mb_data->sq_index, mb_data->cq0_index);
        return -EACCES;
    }

    ctx.sqid = mb_data->sq_index;
    ctx.sqesize = mb_data->sqesize;
    ctx.sqdepth = mb_data->sqdepth;
    ctx.addr_offset = mb_data->sq_addr;
    ctx.pid = mb_data->pid;
    ret = trs_sec_eh_alloc_sq_mem(sec_eh_cfg, &ctx);
    if (ret != 0) {
        return ret;
    }

    trs_sec_eh_sq_ctx_init(sec_eh_cfg, &ctx);

    mb_data->sq_addr = ctx.sq_paddr;
    trs_debug("Id info. (devid=%u; tsid=%u; sq=%u; cq=%u; pid=%d)\n",
        sec_eh_cfg->inst.devid, sec_eh_cfg->inst.tsid, mb_data->sq_index, mb_data->cq0_index, mb_data->pid);

    return ret;
}

static int trs_sec_eh_free_sqcq_mb_update(struct trs_sec_eh_ts_inst *sec_eh_cfg, void *data)
{
    struct trs_normal_cqsq_mailbox *mb_data = (struct trs_normal_cqsq_mailbox *)data;

    if ((!trs_sec_eh_id_is_belong_to_vf(&sec_eh_cfg->id_info[TRS_HW_SQ], mb_data->sq_index)) ||
        (!trs_sec_eh_id_is_belong_to_vf(&sec_eh_cfg->id_info[TRS_HW_CQ], mb_data->cq0_index))) {
        trs_debug("Invalid. (devid=%u; sq=%u; cq=%u)\n",
            sec_eh_cfg->inst.devid, mb_data->sq_index, mb_data->cq0_index);
        return -EACCES;
    }

    trs_sec_eh_free_sq_mem(sec_eh_cfg, mb_data->sq_index);
    trs_sec_eh_sq_ctx_uninit(sec_eh_cfg, mb_data->sq_index);
    trs_debug("Id info. (devid=%u; tsid=%u; sq=%u; cq=%u; pid=%d)\n",
        sec_eh_cfg->inst.devid, sec_eh_cfg->inst.tsid, mb_data->sq_index, mb_data->cq0_index, mb_data->pid);

    return 0;
}

static int trs_sec_eh_free_all_sqcq_mb_update(struct trs_sec_eh_ts_inst *sec_eh_cfg, void *data)
{
    struct recycle_proc_msg *mb_data = (struct recycle_proc_msg *)data;
    struct trs_sec_eh_sq_ctx *sq_ctx = sec_eh_cfg->sq_ctx;
    u32 sqid;

    for (sqid = 0; sqid < sec_eh_cfg->id_info[TRS_HW_SQ].end; sqid++) {
        if (sq_ctx[sqid].pid == mb_data->proc_info.pid[0]) {
            trs_sec_eh_free_sq_mem(sec_eh_cfg, sqid);
            trs_sec_eh_sq_ctx_uninit(sec_eh_cfg, sqid);
            trs_debug("Id info. (devid=%u; tsid=%u; pid=%d; sqid=%u)\n",
                sec_eh_cfg->inst.devid, sec_eh_cfg->inst.tsid, mb_data->proc_info.pid[0], sqid);
        }
    }

    return 0;
}

static const mb_update_func sec_eh_mb_update_handle[TRS_MBOX_CMD_MAX] = {
    [TRS_MBOX_CREATE_CQSQ_CALC] = trs_sec_eh_create_sqcq_mb_update,
    [TRS_MBOX_RELEASE_CQSQ_CALC] = trs_sec_eh_free_sqcq_mb_update,
    [TRS_MBOX_RECYCLE_PID] = trs_sec_eh_free_all_sqcq_mb_update,
};

int trs_sec_eh_mb_update(struct trs_id_inst *inst, u16 cmd, void *mb_data)
{
    struct trs_sec_eh_ts_inst *sec_eh_cfg = NULL;
    int ret = -EINVAL;

    if (cmd >= TRS_MBOX_CMD_MAX) {
        return ret;
    }

    if (sec_eh_mb_update_handle[cmd] == NULL) {
        return 0;
    }

    sec_eh_cfg = trs_sec_eh_ts_inst_get(inst);
    if (sec_eh_cfg != NULL) {
        ret = sec_eh_mb_update_handle[cmd](sec_eh_cfg, mb_data);
        trs_sec_eh_ts_inst_put(sec_eh_cfg);
    }

    return ret;
}
