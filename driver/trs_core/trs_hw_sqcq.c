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
#include <uapi/linux/sched/types.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include "securec.h"

#include "trs_mailbox_def.h"
#include "trs_chan.h"
#include "trs_ts_inst.h"
#include "trs_sqcq_map.h"
#include "trs_proc_fs.h"
#include "trs_hw_sqcq.h"

static bool trs_is_sqcq_pair_mode(struct trs_core_ts_inst *ts_inst)
{
    return trs_is_stars_inst(ts_inst);
}

static inline u32 trs_hw_get_sq_head(struct trs_sq_ctx *sq_ctx)
{
    return *(u32 *)sq_ctx->head.kva;
}

static inline void trs_hw_set_sq_head(struct trs_sq_ctx *sq_ctx, u32 head)
{
    smp_mb();

    *(u32 *)sq_ctx->head.kva = head;
}

static inline u32 trs_hw_get_sq_tail(struct trs_sq_ctx *sq_ctx)
{
    return *(u32 *)sq_ctx->tail.kva;
}

static void trs_trace_hw_cqe_fill(struct trs_id_inst *inst, struct trs_chan_cq_trace *cq_trace, void *cqe)
{
    struct trs_core_ts_inst *ts_inst = trs_core_ts_inst_get(inst);
    if (ts_inst != NULL) {
        if (ts_inst->ops.trace_cqe_fill != NULL) {
            ts_inst->ops.trace_cqe_fill(inst, cq_trace, cqe);
        }
        trs_core_ts_inst_put(ts_inst);
    }
}

static void trs_trace_hw_sqe_fill(struct trs_id_inst *inst, struct trs_chan_sq_trace *sq_trace, void *sqe)
{
    struct trs_core_ts_inst *ts_inst = trs_core_ts_inst_get(inst);
    if (ts_inst != NULL) {
        if (ts_inst->ops.trace_sqe_fill != NULL) {
            ts_inst->ops.trace_sqe_fill(inst, sq_trace, sqe);
        }
        trs_core_ts_inst_put(ts_inst);
    }
}

int trs_hw_sq_send_task(struct trs_sq_ctx *sq_ctx)
{
    struct trs_id_inst *inst = &sq_ctx->inst;
    struct trs_chan_send_para para = {.timeout = 1};
    u32 head, tail, num = 0;
    u32 limit_cnt = 512; /* 512 */

    if ((sq_ctx->status == 0) || (sq_ctx->head.kva == NULL) || (mutex_trylock(&sq_ctx->mutex) == 0)) {
        return 0;
    }

    if ((sq_ctx->status == 0) || (sq_ctx->head.kva == NULL)) {
        mutex_unlock(&sq_ctx->mutex);
        return 0;
    }

    while ((head = trs_hw_get_sq_head(sq_ctx)) != (tail = trs_hw_get_sq_tail(sq_ctx))) {
        int ret;

        if ((head >= sq_ctx->sq_depth) || (tail >= sq_ctx->sq_depth)) {
            trs_err("Invalid head tail. (devid=%u; tsid=%u; sqId=%u; sq_depth=%u; head=%u; tail=%u)\n",
                inst->devid, inst->tsid, sq_ctx->sqid, sq_ctx->sq_depth, head, tail);
            break;
        }

        para.sqe = (u8 *)sq_ctx->que_mem.kva + head * sq_ctx->sqe_size;
        para.sqe_num = 1; /* make sure stars run and task submit at the same time */
        ret = hal_kernel_trs_chan_send(inst, sq_ctx->chan_id, &para);
        if (ret != 0) {
            sq_ctx->send_fail++;
            trs_debug("Send failed. (devid=%u; tsid=%u; sqId=%u; sq_depth=%u; head=%u; tail=%u; ret=%d)\n",
                inst->devid, inst->tsid, sq_ctx->sqid, sq_ctx->sq_depth, head, tail, ret);
            break;
        }

        trs_hw_set_sq_head(sq_ctx, (head + para.sqe_num) % sq_ctx->sq_depth);
        num += para.sqe_num;
        if (num % limit_cnt == 0) {
            cond_resched();
        }
    }

    mutex_unlock(&sq_ctx->mutex);

    if (num > 1) {
        trs_debug("Tx burst. (devid=%u; tsid=%u; sqId=%u; num=%u; tail=%u)\n",
            inst->devid, inst->tsid, sq_ctx->sqid, num, tail);
    }
    return num;
}

static int trs_sq_send_proc(struct trs_core_ts_inst *ts_inst)
{
    struct trs_res_mng *res_mng = &ts_inst->res_mng[TRS_HW_SQ];
    u32 i, sq_max_id = trs_res_get_max_id(ts_inst, TRS_HW_SQ);
    int num = 0;

    /* fast send */
    if (ts_inst->ops.get_trigger_sqid != NULL) {
        u32 sqid;
        int ret = ts_inst->ops.get_trigger_sqid(&ts_inst->inst, &sqid);
        if ((ret == 0) && (sqid < sq_max_id)) {
            if (res_mng->ids[sqid].ref > 0) {
                num += trs_hw_sq_send_task(&ts_inst->sq_ctx[sqid]);
            }
        }
    }

    /* check all sq */
    for (i = 0; i < sq_max_id; i++) {
        if (res_mng->ids[i].ref > 0) {
            num += trs_hw_sq_send_task(&ts_inst->sq_ctx[i]);
        }
    }

    return num;
}

static void trs_set_thread_priority(struct trs_id_inst *inst)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0)
    struct sched_param sp;

    sp.sched_priority = 1;
    if (sched_setscheduler(current, SCHED_FIFO, &sp) != 0) {
        trs_err("Set priority fail. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
    }
#else
    sched_set_fifo_low(current);
#endif
}

static int trs_sq_send_thread(void *arg)
{
    struct trs_core_ts_inst *ts_inst = (struct trs_core_ts_inst *)arg;
    u32 sq_task_sched_time = 4000; /* 4000 us */
    ts_inst->sq_task_flag = 0;

    trs_set_thread_priority(&ts_inst->inst);

    while (!kthread_should_stop()) {
        usleep_range(sq_task_sched_time - 100, sq_task_sched_time); /* 100 us */

        if (ts_inst->sq_task_flag == 0) {
            continue;
        }

        if (trs_sq_send_proc(ts_inst) == 0) {
            sq_task_sched_time = 4000; /* 4000 us */
        } else {
            sq_task_sched_time = 1000; /* 1000 us */
        }
    }

    return 0;
}

static bool trs_sq_work_is_need_retry(struct trs_core_ts_inst *ts_inst, u32 num)
{
    if (delayed_work_pending(&ts_inst->sq_trigger_work)) {
        return false;
    }

#ifndef EMU_ST
    if (num != 0) {
        ts_inst->sq_work_retry_time = 3;    /* 3:retry time */
        return true;
    }

    if ((num == 0) && (ts_inst->sq_work_retry_time > 0)) {
        ts_inst->sq_work_retry_time--;
        return true;
    }

    return false;
#endif
}

