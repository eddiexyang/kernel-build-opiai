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

#ifndef TRS_CB_SQCQ_H
#define TRS_CB_SQCQ_H

#include <linux/types.h>
#include "ascend_hal_define.h"
#include "trs_cmd.h"
#include "trs_proc.h"

#define TRS_CB_GRP_MAX_NUM 128

struct trs_cb_cq {
    u32 sqid;
    u32 cqid;
    u32 valid;
    int pid;
    u32 cqe_size; // cq slot size
    u32 cq_depth; // cq depth
    u32 grpid; // indicats thread group id, 0~127
};

struct trs_cb_phy_sqcq {
    int chan_id;
    u32 sqid;
    u32 cqid;
    u32 cq_irq;
};

struct trs_cb_ctx {
    u32 cq_num;
    struct trs_cb_phy_sqcq phy_sqcq;
    struct trs_cb_cq *cq;
};

struct trs_core_ts_inst;

int trs_cb_sqcq_alloc(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct halSqCqInputInfo *para);
int trs_cb_sqcq_free(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct halSqCqFreeInfo *para);
int trs_cb_sqcq_send(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct halTaskSendInfo *para);

void trs_cb_sqcq_recycle(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, int res_type, u32 res_id);

int trs_cb_sqcq_init(struct trs_core_ts_inst *ts_inst);
void trs_cb_sqcq_uninit(struct trs_core_ts_inst *ts_inst);

#endif

