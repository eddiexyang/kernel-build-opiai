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
* Create: 2022-7-15
*/

#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/kallsyms.h>

#include "kernel_version_adapt.h"
#include "devdrv_manager_comm.h"
#include "devdrv_interface.h"

#include "trs_mailbox_def.h"
#include "trs_id.h"
#include "trs_ts_inst.h"
#include "trs_core.h"
#include "trs_res_mng.h"

int id_type_trans[TRS_CORE_MAX_ID_TYPE] = {
    [TRS_STREAM] = TRS_STREAM_ID,
    [TRS_EVENT] = TRS_EVENT_ID,
    [TRS_MODEL] = TRS_MODEL_ID,
    [TRS_NOTIFY] = TRS_NOTIFY_ID,
    [TRS_CMO] = TRS_CMO_ID,
    [TRS_HW_SQ] = TRS_HW_SQ_ID,
    [TRS_HW_CQ] = TRS_HW_CQ_ID,
    [TRS_SW_SQ] = TRS_SW_SQ_ID,
    [TRS_SW_CQ] = TRS_SW_CQ_ID,
    [TRS_CB_SQ] = TRS_CB_SQ_ID,
    [TRS_CB_CQ] = TRS_CB_CQ_ID,
    [TRS_LOGIC_CQ] = TRS_LOGIC_CQ_ID
};

int trs_res_get_id_type(int res_type)
{
    return id_type_trans[res_type];
}

int trs_res_replace_res_type(struct trs_core_ts_inst *ts_inst, int res_type)
{
    if (!trs_is_stars_inst(ts_inst)) {
        res_type = (res_type == TRS_SW_SQ) ? TRS_HW_SQ : res_type;
        res_type = (res_type == TRS_SW_CQ) ? TRS_HW_CQ : res_type;
    }

    return res_type;
}

int trs_core_get_host_pid(int cur_pid, int *host_pid)
{
    enum devdrv_process_type cp_type;
    u32 chip_id, vfid;
    int ret;

    ret = devdrv_query_process_host_pid(cur_pid, &chip_id, &vfid, (u32 *)host_pid, &cp_type);
    if (ret != 0) {
        trs_err("Query host pid fail. (ret=%d)\n", ret);
    }
    return ret;
}

static bool trs_is_host_pid_match(int cur_pid, int host_pid)
{
    int pid;

    if (trs_core_get_host_pid(cur_pid, &pid) != 0) {
        trs_err("Query host pid fail.\n");
        return false;
    }

    return (pid == host_pid);
}

u32 trs_res_get_id_num(struct trs_core_ts_inst *ts_inst, int res_type)
{
    return ts_inst->res_mng[res_type].id_num;
}

u32 trs_res_get_max_id(struct trs_core_ts_inst *ts_inst, int res_type)
{
    return ts_inst->res_mng[res_type].max_id;
}

u32 trs_res_get_id_used_num(struct trs_core_ts_inst *ts_inst, int res_type)
{
    return ts_inst->res_mng[res_type].use_num;
}

static void trs_stream_ctx_init(struct trs_stream_ctx *stream_ctx)
{
    stream_ctx->sq = -1;
    stream_ctx->cq = -1;
    stream_ctx->logic_cq = -1;
}

static void trs_res_id_init_para(struct trs_core_ts_inst *ts_inst, int res_type, u32 res_id)
{
    if (res_type == TRS_STREAM) {
        trs_stream_ctx_init(&ts_inst->stream_ctx[res_id]);
    }
}

static int trs_event_res_map_notice_ts(struct trs_core_ts_inst *ts_inst, int op, u32 event_id)
{
    struct trs_res_map_msg msg;

    trs_mbox_init_header(&msg.header, TRS_MBOX_RES_MAP);
    msg.resource_type = 1;
    msg.operation_type = op;
    msg.id = event_id;
    msg.host_pid = ts_inst->res_mng[TRS_EVENT].ids[event_id].pid;

    /* adapt fill: vf_id, phy_id */
    return trs_core_notice_ts(ts_inst, (u8 *)&msg, sizeof(msg));
}

