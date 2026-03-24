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
#ifndef TRS_DEVCIE_STARS_V1_STARS_H
#define TRS_DEVCIE_STARS_V1_STARS_H

#include <linux/types.h>

#include "trs_pub_def.h"
#include "trs_stars.h"

int trs_chan_get_cq_group_num(struct trs_id_inst *inst, u32 *group_num);
int trs_chan_get_cq_group(struct trs_id_inst *inst, u32 group_index, u32 *group);
int trs_chan_get_cq_group_index(struct trs_id_inst *inst, u32 group, u32 *group_index);

int trs_chan_stars_v1_ops_stars_init(struct trs_id_inst *inst);
void trs_chan_stars_v1_ops_stars_uninit(struct trs_id_inst *inst);

#endif /* TRS_DEVCIE_STARS_V1_STARS_H */