static void trs_sq_trigger_work(struct work_struct *p_work)
{
    struct trs_core_ts_inst *ts_inst = container_of(p_work, struct trs_core_ts_inst, sq_trigger_work.work);
    u32 num;

    num = (u32)trs_sq_send_proc(ts_inst);
    if (trs_sq_work_is_need_retry(ts_inst, num)) {
#ifndef EMU_ST
        (void)mod_delayed_work(ts_inst->work_queue, &ts_inst->sq_trigger_work, 0);
#endif
    }
}

static irqreturn_t trs_sq_trigger_irq_proc(int irq, void *para)
{
    struct trs_core_ts_inst *ts_inst = (struct trs_core_ts_inst *)para;

    ts_inst->sq_task_flag = 1;
    (void)mod_delayed_work(ts_inst->work_queue, &ts_inst->sq_trigger_work, 0);
    return IRQ_HANDLED;
}

static void trs_sq_set_cqe_status(struct trs_core_ts_inst *ts_inst, u32 sqid)
{
    struct trs_sq_ctx *sq_ctx = &ts_inst->sq_ctx[sqid];
    struct trs_sq_shr_info *shr_info = NULL;

    spin_lock_bh(&sq_ctx->shr_info_lock);
    shr_info = (struct trs_sq_shr_info *)sq_ctx->shr_info.kva;
    if (shr_info != NULL) {
        shr_info->cqe_status = 1;
    }
    spin_unlock_bh(&sq_ctx->shr_info_lock);

    sq_ctx->cqe_status = 1;
}

static void trs_sq_get_and_clr_cqe_status(struct trs_core_ts_inst *ts_inst, u32 sqid, u32 *cqe_status)
{
    struct trs_sq_ctx *sq_ctx = &ts_inst->sq_ctx[sqid];

    *cqe_status = sq_ctx->cqe_status;
    if (*cqe_status == 1) {
        sq_ctx->cqe_status = 0;
    }
}

static int trs_hw_get_sqid_head_form_cqe(struct trs_core_ts_inst *ts_inst, void *cqe, u32 *sqid, u32 *head)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    int ret;

    ret = ts_inst->ops.get_sq_id_head_from_hw_cqe(inst, cqe, sqid, head);
    if (ret != 0) {
        trs_err("Get sq id head failed. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    if (*sqid >= trs_res_get_max_id(ts_inst, TRS_HW_SQ)) {
        trs_err("Invalid sqid. (devid=%u; tsid=%u; sqid=%u)\n", inst->devid, inst->tsid, *sqid);
        return -EINVAL;
    }

    return 0;
}

static int trs_hw_get_logic_cq_from_cqe(struct trs_core_ts_inst *ts_inst, void *cqe)
{
    u32 streamid;
    int ret;

    if (ts_inst->ops.get_stream_id_from_hw_cqe == NULL) {
        return -1;
    }

    ret = ts_inst->ops.get_stream_id_from_hw_cqe(&ts_inst->inst, cqe, &streamid);
    if (ret != 0) {
        trs_err("Get stream id. (devid=%u; tsid=%u; ret=%d)\n", ts_inst->inst.devid, ts_inst->inst.tsid, ret);
        return -1;
    }

    return trs_get_stream_logic_cq(ts_inst, streamid);
}

static int trs_hw_cq_recv_proc(struct trs_core_ts_inst *ts_inst, u32 cqid, u32 logic_cq, void *cqe)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    struct trs_logic_cqe logic_cqe;
    int ret;

    if (ts_inst->ops.hw_cqe_to_logic_cqe == NULL) {
        return -EINVAL;
    }

    ret = ts_inst->ops.hw_cqe_to_logic_cqe(inst, cqe, (void *)&logic_cqe);
    if (ret != 0) {
        trs_err("trans failed. (devid=%u; tsid=%u; logic_cq=%u)\n", inst->devid, inst->tsid, logic_cq);
        return ret;
    }

    if (logic_cqe.drop_flag != 0) {
        ts_inst->cq_ctx[cqid].stat.rx_drop++;
        return -EINVAL;
    }

    ret = trs_logic_cq_enque(ts_inst, logic_cq, logic_cqe.stream_id, logic_cqe.task_id, (void *)&logic_cqe);
    if (ret != 0) {
        ts_inst->cq_ctx[cqid].stat.rx_enque_fail++;
        return ret;
    }
    ts_inst->cq_ctx[cqid].stat.rx_enque++;
    return 0;
}

static int trs_hw_cq_recv(struct trs_id_inst *inst, u32 cqid, void *cqe)
{
    struct trs_core_ts_inst *ts_inst = NULL;
    u32 sqid, sq_head;
    int recv_result = CQ_RECV_FINISH;
    int logic_cqid, ret = 0;

    ts_inst = trs_core_ts_inst_get(inst);
    if (ts_inst == NULL) {
        trs_err("Invalid para. (devid=%u; tsid=%u; cqid=%u)\n", inst->devid, inst->tsid, cqid);
        return CQ_RECV_FINISH;
    }

    ts_inst->cq_ctx[cqid].stat.rx++;
    logic_cqid = trs_hw_get_logic_cq_from_cqe(ts_inst, cqe);
    if (logic_cqid >= 0) {
        ret = trs_hw_cq_recv_proc(ts_inst, cqid, (u32)logic_cqid, cqe);
    } else {
        recv_result = CQ_RECV_CONTINUE;
    }

    if (trs_hw_get_sqid_head_form_cqe(ts_inst, cqe, &sqid, &sq_head) == 0) {
        if (ret == 0) {
            trs_sq_set_cqe_status(ts_inst, sqid);
        }

        if (!trs_is_sqcq_pair_mode(ts_inst)) {
            (void)trs_chan_ctrl(inst, ts_inst->sq_ctx[sqid].chan_id, CHAN_CTRL_CMD_SQ_HEAD_UPDATE, sq_head);
        }
    }

    trs_core_ts_inst_put(ts_inst);

    return recv_result;
}

static int trs_chan_cq_full_proc(struct trs_id_inst *inst, int chan_id)
{
    u8 cqe[128];    /* max cqe size is 128B */
    struct trs_chan_recv_para recv_para = {
        .cqe = cqe,
        .cqe_num = 1,
        .recv_cqe_num = 0,
        .timeout = 1000
    };
    int ret;

    ret = trs_chan_ctrl(inst, chan_id, CHAN_CTRL_CMD_SQ_STATUS_SET, 0);
    if (ret != 0) {
        trs_err("Set sq status failed. (devid=%u; tsid=%u; chan_id=%d; ret=%d)\n",
            inst->devid, inst->tsid, chan_id, ret);
        return ret;
    }

    ret = hal_kernel_trs_chan_recv(inst, chan_id, &recv_para);
    if (ret != 0) {
        trs_err("Fetch cq and update cq head failed. (devid=%u; tsid=%u; chan_id=%d; ret=%d)\n",
            inst->devid, inst->tsid, chan_id, ret);
        return ret;
    }

    return 0;
}

