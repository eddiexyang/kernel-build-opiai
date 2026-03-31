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
#ifndef TRS_TSCPU_CHAN_NEAR_OPS_H__
#define TRS_TSCPU_CHAN_NEAR_OPS_H__

#include "trs_chan.h"
#include "trs_pub_def.h"

struct trs_chan_adapt_ops *trs_chan_get_tscpu_adapt_ops(void);

int trs_tscpu_chan_ops_init(struct trs_id_inst *inst);
void trs_tscpu_chan_ops_uninit(struct trs_id_inst *inst);

int trs_chan_ops_agent_init(struct trs_id_inst *inst);
void trs_chan_ops_agent_uninit(struct trs_id_inst *inst);

#endif
