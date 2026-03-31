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
* Create: 2022-7-15
*/

#ifndef CHAN_RXTX_H
#define CHAN_RXTX_H

#include <linux/interrupt.h>

#include "chan_init.h"

void trs_chan_irq_proc(int irq_type, int irq_index, void *para, u32 cqid[], u32 cq_num);
void trs_chan_tasklet(unsigned long data);
void trs_chan_work(struct work_struct *p_work);
void trs_chan_fetch_wakeup(struct trs_chan *chan);
void trs_chan_submit_wakeup(struct trs_chan *chan);
int _trs_chan_to_string(struct trs_chan *chan, char *buff, u32 buff_len);

#endif

