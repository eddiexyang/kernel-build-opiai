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
 * Create: 2023-05-24
 */
#include "trs_mia_device_id.h"
#include "trs_chip_def.h"
#include "trs_device_ts_db.h"
#include "dbl/uda.h"
#include "soc_res.h"
/* db zone info description:
   |--------------|--------------|--------------|--------------|--------------|--------------|
   pf(vf0)[0~63]  vf1[64~127]    vf2[128~191]   vf3[192~255]   vf4[256~319]               MAX(512)
*/
#define TRS_DB_VF_START      0
#define TRS_DB_VF_OFFSET     64
#define TRS_DB_VF_NUM        5    /* pf:1 + vf:4 = 5 */
#define TRS_DB_VF_END        (TRS_DB_VF_START + TRS_DB_VF_OFFSET * TRS_DB_VF_NUM)
#define TRS_DB_MAX           512
#define TRS_TS_DB_STRIDE     (4 * 1024)

static int trs_mia_ts_set_db_res(struct trs_id_inst *inst)
{
    struct soc_reg_base_info io_base, mia_io_base;
    struct res_inst_info res_inst;
    u32 devid = inst->devid;
    u32 tsid = inst->tsid;
    u32 pf_id, vf_id;
    int ret;

    ret = vmngd_get_pfvf_id_by_devid(devid, &pf_id, &vf_id);
    if (ret != 0) {
        trs_err("Get pf vf id failed. (devid=%u; pf_id=%u; vf_id=%u)\n", devid, pf_id, vf_id);
        return ret;
    }

    soc_resmng_inst_pack(&res_inst, pf_id, TS_SUBSYS, tsid);
    ret = soc_resmng_get_reg_base(&res_inst, "TS_DOORBELL_REG", &io_base);
    if (ret != 0) {
        trs_err("Get ts doorbell reg fail. (devid=%u; tsid=%u; ret=%d)\n", devid, tsid, ret);
        return ret;
    }

    soc_resmng_inst_pack(&res_inst, devid, TS_SUBSYS, tsid);
    mia_io_base.io_base = io_base.io_base + TRS_DB_VF_OFFSET * TRS_TS_DB_STRIDE * vf_id;
    mia_io_base.io_base_size = TRS_DB_VF_OFFSET * TRS_TS_DB_STRIDE;
    ret = soc_resmng_set_reg_base(&res_inst, "TS_DOORBELL_REG", &mia_io_base);
    if (ret != 0) {
        trs_err("Set reg failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    return 0;
}
int trs_mia_ts_db_config(struct trs_id_inst *inst)
{
    u32 start, end;
    int ret;

    if (trs_get_trigger_sq_db_cfg(inst, &start, &end)) {
        trs_info("Not support uio. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return 0;
    }

    ret = trs_mia_ts_set_db_res(inst);
    if (ret != 0) {
        trs_err("Set db reg failed. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    return trs_ts_db_config(inst);
}

void trs_mia_ts_db_deconfig(struct trs_id_inst *inst)
{
    u32 start, end;
    if (trs_get_trigger_sq_db_cfg(inst, &start, &end)) {
        return;
    }

    trs_ts_db_deconfig(inst);
}
