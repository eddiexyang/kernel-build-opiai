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
 * Create: 2022-08-13
 */
#ifndef LOG_DRV_CHAN_H
#define LOG_DRV_CHAN_H

#include "drv_type.h"

int log_sqcq_alloc(u32 devid, u32 tsid, u32 *sq_id, u32 *cq_0_id, u32 *cq_1_id);
void log_sqcq_free(u32 devid, u32 tsid, u32 sq_id, u32 cq_0_id, u32 cq_1_id);
int log_sqcq_send(u32 devid, u32 tsid, u8 *sqe);

#endif