typedef int (*trs_hw_sqcq_abnormal_handle)(struct trs_id_inst *inst, int chan_id);
static const trs_hw_sqcq_abnormal_handle hw_sqcq_abnormal_handle[ABNORMAL_ERR_TYPE_MAX] = {
    [ABNORMAL_ERR_TYPE_CQ_FULL] = trs_chan_cq_full_proc
};

static int trs_hw_sqcq_abnormal_proc(struct trs_id_inst *inst, int chan_id, u8 err_type)
{
    if (err_type >= ABNORMAL_ERR_TYPE_MAX) {
        return -EINVAL;
    }

    if (hw_sqcq_abnormal_handle[err_type] != NULL) {
        return hw_sqcq_abnormal_handle[err_type](inst, chan_id);
    }

    return 0;
}

static int trs_hw_alloc_chan(struct trs_proc_ctx *proc_ctx, struct trs_id_inst *inst,
    struct halSqCqInputInfo *para, int *chan_id)
{
    struct trs_chan_para chan_para = {0};
    int ret;

    chan_para.types.type = CHAN_TYPE_HW;
    chan_para.types.sub_type = ((para->flag & TSDRV_FLAG_ONLY_SQCQ_ID)) == 0 ?
        CHAN_SUB_TYPE_HW_RTS : CHAN_SUB_TYPE_HW_TS;

    chan_para.ssid = proc_ctx->cp_ssid;
    chan_para.flag = 0;
    if ((para->flag & TSDRV_FLAG_REUSE_SQ) == 0) {
        chan_para.sq_para.sqe_size = para->sqeSize;
        chan_para.sq_para.sq_depth = para->sqeDepth;
        chan_para.ops.trace_sqe_fill = trs_trace_hw_sqe_fill;
        chan_para.flag |= (0x1 << CHAN_FLAG_ALLOC_SQ_BIT);
    }

    if ((para->flag & TSDRV_FLAG_REUSE_CQ) == 0) {
        chan_para.cq_para.cqe_size = para->cqeSize;
        chan_para.cq_para.cq_depth = para->cqeDepth;
        chan_para.ops.cqe_is_valid = NULL;
        chan_para.ops.trace_cqe_fill = trs_trace_hw_cqe_fill;
        chan_para.ops.get_sq_head_in_cqe = NULL;
        chan_para.ops.cq_recv = trs_hw_cq_recv;
        chan_para.ops.abnormal_proc = trs_hw_sqcq_abnormal_proc;
        chan_para.flag |= (0x1 << CHAN_FLAG_ALLOC_CQ_BIT);
    }

    if (((para->flag & TSDRV_FLAG_REUSE_SQ) == 0) && ((para->flag & TSDRV_FLAG_REUSE_CQ) == 0)) {
        chan_para.flag |= (0x1 << CHAN_FLAG_NOTICE_TS_BIT) | (0x1 << CHAN_FLAG_AUTO_UPDATE_SQ_HEAD_BIT);
    }

    if ((para->flag & TSDRV_FLAG_ONLY_SQCQ_ID) != 0) {
        chan_para.flag |= (0x1 << CHAN_FLAG_NO_ALLOC_SQCQ_MEM_BIT);
    }

    ret = memcpy_s(chan_para.msg, sizeof(chan_para.msg), para->info, sizeof(para->info));
    if (ret != 0) {
        trs_err("Memcopy failed. (dest_len=%lx; src_len=%lx)\n", sizeof(chan_para.msg), sizeof(para->info));
        return ret;
    }

    if ((para->flag & TSDRV_FLAG_REMOTE_ID) != 0) {
        int master_pid;

        chan_para.flag |= (0x1 << CHAN_FLAG_USE_MASTER_PID_BIT);
        ret = trs_core_get_host_pid(current->tgid, &master_pid);
        if (ret != 0) {
            return ret;
        }
        chan_para.msg[SQCQ_INFO_LENGTH - 1] = (u32)master_pid;
    }

    return hal_kernel_trs_chan_create(inst, &chan_para, chan_id);
}

static int trs_pair_mode_alloc_para_check(struct trs_id_inst *inst, struct halSqCqInputInfo *para)
{
    if (((para->flag & TSDRV_FLAG_REUSE_SQ) != 0) || ((para->flag & TSDRV_FLAG_REUSE_CQ) != 0)) {
        trs_err("Not surport reuse, pair. (devid=%u; tsid=%u; flag=%x)\n", inst->devid, inst->tsid, para->flag);
        return -EINVAL;
    }

    if (para->sqeSize != 64) {  /* rtsq sqe size must be 64 */
        trs_err("Invalid sqe size. (devid=%u; tsid=%u; sqeSize=%u)\n",
            inst->devid, inst->tsid, para->sqeSize);
        return -EINVAL;
    }

    return 0;
}

static int trs_hw_sqcq_alloc_pair(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct halSqCqInputInfo *para)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    struct trs_sqcq_reg_map_para reg_map_para;
    struct trs_chan_sq_info sq_info;
    struct trs_chan_cq_info cq_info;
    u32 stream_id = para->info[0];
    int chan_id, ret;

    ret = trs_pair_mode_alloc_para_check(inst, para);
    if (ret != 0) {
        return ret;
    }

    if (trs_is_proc_res_limited(proc_ctx, ts_inst, TRS_HW_SQ)) {
        return -ENOSPC;
    }

    ret = trs_hw_alloc_chan(proc_ctx, &ts_inst->inst, para, &chan_id);
    if (ret != 0) {
        trs_err("Alloc chan failed. (devid=%u; tsid=%u; type=%d)\n", inst->devid, inst->tsid, para->type);
        return ret;
    }

    (void)trs_chan_get_sq_info(inst, chan_id, &sq_info);
    (void)trs_chan_get_cq_info(inst, chan_id, &cq_info);

    ret = trs_proc_add_res(proc_ctx, ts_inst, TRS_HW_SQ, sq_info.sqid);
    if (ret != 0) {
        goto destroy_chan;
    }

    ret = trs_proc_add_res(proc_ctx, ts_inst, TRS_HW_CQ, cq_info.cqid);
    if (ret != 0) {
        goto del_sq_res;
    }

    para->sqId = sq_info.sqid;
    para->cqId = cq_info.cqid;

    trs_sq_ctx_init(inst, &ts_inst->sq_ctx[sq_info.sqid], para, stream_id, chan_id);
    trs_cq_ctx_init(&ts_inst->cq_ctx[cq_info.cqid], U32_MAX, chan_id);

    reg_map_para.stream_id = stream_id;
    reg_map_para.sqid = sq_info.sqid;
    reg_map_para.cqid = cq_info.cqid;
    reg_map_para.host_pid = proc_ctx->pid;
    trs_sqcq_reg_map(ts_inst, &reg_map_para);
    /* If the map fails, the UIO is not used. Therefore, the return value of the map is ignored. */
    (void)trs_sq_remap(proc_ctx, ts_inst, para, &ts_inst->sq_ctx[sq_info.sqid], &sq_info);
    trs_set_sq_status(&ts_inst->sq_ctx[sq_info.sqid], 1);

    trs_debug("Alloc pair sqcq. (devid=%u; tsid=%u; sqId=%u; cqId=%u)\n",
        inst->devid, inst->tsid, para->sqId, para->cqId);
    return 0;

