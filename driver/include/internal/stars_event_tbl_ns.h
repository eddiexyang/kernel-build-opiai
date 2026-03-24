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
 * Create: 2023-3-1
 */
#ifndef STARS_EVENT_TBL_NS_H
#define STARS_EVENT_TBL_NS_H
#include <linux/types.h>

#include "trs_pub_def.h"
int trs_init_event_tbl_ns_base_addr(struct trs_id_inst *inst);
void trs_uninit_event_tbl_ns_base_addr(struct trs_id_inst *inst);
int trs_stars_get_event_tbl_flag(struct trs_id_inst *inst, u32 id);
void trs_stars_set_event_tbl_flag(struct trs_id_inst *inst, u32 id, u32 val);
#endif
