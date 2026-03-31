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
* Create: 2023-8-25
*/
#include "trs_ts_db.h"
#include "soc_adapt.h"

void trs_chan_maint_db_uninit(struct trs_id_inst *inst)
{
    trs_ts_db_uninit(inst, TRS_DB_MAINT_CQ);
    trs_ts_db_uninit(inst, TRS_DB_MAINT_SQ);
}

int trs_chan_maint_db_init(struct trs_id_inst *inst)
{
    u32 start, end;
    int ret;

    ret = trs_soc_get_db_cfg(inst, TRS_DB_MAINT_SQ, &start, &end);
    if ((ret != 0) || (start >= end)) {
        if (ret == -EOPNOTSUPP) {
            return 0;
        }
        trs_err("Trs get db cfg fail. (devid=%u; tsid=%u; ret=%d; start=%u; end=%u)\n",
            inst->devid, inst->tsid, ret, start, end);
        return -ENODEV;
    }
    ret = trs_ts_db_init(inst, TRS_DB_MAINT_SQ, start, end);
    if (ret != 0) {
        trs_err("Ts db init fail. (ret=%d)\n", ret);
        return ret;
    }

    ret = trs_soc_get_db_cfg(inst, TRS_DB_MAINT_CQ, &start, &end);
    if ((ret != 0) || (start >= end)) {
        trs_ts_db_uninit(inst, TRS_DB_MAINT_SQ);
        if (ret == -EOPNOTSUPP) {
            return 0;
        }

        trs_err("Trs get db cfg fail. (devid=%u; tsid=%u; ret=%d; start=%u; end=%u)\n",
            inst->devid, inst->tsid, ret, start, end);
        return -ENODEV;
    }
    ret = trs_ts_db_init(inst, TRS_DB_MAINT_CQ, start, end);
    if (ret != 0) {
        trs_ts_db_uninit(inst, TRS_DB_MAINT_SQ);
        trs_err("Ts db init fail. (ret=%d)\n", ret);
        return ret;
    }

    trs_info("Trs db init. Maint cq. (cq db=%u).\n", start);
    return 0;
}