del_sq_res:
    (void)trs_proc_del_res(proc_ctx, ts_inst, TRS_HW_SQ, sq_info.sqid);

destroy_chan:
    hal_kernel_trs_chan_destroy(inst, chan_id);
    trs_err("Sqcq Alloc failed. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
    return ret;
}

static int pair_mode_free_para_check(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct halSqCqFreeInfo *para)
{
    struct trs_id_inst *inst = &ts_inst->inst;

    if ((para->flag & TSDRV_FLAG_REUSE_CQ) != 0) {
        trs_err("Not surport only free sq. (devid=%u; tsid=%u; flag=%x)\n", inst->devid, inst->tsid, para->flag);
        return -EINVAL;
    }

    if (!trs_proc_has_res(proc_ctx, ts_inst, TRS_HW_SQ, para->sqId)) {
        trs_err("Not proc sq. (devid=%u; tsid=%u; sq=%u; cq=%u)\n", inst->devid, inst->tsid, para->sqId, para->cqId);
        return -EINVAL;
    }

    if (!trs_proc_has_res(proc_ctx, ts_inst, TRS_HW_CQ, para->cqId)) {
        trs_err("Not proc cq. (devid=%u; tsid=%u; sq=%u; cq=%u)\n", inst->devid, inst->tsid, para->sqId, para->cqId);
        return -EINVAL;
    }

    if (ts_inst->sq_ctx[para->sqId].chan_id != ts_inst->cq_ctx[para->cqId].chan_id) {
        trs_err("Not pair sqcq. (devid=%u; tsid=%u; sqId=%u; cqId=%u)\n",
            inst->devid, inst->tsid, para->sqId, para->cqId);
        return -EINVAL;
    }

    return 0;
}

static void _trs_hw_sqcq_free_pair(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, u32 sqId, u32 cqId)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    int chan_id;

    trs_debug("Free pair sqcq. (devid=%u; tsid=%u; sqId=%u; cqId=%u)\n", inst->devid, inst->tsid, sqId, cqId);

    chan_id = ts_inst->sq_ctx[sqId].chan_id;

    trs_sq_ctx_uninit(&ts_inst->sq_ctx[sqId]);
    trs_cq_ctx_uninit(&ts_inst->cq_ctx[cqId]);
    if (trs_cq_is_need_show(&ts_inst->cq_ctx[cqId])) {
        trs_hw_cq_show(ts_inst, cqId);
    }
    (void)trs_proc_del_res(proc_ctx, ts_inst, TRS_HW_SQ, sqId);
    (void)trs_proc_del_res(proc_ctx, ts_inst, TRS_HW_CQ, cqId);
    hal_kernel_trs_chan_destroy(inst, chan_id);
}

static int trs_hw_sqcq_free_pair(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct halSqCqFreeInfo *para)
{
    struct trs_sqcq_reg_map_para reg_map_para;
    struct trs_sq_ctx *sq_ctx = NULL;
    int ret;

    ret = pair_mode_free_para_check(proc_ctx, ts_inst, para);
    if (ret != 0) {
        return ret;
    }
    sq_ctx = &ts_inst->sq_ctx[para->sqId];
    trs_hw_sq_show(ts_inst, para->sqId);
    trs_set_sq_status(sq_ctx, 0);

    reg_map_para.stream_id = sq_ctx->stream_id;
    reg_map_para.sqid = para->sqId;
    reg_map_para.host_pid = proc_ctx->pid;
    trs_sqcq_reg_unmap(ts_inst, &reg_map_para);
    trs_sq_unmap(proc_ctx, ts_inst, sq_ctx);
    _trs_hw_sqcq_free_pair(proc_ctx, ts_inst, para->sqId, para->cqId);
    return 0;
}

static int non_pair_mode_alloc_para_check(struct trs_proc_ctx *proc_ctx,
    struct trs_core_ts_inst *ts_inst, struct halSqCqInputInfo *para)
{
    struct trs_id_inst *inst = &ts_inst->inst;

    if (((para->flag & TSDRV_FLAG_REUSE_SQ) != 0) && ((para->flag & TSDRV_FLAG_REUSE_CQ) == 0)) {
        trs_err("Not surport only reuse sq. (devid=%u; tsid=%u; flag=%x)\n", inst->devid, inst->tsid, para->flag);
        return -EINVAL;
    }

    if ((para->flag & TSDRV_FLAG_REUSE_SQ) != 0) {
        if (!trs_proc_has_res(proc_ctx, ts_inst, TRS_HW_SQ, para->sqId)) {
            trs_err("Not proc owner sq. (devid=%u; tsid=%u; sqId=%u)\n", inst->devid, inst->tsid, para->sqId);
            return -EINVAL;
        }
    }

    if ((para->flag & TSDRV_FLAG_REUSE_CQ) != 0) {
        if (!trs_proc_has_res(proc_ctx, ts_inst, TRS_HW_CQ, para->cqId)) {
            trs_err("Not proc owner cq. (devid=%u; tsid=%u; cqId=%u)\n", inst->devid, inst->tsid, para->cqId);
            return -EINVAL;
        }
    }

    return 0;
}

static int trs_hw_sqcq_alloc_sq_chan(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct halSqCqInputInfo *para, struct trs_chan_sq_info *sq_info, int *chan_id)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    int ret;

    if (trs_is_proc_res_limited(proc_ctx, ts_inst, TRS_HW_SQ)) {
        return -ENOSPC;
    }

    para->flag = TSDRV_FLAG_REUSE_CQ;
    ret = trs_hw_alloc_chan(proc_ctx, &ts_inst->inst, para, chan_id);
    if (ret != 0) {
        trs_err("Alloc sq chan failed. (devid=%u; tsid=%u; type=%d)\n", inst->devid, inst->tsid, (int)para->type);
        return ret;
    }

    (void)trs_chan_get_sq_info(inst, *chan_id, sq_info);

    ret = trs_proc_add_res(proc_ctx, ts_inst, TRS_HW_SQ, sq_info->sqid);
    if (ret != 0) {
        hal_kernel_trs_chan_destroy(inst, *chan_id);
        return ret;
    }

    return 0;
}

