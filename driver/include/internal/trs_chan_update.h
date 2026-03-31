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
* Create: 2022-10-15
*/
#ifndef TRS_CHAN_UPDATE_H
#define TRS_CHAN_UPDATE_H

#include "trs_pub_def.h"
#include "trs_res_id_def.h"

struct trs_res_ops {
    bool (*res_belong_proc[TRS_MAX_ID_TYPE])(struct trs_id_inst *inst, int pid, int res_type, u32 res_id);
    int (*res_get_info[TRS_MAX_ID_TYPE])(struct trs_id_inst *inst, int res_type, u32 res_id, void *info);
};

void trs_res_ops_register(u32 devid, struct trs_res_ops *ops);
void trs_res_ops_unregister(u32 devid);

int trs_mb_update(struct trs_id_inst *inst, int pid, void *data, u32 size);
int trs_chan_ops_sqe_update(struct trs_id_inst *inst, int pid, void *sqe);
int trs_chan_ops_cqe_update(struct trs_id_inst *inst, int pid, void *cqe);

int trs_get_res_info(struct trs_id_inst *inst, int res_type, u32 res_id, void *info);

#endif
