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
* Create: 2022-11-1
*/

#ifndef TRS_CACHE_ID_RECYCLE_H
#define TRS_CACHE_ID_RECYCLE_H
#include <linux/kref.h>

#include "trs_pub_def.h"

int trs_id_recycle_create(struct trs_id_inst *inst);
void trs_id_recycle_destroy(struct trs_id_inst *inst);

void trs_id_recycle_set(struct trs_id_inst *inst, int type, u32 id);
int trs_id_recycle_clear(struct trs_id_inst *inst, int type, u32 id);
void trs_id_recycle_clear_all(struct trs_id_inst *inst, int type);
int trs_get_id_recycle_num(struct trs_id_inst *inst, int type);
#endif
