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

#ifndef HB_SQCQ_H
#define HB_SQCQ_H

int hb_sqcq_alloc(u32 devid, u32 tsid, int *chan_id);
void hb_sqcq_free(u32 devid, u32 tsid, int chan_id);
int hb_sqcq_send(u32 devid, u32 tsid, int chan_id, u8 *sqe);

#endif