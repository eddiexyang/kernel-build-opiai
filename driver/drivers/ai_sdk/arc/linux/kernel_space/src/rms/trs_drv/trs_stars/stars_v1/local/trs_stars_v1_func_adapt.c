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
 * Create: 2023-3-1
 */
#include "dbl/uda.h"

#include "trs_stars_v1_func_com.h"
#include "stars_simple_sq0.h"
#include "soc_res.h"
#include "stars_event_tbl_ns.h"
#include "stars_notify_tbl.h"
#include "trs_res_id_def.h"
#include "trs_stars_com.h"
#include "trs_stars_comm.h"
#include "stars_rtsq_ns_reg.h"
#include "cp_identity_interface.h"
void trs_stars_func_set_sq_head(struct trs_id_inst *inst, u32 sqid, u32 val)
{
    trs_stars_set_p0_sq_head(inst, sqid, val);
}

int trs_stars_func_get_sq_head(struct trs_id_inst *inst, u32 sqid)
{
    return trs_stars_get_p0_sq_head(inst, sqid);
}

void trs_stars_func_set_sq_tail(struct trs_id_inst *inst, u32 sqid, u32 val)
{
    trs_stars_set_p0_sq_tail(inst, sqid, val);
}

int trs_stars_func_get_sq_tail(struct trs_id_inst *inst, u32 sqid)
{
    return trs_stars_get_p0_sq_tail(inst, sqid);
}

int trs_stars_func_get_sq_fsm_status(struct trs_id_inst *inst, u32 sqid)
{
    trs_stars_set_dfx_rtsq_fsm_sel(inst, sqid);
    return trs_stars_get_dfx_rtsq_fsm_state(inst);
}

void trs_stars_func_set_sq_status(struct trs_id_inst *inst, u32 sqid, u32 status)
{
    trs_stars_set_p0_sq_en(inst, sqid, status);
}

int trs_stars_func_get_sq_status(struct trs_id_inst *inst, u32 sqid)
{
    return trs_stars_get_p0_sq_en(inst, sqid);
}

static bool trs_stars_func_sq_could_active(struct trs_id_inst *inst, u32 sqid)
{
    u32 sq_status, sq_head, sq_tail, sq_fsm_status;

    sq_status = trs_stars_func_get_sq_status(inst, sqid);
    sq_head = trs_stars_func_get_sq_head(inst, sqid);
    sq_tail = trs_stars_func_get_sq_tail(inst, sqid);
    sq_fsm_status = trs_stars_func_get_sq_fsm_status(inst, sqid);

    trs_info("Sq status show. (sqid=%u; sq_status=%u; sq_head=%u; sq_tail=%u; sq_fsm_status=%u)\n",
        sqid, sq_status, sq_head, sq_tail, sq_fsm_status);

    if (sq_fsm_status != 0) {
        trs_err("Invalid sq fsm status. (devid=%u; sqid=%u; sq_fsm_status=%u)\n", inst->devid, sqid, sq_fsm_status);
        return false;
    }

    if ((sq_status != 0) && (sq_head != sq_tail)) {
        trs_err("Invalid sq status. (devid=%u; sqid=%u; sq_status=%u; sq_head=%u; sq_tail=%u)\n",
            inst->devid, sqid, sq_status, sq_head, sq_tail);
        return false;
    }

    return true;
}

static void _trs_stars_soc_active_sq(struct trs_id_inst *inst, u32 sqid)
{
    trs_stars_func_set_sq_status(inst, sqid, 0);
    trs_stars_func_set_sq_head(inst, sqid, 0);
    trs_stars_func_set_sq_status(inst, sqid, 1);
}

static int trs_stars_soc_active_sq(struct trs_id_inst *inst, u32 sqid)
{
    if (!trs_stars_func_sq_could_active(inst, sqid)) {
        return -EPERM;
    }

    _trs_stars_soc_active_sq(inst, sqid);
    trs_info("Active success. (devid=%u; tsid=%u; id=%u)\n", inst->devid, inst->tsid, sqid);
    return 0;
}

static int trs_stars_func_event_id_ctrl(struct trs_id_inst *inst, u32 id, u32 cmd)
{
    if (cmd == TRS_STARS_RES_OP_RESET) {
        return trs_stars_func_event_id_reset(inst, id);
    }

    if (cmd == TRS_STARS_RES_OP_CHECK_AND_RESET) {
        return trs_stars_func_event_id_check_and_reset(inst, id);
    }

    if (cmd == TRS_STARS_RES_OP_RECORD) {
        pid_t pid = current->tgid;
        int ret;
        ret = devdrv_get_dev_process(pid);
        if (ret != 0) {
            trs_err("Can not get process. (devid=%u; tsid=%u; ret=%d).\n", inst->devid, inst->tsid, ret);
            return -EINVAL;
        }

        if (trs_stars_get_event_tbl_flag(inst, id) != 1) {
            trs_stars_set_event_tbl_flag(inst, id, 1);
        }
        devdrv_put_dev_process(pid);
        trs_info("Record success. (devid=%u; tsid=%u; id=%u)\n", inst->devid, inst->tsid, id);
        return 0;
    }

    trs_err("Invalid ctrl cmd. (devid=%u; tsid=%u; id=%u; cmd=%u)\n", inst->devid, inst->tsid, id, cmd);
    return -EINVAL;
}