static void trs_hw_sqcq_free_sq_chan(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, u32 sqid)
{
    (void)trs_proc_del_res(proc_ctx, ts_inst, TRS_HW_SQ, sqid);
    hal_kernel_trs_chan_destroy(&ts_inst->inst, ts_inst->sq_ctx[sqid].chan_id);
    trs_sq_ctx_uninit(&ts_inst->sq_ctx[sqid]);
}

static int trs_hw_sqcq_alloc_cq_chan(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct halSqCqInputInfo *para, struct trs_chan_cq_info *cq_info, int *chan_id)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    int ret;

    para->flag = TSDRV_FLAG_REUSE_SQ;
    ret = trs_hw_alloc_chan(proc_ctx, &ts_inst->inst, para, chan_id);
    if (ret != 0) {
        trs_err("Alloc cq chan failed. (devid=%u; tsid=%u; type=%d)\n", inst->devid, inst->tsid, para->type);
        return ret;
    }

    (void)trs_chan_get_cq_info(inst, *chan_id, cq_info);

    ret = trs_proc_add_res(proc_ctx, ts_inst, TRS_HW_CQ, cq_info->cqid);
    if (ret != 0) {
        hal_kernel_trs_chan_destroy(inst, *chan_id);
        return ret;
    }

    return 0;
}

static void trs_hw_sqcq_free_cq_chan(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, u32 cqid)
{
    (void)trs_proc_del_res(proc_ctx, ts_inst, TRS_HW_CQ, cqid);
    hal_kernel_trs_chan_destroy(&ts_inst->inst, ts_inst->cq_ctx[cqid].chan_id);
    trs_cq_ctx_uninit(&ts_inst->cq_ctx[cqid]);
}

static void trs_fill_notice_ts_msg(struct trs_proc_ctx *proc_ctx, u16 cmd_type,
    struct trs_chan_sq_info *sq_info, struct trs_chan_cq_info *cq_info, struct trs_normal_cqsq_mailbox *msg)
{
    trs_mbox_init_header(&msg->header, cmd_type);

    msg->sq_index = sq_info->sqid;
    msg->sqesize = sq_info->sq_para.sqe_size;
    msg->sqdepth = sq_info->sq_para.sq_depth;
    msg->sq_addr = sq_info->sq_phy_addr;

    msg->cq0_index = cq_info->cqid;
    msg->cqesize = cq_info->cq_para.cqe_size;
    msg->cqdepth = cq_info->cq_para.cq_depth;
    msg->cq0_addr = cq_info->cq_phy_addr;

    msg->pid = proc_ctx->pid;
    msg->cq_irq = (u16)cq_info->irq;
    msg->ssid = proc_ctx->cp_ssid;

    /* adapt fill: app_type, fid, sq_cq_side */
}

static int trs_hw_sqcq_alloc_notice_ts(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct halSqCqInputInfo *para, struct trs_chan_sq_info *sq_info, struct trs_chan_cq_info *cq_info)
{
    struct trs_normal_cqsq_mailbox msg;
    int ret;

    ret = memcpy_s(msg.info, sizeof(msg.info), para->info, sizeof(para->info));
    if (ret != 0) {
        trs_err("Memcopy failed. (dest_len=%lx; src_len=%lx)\n", sizeof(msg.info), sizeof(para->info));
        return ret;
    }

    trs_fill_notice_ts_msg(proc_ctx, TRS_MBOX_CREATE_CQSQ_CALC, sq_info, cq_info, &msg);

    return trs_core_notice_ts(ts_inst, (u8 *)&msg, sizeof(msg));
}

static int trs_hw_sqcq_alloc_non_pair(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct halSqCqInputInfo *para)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    struct trs_sqcq_reg_map_para reg_map_para;
    struct trs_chan_sq_info sq_info;
    struct trs_chan_cq_info cq_info;
    u32 flag = para->flag;
    u32 stream_id = para->info[0];
    int sq_chan_id, cq_chan_id, ret;

    ret = non_pair_mode_alloc_para_check(proc_ctx, ts_inst, para);
    if (ret != 0) {
        return ret;
    }

    if ((flag & TSDRV_FLAG_REUSE_SQ) == 0) {
        ret = trs_hw_sqcq_alloc_sq_chan(proc_ctx, ts_inst, para, &sq_info, &sq_chan_id);
        if (ret != 0) {
            return ret;
        }
        para->sqId = sq_info.sqid;
    } else {
        sq_info.sqid = para->sqId;
        sq_info.sq_phy_addr = 0;
        sq_info.tail_addr = 0;
    }

    if ((flag & TSDRV_FLAG_REUSE_CQ) == 0) {
        ret = trs_hw_sqcq_alloc_cq_chan(proc_ctx, ts_inst, para, &cq_info, &cq_chan_id);
        if (ret != 0) {
            if (sq_info.sq_phy_addr != 0) {
                trs_hw_sqcq_free_sq_chan(proc_ctx, ts_inst, sq_info.sqid);
            }
            return ret;
        }
        para->cqId = cq_info.cqid;
    } else {
        cq_info.cqid = para->cqId;
        cq_info.cq_phy_addr = 0;
        cq_info.irq = 0;
    }

    ts_inst->sq_ctx[sq_info.sqid].cqid = para->cqId;

    para->flag = flag;

    ret = trs_hw_sqcq_alloc_notice_ts(proc_ctx, ts_inst, para, &sq_info, &cq_info);
    if (ret != 0) {
        if ((flag & TSDRV_FLAG_REUSE_SQ) == 0) {
            trs_hw_sqcq_free_sq_chan(proc_ctx, ts_inst, sq_info.sqid);
        }
        if ((flag & TSDRV_FLAG_REUSE_CQ) == 0) {
            trs_hw_sqcq_free_cq_chan(proc_ctx, ts_inst, cq_info.cqid);
        }
        trs_err("Notice ts failed. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    if (sq_info.sq_phy_addr != 0) {
        trs_sq_ctx_init(inst, &ts_inst->sq_ctx[sq_info.sqid], para, stream_id, sq_chan_id);
    }
    if (cq_info.cq_phy_addr != 0) {
        trs_cq_ctx_init(&ts_inst->cq_ctx[cq_info.cqid], U32_MAX, cq_chan_id);
    }
    reg_map_para.sqid = sq_info.sqid;
    reg_map_para.cqid = cq_info.cqid;
    reg_map_para.host_pid = proc_ctx->pid;
    reg_map_para.stream_id = stream_id;
    trs_sqcq_reg_map(ts_inst, &reg_map_para);

    if (sq_info.sq_phy_addr != 0) {
        /* If the map fails, the UIO is not used. Therefore, the return value of the map is ignored. */
        (void)trs_sq_remap(proc_ctx, ts_inst, para, &ts_inst->sq_ctx[sq_info.sqid], &sq_info);
        trs_set_sq_status(&ts_inst->sq_ctx[sq_info.sqid], 1);
    }

    trs_info("Alloc non pair sqcq. (devid=%u; tsid=%u; sqId=%u; cqId=%u; flag=%x)\n",
        inst->devid, inst->tsid, para->sqId, para->cqId, para->flag);

    return 0;
}

