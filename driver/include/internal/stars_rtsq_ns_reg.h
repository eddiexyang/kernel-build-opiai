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
#ifndef STARS_RTSQ_NS_REG_H
#define STARS_RTSQ_NS_REG_H
#include <linux/types.h>

#include "trs_pub_def.h"
int trs_init_rtsq_ns_base_addr(struct trs_id_inst *inst);
void trs_uninit_rtsq_ns_base_addr(struct trs_id_inst *inst);
void trs_stars_set_dfx_rtsq_fsm_sel(struct trs_id_inst *inst, u32 sqid);
u32 trs_stars_get_dfx_rtsq_fsm_state(struct trs_id_inst *inst);
#endif
