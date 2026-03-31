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
* Create: 2023-1-10
*/
#include "trs_host_comm.h"
#include "trs_pm_adapt.h"
#include "trs_core.h"
#include "trs_mailbox_def.h"
#include "trs_tscpu_cq.h"
#include "trs_core_near_ops_mbox.h"
#include "trs_core_near_ops.h"

static int trs_tscpu_core_ops_sq_id_head_from_hw_cqe(struct trs_id_inst *inst, void *hw_cqe, u32 *sqid, u32 *sq_head)
{
    trs_tscpu_cqe_get_sq_id(inst, hw_cqe, sqid);
    trs_tscpu_cqe_get_sq_head(inst, hw_cqe, sq_head);
    return 0;
}

static int trs_tscpu_core_ops_notice_ts(struct trs_id_inst *inst, u8 *msg, u32 len)
{
    return trs_core_ops_notice_ts(inst, msg, len);
}

static struct trs_core_adapt_ops trs_tscpu_core_ops = {
    .owner = THIS_MODULE,
    .ssid_query = trs_host_get_ssid,
    .get_res_surport_proc_num = trs_core_ops_get_proc_num,
    .get_sq_id_head_from_hw_cqe = trs_tscpu_core_ops_sq_id_head_from_hw_cqe,
    .notice_ts = trs_tscpu_core_ops_notice_ts,
    .get_res_reg_offset = trs_core_ops_get_res_reg_offset,
    .get_res_reg_total_size = trs_core_ops_get_res_reg_total_size,
};

struct trs_core_adapt_ops *trs_core_get_tscpu_adapt_ops(void)
{
    return &trs_tscpu_core_ops;
}

