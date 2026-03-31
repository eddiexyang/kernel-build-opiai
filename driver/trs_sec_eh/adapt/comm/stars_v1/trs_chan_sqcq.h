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
* Create: 2022-10-15
*/

#ifndef TRS_CHAN_SQCQ_H
#define TRS_CHAN_SQCQ_H

#include <linux/types.h>
#include "trs_pub_def.h"

int trs_stars_chan_ops_ctrl_sqcq(struct trs_id_inst *inst, u32 id, u32 cmd, u32 para);
int trs_stars_chan_ops_query_sqcq(struct trs_id_inst *inst, u32 id, u32 cmd, u64 *value);

#endif

