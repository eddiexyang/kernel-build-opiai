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
#ifndef TRS_CHAN_MBOX_OPS_H
#define TRS_CHAN_MBOX_OPS_H

#include "trs_pm_adapt.h"
#include "trs_pub_def.h"
#include "trs_chan.h"
#include "trs_device_mbox.h"

int trs_chan_ops_mbox_send(struct trs_id_inst *inst, struct trs_chan_info *chan_info);

#endif /* TRS_CHAN_MBOX_OPS_H */