static int non_pair_mode_free_para_check(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct halSqCqFreeInfo *para)
{
    struct trs_id_inst *inst = &ts_inst->inst;

    if (!trs_proc_has_res(proc_ctx, ts_inst, TRS_HW_SQ, para->sqId)) {
        trs_err("Not proc owner sq. (devid=%u; tsid=%u; sqId=%u)\n", inst->devid, inst->tsid, para->sqId);
        return -EINVAL;
    }

    if ((para->flag & TSDRV_FLAG_REUSE_CQ) == 0) {
        if (!trs_proc_has_res(proc_ctx, ts_inst, TRS_HW_CQ, para->cqId)) {
            trs_err("Not proc owner cq. (devid=%u; tsid=%u; cqId=%u)\n", inst->devid, inst->tsid, para->cqId);
            return -EINVAL;
        }
    }

    return 0;
}

static int trs_hw_sqcq_free_notice_ts(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct halSqCqFreeInfo *para)
{
    struct trs_normal_cqsq_mailbox msg;
    struct trs_chan_sq_info sq_info;
    struct trs_chan_cq_info cq_info;
    int chan_id = ts_inst->sq_ctx[para->sqId].chan_id;

    (void)trs_chan_get_sq_info(&ts_inst->inst, chan_id, &sq_info);

    if ((para->flag & TSDRV_FLAG_REUSE_CQ) == 0) {
        chan_id = ts_inst->cq_ctx[para->cqId].chan_id;
        (void)trs_chan_get_cq_info(&ts_inst->inst, chan_id, &cq_info);
    } else {
        cq_info.cqid = TRS_MBOX_INVALID_INDEX;
    }

    msg.info[0] = ts_inst->sq_ctx[sq_info.sqid].stream_id;
    trs_fill_notice_ts_msg(proc_ctx, TRS_MBOX_RELEASE_CQSQ_CALC, &sq_info, &cq_info, &msg);

    return trs_core_notice_ts(ts_inst, (u8 *)&msg, sizeof(msg));
}

static int trs_hw_sqcq_free_non_pair(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct halSqCqFreeInfo *para)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    int ret;

    ret = non_pair_mode_free_para_check(proc_ctx, ts_inst, para);
    if (ret != 0) {
        return ret;
    }

    ret = trs_hw_sqcq_free_notice_ts(proc_ctx, ts_inst, para);
    if (ret != 0) {
        trs_info("Notice ts failed. (devid=%u; tsid=%u; sqId=%u; cqId=%u; flag=%x)\n",
            inst->devid, inst->tsid, para->sqId, para->cqId, para->flag);
        return ret;
    }

    trs_info("Free non pair sqcq. (devid=%u; tsid=%u; sqId=%u; cqId=%u; flag=%x)\n",
        inst->devid, inst->tsid, para->sqId, para->cqId, para->flag);

    trs_set_sq_status(&ts_inst->sq_ctx[para->sqId], 0);
    trs_sq_unmap(proc_ctx, ts_inst, &ts_inst->sq_ctx[para->sqId]);
    trs_hw_sqcq_free_sq_chan(proc_ctx, ts_inst, para->sqId);

    if ((para->flag & TSDRV_FLAG_REUSE_CQ) == 0) {
        trs_hw_sqcq_free_cq_chan(proc_ctx, ts_inst, para->cqId);
    }

    return 0;
}

int trs_hw_sqcq_alloc(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct halSqCqInputInfo *para)
{
    u32 stream_id = para->info[0];

    if (!trs_proc_has_res(proc_ctx, ts_inst, TRS_STREAM, stream_id)) {
        trs_err("No stream. (devid=%u; tsid=%u; stream_id=%u)\n", ts_inst->inst.devid, ts_inst->inst.tsid, stream_id);
        return -EINVAL;
    }

    if (trs_is_sqcq_pair_mode(ts_inst)) {
        return trs_hw_sqcq_alloc_pair(proc_ctx, ts_inst, para);
    } else {
        return trs_hw_sqcq_alloc_non_pair(proc_ctx, ts_inst, para);
    }
}

int trs_hw_sqcq_free(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct halSqCqFreeInfo *para)
{
    if (trs_is_sqcq_pair_mode(ts_inst)) {
        return trs_hw_sqcq_free_pair(proc_ctx, ts_inst, para);
    } else {
        return trs_hw_sqcq_free_non_pair(proc_ctx, ts_inst, para);
    }
}

int trs_sqcq_config(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct halSqCqConfigInfo *para)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    u32 sqid = para->sqId;
    int chan_id, ret;

    if (!trs_proc_has_res(proc_ctx, ts_inst, TRS_HW_SQ, sqid)) {
        trs_err("Proc no sq. (devid=%u; tsid=%u; sqId=%u)\n", ts_inst->inst.devid, ts_inst->inst.tsid, sqid);
        return -EINVAL;
    }

    chan_id = ts_inst->sq_ctx[sqid].chan_id;
    switch (para->prop) {
        case DRV_SQCQ_PROP_SQ_STATUS:
            ret = trs_chan_ctrl(inst, chan_id, CHAN_CTRL_CMD_SQ_STATUS_SET, para->value[0]);
            break;
        case DRV_SQCQ_PROP_SQ_HEAD:
            ret = trs_chan_ctrl(inst, chan_id, CHAN_CTRL_CMD_SQ_HEAD_SET, para->value[0]);
            break;
        case DRV_SQCQ_PROP_SQ_DISABLE_TO_ENABLE:
            ret = trs_chan_ctrl(inst, chan_id, CHAN_CTRL_CMD_SQ_DISABLE_TO_ENABLE, para->value[0]);
            break;
        default:
            trs_err("Invalid prop. (devid=%u; tsid=%u, prop=%u)\n", inst->devid, inst->tsid, para->prop);
            return -EINVAL;
    }

    if (ret != 0) {
        trs_err("Cfg failed. (devid=%u; tsid=%u; sqId=%u; prop=%d)\n", inst->devid, inst->tsid, sqid, para->prop);
    }

    return ret;
}

