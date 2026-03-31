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
#include "trs_pm_adapt.h"
#include "trs_core.h"
#include "trs_device_comm.h"
#include "trs_core_ops.h"
#include "trs_mailbox_def.h"
#include "trs_tscpu_cq.h"
#include "trs_chip_def.h"

int trs_core_ops_get_sq_reg_paddr(struct trs_id_inst *inst, u32 sqid, phys_addr_t *paddr, size_t *size)
{
    return -ENOSPC;
}
EXPORT_SYMBOL(trs_core_ops_get_sq_reg_paddr);

int trs_tscpu_core_ops_sq_id_head_from_hw_cqe(struct trs_id_inst *inst, void *hw_cqe, u32 *sqid, u32 *sq_head)
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
    .ssid_query = trs_device_get_ssid,
    .get_res_surport_proc_num = trs_core_ops_get_support_proc_num,
    .get_sq_id_head_from_hw_cqe = trs_tscpu_core_ops_sq_id_head_from_hw_cqe,
    .notice_ts = trs_tscpu_core_ops_notice_ts,
    .get_res_reg_offset = trs_core_ops_get_res_reg_offset,
    .get_res_reg_total_size = trs_core_ops_get_res_reg_total_size,
};

struct trs_core_adapt_ops *trs_core_get_adapt_ops(void)
{
    return &trs_tscpu_core_ops;
}

int trs_sq_send_trigger_db_init(struct trs_id_inst *inst)
{
    return 1;
}
EXPORT_SYMBOL(trs_sq_send_trigger_db_init);

void trs_sq_send_trigger_db_uninit(struct trs_id_inst *inst)
{
}
EXPORT_SYMBOL(trs_sq_send_trigger_db_uninit);

int trs_core_ops_get_sq_trigger_irq(struct trs_id_inst *inst, u32 *irq)
{
    return 1;
}
EXPORT_SYMBOL(trs_core_ops_get_sq_trigger_irq);

int trs_core_ops_get_trigger_sqid(struct trs_id_inst *inst, u32 *sqid)
{
    return 1;
}
EXPORT_SYMBOL(trs_core_ops_get_trigger_sqid);

void trs_core_ops_set_trigger_irq_affinity(struct trs_id_inst *inst, u32 irq, u32 op)
{
}
EXPORT_SYMBOL(trs_core_ops_set_trigger_irq_affinity);

void trs_set_thread_affinity(struct trs_id_inst *inst, struct task_struct *thread)
{
}
EXPORT_SYMBOL(trs_set_thread_affinity);
