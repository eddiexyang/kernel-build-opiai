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
* Create: 2023-1-12
*/
#include "soc_adapt.h"
#include "trs_core.h"
#include "trs_chan.h"
#include "trs_cdqm.h"
#include "trs_chan_update.h"
#include "trs_chip_def_comm.h"

static int trs_chan_get_sq_info_by_sqid(struct trs_id_inst *inst, int res_type, u32 res_id, void *info)
{
    struct trs_chan_sq_info *sq_info = (struct trs_chan_sq_info *)info;
    int ret, chan_id;

    ret = trs_chan_get_chan_id(inst, res_type, res_id, &chan_id);
    if (ret != 0) {
        return -EINVAL;
    }

    ret = trs_chan_get_sq_info(inst, chan_id, sq_info);
    if (ret == 0) {
        trs_debug("Sq. (devid=%u; tsid=%u; sqid=%u)\n", inst->devid, inst->tsid, res_id);
    }

    return ret;
}

void trs_ts_adapt_init(struct trs_id_inst *inst)
{
    int chip_type = trs_soc_get_chip_type(inst->devid);
    int hw_type = trs_get_hw_type_by_chip_type(chip_type);
    struct trs_res_ops ops = {{0}};
    int type;

    for (type = 0; type < TRS_CORE_MAX_ID_TYPE; type++) {
        ops.res_belong_proc[type] = trs_res_is_belong_to_proc;
    }

    if (hw_type == TRS_HW_TYPE_STARS) {
#ifdef CFG_FEATURE_CDQM
        ops.res_belong_proc[TRS_CDQ] = cdqid_is_belong_to_proc;
#endif
        ops.res_get_info[TRS_HW_SQ] = trs_chan_get_sq_info_by_sqid;
    } else {
        ops.res_belong_proc[TRS_CDQ] = NULL;
    }
    trs_res_ops_register(inst->devid, &ops);
}

void trs_ts_adapt_uninit(struct trs_id_inst *inst)
{
    trs_res_ops_unregister(inst->devid);
}

