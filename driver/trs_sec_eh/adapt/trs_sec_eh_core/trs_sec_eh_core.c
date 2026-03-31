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
* Create: 2023-1-18
*/
#include "trs_core.h"
#include "trs_chip_def_comm.h"
#include "trs_core_stars_v1_ops.h"
#include "trs_tscpu_core_near_ops.h"
#include "soc_adapt.h"
#include "trs_sec_eh_msg.h"
#include "trs_sec_eh_vpc.h"
#include "trs_sec_eh_core.h"

 static int trs_sec_eh_core_stars_soc_res_ctrl(struct trs_id_inst *inst, u32 id_type, u32 id, u32 cmd)
{
    struct trs_sec_eh_res_ctrl_info res_ctrl;
    int ret;

    res_ctrl.head.cmd_type = TRS_SEC_EH_RES_CTRL;
    res_ctrl.head.result = 0;
    res_ctrl.head.tsid = inst->tsid;
    res_ctrl.head.rsv = 0;
    res_ctrl.id_type = id_type;
    res_ctrl.id = id;
    res_ctrl.cmd = cmd;
    ret = trs_sec_eh_vpc_msg_send(inst->devid, (void *)&res_ctrl, sizeof(struct trs_sec_eh_res_ctrl_info));
    if ((ret != 0) || (res_ctrl.head.result != 0)) {
        trs_err("Vpc send fail. (devid=%u; ret=%d; result=%d)\n", inst->devid, ret, res_ctrl.head.result);
        return -EFAULT;
    }

    return 0;
}

int trs_core_config(struct trs_id_inst *inst)
{
    int chip_type = trs_soc_get_chip_type(inst->devid);
    int hw_type = trs_get_hw_type_by_chip_type(chip_type);
    int ret = 0;

    if (hw_type == TRS_HW_TYPE_STARS) {
        struct trs_core_adapt_ops *ops = trs_core_get_stars_v1_adapt_ops();
        ops->res_id_ctrl = trs_sec_eh_core_stars_soc_res_ctrl;
        ret = trs_sq_send_trigger_db_init(inst);
        if (ret != 0) {
            return ret;
        }
        ret = trs_core_ts_inst_register(inst, hw_type, ops);
    } else {
        ret = trs_core_ts_inst_register(inst, hw_type, trs_core_get_tscpu_adapt_ops());
    }

    return ret;
}

void trs_core_deconfig(struct trs_id_inst *inst)
{
    int chip_type = trs_soc_get_chip_type(inst->devid);
    int hw_type = trs_get_hw_type_by_chip_type(chip_type);

    trs_core_ts_inst_unregister(inst);

    if (hw_type == TRS_HW_TYPE_STARS) {
        trs_sq_send_trigger_db_uninit(inst);
    }
}
