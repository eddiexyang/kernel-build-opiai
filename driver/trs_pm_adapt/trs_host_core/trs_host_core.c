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
#include "dbl/uda.h"
#include "trs_core.h"
#include "trs_chip_def_comm.h"
#include "trs_core_stars_v1_ops.h"
#include "trs_tscpu_core_near_ops.h"
#include "soc_adapt.h"
#include "trs_stars_comm.h"
#include "trs_host_core.h"

static u32 trsResCmdTrans[TRS_RES_OP_MAX] = {
    [TRS_RES_OP_RESET] = TRS_STARS_RES_OP_RESET,
    [TRS_RES_OP_RECORD] = TRS_STARS_RES_OP_RECORD,
    [TRS_RES_OP_ENABLE] = TRS_STARS_RES_OP_ENABLE,
    [TRS_RES_OP_DISABLE] = TRS_STARS_RES_OP_DISABLE,
    [TRS_RES_OP_CHECK_AND_RESET] = TRS_STARS_RES_OP_CHECK_AND_RESET,
};

int trs_core_ops_stars_soc_res_ctrl(struct trs_id_inst *inst, u32 id_type, u32 id, u32 cmd)
{
    struct uda_mia_dev_para dev_para;
    struct trs_id_inst pm_inst;
    int ret;

    dev_para.phy_devid = inst->devid;
    if (!uda_is_phy_dev(inst->devid)) {
        ret = uda_udevid_to_mia_devid(inst->devid, &dev_para);
        if (ret != 0) {
            trs_err("Failed to get devid. (devid=%u)\n", inst->devid);
            return ret;
        }
    }
    trs_debug("devid=%u; tsid=%u; id=%u\n", dev_para.phy_devid, inst->tsid, id);
    trs_id_inst_pack(&pm_inst, dev_para.phy_devid, inst->tsid);
    return trs_stars_soc_res_ctrl(&pm_inst, id_type, id, trsResCmdTrans[cmd]);
}
EXPORT_SYMBOL(trs_core_ops_stars_soc_res_ctrl);

int trs_core_config(struct trs_id_inst *inst)
{
    int chip_type = trs_soc_get_chip_type(inst->devid);
    int hw_type = trs_get_hw_type_by_chip_type(chip_type);
    int ret = 0;

    if (hw_type == TRS_HW_TYPE_STARS) {
        struct trs_core_adapt_ops *ops = trs_core_get_stars_v1_adapt_ops();
        ret = trs_sq_send_trigger_db_init(inst);
        if (ret != 0) {
            return ret;
        }

        ops->res_id_ctrl = trs_core_ops_stars_soc_res_ctrl;
        ret = trs_core_ts_inst_register(inst, hw_type, ops);
        if (ret != 0) {
            trs_sq_send_trigger_db_uninit(inst);
            return ret;
        }
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
