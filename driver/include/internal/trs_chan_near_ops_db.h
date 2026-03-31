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
* Create: 2023-8-25
*/
#ifndef TRS_CHAN_NEAR_OPS_DB_H
#define TRS_CHAN_NEAR_OPS_DB_H
#include "trs_pub_def.h"

void trs_chan_maint_db_uninit(struct trs_id_inst *inst);
int trs_chan_maint_db_init(struct trs_id_inst *inst);

#endif /* TRS_CHAN_NEAR_OPS_DB_H */