static int trs_event_notice_ts(struct trs_core_ts_inst *ts_inst, int op, u32 event_id)
{
    return trs_event_res_map_notice_ts(ts_inst, op, event_id);
}

static int trs_notify_notice_ts(struct trs_core_ts_inst *ts_inst, int op, u32 notify_id)
{
    struct trs_notify_msg msg;

    trs_mbox_init_header(&msg.header, TRS_MBOX_RESET_NOTIFY);
    msg.notifyId = notify_id;
    msg.tgid = ts_inst->res_mng[TRS_NOTIFY].ids[notify_id].pid;
    msg.notify_type = 0; /* adapt may change it */

    /* adapt fill: plat_type, phy_notifyId, fid */
    return trs_core_notice_ts(ts_inst, (u8 *)&msg, sizeof(msg));
}

static int trs_res_notice_ts(struct trs_core_ts_inst *ts_inst, int op, int res_type, u32 res_id)
{
    if (res_type == TRS_EVENT) {
        return trs_event_notice_ts(ts_inst, op, res_id);
    } else if (res_type == TRS_NOTIFY) {
        return trs_notify_notice_ts(ts_inst, op, res_id);
    } else {
        /* do nothing */
    }

    return 0;
}

static int trs_res_mng_id_alloc(struct trs_core_ts_inst *ts_inst, int res_type, u32 para, u32 *res_id)
{
    if (ts_inst->ops.id_alloc[res_type] != NULL) {
        return ts_inst->ops.id_alloc[res_type](&ts_inst->inst, para, res_id);
    } else {
        return trs_id_alloc(&ts_inst->inst, trs_res_get_id_type(res_type), res_id);
    }
}

static void trs_res_mng_id_free(struct trs_core_ts_inst *ts_inst, int res_type, u32 res_id)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    int ret;

    if (ts_inst->ops.id_free[res_type] != NULL) {
        ret = ts_inst->ops.id_free[res_type](inst, res_id);
    } else {
        ret = trs_id_free(inst, trs_res_get_id_type(res_type), res_id);
    }

    if (ret != 0) {
        trs_warn("Free failed. (devid=%u; tsid=%u; res_type=%d; res_id=%u; ret=%d)\n",
            inst->devid, inst->tsid, res_type, res_id, ret);
    }
}

static bool trs_res_id_need_reset(int res_type)
{
    return (res_type == TRS_EVENT);
}

static int trs_res_id_ctrl(struct trs_core_ts_inst *ts_inst, int res_type, u32 res_id, u32 cmd)
{
    if (ts_inst->ops.res_id_ctrl != NULL) {
        return ts_inst->ops.res_id_ctrl(&ts_inst->inst, (u32)res_type, res_id, cmd);
    }
    return 0;
}

static int trs_res_id_query_addr(struct trs_core_ts_inst *ts_inst, int res_type, u32 res_id, u64 *value)
{
    if (ts_inst->ops.res_id_query != NULL) {
        return ts_inst->ops.res_id_query(&ts_inst->inst, (u32)res_type, res_id, TRS_RES_QUERY_ADDR, value);
    }
    return 0;
}

