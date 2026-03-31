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
 * Create: 2022-7-26
 */
#ifndef PROF_CHAN_H
#define PROF_CHAN_H

#include "prof_ts.h"

#define PROF_CQ0_OFFSET 8
#define PROF_CQ1_OFFSET 4
#define PROF_CQ2_OFFSET 4

int prof_sqcq_alloc(u32 devid, u32 tsid, struct prof_cqsq_info *cqsq_info);
void prof_sqcq_free(u32 devid, u32 tsid, struct prof_cqsq_info *cqsq_info);
int prof_sqcq_send(u32 devid, u32 tsid, u8 *sqe);
int prof_sqcq_recv(u32 devid, u32 tsid, u8 *cqe, int timeout);

#endif

