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
* Create: 2023-7-05
*/
#ifndef TRS_STARS_SQ_H
#define TRS_STARS_SQ_H

#include "trs_pub_def.h"
#include "trs_chan.h"

void trs_stars_trace_sqe_fill(struct trs_id_inst *inst, struct trs_chan_sq_trace *sq_trace, void *sqe);

#endif