int trs_res_id_alloc(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_res_id_para *para)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    u64 value;
    int ret;

    ret = trs_res_mng_id_alloc(ts_inst, para->res_type, para->para, &para->id);
    if (ret != 0) {
        trs_err("Alloc failed. (devid=%u; tsid=%u; res_type=%d; ret=%d)\n",
            inst->devid, inst->tsid, para->res_type, ret);
        return ret;
    }

    trs_res_id_init_para(ts_inst, para->res_type, para->id);

    ret = trs_res_notice_ts(ts_inst, 0, para->res_type, para->id);
    if (ret != 0) {
        trs_res_mng_id_free(ts_inst, para->res_type, para->id);
        trs_err("Notice ts failed. (devid=%u; tsid=%u; res_type=%d; ret=%d)\n",
            inst->devid, inst->tsid, para->res_type, ret);
        return ret;
    }

    ret = trs_proc_add_res(proc_ctx, ts_inst, para->res_type, para->id);
    if (ret != 0) {
        (void)trs_res_notice_ts(ts_inst, 1, para->res_type, para->id);
        trs_res_mng_id_free(ts_inst, para->res_type, para->id);
        return ret;
    }

    para->value[0] = 0;
    para->value[1] = 0;
    ret = trs_res_id_query_addr(ts_inst, para->res_type, para->id, &value);
    if (ret == 0) {
        para->value[0] = (u32)(value & 0xffffffffU);  /* 0xffffffffU for low 32bit */
        para->value[1] = (u32)(value >> 32U);  /* 32 for high bit */
    }

    if (trs_res_id_need_reset(para->res_type)) {
        (void)trs_res_id_ctrl(ts_inst, para->res_type, para->id, TRS_RES_OP_CHECK_AND_RESET);
    }

    return 0;
}

static int _trs_res_id_free(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, int res_type, u32 res_id)
{
    int ret;

    ret = trs_proc_del_res(proc_ctx, ts_inst, res_type, res_id);
    if (ret < 0) {
        return ret;
    }

    if (ret == 0) {
        if (trs_res_id_need_reset(res_type)) {
            (void)trs_res_id_ctrl(ts_inst, res_type, res_id, TRS_RES_OP_RESET);
        }
        (void)trs_res_notice_ts(ts_inst, 1, res_type, res_id);
        trs_res_mng_id_free(ts_inst, res_type, res_id);
    }

    return 0;
}

int trs_res_id_free(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_res_id_para *para)
{
    return _trs_res_id_free(proc_ctx, ts_inst, para->res_type, para->id);
}

static trs_res_share_by_proc_ops_t shr_proc_ops[TRS_MAX_ID_TYPE];

void trs_res_share_proc_ops_register(int res_type, trs_res_share_by_proc_ops_t func)
{
    shr_proc_ops[res_type] = func;
}
EXPORT_SYMBOL(trs_res_share_proc_ops_register);

