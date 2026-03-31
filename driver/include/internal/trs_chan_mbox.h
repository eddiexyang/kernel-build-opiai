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
* Create: 2022-10-10
*/
#ifndef TRS_CHAN_MBOX_H
#define TRS_CHAN_MBOX_H

#include "trs_pm_adapt.h"
#include "trs_pub_def.h"
#include "trs_mailbox_def.h"
#include "trs_chan.h"

int trs_chan_ops_get_hw_irq(struct trs_id_inst *inst, int irq_type, u32 irq, u32 *hwirq);

#endif /* TRS_CHAN_MBOX_OPS_H */