int trs_set_sq_reg_vaddr(struct trs_id_inst *inst, u32 sqid, u64 va, size_t size)
{
    struct trs_core_ts_inst *ts_inst = NULL;

    if (inst == NULL) {
        trs_err("Null ptr. (sqid=%u)\n", sqid);
        return -EINVAL;
    }
    ts_inst = trs_core_ts_inst_get(inst);
    if (ts_inst == NULL) {
        trs_err("Invalid para. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -EINVAL;
    }
    if (sqid >= trs_res_get_max_id(ts_inst, TRS_HW_SQ)) {
        trs_core_ts_inst_put(ts_inst);
        trs_err("Invalid para. (devid=%u; tsid=%u; sqid=%u)\n",
            inst->devid, inst->tsid, sqid);
        return -EINVAL;
    }
    ts_inst->sq_ctx[sqid].reg_mem.uva = va;
    ts_inst->sq_ctx[sqid].reg_mem.len = size;
    trs_core_ts_inst_put(ts_inst);
    return 0;
}
EXPORT_SYMBOL(trs_set_sq_reg_vaddr);

int trs_get_sq_reg_vaddr(struct trs_id_inst *inst, u32 sqid, u64 *va, size_t *size)
{
    struct trs_core_ts_inst *ts_inst = NULL;

    if (inst == NULL) {
        trs_err("Null ptr. (sqid=%u)\n", sqid);
        return -EINVAL;
    }
    ts_inst = trs_core_ts_inst_get(inst);
    if (ts_inst == NULL) {
        trs_err("Invalid para. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -EINVAL;
    }
    if (sqid >= trs_res_get_max_id(ts_inst, TRS_HW_SQ)) {
        trs_core_ts_inst_put(ts_inst);
        trs_err("Invalid para. (devid=%u; tsid=%u; sqid=%u)\n", inst->devid, inst->tsid, sqid);
        return -EINVAL;
    }
    if (va != NULL) {
        *va = ts_inst->sq_ctx[sqid].reg_mem.uva;
    }
    if (size != NULL) {
        *size = ts_inst->sq_ctx[sqid].reg_mem.len;
    }
    trs_core_ts_inst_put(ts_inst);
    return 0;
}
EXPORT_SYMBOL(trs_get_sq_reg_vaddr);

static int trs_query_sq_reg_vaddr(struct trs_id_inst *inst, u32 sqid, struct halSqCqQueryInfo *para)
{
    size_t size;
    int ret;
    u64 va;

    ret = trs_get_sq_reg_vaddr(inst, sqid, &va, &size);
    if (ret == 0) {
        u32 mask = sizeof(uint32_t) * BITS_PER_BYTE;
        para->value[0] = (uint32_t)(va >> mask);
        para->value[1] = (uint32_t)(va & ((1ULL << mask) - 1));
        para->value[2] = (uint32_t)size; /* 2 return Sq reg size */
    }
    return ret;
}

int trs_sqcq_query(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct halSqCqQueryInfo *para)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    u32 sqid = para->sqId;
    int chan_id, ret;

    if (!trs_proc_has_res(proc_ctx, ts_inst, TRS_HW_SQ, sqid)) {
        trs_err("Proc no sq. (devid=%u; tsid=%u; sqId=%u)\n", ts_inst->inst.devid, ts_inst->inst.tsid, sqid);
        return -EINVAL;
    }

    chan_id = ts_inst->sq_ctx[sqid].chan_id;
    switch (para->prop) {
        case DRV_SQCQ_PROP_SQ_STATUS:
            ret = trs_chan_query(inst, chan_id, CHAN_QUERY_CMD_SQ_STATUS, &para->value[0]);
            break;
        case DRV_SQCQ_PROP_SQ_HEAD:
            ret = trs_chan_query(inst, chan_id, CHAN_QUERY_CMD_SQ_HEAD, &para->value[0]);
            break;
        case DRV_SQCQ_PROP_SQ_TAIL:
            ret = trs_chan_query(inst, chan_id, CHAN_QUERY_CMD_SQ_TAIL, &para->value[0]);
            break;
        case DRV_SQCQ_PROP_SQ_CQE_STATUS:
            trs_sq_get_and_clr_cqe_status(ts_inst, sqid, &para->value[0]);
            ret = 0;
            break;
        case DRV_SQCQ_PROP_SQ_REG_BASE:
            ret = trs_query_sq_reg_vaddr(inst, sqid, para);
            break;
        default:
            trs_err("Invalid prop. (devid=%u; tsid=%u, prop=%u)\n", inst->devid, inst->tsid, para->prop);
            return -EINVAL;
    }

    if (ret != 0) {
        trs_err("Query failed. (devid=%u; tsid=%u; sqId=%u; prop=%d; ret=%d)\n",
            inst->devid, inst->tsid, sqid, para->prop, ret);
    }

    return ret;
}

int trs_hw_sqcq_send(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct halTaskSendInfo *para)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    struct trs_chan_send_para send_para;
    int ret;

    if (!trs_proc_has_res(proc_ctx, ts_inst, TRS_HW_SQ, para->sqId)) {
        trs_err("Not proc owner sq. (devid=%u; tsid=%u; sqId=%u)\n", inst->devid, inst->tsid, para->sqId);
        return -EINVAL;
    }

    send_para.sqe = para->sqe_addr;
    send_para.sqe_num = para->sqe_num;
    send_para.timeout = para->timeout;

    ret = trs_chan_send(inst, ts_inst->sq_ctx[para->sqId].chan_id, &send_para);
    para->pos = send_para.first_pos;
    return ret;
}

int trs_hw_sqcq_recv(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct halReportRecvInfo *para)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    struct trs_chan_recv_para recv_para;
    int ret;

    if (!trs_proc_has_res(proc_ctx, ts_inst, TRS_HW_CQ, para->cqId)) {
        trs_err("Not proc owner cq. (devid=%u; tsid=%u; cqId=%u)\n", inst->devid, inst->tsid, para->cqId);
        return -EINVAL;
    }

    recv_para.cqe = para->cqe_addr;
    recv_para.cqe_num = para->cqe_num;
    recv_para.timeout = para->timeout;

    ret = trs_chan_recv(inst, ts_inst->cq_ctx[para->cqId].chan_id, &recv_para);
    if (ret != 0) {
        return ret;
    }

    para->report_cqe_num = recv_para.recv_cqe_num;

    return 0;
}