bool trs_res_is_belong_to_proc(struct trs_id_inst *inst, int pid, int res_type, u32 res_id)
{
    struct trs_core_ts_inst *ts_inst = NULL;
    bool ret;

    ts_inst = trs_core_ts_inst_get(inst);
    if (ts_inst == NULL) {
        trs_err("Invalid para. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return false;
    }

    ret = trs_proc_has_res_with_pid(ts_inst, pid, res_type, res_id);
    trs_core_inst_put(ts_inst);

    if ((ret != true) && (shr_proc_ops[res_type] != NULL)) {
        /* res_id does not belong to this proc, further check whether it is shared by this process */
        ret = shr_proc_ops[res_type](inst, pid, res_type, res_id);
    }

    return ret;
}
EXPORT_SYMBOL(trs_res_is_belong_to_proc);

int trs_res_id_check(struct trs_id_inst *inst, int res_type, u32 res_id)
{
    struct trs_core_ts_inst *ts_inst = NULL;
    int ret = 0;

    ts_inst = trs_core_ts_inst_get(inst);
    if (ts_inst == NULL) {
        trs_err("Invalid para. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return false;
    }

    if (ts_inst->ops.res_id_check != NULL) {
        ret = ts_inst->ops.res_id_check(inst, res_type, res_id);
    }
    trs_core_inst_put(ts_inst);

    return ret;
}
EXPORT_SYMBOL(trs_res_id_check);

static int trs_res_id_para_check(struct trs_id_inst *inst, int res_type, u32 res_id)
{
    if (trs_id_inst_check(inst) != 0) {
        return -EINVAL;
    }

    if (res_type >= TRS_HW_SQ) {
        trs_err("Invalid para. (devid=%u; tsid=%u; res_type=%d)\n", inst->devid, inst->tsid, res_type);
        return -EINVAL;
    }

    return 0;
}

int trs_res_id_get(struct trs_id_inst *inst, int res_type, u32 res_id)
{
    struct trs_core_ts_inst *ts_inst = NULL;
    int ret = trs_res_id_para_check(inst, res_type, res_id);
    if (ret != 0) {
        return ret;
    }

    ts_inst = trs_core_ts_inst_get(inst);
    if (ts_inst == NULL) {
        trs_err("Invalid para. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -EINVAL;
    }

    ret = trs_res_get(ts_inst, current->tgid, res_type, res_id);

    trs_core_inst_put(ts_inst);
    return ret;
}
EXPORT_SYMBOL(trs_res_id_get);

int trs_res_id_put(struct trs_id_inst *inst, int res_type, u32 res_id)
{
    struct trs_core_ts_inst *ts_inst = NULL;
    int ret = trs_res_id_para_check(inst, res_type, res_id);
    if (ret != 0) {
        return ret;
    }

    ts_inst = trs_core_ts_inst_get(inst);
    if (ts_inst == NULL) {
        trs_err("Invalid para. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -EINVAL;
    }

    ret = trs_res_put(ts_inst, res_type, res_id);
    if (ret == 0) {
        (void)trs_res_notice_ts(ts_inst, 1, res_type, res_id);
        trs_res_mng_id_free(ts_inst, res_type, res_id);
    }

    trs_core_inst_put(ts_inst);
    return (ret < 0) ? ret : 0;
}
EXPORT_SYMBOL(trs_res_id_put);

int trs_res_id_num_query(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_res_id_para *para)
{
    (void)proc_ctx;
    para->para = trs_res_get_id_num(ts_inst, para->res_type);
    return 0;
}

int trs_res_id_max_query(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_res_id_para *para)
{
    (void)proc_ctx;
    para->para = trs_res_get_max_id(ts_inst, para->res_type);
    return 0;
}

int trs_res_id_used_query(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_res_id_para *para)
{
    u32 used_num;
    int ret;

    (void)proc_ctx;
    ret = trs_id_get_used_num(&ts_inst->inst, trs_res_get_id_type(para->res_type), &used_num);
    if (ret != 0) {
        trs_err("Get failed. (devid=%u; tsid=%u; type=%d)\n", ts_inst->inst.devid, ts_inst->inst.tsid, para->res_type);
        return ret;
    }
    para->para = used_num;

    if (!trs_is_stars_inst(ts_inst)) {
        /* non stars hw res store in TRS_HW_* and TRS_SW_* type */
        if (para->res_type == TRS_HW_SQ) {
            ret = trs_id_get_used_num(&ts_inst->inst, trs_res_get_id_type(TRS_SW_SQ), &used_num);
            if (ret != 0) {
                return ret;
            }
            para->para += used_num;
        }
        if (para->res_type == TRS_HW_CQ) {
            ret = trs_id_get_used_num(&ts_inst->inst, trs_res_get_id_type(TRS_SW_CQ), &used_num);
            if (ret != 0) {
                return ret;
            }
            para->para += used_num;
        }
    }
    return 0;
}

int trs_res_id_avail_query(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct trs_res_id_para *para)
{
    int type = trs_res_get_id_type(para->res_type);
    int ret;

    (void)proc_ctx;
    ret = trs_id_get_avail_num_in_pool(&ts_inst->inst, type, &para->para);
    if (ret != 0) {
        trs_warn("Not support. (devid=%u; tsid=%u; type=%d)\n", ts_inst->inst.devid, ts_inst->inst.tsid, type);
    }

    return 0;
}

int trs_res_id_reg_offset_query(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct trs_res_id_para *para)
{
    (void)proc_ctx;

    if (!((para->res_type == TRS_NOTIFY) || (para->res_type == TRS_EVENT)) ||
        (para->id >= trs_res_get_max_id(ts_inst, para->res_type))) {
        trs_err("Invalid para. (res_type=%d; res_id=%u)\n", para->res_type, para->id);
        return -EINVAL;
    }

    return ts_inst->ops.get_res_reg_offset(&ts_inst->inst, trs_res_get_id_type(para->res_type), para->id, &para->para);
}

int trs_res_id_reg_size_query(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct trs_res_id_para *para)
{
    (void)proc_ctx;

    if (!((para->res_type == TRS_NOTIFY) || (para->res_type == TRS_EVENT))) {
        trs_err("Invalid para. (res_type=%d)\n", para->res_type);
        return -EINVAL;
    }

    return ts_inst->ops.get_res_reg_total_size(&ts_inst->inst, trs_res_get_id_type(para->res_type), &para->para);
}

int trs_get_stream_logic_cq(struct trs_core_ts_inst *ts_inst, u32 stream_id)
{
    if (stream_id >= trs_res_get_max_id(ts_inst, TRS_STREAM)) {
        struct trs_id_inst *inst = &ts_inst->inst;
        trs_err("Invalid stream id. (devid=%u; tsid=%u; stream_id=%u)\n", inst->devid, inst->tsid, stream_id);
        return -1;
    }

    return ts_inst->stream_ctx[stream_id].logic_cq;
}

static int trs_stream_bind_logic_cq(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    u32 stream_id, u32 logic_cq)
{
    struct trs_stream_ctx *stream_ctx = &ts_inst->stream_ctx[stream_id];
    struct trs_id_inst *inst = &ts_inst->inst;

    if (!trs_proc_has_res(proc_ctx, ts_inst, TRS_LOGIC_CQ, logic_cq)) {
        trs_err("Logic cq not belong to proc. (devid=%u; tsid=%u; stream_id=%u; logic_cq=%u)\n",
                inst->devid, inst->tsid, stream_id, logic_cq);
        return -EINVAL;
    }

    if ((stream_ctx->logic_cq >= 0) && (stream_ctx->logic_cq != logic_cq)) {
        trs_info("Stream Bind new logic cq. (devid=%u; tsid=%u; stream_id=%u; logic_cq=%u; new_logic_cq=%u)\n",
            inst->devid, inst->tsid, stream_id, stream_ctx->logic_cq, logic_cq);
    }

    stream_ctx->logic_cq = (int)logic_cq;
    return 0;
}

static int trs_stream_unbind_logic_cq(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, u32 stream_id)
{
    ts_inst->stream_ctx[stream_id].logic_cq = -1;
    return 0;
}

static int trs_stream_cfg(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_res_id_para *para)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    u32 stream_id = para->id;
    int ret;

    if (!trs_proc_has_res(proc_ctx, ts_inst, TRS_STREAM, stream_id)) {
        trs_err("Stream not belong to proc. (devid=%u; tsid=%u; stream_id=%u)\n", inst->devid, inst->tsid, stream_id);
        return -EINVAL;
    }

    if (para->prop == DRV_STREAM_BIND_LOGIC_CQ) {
        ret = trs_stream_bind_logic_cq(proc_ctx, ts_inst, stream_id, para->para);
    } else if (para->prop == DRV_STREAM_UNBIND_LOGIC_CQ) {
        ret = trs_stream_unbind_logic_cq(proc_ctx, ts_inst, stream_id);
    } else {
        trs_err("Not surport. (devid=%u; tsid=%u; prop=%d)\n", inst->devid, inst->tsid, para->prop);
        ret = -EINVAL;
    }

    return ret;
}

static int trs_set_res_id_ctrl(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    u32 type, u32 id, u32 cmd)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    int ret;

    if ((id >= trs_res_get_id_num(ts_inst, (int)type)) || (ts_inst->ops.res_id_ctrl == NULL)) {
        trs_err("Invalid para. (devid=%u; tsid=%u; type=%u; id=%u)\n", inst->devid, inst->tsid, type, id);
        return -EINVAL;
    }

    ret = ts_inst->ops.res_id_ctrl(inst, type, id, cmd);
    if (ret != 0) {
        trs_err("Set id record failed. (devid=%u; tsid=%u; type=%u; id=%u; cmd=%u; ret=%d)\n",
            inst->devid, inst->tsid, type, id, cmd, ret);
        return ret;
    }

    return 0;
}

static int trs_notify_cfg(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_res_id_para *para)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    u32 notify_id = para->id;

    if (para->prop == DRV_ID_RECORD) {
        return trs_set_res_id_ctrl(proc_ctx, ts_inst, TRS_NOTIFY, notify_id, TRS_RES_OP_RECORD);
    }

    trs_warn("Not surport. (devid=%u; tsid=%u; prop=%d)\n", inst->devid, inst->tsid, para->prop);
    return -EINVAL;
}

static int trs_event_cfg(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_res_id_para *para)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    u32 event_id = para->id;

    if (para->prop == DRV_ID_RECORD) {
        return trs_set_res_id_ctrl(proc_ctx, ts_inst, TRS_EVENT, event_id, TRS_RES_OP_RECORD);
    }

    trs_warn("Not surport. (devid=%u; tsid=%u; prop=%d)\n", inst->devid, inst->tsid, para->prop);
    return -EINVAL;
}

