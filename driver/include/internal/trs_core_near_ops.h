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
#ifndef TRS_CORE_NEAR_OPS_H
#define TRS_CORE_NEAR_OPS_H

#include "trs_pub_def.h"

int trs_core_ops_get_proc_num(struct trs_id_inst *inst, u32 *proc_num);
int trs_core_ops_get_ts_inst_status(struct trs_id_inst *inst, u32 *status);

#endif /* TRS_CORE_NEAR_OPS_H */
