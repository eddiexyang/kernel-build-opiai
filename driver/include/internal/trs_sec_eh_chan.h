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
* Create: 2022-11-09
*/
#ifndef TRS_SEC_EH_CHAN_H
#define TRS_SEC_EH_CHAN_H

#include "trs_pub_def.h"

int trs_chan_config(struct trs_id_inst *inst);
void trs_chan_deconfig(struct trs_id_inst *inst);

#endif