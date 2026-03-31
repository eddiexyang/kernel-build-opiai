/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
* Create: 2023-1-10
*/
#ifndef TRS_CHAN_NEAR_OPS_MEM_H
#define TRS_CHAN_NEAR_OPS_MEM_H
#include "trs_pub_def.h"

void *trs_chan_sq_mem_alloc(struct trs_id_inst *inst, u32 sqe_size, u32 sq_depth, u64 *phy_addr, u32 *attr);
void trs_chan_sq_mem_free(struct trs_id_inst *inst, u32 sqe_size, u32 sq_depth, void *sq_addr, u64 phy_addr);

void trs_chan_flush_sqe_cache(struct trs_id_inst *inst, u64 pa, u32 len);

#endif
