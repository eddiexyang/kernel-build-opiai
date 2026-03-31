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
    struct trs_core_adapt_ops *ops = trs_core_get_adapt_ops();
    int ret;

    ret = trs_sq_send_trigger_db_init(inst);
    if (ret == 0) {
        ops->get_sq_trigger_irq = trs_core_ops_get_sq_trigger_irq;
        ops->get_trigger_sqid = trs_core_ops_get_trigger_sqid;
        ops->set_trigger_irq_affinity = trs_core_ops_set_trigger_irq_affinity;
    }

    return trs_core_ts_inst_register(inst, trs_get_hw_type(), ops);
}

void trs_core_deconfig(struct trs_id_inst *inst)
{
    trs_sq_send_trigger_db_uninit(inst);
    trs_core_ts_inst_unregister(inst);
}

