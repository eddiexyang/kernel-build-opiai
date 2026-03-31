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
* Create: 2022-8-15
*/
#ifndef TRS_TSCPU_CQ_H__
#define TRS_TSCPU_CQ_H__

#include <linux/types.h>
#include "trs_pub_def.h"
#include "trs_chan.h"

int trs_tscpu_cqe_get_streamid(struct trs_id_inst *inst, void *cqe, u32 *stream_id);
bool trs_tscpu_cqe_is_valid(struct trs_id_inst *inst, void *cqe, u32 loop);
void trs_tscpu_cqe_get_sq_id(struct trs_id_inst *inst, void *cqe, u32 *sqid);
void trs_tscpu_cqe_get_sq_head(struct trs_id_inst *inst, void *cqe, u32 *sq_head);
int trs_tscpu_cq_head_update(struct trs_id_inst *inst, u32 cqid, u32 head);

#endif /* TRS_TSCPU_CQ_H__ */
