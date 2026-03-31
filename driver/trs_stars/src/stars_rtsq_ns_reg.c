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
 * Create: 2023-1-11
 */
#include <linux/io.h>

#include "soc_res.h"
#include "stars_rtsq_ns_reg_c_define.h"
#include "trs_stars_comm.h"
static volatile STARS_RTSQ_NS_REG_REGS_TYPE_STRU *stars_rtsq_ns_reg_all_reg[TRS_DEV_MAX_NUM][TRS_TS_MAX_NUM] = {NULL};
static DEFINE_RWLOCK(rtsq_reg_all_reg_lock);
int trs_init_rtsq_ns_base_addr(struct trs_id_inst *inst)
{
    struct soc_reg_base_info io_base;
    struct res_inst_info res_inst;
    void *base_va;
    int ret;

    soc_resmng_inst_pack(&res_inst, inst->devid, TS_SUBSYS, inst->tsid);
    ret = soc_resmng_get_reg_base(&res_inst, "TS_STARS_RTSQ_CFG_REG", &io_base);
    if (ret != 0) {
        trs_err("Failed to get rtsq cdg reg base. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return ret;
    }

    base_va = ioremap(io_base.io_base, io_base.io_base_size);
    if (base_va == NULL) {
        trs_err("Failed to ioremap. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -ENOMEM;
    }

    write_lock(&rtsq_reg_all_reg_lock);
    stars_rtsq_ns_reg_all_reg[inst->devid][inst->tsid] = base_va;
    write_unlock(&rtsq_reg_all_reg_lock);
    return 0;
}

void trs_uninit_rtsq_ns_base_addr(struct trs_id_inst *inst)
{
    void *reg_va = NULL;

    write_lock(&rtsq_reg_all_reg_lock);
    reg_va = stars_rtsq_ns_reg_all_reg[inst->devid][inst->tsid];
    stars_rtsq_ns_reg_all_reg[inst->devid][inst->tsid] = NULL;
    write_unlock(&rtsq_reg_all_reg_lock);

    if (reg_va != NULL) {
        iounmap(reg_va);
    }
}

STARS_RTSQ_NS_REG_REGS_TYPE_STRU *trs_get_rtsq_ns_base_addr(struct trs_id_inst *inst)
{
    return stars_rtsq_ns_reg_all_reg[inst->devid][inst->tsid];
}

void trs_stars_set_dfx_rtsq_fsm_sel(struct trs_id_inst *inst, u32 sqid)
{
    STARS_RTSQ_NS_REG_REGS_TYPE_STRU *stars_rtsq_ns = NULL;

    read_lock(&rtsq_reg_all_reg_lock);
    stars_rtsq_ns = trs_get_rtsq_ns_base_addr(inst);
    if (stars_rtsq_ns != NULL) {
        stars_rtsq_ns->stars_rtsq_fsm_sel.bits.dfx_rtsq_fsm_sel = sqid;
    }
    read_unlock(&rtsq_reg_all_reg_lock);
}

u32 trs_stars_get_dfx_rtsq_fsm_state(struct trs_id_inst *inst)
{
    STARS_RTSQ_NS_REG_REGS_TYPE_STRU *stars_rtsq_ns = NULL;
    u32 fsm_state = U32_MAX;

    read_lock(&rtsq_reg_all_reg_lock);
    stars_rtsq_ns = trs_get_rtsq_ns_base_addr(inst);
    if (stars_rtsq_ns != NULL) {
        fsm_state = stars_rtsq_ns->stars_rtsq_fsm_state.bits.dfx_rtsq_fsm_state;
    }
    read_unlock(&rtsq_reg_all_reg_lock);

    return fsm_state;
}
