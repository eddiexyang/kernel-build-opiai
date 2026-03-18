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
#include <asm/page.h>
#include <linux/kthread.h>

#include "soc_res.h"
#include "trs_pm_adapt.h"
#include "trs_core.h"
#include "trs_stars.h"
#include "trs_stars_cq.h"
#include "trs_stars_sq.h"
#include "trs_device_comm.h"
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

static u32 trsResCmdTrans[TRS_RES_OP_MAX] = {
    [TRS_RES_OP_RESET] = TRS_STARS_RES_OP_RESET,
    [TRS_RES_OP_RECORD] = TRS_STARS_RES_OP_RECORD,
    [TRS_RES_OP_ENABLE] = TRS_STARS_RES_OP_ENABLE,
    [TRS_RES_OP_DISABLE] = TRS_STARS_RES_OP_DISABLE,
    [TRS_RES_OP_CHECK_AND_RESET] = TRS_STARS_RES_OP_CHECK_AND_RESET,
};

int trs_stars_res_id_ctrl(struct trs_id_inst *inst, u32 id_type, u32 id, u32 cmd)
{
    return trs_stars_soc_res_ctrl(inst, id_type, id, trsResCmdTrans[cmd]);
}
EXPORT_SYMBOL(trs_stars_res_id_ctrl);

int trs_core_ops_sqcq_reg_map(struct trs_id_inst *inst, struct trs_sqcq_reg_map_para *para)
{
    phys_addr_t pa;
    size_t size;
    int ret;
    u64 va;

    ret = trs_core_ops_get_sq_reg_paddr(inst, para->sqid, &pa, &size);
    if (ret != 0) {
        trs_debug("No rtsq stars. (ret=%d; devid=%u; tsid=%u; sqid=%u)\n", ret, inst->devid, inst->tsid, para->sqid);
        return 0;
    }

    ret = trs_core_ops_mmap(current->tgid, pa, size, &va);
    if (ret != 0) {
        trs_err("Mmap fail. (ret=%d; devid=%u; tsid=%u; sqid=%u)\n", ret, inst->devid, inst->tsid, para->sqid);
        return ret;
    }

    ret = trs_set_sq_reg_vaddr(inst, para->sqid, va, size);
    if (ret != 0) {
        trs_core_ops_unmap(current->tgid, va, size);
        trs_err("Set sq reg vaddr fail. (ret=%d; devid=%u; tsid=%u; sqid=%u)\n",
            ret, inst->devid, inst->tsid, para->sqid);
        return ret;
    }
    return 0;
}
EXPORT_SYMBOL(trs_core_ops_sqcq_reg_map);

int trs_core_ops_sqcq_reg_unmap(struct trs_id_inst *inst, struct trs_sqcq_reg_map_para *para)
{
    size_t size;
    int ret;
    u64 va;

    ret = trs_get_sq_reg_vaddr(inst, para->sqid, &va, &size);
    if ((ret != 0) || (va == 0)) {
        return 0;
    }

    ret = trs_core_ops_unmap(current->tgid, va, size);
    if (ret != 0) {
        trs_err("Unmap fail. (ret=%d)\n", ret);
        return ret;
    }
    (void)trs_set_sq_reg_vaddr(inst, para->sqid, 0, 0);
    return ret;
}
EXPORT_SYMBOL(trs_core_ops_sqcq_reg_unmap);

int trs_core_ops_get_sq_trigger_irq(struct trs_id_inst *inst, u32 *irq)
{
    struct res_inst_info res_inst;

    soc_resmng_inst_pack(&res_inst, inst->devid, TS_SUBSYS, inst->tsid);
    return soc_resmng_get_irq_by_index(&res_inst, TS_SQ_SEND_TRIGGER_IRQ, 0, irq);
}
EXPORT_SYMBOL(trs_core_ops_get_sq_trigger_irq);

int trs_core_ops_get_trigger_sqid(struct trs_id_inst *inst, u32 *sqid)
{
    return trs_get_ts_db_val(inst, TRS_DB_TRIGGER_SQ, 0, sqid);
}
EXPORT_SYMBOL(trs_core_ops_get_trigger_sqid);

void trs_core_ops_set_trigger_irq_affinity(struct trs_id_inst *inst, u32 irq, u32 op)
{
    if (op == 0) {
        (void)irq_set_affinity_hint(irq, NULL);
    } else {
        (void)irq_set_affinity_hint(irq, get_cpu_mask(0));
    }
}
EXPORT_SYMBOL(trs_core_ops_set_trigger_irq_affinity);

void trs_set_thread_affinity(struct trs_id_inst *inst, struct task_struct *thread)
{
    kthread_bind(thread, 0); /* cpu 0: ctrl cpu */
}
EXPORT_SYMBOL(trs_set_thread_affinity);

