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
#ifndef TRS_DEVICE_CHAN_H
#define TRS_DEVICE_CHAN_H

#include <linux/types.h>

#include "trs_pub_def.h"
#include "trs_chan.h"

struct trs_chan_adapt_ops *trs_chan_get_adapt_ops(void);

int trs_chan_ops_init(struct trs_id_inst *inst);
void trs_chan_ops_uninit(struct trs_id_inst *inst);

int trs_chan_config(struct trs_id_inst *inst);
void trs_chan_deconfig(struct trs_id_inst *inst);

#endif /* TRS_DEVICE_CHAN_H */
