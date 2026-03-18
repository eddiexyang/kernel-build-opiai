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
* Create: 2023-8-28
*/

#ifndef TRS_CORE_SMMU_H
#define TRS_CORE_SMMU_H

#include <linux/types.h>

#include "trs_pub_def.h"

int trs_core_ops_proc_bind_smmu(struct trs_id_inst *inst);
void trs_core_ops_proc_unbind_smmu(struct trs_id_inst *inst);
int trs_core_ops_get_ssid(struct trs_id_inst *inst, int *ssid);


int trs_smmu_config(struct trs_id_inst *inst);
void trs_smmu_deconfig(struct trs_id_inst *inst);

#endif