static int trs_stars_func_notify_id_ctrl(struct trs_id_inst *inst, u32 id, u32 cmd)
{
    if (cmd == TRS_STARS_RES_OP_RECORD) {
        pid_t pid = current->tgid;
        int ret;
        ret = devdrv_get_dev_process(pid);
        if (ret != 0) {
            trs_err("Can not get process. (devid=%u; tsid=%u; ret=%d).\n", inst->devid, inst->tsid, ret);
            return -EINVAL;
        }

        if (trs_stars_get_notify_tbl_flag(inst, id) != 1) {
            trs_stars_set_notify_tbl_flag(inst, id, 1);
        }
        devdrv_put_dev_process(pid);
        trs_info("Record success. (devid=%u; tsid=%u; id=%u)\n", inst->devid, inst->tsid, id);
        return 0;
    }

    trs_err("Invalid ctrl cmd. (devid=%u; tsid=%u; id=%u; cmd=%u)\n", inst->devid, inst->tsid, id, cmd);
    return -EINVAL;
}

static int trs_stars_func_sq_id_ctrl(struct trs_id_inst *inst, u32 id, u32 cmd)
{
    if (cmd == TRS_STARS_RES_OP_ENABLE) {
        return trs_stars_soc_active_sq(inst, id);
    }

    if (cmd == TRS_STARS_RES_OP_DISABLE) {
        trs_stars_func_set_sq_status(inst, id, 0);
        trs_info("Disable success. (devid=%u; tsid=%u; id=%u)\n", inst->devid, inst->tsid, id);
        return 0;
    }

    trs_err("Invalid ctrl cmd. (devid=%u; tsid=%u; id=%u; cmd=%u)\n", inst->devid, inst->tsid, id, cmd);
    return -EINVAL;
}

static int trs_stars_func_set_id_status(struct trs_id_inst *inst, u32 type, u32 id, u32 cmd)
{
    switch (type) {
        case TRS_EVENT:
            return trs_stars_func_event_id_ctrl(inst, id, cmd);
        case TRS_NOTIFY:
            return trs_stars_func_notify_id_ctrl(inst, id, cmd);
        case TRS_HW_SQ:
            return trs_stars_func_sq_id_ctrl(inst, id, cmd);
        default:
            trs_err("Invaild id type. (type=%u)\n", type);
            return -EINVAL;
    }
    return 0;
}

int trs_stars_func_get_id_status(struct trs_id_inst *inst, u32 type, u32 id, u32 *status)
{
    if (type == TRS_HW_SQ) {
        *status = trs_stars_get_p0_sq_en(inst, id);
        return 0;
    }

    trs_err("Invaild id type. (type=%u)\n", type);
    return -EINVAL;
}

static struct trs_stars_ops trs_stars_func_ops = {
    .res_id_ctrl = trs_stars_func_set_id_status,
    .get_id_status = trs_stars_func_get_id_status,
    .set_sq_head = trs_stars_func_set_sq_head,
    .get_sq_head = trs_stars_func_get_sq_head,
    .set_sq_tail = trs_stars_func_set_sq_tail,
    .get_sq_tail = trs_stars_func_get_sq_tail,
    .get_sq_fsm_status = trs_stars_func_get_sq_fsm_status
};

struct trs_stars_ops *trs_stars_func_op_get(void)
{
    return &trs_stars_func_ops;
}

int trs_stars_func_init(struct trs_id_inst *inst)
{
    int ret;

    ret = trs_init_sq0_base_addr(inst);
    if (ret != 0) {
        trs_err("Failed to init sq0 base. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return ret;
    }

    ret = trs_init_rtsq_ns_base_addr(inst);
    if (ret != 0) {
        trs_uninit_sq0_base_addr(inst);
        trs_err("Failed to init rtsq ns base. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return ret;
    }

    ret = trs_init_event_tbl_ns_base_addr(inst);
    if (ret != 0) {
        trs_uninit_rtsq_ns_base_addr(inst);
        trs_uninit_sq0_base_addr(inst);
        trs_err("Failed to init event tbl ns base. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return ret;
    }

    ret = trs_init_notify_tbl_ns_base_addr(inst);
    if (ret != 0) {
        trs_uninit_event_tbl_ns_base_addr(inst);
        trs_uninit_rtsq_ns_base_addr(inst);
        trs_uninit_sq0_base_addr(inst);
        trs_err("Failed to init notify tbl ns base. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return ret;
    }

    return 0;
}

void trs_stars_func_uninit(struct trs_id_inst *inst)
{
    trs_uninit_notify_tbl_ns_base_addr(inst);
    trs_uninit_event_tbl_ns_base_addr(inst);
    trs_uninit_rtsq_ns_base_addr(inst);
    trs_uninit_sq0_base_addr(inst);
}

#define TRS_STARS_NOTIFIER "trs_stars"
int trs_stars_notifier_register(void)
{
    struct uda_dev_type type;
    int ret;

    uda_davinci_local_real_entity_type_pack(&type);
    ret = uda_notifier_register(TRS_STARS_NOTIFIER, &type, UDA_PRI1, trs_stars_notifier_func);
    if (ret != 0) {
        trs_err("Register local notifier failed. (ret=%d)\n", ret);
        return ret;
    }
    return ret;
}

void trs_stars_notifier_unregister(void)
{
    struct uda_dev_type type;

    uda_davinci_local_real_entity_type_pack(&type);
    (void)uda_notifier_unregister(TRS_STARS_NOTIFIER, &type);
}