static int (*const trs_res_id_cfg_handles[TRS_MAX_ID_TYPE])(struct trs_proc_ctx *proc_ctx,
    struct trs_core_ts_inst *ts_inst, struct trs_res_id_para *para) = {
    [TRS_STREAM] = trs_stream_cfg,
    [TRS_EVENT] = trs_event_cfg,
    [TRS_NOTIFY] = trs_notify_cfg
};

int trs_res_id_cfg(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_res_id_para *para)
{
    struct trs_id_inst *inst = &ts_inst->inst;

    if (trs_res_id_cfg_handles[para->res_type] == NULL) {
        trs_err("Not surport. (devid=%u; tsid=%u; res_type=%d)\n", inst->devid, inst->tsid, para->res_type);
        return -EINVAL;
    }

    return trs_res_id_cfg_handles[para->res_type](proc_ctx, ts_inst, para);
}

static int trs_stream_id_ctrl(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    u32 stream_id, u32 cmd)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    struct trs_stream_ctx *stream_ctx = NULL;
    int sqid, ret;

    if ((stream_id >= trs_res_get_id_num(ts_inst, TRS_STREAM)) || (ts_inst->ops.res_id_ctrl == NULL)) {
        trs_err("Invalid para. (devid=%u; tsid=%u; stream_id=%u)\n", inst->devid, inst->tsid, stream_id);
        return -EINVAL;
    }

    stream_ctx = &ts_inst->stream_ctx[stream_id];
    sqid = stream_ctx->sq;
    if ((sqid < 0) || ((u32)sqid >= trs_res_get_max_id(ts_inst, TRS_HW_SQ))) {
        trs_err("Stream no sq. (devid=%u; tsid=%u; stream_id=%u; sqId=%d)\n", inst->devid, inst->tsid, stream_id, sqid);
        return -EINVAL;
    }

    if (!trs_proc_has_res(proc_ctx, ts_inst, TRS_STREAM, stream_id)) {
        if (!trs_is_host_pid_match(proc_ctx->pid, stream_ctx->host_pid)) {
            trs_err("Stream not belong to proc. (devid=%u; tsid=%u; stream_id=%u; pid=%d)\n",
                inst->devid, inst->tsid, stream_id, stream_ctx->host_pid);
            return -EINVAL;
        }
    }

    ret = ts_inst->ops.res_id_ctrl(inst, TRS_HW_SQ, (u32)sqid, cmd);
    if (ret != 0) {
        trs_err("Set sq status failed. (devid=%u; tsid=%u; stream_id=%u; sqId=%d; cmd=%u; ret=%d)\n",
            inst->devid, inst->tsid, stream_id, sqid, cmd, ret);
        return ret;
    }

    return 0;
}

