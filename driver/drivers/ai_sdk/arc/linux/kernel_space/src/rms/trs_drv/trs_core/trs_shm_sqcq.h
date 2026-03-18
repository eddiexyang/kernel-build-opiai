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

#ifndef TRS_SHM_SQCQ_H
#define TRS_SHM_SQCQ_H

#include <asm/page.h>
#include <linux/mm_types.h>
#include "ascend_hal_define.h"
#include "trs_proc.h"

struct trs_shm_ctx {
    struct mutex mutex;
    u32 ref;
    int chan_id;
    u32 sqid;
    u64 sq_pa;
};

struct trs_core_ts_inst;

int trs_shm_sqcq_alloc(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct halSqCqInputInfo *para);
int trs_shm_sqcq_free(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct halSqCqFreeInfo *para);
void trs_shm_sqcq_recycle(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst);

int trs_shm_sqcq_init(struct trs_core_ts_inst *ts_inst);
void trs_shm_sqcq_uninit(struct trs_core_ts_inst *ts_inst);

#endif

