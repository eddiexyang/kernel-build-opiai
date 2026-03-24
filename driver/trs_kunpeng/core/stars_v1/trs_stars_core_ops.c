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
* Create: 2023-8-15
*/
#include <asm/page.h>

#include "soc_res.h"
#include "trs_pm_adapt.h"
#include "trs_core.h"
#include "trs_stars.h"
#include "trs_stars_cq.h"
#include "trs_core_smmu.h"
#include "trs_core_ops.h"
#include "trs_mailbox_def.h"
#include "trs_stars_comm.h"
#include "trs_chip_def.h"
#include "trs_res_mng.h"
#include "trs_ts_db.h"
int trs_core_ops_get_sq_id_head_from_hw_cqe(struct trs_id_inst *inst, void *hw_cqe, u32 *sqid, u32 *sq_head)
{
    trs_stars_cqe_get_sqid(inst, hw_cqe, sqid);
    trs_stars_cqe_get_sq_head(inst, hw_cqe, sq_head);
    return 0;
}
EXPORT_SYMBOL(trs_core_ops_get_sq_id_head_from_hw_cqe);

int trs_core_ops_get_stream_from_cqe(struct trs_id_inst *inst, void *hw_cqe, u32 *stream_id)
{
    return trs_stars_cqe_get_streamid(inst, hw_cqe, stream_id);
}
EXPORT_SYMBOL(trs_core_ops_get_stream_from_cqe);

int trs_core_ops_cqe_to_logic_cqe(struct trs_id_inst *inst, void *hw_cqe,
    struct trs_logic_cqe *logic_cqe)
{
    trs_stars_cqe_to_logic_cqe(hw_cqe, logic_cqe);
    return 0;
}
EXPORT_SYMBOL(trs_core_ops_cqe_to_logic_cqe);

static int trs_stars_core_ops_notice_ts(struct trs_id_inst *inst, u8 *msg, u32 len)
{
    struct trs_mb_header *header = (struct trs_mb_header *)msg;

    /* stars not need res map */
    if (header->cmd_type == TRS_MBOX_RES_MAP) {
        return 0;
    }

    return trs_core_ops_notice_ts(inst, msg, len);
}

static struct trs_core_adapt_ops trs_core_ops = {
    .owner = THIS_MODULE,
    .proc_bind_smmu = trs_core_ops_proc_bind_smmu,
    .proc_unbind_smmu = trs_core_ops_proc_unbind_smmu,
    .ssid_query = trs_core_ops_get_ssid,
    .get_res_surport_proc_num = trs_core_ops_get_support_proc_num,
    .get_sq_id_head_from_hw_cqe = trs_core_ops_get_sq_id_head_from_hw_cqe,
    .get_stream_id_from_hw_cqe = trs_core_ops_get_stream_from_cqe,
    .hw_cqe_to_logic_cqe = trs_core_ops_cqe_to_logic_cqe,
    .notice_ts = trs_stars_core_ops_notice_ts,
    .get_res_reg_offset = trs_core_ops_get_res_reg_offset,
    .get_res_reg_total_size = trs_core_ops_get_res_reg_total_size,
    .get_ts_inst_status = trs_core_ops_get_ts_inst_status
};

struct trs_core_adapt_ops *trs_core_get_adapt_ops(void)
{
    return &trs_core_ops;
}

