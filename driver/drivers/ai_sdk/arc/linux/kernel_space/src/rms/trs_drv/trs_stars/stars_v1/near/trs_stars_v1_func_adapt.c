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
#include <linux/module.h>

#include "dbl/uda.h"

#include "trs_stars_v1_func_com.h"
#include "soc_res.h"
#include "stars_event_tbl_ns.h"
#include "trs_res_id_def.h"
#include "trs_stars_com.h"
#include "trs_stars_comm.h"

static int trs_stars_func_event_id_ctrl(struct trs_id_inst *inst, u32 id, u32 cmd)
{
    if (cmd == TRS_STARS_RES_OP_RESET) {
        return trs_stars_func_event_id_reset(inst, id);
    }

    if (cmd == TRS_STARS_RES_OP_CHECK_AND_RESET) {
        return trs_stars_func_event_id_check_and_reset(inst, id);
    }

    trs_err("Invalid ctrl cmd. (devid=%u; tsid=%u; id=%u; cmd=%u)\n", inst->devid, inst->tsid, id, cmd);
    return -EINVAL;
}

static int trs_stars_func_res_id_ctrl(struct trs_id_inst *inst, u32 type, u32 id, u32 cmd)
{
    if (type == TRS_EVENT) {
        return trs_stars_func_event_id_ctrl(inst, id, cmd);
    }

    trs_err("Not support id_type or cmd. (type=%u; cmd=%u)\n", type, cmd);
    return -EINVAL;
}

static struct trs_stars_ops trs_stars_func_ops = {
    .res_id_ctrl = trs_stars_func_res_id_ctrl,
};

struct trs_stars_ops *trs_stars_func_op_get(void)
{
    return &trs_stars_func_ops;
}

int trs_stars_func_init(struct trs_id_inst *inst)
{
    int ret;

    ret = trs_init_event_tbl_ns_base_addr(inst);
    if (ret != 0) {
        trs_err("Failed to init event tbl ns base. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return ret;
    }

    return 0;
}

void trs_stars_func_uninit(struct trs_id_inst *inst)
{
    trs_uninit_event_tbl_ns_base_addr(inst);
}

#define TRS_STARS_NOTIFIER "trs_stars"
int trs_stars_notifier_register(void)
{
    struct uda_dev_type type;
    int ret;

    uda_davinci_near_real_entity_type_pack(&type);
    ret = uda_notifier_register(TRS_STARS_NOTIFIER, &type, UDA_PRI1, trs_stars_notifier_func);
    if (ret != 0) {
        trs_err("Register near notifier failed. (ret=%d)\n", ret);
        return ret;
    }
    return ret;
}

void trs_stars_notifier_unregister(void)
{
    struct uda_dev_type type;

    uda_davinci_near_real_entity_type_pack(&type);
    (void)uda_notifier_unregister(TRS_STARS_NOTIFIER, &type);
}
MODULE_SOFTDEP("pre: ascend_soc_platform");

