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
* Create: 2022-10-15
*/
#include "trs_chan_ops_db.h"
#include "trs_chip_def.h"
#include "trs_ts_db.h"

int trs_chan_ops_db_init(struct trs_id_inst *inst)
{
    u32 start, end;
    int ret;

    trs_get_maint_sq_db_cfg(inst->devid, inst->tsid, &start, &end);
    ret = trs_ts_db_init(inst, TRS_DB_MAINT_SQ, start, end);
    if (ret != 0) {
        trs_err("Ts db init fail. (ret=%d)\n", ret);
        return ret;
    }
    trs_debug("Trs db init. Maint sq. (start=%u; end=%u).\n", start, end);

    trs_get_maint_cq_db_cfg(inst->devid, inst->tsid, &start, &end);
    ret = trs_ts_db_init(inst, TRS_DB_MAINT_CQ, start, end);
    if (ret != 0) {
        trs_ts_db_uninit(inst, TRS_DB_MAINT_SQ);
        trs_err("Ts db init fail. (ret=%d)\n", ret);
        return ret;
    }
    trs_debug("Trs db init. Maint cq. (start=%u; end=%u).\n", start, end);

    return 0;
}

void trs_chan_ops_db_uninit(struct trs_id_inst *inst)
{
    trs_ts_db_uninit(inst, TRS_DB_MAINT_CQ);
    trs_ts_db_uninit(inst, TRS_DB_MAINT_SQ);
}

