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
* Create: 2022-10-12
*/

#ifndef TRS_MIA_DEVICE_ID_H
#define TRS_MIA_DEVICE_ID_H

#include <linux/types.h>

#include "trs_pub_def.h"
#include "trs_mia_cfg.h"

int trs_mia_device_id_config(struct trs_id_inst *inst);
void trs_mia_device_id_deconfig(struct trs_id_inst *inst);

int trs_mia_adapt_trans_cqid(struct trs_mia_cfg *mia_cfg, u32 cqid, u32 *trans_cqid);

int trs_device_get_id_range(struct trs_id_inst *inst, int type, u32 *start, u32 *end);
int trs_device_get_id_total_num(struct trs_id_inst *inst, int type, u32 *total_num);
int trs_device_get_id_split(struct trs_id_inst *inst, int type, u32 *split);

void trs_mia_adapt_free_id(struct trs_id_inst *inst, int type, u32 id);

#endif /* TRS_MIA_DEVICE_ID_H */

