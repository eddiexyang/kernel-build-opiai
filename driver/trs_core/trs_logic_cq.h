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

#ifndef TRS_LOGIC_CQ_H
#define TRS_LOGIC_CQ_H

#include <linux/wait.h>
#include <linux/mutex.h>

#include "ascend_hal_define.h"

#include "trs_proc.h"

struct trs_thread_bind_intr_ctx {
    int valid;
    u32 irq;
    u32 logic_cqid;
    atomic_t *wait_flag; /* 1 valid, 0 invalid */
    wait_queue_head_t *wait_queue;
};

struct trs_thread_bind_intr_mng {
    u32 irq_num;
    struct mutex mutex;
    struct trs_thread_bind_intr_ctx *irq_ctx;
};

#define LOGIC_CQE_VERSION_V1 1
#define LOGIC_CQE_VERSION_V2 2

struct trs_logic_cq_stat {
    u64 enque;
    u64 full_drop;
    u64 wakeup;
    u64 recv;
    u64 recv_in;
    u64 timeout;
};

struct trs_logic_cq {
    u32 flag;
    u32 cqid;
    u32 valid;
    u32 cqe_verion; /* 1: tsfw fill cqe, 2 general cqe */
    int thread_bind_irq;
    u32 head;
    u32 tail;
    u32 cqe_size; // cq slot size
    u32 cq_depth; // cq depth
    u8 *addr;
    spinlock_t lock;
    struct mutex mutex;
    atomic_t wakeup_num;
    atomic_t wait_thread_num;
    wait_queue_head_t wait_queue;
    struct trs_logic_cq_stat stat;
};

struct trs_logic_phy_cq {
    int chan_id;
    u32 notice_flag;
    u32 cqid;
    u32 cq_irq;
    u64 cq_phy_addr;
};

struct trs_logic_cq_ctx {
    u32 cq_num;
    struct trs_logic_phy_cq phy_cq;
    struct trs_logic_cq *cq;
    struct trs_thread_bind_intr_mng intr_mng;
};

struct trs_core_ts_inst;

int trs_logic_cq_alloc(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct halSqCqInputInfo *para);
int trs_logic_cq_free(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct halSqCqFreeInfo *para);
int trs_logic_cq_recv(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct halReportRecvInfo *para);

void trs_logic_set_cqe_version(struct trs_core_ts_inst *ts_inst, u32 logic_cqid, u32 cqe_verion);
int trs_logic_cq_enque(struct trs_core_ts_inst *ts_inst, u32 logic_cq_id, u32 stream_id, u32 task_id, void *cqe);

void trs_logic_cq_recycle(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, int res_type, u32 res_id);

void trs_thread_bind_irq_hw_res_uninit(struct trs_core_ts_inst *ts_inst, u32 irq_num);

int trs_logic_cq_init(struct trs_core_ts_inst *ts_inst);
void trs_logic_cq_uninit(struct trs_core_ts_inst *ts_inst);

#endif

