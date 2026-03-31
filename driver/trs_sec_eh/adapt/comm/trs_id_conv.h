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
#ifndef TRS_ID_CONV_H
#define TRS_ID_CONV_H

#include <linux/types.h>

#include "trs_pub_def.h"

int id_conv_init(struct trs_id_inst *inst, int type, u32 max_id);
void id_conv_exit(struct trs_id_inst *inst, int type);
int id_conv_add(struct trs_id_inst *inst, int type, u32 id, u32 phy_id);
void id_conv_del(struct trs_id_inst *inst, int type, u32 id);
int id_conv_v2p(struct trs_id_inst *inst, int type, u32 id, u32 *phy_id);

#endif  /* TRS_ID_CONV_H */