void trs_hw_sqcq_recycle(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, int res_type, u32 res_id)
{
    struct trs_id_inst *inst = &ts_inst->inst;

    if (res_type == TRS_HW_SQ) {
        struct trs_sq_ctx *sq_ctx = &ts_inst->sq_ctx[res_id];
        trs_set_sq_status(sq_ctx, 0);
        trs_sq_ctx_mem_free(sq_ctx);
        trs_sq_clear_map_info(sq_ctx);
        if (trs_is_sqcq_pair_mode(ts_inst)) {
            _trs_hw_sqcq_free_pair(proc_ctx, ts_inst, res_id, sq_ctx->cqid);
        } else {
            trs_hw_sqcq_free_sq_chan(proc_ctx, ts_inst, res_id);
        }
    } else {
        if (trs_is_sqcq_pair_mode(ts_inst)) {
            trs_err("Unexpected. (devid=%u; tsid=%u; cqId=%u)\n", inst->devid, inst->tsid, res_id);
        } else {
            trs_hw_sqcq_free_cq_chan(proc_ctx, ts_inst, res_id);
        }
    }
}

int trs_hw_sq_send_thread_create(struct trs_core_ts_inst *ts_inst)
{
    struct trs_id_inst *inst = &ts_inst->inst;

    ts_inst->sq_task = kthread_create(trs_sq_send_thread, (void *)ts_inst, "dev%u_sq_task", inst->devid);
    if (IS_ERR_OR_NULL(ts_inst->sq_task)) {
        trs_err("Failed to start hw sq send thread. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -1;
    }

    if (ts_inst->ops.set_thread_affinity != NULL) {
        ts_inst->ops.set_thread_affinity(inst, ts_inst->sq_task);
    }

    wake_up_process(ts_inst->sq_task);

    return 0;
}

void trs_hw_sq_send_thread_destroy(struct trs_core_ts_inst *ts_inst)
{
    if (!IS_ERR_OR_NULL(ts_inst->sq_task)) {
        (void)kthread_stop(ts_inst->sq_task);
        ts_inst->sq_task = NULL;
    }
}

static int trs_hw_sq_trigger_init(struct trs_core_ts_inst *ts_inst)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    int ret;

    if (ts_inst->ops.get_sq_trigger_irq == NULL) {
        return 0;
    }

    ret = ts_inst->ops.get_sq_trigger_irq(inst, &ts_inst->sq_trigger_irq);
    if (ret != 0) {
        trs_err("Get irq failed. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    ts_inst->work_queue = alloc_workqueue("dev%u_sq_send_wq", __WQ_LEGACY | WQ_MEM_RECLAIM | WQ_HIGHPRI | WQ_UNBOUND,
        1, inst->devid);
    if (ts_inst->work_queue == NULL) {
        trs_err("Createn wq failed. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return ret;
    }

    INIT_DELAYED_WORK(&ts_inst->sq_trigger_work, trs_sq_trigger_work);
    ret = trs_hw_sq_send_thread_create(ts_inst);
    if (ret != 0) {
        destroy_workqueue(ts_inst->work_queue);
        trs_err("Failed to create hw sq send thread. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    ret = request_irq(ts_inst->sq_trigger_irq, trs_sq_trigger_irq_proc, 0, "sq_trigger", (void *)ts_inst);
    if (ret != 0) {
        destroy_workqueue(ts_inst->work_queue);
        trs_hw_sq_send_thread_destroy(ts_inst);
        trs_err("Request irq failed. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    if (ts_inst->ops.set_trigger_irq_affinity != NULL) {
        ts_inst->ops.set_trigger_irq_affinity(inst, ts_inst->sq_trigger_irq, 1);
    }
    trs_info("Sq trigger send thread init ok. (devid=%u)\n", inst->devid);
    return 0;
}

void trs_hw_sq_trigger_irq_hw_res_uninit(struct trs_core_ts_inst * ts_inst)
{
    if (ts_inst->ops.get_sq_trigger_irq != NULL) {
        if (ts_inst->ops.set_trigger_irq_affinity != NULL) {
            ts_inst->ops.set_trigger_irq_affinity(&ts_inst->inst, ts_inst->sq_trigger_irq, 0);
        }
        (void)free_irq(ts_inst->sq_trigger_irq, (void *)ts_inst);
        if (ts_inst->work_queue != NULL) {
            (void)cancel_delayed_work_sync(&ts_inst->sq_trigger_work);
            destroy_workqueue(ts_inst->work_queue);
        }
        trs_hw_sq_send_thread_destroy(ts_inst);
    }
}

int trs_hw_sqcq_init(struct trs_core_ts_inst *ts_inst)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    u32 sq_id_num = trs_res_get_id_num(ts_inst, TRS_HW_SQ);
    u32 sq_max_id = trs_res_get_max_id(ts_inst, TRS_HW_SQ);
    u32 cq_id_num = trs_res_get_id_num(ts_inst, TRS_HW_CQ);
    u32 cq_max_id = trs_res_get_max_id(ts_inst, TRS_HW_CQ);
    int ret;

    trs_debug("Init hw sqcq. (devid=%u; tsid=%u; sq_id_num=%u; sq_max_id=%u; cq_id_num=%u; cq_max_id=%u)\n",
        inst->devid, inst->tsid, sq_id_num, sq_max_id, cq_id_num, cq_max_id);

    ts_inst->sq_ctx = (struct trs_sq_ctx *)vzalloc(sizeof(struct trs_sq_ctx) * sq_max_id);
    if (ts_inst->sq_ctx == NULL) {
        trs_err("Mem alloc failed. (devid=%u; tsid=%u; size=%lx)\n",
            inst->devid, inst->tsid, sizeof(struct trs_sq_ctx) * sq_max_id);
        return -ENOMEM;
    }

    trs_sq_ctxs_init(ts_inst->sq_ctx, sq_max_id);

    ts_inst->cq_ctx = (struct trs_cq_ctx *)vzalloc(sizeof(struct trs_cq_ctx) * cq_max_id);
    if (ts_inst->cq_ctx == NULL) {
        vfree(ts_inst->sq_ctx);
        ts_inst->sq_ctx = NULL;
        trs_err("Mem alloc failed. (devid=%u; tsid=%u; size=%lx)\n",
            inst->devid, inst->tsid, sizeof(struct trs_cq_ctx) * cq_max_id);
        return -ENOMEM;
    }

    ret = trs_hw_sq_trigger_init(ts_inst);
    if (ret != 0) {
        vfree(ts_inst->sq_ctx);
        ts_inst->sq_ctx = NULL;
        vfree(ts_inst->cq_ctx);
        ts_inst->cq_ctx = NULL;
        return ret;
    }

    return 0;
}

void trs_hw_sqcq_uninit(struct trs_core_ts_inst *ts_inst)
{
    vfree(ts_inst->sq_ctx);
    ts_inst->sq_ctx = NULL;
    vfree(ts_inst->cq_ctx);
    ts_inst->cq_ctx = NULL;
}
