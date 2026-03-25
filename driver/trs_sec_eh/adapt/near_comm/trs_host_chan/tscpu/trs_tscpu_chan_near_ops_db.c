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
* Create: 2023-1-10
*/
#include "trs_ts_db.h"
#include "soc_adapt.h"
#include "trs_chip_def_comm.h"

#include "trs_tscpu_chan_near_ops_db.h"

int trs_tscpu_chan_near_ops_db_init(struct trs_id_inst *inst)
{
    u32 start, end;
    int ret;

    ret = trs_soc_get_db_cfg(inst, TRS_DB_TASK_SQ, &start, &end);
    if ((ret != 0) || (start >= end)) {
        trs_err("Trs get db cfg fail. (devid=%u; tsid=%u; ret=%d; start=%u; end=%u)\n",
            inst->devid, inst->tsid, ret, start, end);
        return -ENODEV;
    }

    trs_info("Trs db init. task sq. (start=%u; end=%u).\n", start, end);
    ret |= trs_ts_db_init(inst, TRS_DB_TASK_SQ, start, end);

    ret |= trs_soc_get_db_cfg(inst, TRS_DB_TASK_CQ, &start, &end);
    if ((ret != 0) || (start >= end)) {
        trs_tscpu_chan_near_ops_db_uninit(inst);
        trs_err("Trs get db cfg fail. (devid=%u; tsid=%u; ret=%d; start=%u; end=%u)\n",
            inst->devid, inst->tsid, ret, start, end);
        return -ENODEV;
    }

    trs_info("Trs db init. task cq. (start=%u; end=%u).\n", start, end);
    ret |= trs_ts_db_init(inst, TRS_DB_TASK_CQ, start, end);
    if (ret != 0) {
        trs_tscpu_chan_near_ops_db_uninit(inst);
        trs_err("Ts db init fail. (ret=%d)\n", ret);
    }

    return ret;
}

void trs_tscpu_chan_near_ops_db_uninit(struct trs_id_inst *inst)
{
    trs_ts_db_uninit(inst, TRS_DB_TASK_CQ);
    trs_ts_db_uninit(inst, TRS_DB_TASK_SQ);
}

