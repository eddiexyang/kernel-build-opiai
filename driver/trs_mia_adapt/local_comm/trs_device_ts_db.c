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
* Create: 2022-8-15
*/
#include "soc_res.h"

#include "trs_chip_def.h"
#include "trs_device_ts_db.h"
#include "trs_ts_db.h"

int trs_ts_db_config(struct trs_id_inst *inst)
{
    struct soc_reg_base_info io_base;
    struct res_inst_info res_inst;
    int ret;

    soc_resmng_inst_pack(&res_inst, inst->devid, TS_SUBSYS, inst->tsid);
    ret = soc_resmng_get_reg_base(&res_inst, "TS_DOORBELL_REG", &io_base);
    if (ret != 0) {
        trs_err("Get ts doorbell reg fail. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    ret = trs_ts_db_cfg(inst, io_base.io_base, io_base.io_base_size, TRS_TS_DB_STRIDE);
    if (ret != 0) {
        trs_err("Config ts doorbell reg addr fail. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
    }

    return ret;
}

void trs_ts_db_deconfig(struct trs_id_inst *inst)
{
    trs_ts_db_decfg(inst);
}
