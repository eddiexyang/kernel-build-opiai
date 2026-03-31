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
* Create: 2022-12-15
*/
#ifndef TRS_TSCPU_CHAN_SQCQ_H__
#define TRS_TSCPU_CHAN_SQCQ_H__

#include "trs_chan.h"

void trs_tscpu_chan_ops_get_sq_head_in_cqe(struct trs_id_inst *inst, void *cqe, u32 *sq_head);
bool trs_tscpu_chan_ops_cqe_is_valid(struct trs_id_inst *inst, void *cqe, u32 loop);
int trs_tscpu_chan_ops_ctrl_sqcq(struct trs_id_inst *inst, u32 id, u32 cmd, u32 para);
int trs_tscpu_chan_ops_query_sqcq(struct trs_id_inst *inst, u32 id, u32 cmd, u64 *value);

#endif
