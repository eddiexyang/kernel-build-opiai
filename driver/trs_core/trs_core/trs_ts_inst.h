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

#ifndef TRS_TS_INST_H
#define TRS_TS_INST_H

#include <linux/types.h>
#include <linux/kref.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>

#include "trs_mailbox_def.h"
#include "trs_pub_def.h"
#include "trs_core.h"

#include "trs_res_mng.h"
#include "trs_sqcq_ctx.h"
#include "trs_hw_sqcq.h"
#include "trs_sw_sqcq.h"
#include "trs_cb_sqcq.h"
#include "trs_logic_cq.h"
#include "trs_shm_sqcq.h"

struct trs_core_ts_inst {
    struct kref ref;
    int hw_type;
    int status;
    u32 featur_mode;  /* 0:all; 1:part */
    u32 surport_proc_num; /* 0: The resources used by processes are not limited. */
    struct trs_id_inst inst;
    struct trs_core_adapt_ops ops;
    struct rw_semaphore sem;
    u32 cur_task_id;
    u32 sq_trigger_irq;
    struct delayed_work sq_trigger_work;
    struct workqueue_struct *work_queue;
    u32 sq_work_retry_time;
    struct task_struct *sq_task;
    u32 sq_task_flag; /* Write the db_reg before reading it, so sq_task can`t work before receiving sq_trigger irq */
    struct list_head proc_list_head; /* tsid 0 use */
    struct list_head exit_proc_list_head; /* tsid 0 use */
    struct trs_res_mng res_mng[TRS_CORE_MAX_ID_TYPE];
    struct trs_stream_ctx *stream_ctx;
    struct trs_sq_ctx *sq_ctx;
    struct trs_cq_ctx *cq_ctx;
    struct trs_sq_ctx *sw_sq_ctx;
    struct trs_cq_ctx *sw_cq_ctx;
    struct trs_cb_ctx cb_ctx;
    struct trs_shm_ctx shm_ctx;
    struct trs_logic_cq_ctx logic_cq_ctx;
    struct proc_dir_entry *entry;
    bool trace_enable;
};

static inline bool trs_is_stars_inst(struct trs_core_ts_inst *ts_inst)
{
    return (ts_inst->hw_type == TRS_HW_TYPE_STARS);
}

struct trs_core_ts_inst *trs_core_ts_inst_get(struct trs_id_inst *inst);
void trs_core_ts_inst_put(struct trs_core_ts_inst *ts_inst);

static inline struct trs_core_ts_inst *trs_core_inst_get(u32 devid, u32 tsid)
{
    struct trs_id_inst inst = {.devid = devid, .tsid = tsid};
    return trs_core_ts_inst_get(&inst);
}

static inline void trs_core_inst_put(struct trs_core_ts_inst *ts_inst)
{
    trs_core_ts_inst_put(ts_inst);
}

static inline int trs_core_notice_ts(struct trs_core_ts_inst *ts_inst, u8 *msg, u32 len)
{
    struct trs_mb_header *header = (struct trs_mb_header *)(void *)msg;
    int ret = ts_inst->ops.notice_ts(&ts_inst->inst, msg, len);
    if (((ret != 0) || (header->result != 0)) && (header->cmd_type != TRS_MBOX_RECYCLE_CHECK)) {
        trs_err("Ts resp failed. (devid=%u; tsid=%u; cmd_type=%u; ret=%d; result=%u)\n",
            ts_inst->inst.devid, ts_inst->inst.tsid, header->cmd_type, ret, header->result);
    }

    return ((ret == 0) && (header->result == 0)) ? 0 : -EFAULT;
}

static inline bool trs_core_trace_is_enabled(struct trs_core_ts_inst *ts_inst)
{
    return ts_inst->trace_enable;
}

bool trs_still_has_proc(struct trs_core_ts_inst *ts_inst);

void trs_core_inst_init(void);
void trs_core_inst_uninit(void);

int trs_core_get_host_pid(int cur_pid, int *host_pid);

#endif

