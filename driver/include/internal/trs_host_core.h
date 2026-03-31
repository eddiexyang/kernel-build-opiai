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
* Create: 2022-8-15
*/
#ifndef TRS_HOST_CORE_H
#define TRS_HOST_CORE_H

#include "trs_pub_def.h"

int trs_core_ops_stars_soc_res_ctrl(struct trs_id_inst *inst, u32 id_type, u32 id, u32 cmd);
int trs_core_config(struct trs_id_inst *inst);
void trs_core_deconfig(struct trs_id_inst *inst);

#endif /* TRS_HOST_CORE_H */