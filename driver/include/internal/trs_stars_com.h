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
#ifndef TRS_STARS_COM_H
#define TRS_STARS_COM_H

#include "uda.h"
#include "trs_pub_def.h"

struct trs_stars_ops {
    int (*res_id_ctrl)(struct trs_id_inst *inst, u32 type, u32 id, u32 cmd);
    int (*get_id_status)(struct trs_id_inst *inst, u32 type, u32 id, u32 *status);
    void (*set_sq_head)(struct trs_id_inst *inst, u32 sqid, u32 val);
    int (*get_sq_head)(struct trs_id_inst *inst, u32 sqid);
    void (*set_sq_tail)(struct trs_id_inst *inst, u32 sqid, u32 val);
    int (*get_sq_tail)(struct trs_id_inst *inst, u32 sqid);
    int (*get_sq_fsm_status)(struct trs_id_inst *inst, u32 sqid);
};
int trs_stars_notifier_func(u32 udevid, enum uda_notified_action action);
#endif
