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
* Create: 2022-7-15
*/
#include "trs_chip_def.h"
#include "trs_core_ops.h"
#include "trs_device_core.h"

int trs_core_config(struct trs_id_inst *inst)
{
    return trs_core_ts_inst_register(inst, trs_get_hw_type(), trs_core_get_adapt_ops());
}

void trs_core_deconfig(struct trs_id_inst *inst)
{
    trs_core_ts_inst_unregister(inst);
}

