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
* Create: 2023-6-8
*/
#ifndef TRS_TS_STATUS_H
#define TRS_TS_STATUS_H

#include <linux/types.h>

#include "trs_pub_def.h"
int trs_set_ts_status(struct trs_id_inst *inst, u32 status);
int trs_get_ts_status(struct trs_id_inst *inst, u32 *status);
void trs_ts_status_mng_init(struct trs_id_inst *inst);
void trs_ts_status_mng_exit(struct trs_id_inst *inst);
#endif
