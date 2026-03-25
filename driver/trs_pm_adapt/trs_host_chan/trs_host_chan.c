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
* Create: 2022-7-15
*/
#include "trs_chan.h"
#include "trs_chip_def_comm.h"
#include "trs_res_id_def.h"
#include "trs_chan_stars_v1_ops.h"
#include "trs_tscpu_chan_near_ops.h"
#include "trs_chan_near_ops_rsv_mem.h"
#include "trs_chan_near_ops_db.h"
#include "soc_adapt.h"
#include "trs_sqe_update.h"

#include "trs_host_chan.h"

static int trs_chan_stars_v1_init(struct trs_id_inst *inst)
{
    int ret;

    ret = trs_chan_stars_v1_ops_init(inst);
    if (ret != 0) {
        return ret;
    }

    ret = trs_chan_ts_inst_register(inst, trs_soc_get_hw_type(inst->devid), trs_chan_get_stars_v1_adapt_ops());
    if (ret != 0) {
        trs_chan_stars_v1_ops_uninit(inst);
    }
    return ret;
}

static void trs_chan_stars_v1_uninit(struct trs_id_inst *inst)
{
    trs_chan_ts_inst_unregister(inst);
    trs_chan_stars_v1_ops_uninit(inst);
}

static int trs_chan_tscpu_init(struct trs_id_inst *inst)
{
    int ret;

    ret = trs_tscpu_chan_ops_init(inst);
    if (ret != 0) {
        return ret;
    }

    ret = trs_chan_ts_inst_register(inst, trs_soc_get_hw_type(inst->devid), trs_chan_get_tscpu_adapt_ops());
    if (ret != 0) {
        trs_tscpu_chan_ops_uninit(inst);
    }
    return ret;
}

static void trs_chan_tscpu_uninit(struct trs_id_inst *inst)
{
    trs_chan_ts_inst_unregister(inst);
    trs_tscpu_chan_ops_uninit(inst);
}

int trs_chan_config(struct trs_id_inst *inst)
{
    int hw_type;
    int ret;

    ret = trs_chan_near_ops_rsv_mem_init(inst);
    if (ret != 0) {
        trs_err("Rsv mem init fail. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    hw_type = trs_soc_get_hw_type(inst->devid);
    if (hw_type == TRS_HW_TYPE_STARS) {
        ret = trs_chan_stars_v1_init(inst);
        if (ret != 0) {
            trs_chan_near_ops_rsv_mem_uninit(inst);
            return ret;
        }
        ret = trs_chan_maint_db_init(inst);
        if (ret != 0) {
            trs_chan_stars_v1_uninit(inst);
            trs_chan_near_ops_rsv_mem_uninit(inst);
            return ret;
        }
    } else {
        ret = trs_chan_tscpu_init(inst);
        if (ret != 0) {
            trs_chan_near_ops_rsv_mem_uninit(inst);
            return ret;
        }
    }

    trs_sqe_update_init(inst->devid);

    ret = trs_chan_ops_agent_init(inst);
    if (ret != 0) {
        if (hw_type == TRS_HW_TYPE_STARS) {
            trs_chan_stars_v1_uninit(inst);
        } else {
            trs_chan_tscpu_uninit(inst);
        }
        trs_chan_near_ops_rsv_mem_uninit(inst);
    }
    return ret;
}

void trs_chan_deconfig(struct trs_id_inst *inst)
{
    int hw_type;

    trs_chan_ops_agent_uninit(inst);

    hw_type = trs_soc_get_hw_type(inst->devid);
    if (hw_type == TRS_HW_TYPE_STARS) {
        trs_chan_maint_db_uninit(inst);
        trs_chan_stars_v1_uninit(inst);
    } else {
        trs_chan_tscpu_uninit(inst);
    }

    trs_chan_near_ops_rsv_mem_uninit(inst);
}

