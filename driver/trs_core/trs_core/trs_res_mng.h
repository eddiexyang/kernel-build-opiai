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

#ifndef TRS_RES_MNG_H
#define TRS_RES_MNG_H

#include <linux/types.h>
#include <linux/mutex.h>

#include "trs_cmd.h"
#include "trs_core.h"
#include "trs_proc.h"

struct trs_core_ts_inst;

struct trs_stream_ctx {
    int host_pid;
    int sq;
    int cq;
    int logic_cq; /* -1: invalid */
};

#define RES_STATUS_NORMAL 0
#define RES_STATUS_DEL 1
struct trs_res_ids {
    int status; /* 0:normal(alloc: pid is set, free: pid is 0), 1:del */
    u32 task_id;
    int pid;
    int ref;
};

struct trs_res_mng {
    struct trs_core_ts_inst *ts_inst;
    struct mutex mutex;
    int res_type;
    u32 id_num;
    u32 max_id;
    u32 use_num;
    struct trs_res_ids *ids;
};

u32 trs_res_get_id_num(struct trs_core_ts_inst *ts_inst, int res_type);
u32 trs_res_get_max_id(struct trs_core_ts_inst *ts_inst, int res_type);
void trs_sqcq_reg_map(struct trs_core_ts_inst *ts_inst, struct trs_sqcq_reg_map_para *para);
void trs_sqcq_reg_unmap(struct trs_core_ts_inst *ts_inst, struct trs_sqcq_reg_map_para *para);
int trs_res_id_num_query(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_res_id_para *para);
int trs_res_id_max_query(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_res_id_para *para);
int trs_res_id_used_query(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct trs_res_id_para *para);
int trs_res_id_avail_query(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct trs_res_id_para *para);
int trs_res_id_reg_offset_query(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct trs_res_id_para *para);
int trs_res_id_reg_size_query(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct trs_res_id_para *para);
int trs_res_id_cfg(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_res_id_para *para);
int trs_res_id_alloc(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_res_id_para *para);
int trs_res_id_free(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_res_id_para *para);
int trs_res_id_enable(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_res_id_para *para);
int trs_res_id_disable(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_res_id_para *para);
void trs_res_id_recycle(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, int res_type, u32 res_id);

int trs_get_stream_logic_cq(struct trs_core_ts_inst *ts_inst, u32 stream_id);

int trs_res_get_id_type(int res_type);
int trs_res_replace_res_type(struct trs_core_ts_inst *ts_inst, int res_type);

int trs_res_mng_init(struct trs_core_ts_inst *ts_inst);
void trs_res_mng_uninit(struct trs_core_ts_inst *ts_inst);

#endif

