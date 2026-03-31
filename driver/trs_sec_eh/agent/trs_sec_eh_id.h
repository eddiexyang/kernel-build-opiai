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
* Create: 2022-11-10
*/

#ifndef TRS_SEC_EH_ID_H
#define TRS_SEC_EH_ID_H

#include <linux/types.h>
#include "trs_pub_def.h"

#include "trs_sec_eh_cfg.h"

bool trs_sec_eh_id_is_belong_to_vf(struct trs_sec_eh_id_info *id_info, u32 id);

void trs_sec_eh_id_config(struct trs_id_inst *inst);
void trs_sec_eh_id_deconfig(struct trs_id_inst *inst);

int _trs_sec_eh_res_ctrl(struct trs_id_inst *inst, u32 id_type, u32 res_id, u32 cmd);

#endif /* TRS_SEC_EH_ID_H */
