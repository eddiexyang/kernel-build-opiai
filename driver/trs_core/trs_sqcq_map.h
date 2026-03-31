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
 * Create: 2022-12-23
 */

#ifndef TRS_SQCQ_MAP_H
#define TRS_SQCQ_MAP_H

#include "ascend_hal_define.h"

#include "trs_chan.h"
#include "trs_proc.h"
#include "trs_sqcq_ctx.h"

struct trs_core_ts_inst;

int trs_sq_remap(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct halSqCqInputInfo *para, struct trs_sq_ctx *sq_ctx, struct trs_chan_sq_info *sq_info);
void trs_sq_unmap(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_sq_ctx *sq_ctx);
void trs_sq_clear_map_info(struct trs_sq_ctx *sq_ctx);
void trs_sq_ctx_mem_free(struct trs_sq_ctx *sq_ctx);

#endif