int trs_res_id_enable(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_res_id_para *para)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    int ret;

    if (para->res_type == TRS_STREAM) {
        ret = trs_stream_id_ctrl(proc_ctx, ts_inst, para->id, TRS_RES_OP_ENABLE);
    } else {
        trs_err("Not surport. (devid=%u; tsid=%u; res_type=%d)\n", inst->devid, inst->tsid, para->res_type);
        ret = -EINVAL;
    }

    return ret;
}

int trs_res_id_disable(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_res_id_para *para)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    int ret;

    if (para->res_type == TRS_STREAM) {
        ret = trs_stream_id_ctrl(proc_ctx, ts_inst, para->id, TRS_RES_OP_DISABLE);
    } else {
        trs_err("Not surport. (devid=%u; tsid=%u; res_type=%d)\n", inst->devid, inst->tsid, para->res_type);
        ret = -EINVAL;
    }

    return ret;
}

void trs_res_id_recycle(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, int res_type, u32 res_id)
{
    (void)_trs_res_id_free(proc_ctx, ts_inst, res_type, res_id);
}

static void trs_stream_set_bind_sqcq(struct trs_core_ts_inst *ts_inst, u32 stream_id,
    u32 sqid, u32 cqid, int host_pid)
{
    struct trs_stream_ctx *stream_ctx = &ts_inst->stream_ctx[stream_id];

    stream_ctx->sq = (int)sqid;
    stream_ctx->cq = (int)cqid;
    stream_ctx->host_pid = host_pid;
}

