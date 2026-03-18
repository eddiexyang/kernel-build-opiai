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
#include "stars_simple_sq0_c_define.h"
#include "trs_stars_comm.h"
static volatile STARS_SIMPLE_SQ0_REGS_TYPE_STRU *stars_simple_sq0_all_reg[TRS_DEV_MAX_NUM][TRS_TS_MAX_NUM] = {NULL};
static DEFINE_RWLOCK(simple_sq0_all_reg_lock);
int trs_init_sq0_base_addr(struct trs_id_inst *inst)
{
    struct soc_reg_base_info io_base;
    struct res_inst_info res_inst;
    void *base_va;
    int ret;

    soc_resmng_inst_pack(&res_inst, inst->devid, TS_SUBSYS, inst->tsid);
    ret = soc_resmng_get_reg_base(&res_inst, "TS_STARS_RTSQ_SCHED_REG", &io_base);
    if (ret != 0) {
        trs_err("Failed to get rtsq sched reg base. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return ret;
    }

    base_va = ioremap(io_base.io_base, io_base.io_base_size);
    if (base_va == NULL) {
        trs_err("Failed to ioremap. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -ENOMEM;
    }
    write_lock(&simple_sq0_all_reg_lock);
    stars_simple_sq0_all_reg[inst->devid][inst->tsid] = base_va;
    write_unlock(&simple_sq0_all_reg_lock);
    return 0;
}

void trs_uninit_sq0_base_addr(struct trs_id_inst *inst)
{
    void *reg_va = NULL;

    write_lock(&simple_sq0_all_reg_lock);
    reg_va = stars_simple_sq0_all_reg[inst->devid][inst->tsid];
    stars_simple_sq0_all_reg[inst->devid][inst->tsid] = NULL;
    write_unlock(&simple_sq0_all_reg_lock);

    if (reg_va != NULL) {
        iounmap(reg_va);
    }
}

static REGS_STARS_P0_SQ_DB_TO_P0_SQ_TO_ACTIVE_SQ_MAP1 *trs_get_sq_base_addr(struct trs_id_inst *inst, u32 sqid)
{
    if (stars_simple_sq0_all_reg[inst->devid][inst->tsid] == NULL) {
        return NULL;
    }
    return &(stars_simple_sq0_all_reg[inst->devid][inst->tsid]->stars_p0_sq_db[sqid]);
}

void trs_stars_set_p0_sq_head(struct trs_id_inst *inst, u32 sqid, u32 val)
{
    REGS_STARS_P0_SQ_DB_TO_P0_SQ_TO_ACTIVE_SQ_MAP1 *sq_base = NULL;

    read_lock(&simple_sq0_all_reg_lock);
    sq_base = trs_get_sq_base_addr(inst, sqid);
    if (sq_base != NULL) {
        sq_base->stars_p0_sq_cfg4.bits.p0_sq_head = val;
    }
    read_unlock(&simple_sq0_all_reg_lock);
}

u32 trs_stars_get_p0_sq_head(struct trs_id_inst *inst, u32 sqid)
{
    REGS_STARS_P0_SQ_DB_TO_P0_SQ_TO_ACTIVE_SQ_MAP1 *sq_base = NULL;
    u32 sq_head = U32_MAX;

    read_lock(&simple_sq0_all_reg_lock);
    sq_base = trs_get_sq_base_addr(inst, sqid);
    if (sq_base != NULL) {
        sq_head = sq_base->stars_p0_sq_cfg4.bits.p0_sq_head;
    }
    read_unlock(&simple_sq0_all_reg_lock);

    return sq_head;
}

void trs_stars_set_p0_sq_tail(struct trs_id_inst *inst, u32 sqid, u32 val)
{
    REGS_STARS_P0_SQ_DB_TO_P0_SQ_TO_ACTIVE_SQ_MAP1 *sq_base = NULL;

    read_lock(&simple_sq0_all_reg_lock);
    sq_base = trs_get_sq_base_addr(inst, sqid);
    if (sq_base != NULL) {
        sq_base->stars_p0_sq_db.bits.p0_sq_tail = val;
    }
    read_unlock(&simple_sq0_all_reg_lock);
}

u32 trs_stars_get_p0_sq_tail(struct trs_id_inst *inst, u32 sqid)
{
    REGS_STARS_P0_SQ_DB_TO_P0_SQ_TO_ACTIVE_SQ_MAP1 *sq_base = NULL;
    u32 sq_tail = U32_MAX;

    read_lock(&simple_sq0_all_reg_lock);
    sq_base = trs_get_sq_base_addr(inst, sqid);
    if (sq_base != NULL) {
        sq_tail = sq_base->stars_p0_sq_db.bits.p0_sq_tail;
    }
    read_unlock(&simple_sq0_all_reg_lock);

    return sq_tail;
}

void trs_stars_set_p0_sq_en(struct trs_id_inst *inst, u32 sqid, u32 val)
{
    REGS_STARS_P0_SQ_DB_TO_P0_SQ_TO_ACTIVE_SQ_MAP1 *sq_base = NULL;

    read_lock(&simple_sq0_all_reg_lock);
    sq_base = trs_get_sq_base_addr(inst, sqid);
    if (sq_base != NULL) {
        sq_base->stars_p0_sq_cfg5.bits.p0_sq_en = val;
    }
    read_unlock(&simple_sq0_all_reg_lock);
}

u32 trs_stars_get_p0_sq_en(struct trs_id_inst *inst, u32 sqid)
{
    REGS_STARS_P0_SQ_DB_TO_P0_SQ_TO_ACTIVE_SQ_MAP1 *sq_base = NULL;
    u32 sq_status = U32_MAX;

    read_lock(&simple_sq0_all_reg_lock);
    sq_base = trs_get_sq_base_addr(inst, sqid);
    if (sq_base != NULL) {
        sq_status = sq_base->stars_p0_sq_cfg5.bits.p0_sq_en;
    }
    read_unlock(&simple_sq0_all_reg_lock);

    return sq_status;
}