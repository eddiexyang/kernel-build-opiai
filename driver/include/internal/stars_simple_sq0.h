/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * Create: 2023-1-11
 */
#ifndef STARS_SIMPLE_SQ0_H
#define STARS_SIMPLE_SQ0_H
#include <linux/types.h>

#include "trs_pub_def.h"
int trs_init_sq0_base_addr(struct trs_id_inst *inst);
void trs_uninit_sq0_base_addr(struct trs_id_inst *inst);
void trs_stars_set_p0_sq_head(struct trs_id_inst *inst, u32 sqid, u32 val);
u32 trs_stars_get_p0_sq_head(struct trs_id_inst *inst, u32 sqid);
void trs_stars_set_p0_sq_tail(struct trs_id_inst *inst, u32 sqid, u32 val);
u32 trs_stars_get_p0_sq_tail(struct trs_id_inst *inst, u32 sqid);
void trs_stars_set_p0_sq_en(struct trs_id_inst *inst, u32 sqid, u32 val);
u32 trs_stars_get_p0_sq_en(struct trs_id_inst *inst, u32 sqid);
#endif