void trs_sqcq_reg_map(struct trs_core_ts_inst *ts_inst, struct trs_sqcq_reg_map_para *para)
{
    trs_stream_set_bind_sqcq(ts_inst, para->stream_id, para->sqid, para->cqid, para->host_pid);
    if (ts_inst->ops.sqcq_reg_map != NULL) {
        ts_inst->sq_ctx[para->sqid].reg_mem.cp_proc_state =
            (ts_inst->ops.sqcq_reg_map(&ts_inst->inst, para) == 0) ? 0 : 1;
    }
}

void trs_sqcq_reg_unmap(struct trs_core_ts_inst *ts_inst, struct trs_sqcq_reg_map_para *para)
{
    trs_stream_set_bind_sqcq(ts_inst, para->stream_id, (u32)-1, (u32)-1, -1);
    if (ts_inst->ops.sqcq_reg_unmap != NULL) {
        (void)ts_inst->ops.sqcq_reg_unmap(&ts_inst->inst, para);
    }
}

int trs_stream_bind_remote_sqcq(struct trs_id_inst *inst, u32 stream_id, u32 sqid, u32 cqid, int host_pid)
{
    struct trs_core_ts_inst *ts_inst = NULL;

    if (inst == NULL) {
        trs_err("Null ptr. (stream_id=%u)\n", stream_id);
        return -EINVAL;
    }

    ts_inst = trs_core_ts_inst_get(inst);
    if (ts_inst == NULL) {
        trs_err("Invalid para. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -EINVAL;
    }

    if (stream_id >= trs_res_get_max_id(ts_inst, TRS_STREAM)) {
        trs_core_ts_inst_put(ts_inst);
        trs_err("Invalid para. (devid=%u; tsid=%u; stream_id=%u; sqid=%u; cqid=%u; host_pid=%d)\n",
            inst->devid, inst->tsid, stream_id, sqid, cqid, host_pid);
        return -EINVAL;
    }

    trs_stream_set_bind_sqcq(ts_inst, stream_id, sqid, cqid, host_pid);
    trs_core_ts_inst_put(ts_inst);
    return 0;
}
EXPORT_SYMBOL(trs_stream_bind_remote_sqcq);

static int trs_res_stream_init(struct trs_core_ts_inst *ts_inst)
{
    u32 stream_num = trs_res_get_max_id(ts_inst, TRS_STREAM);
    struct trs_id_inst *inst = &ts_inst->inst;
    struct trs_stream_ctx *stream_ctx = NULL;
    u32 i;

    if (stream_num == 0) {
        return 0;
    }

    stream_ctx = (struct trs_stream_ctx *)vzalloc(sizeof(struct trs_stream_ctx) * stream_num);
    if (stream_ctx == NULL) {
        trs_err("Mem alloc failed. (devid=%u; tsid=%u; size=%lx)\n",
            inst->devid, inst->tsid, sizeof(struct trs_stream_ctx) * stream_num);
        return -ENOMEM;
    }

    for (i = 0; i < stream_num; i++) {
        trs_stream_ctx_init(&stream_ctx[i]);
    }

    ts_inst->stream_ctx = stream_ctx;

    return 0;
}

static void trs_res_stream_uninit(struct trs_core_ts_inst *ts_inst)
{
    if (ts_inst->stream_ctx != NULL) {
        vfree(ts_inst->stream_ctx);
        ts_inst->stream_ctx = NULL;
    }
}

static int trs_res_id_init(struct trs_core_ts_inst *ts_inst, struct trs_res_mng *res_mng, int res_type)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    int id_type = trs_res_get_id_type(trs_res_replace_res_type(ts_inst, res_type));
    int ret;

    ret = trs_id_get_total_num(inst, id_type, &res_mng->id_num);
    if (ret != 0) {
        trs_info("Res not support. (devid=%u; tsid=%u; res_type=%d)\n", inst->devid, inst->tsid, res_type);
        res_mng->id_num = 0;
    }

    if (res_mng->id_num > 0) {
        ret = trs_id_get_max_id(inst, id_type, &res_mng->max_id);
        if (ret != 0) {
            trs_err("Get max id failed. (devid=%u; tsid=%u; res_type=%d; ret=%d)\n",
                inst->devid, inst->tsid, res_type, ret);
            return ret;
        }

        res_mng->ids = (struct trs_res_ids *)vzalloc(sizeof(struct trs_res_ids) * res_mng->max_id);
        if (res_mng->ids == NULL) {
            trs_err("Mem alloc failed. (devid=%u; tsid=%u; res_type=%d; size=%lx)\n",
                inst->devid, inst->tsid, res_type, sizeof(struct trs_res_ids) * res_mng->max_id);
            return -ENOMEM;
        }
    }

    mutex_init(&res_mng->mutex);
    res_mng->res_type = res_type;
    res_mng->ts_inst = ts_inst;
    return 0;
}

static void trs_res_id_uninit(struct trs_res_mng *res_mng)
{
    if (res_mng->ids != NULL) {
        vfree(res_mng->ids);
        res_mng->ids = NULL;
    }
}

void trs_res_mng_uninit(struct trs_core_ts_inst *ts_inst)
{
    int i;

    trs_res_stream_uninit(ts_inst);

    for (i = 0; i < TRS_CORE_MAX_ID_TYPE; i++) {
        trs_res_id_uninit(&ts_inst->res_mng[i]);
    }
}

int trs_res_mng_init(struct trs_core_ts_inst *ts_inst)
{
    int i, ret;

    for (i = 0; i < TRS_CORE_MAX_ID_TYPE; i++) {
        ret = trs_res_id_init(ts_inst, &ts_inst->res_mng[i], i);
        if (ret != 0) {
            trs_res_mng_uninit(ts_inst);
            return ret;
        }
    }

    ret = trs_res_stream_init(ts_inst);
    if (ret != 0) {
        trs_res_mng_uninit(ts_inst);
        return ret;
    }

    return 0;
}