void trs_core_ops_trace_sqe_fill(struct trs_id_inst *inst, struct trs_chan_sq_trace *sq_trace, void *sqe)
{
    trs_stars_trace_sqe_fill(inst, sq_trace, sqe);
}
EXPORT_SYMBOL(trs_core_ops_trace_sqe_fill);

void trs_core_ops_trace_cqe_fill(struct trs_id_inst *inst, struct trs_chan_cq_trace *cq_trace, void *cqe)
{
    trs_stars_trace_cqe_fill(inst, cq_trace, cqe);
}
EXPORT_SYMBOL(trs_core_ops_trace_cqe_fill);

static struct trs_core_adapt_ops trs_core_ops = {
    .owner = THIS_MODULE,
    .ssid_query = trs_device_get_ssid,
    .get_res_surport_proc_num = trs_core_ops_get_support_proc_num,
    .get_sq_id_head_from_hw_cqe = trs_core_ops_get_sq_id_head_from_hw_cqe,
    .get_stream_id_from_hw_cqe = trs_core_ops_get_stream_from_cqe,
    .hw_cqe_to_logic_cqe = trs_core_ops_cqe_to_logic_cqe,
    .notice_ts = trs_stars_core_ops_notice_ts,
    .res_id_ctrl = trs_stars_res_id_ctrl,
    .get_res_reg_offset = trs_core_ops_get_res_reg_offset,
    .get_res_reg_total_size = trs_core_ops_get_res_reg_total_size,
    .res_id_query = trs_core_ops_res_id_query,
    .sqcq_reg_map = trs_core_ops_sqcq_reg_map,
    .sqcq_reg_unmap = trs_core_ops_sqcq_reg_unmap,
    .get_ts_inst_status = trs_core_ops_get_ts_inst_status,
    .set_thread_affinity = trs_set_thread_affinity,
    .trace_sqe_fill = trs_core_ops_trace_sqe_fill,
    .trace_cqe_fill = trs_core_ops_trace_cqe_fill
};

int trs_core_ops_get_sq_reg_paddr(struct trs_id_inst *inst, u32 sqid, phys_addr_t *paddr, size_t *size)
{
    return trs_stars_get_rtsq_paddr(inst, sqid, paddr, size);
}
EXPORT_SYMBOL(trs_core_ops_get_sq_reg_paddr);

struct trs_core_adapt_ops *trs_core_get_adapt_ops(void)
{
    return &trs_core_ops;
}

static int trs_sq_trigger_msg_send(struct trs_id_inst *inst, u32 db, u32 hwirq)
{
    struct trs_sq_trigger_msg msg;
    int ret;

    trs_mbox_init_header(&msg.header, TRS_MBOX_NOTICE_SQ_TRIGGER);
    msg.db = db;
    msg.irq = hwirq;
    msg.vfid = 0;
    ret = trs_mbox_send(inst, 0, &msg, sizeof(struct trs_sq_trigger_msg), 3000); /* timeout 3000 ms */
    if ((ret != 0) || (msg.header.result != 0)) {
        trs_err("Mbox send fail. (ret=%d; result=%d)\n", ret, msg.header.result);
        return -EFAULT;
    }

    return 0;
}

int trs_sq_send_trigger_db_init(struct trs_id_inst *inst)
{
    struct res_inst_info res_inst;
    u32 start, end, irq, hwirq;
    int ret;

    ret = trs_get_trigger_sq_db_cfg(inst, &start, &end);
    if (ret != 0) {
        trs_info("Not support UIO mode. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    ret = trs_ts_db_init(inst, TRS_DB_TRIGGER_SQ, start, end);
    if (ret != 0) {
        trs_err("Trs db init fail. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    soc_resmng_inst_pack(&res_inst, inst->devid, TS_SUBSYS, inst->tsid);
    ret = soc_resmng_get_irq_by_index(&res_inst, TS_SQ_SEND_TRIGGER_IRQ, 0, &irq);
    ret |= soc_resmng_get_hwirq(&res_inst, TS_SQ_SEND_TRIGGER_IRQ, irq, &hwirq);
    if (ret != 0) {
        trs_ts_db_uninit(inst, TRS_DB_TRIGGER_SQ);
        trs_err("Get irq fail. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    ret = trs_sq_trigger_msg_send(inst, start, hwirq);
    if (ret != 0) {
        trs_ts_db_uninit(inst, TRS_DB_TRIGGER_SQ);
        return ret;
    }

    trs_info("sq trigger init ok. (devid=%u)\n", inst->devid);
    return 0;
}

void trs_sq_send_trigger_db_uninit(struct trs_id_inst *inst)
{
    u32 start, end;
    if (trs_get_trigger_sq_db_cfg(inst, &start, &end) != 0) {
        return;
    }
    trs_ts_db_uninit(inst, TRS_DB_TRIGGER_SQ);
}
