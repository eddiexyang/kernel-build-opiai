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

#ifndef TRS_ADAPT_H
#define TRS_ADAPT_H
#include <linux/types.h>

#include "trs_res_id_def.h"
#include "trs_sqe_update.h"

struct trs_sqcq_agent_para {
    phys_addr_t rsv_phy_addr;
    size_t      rsv_size;
};

struct trs_sqcq_agent_ops {
    int (*device_init)(u32 devid, u32 tsid, struct trs_sqcq_agent_para *para);
    int (*device_uninit)(u32 devid, u32 tsid);
    int (*sqe_update)(u32 devid, u32 tsid, int pid, void *sqe);
    int (*cqe_update)(u32 devid, u32 tsid, int pid, void *cqe);
    int (*mb_update)(u32 devid, u32 tsid, int pid, void *data, u32 size);
};

void trs_sqcq_agent_ops_register(struct trs_sqcq_agent_ops *ops);
void trs_sqcq_agent_ops_unregister(void);

bool trs_is_proc_has_res(u32 devid, u32 tsid, int pid, int res_type, int res_id);
int trs_res_trans_v2p(u32 devid, u32 tsid, int res_type, int res_id, int *trans_res_id);
int trs_res_trans_p2v(u32 devid, u32 tsid, int res_type, int res_id, int *trans_res_id);

#endif

