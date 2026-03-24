/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-10-17
*/
#include "soc_res.h"
#include "trs_pm_adapt.h"
#include "trs_chip_def.h"
#include "trs_mailbox_def.h"
#include "trs_mia_cfg.h"
#include "trs_id.h"
#include "trs_stars.h"
#include "trs_core_ops.h"
#include "uda.h"
#include "trs_ts_db.h"
#define TRS_MIA_CORE_MBOX_TIMEOUT 3000

static int trs_mia_core_ops_cbcq_msg_fill(struct trs_mia_cfg *mia_cfg, u8 *msg, u32 len)
{
    struct trs_cb_cq_mbox *tmp_msg  = (struct trs_cb_cq_mbox *)msg;

    tmp_msg->plat_type = TRS_MBOX_SEND_FROM_DEVICE;
    return 0;
}

static int trs_mia_core_ops_sqcq_msg_fill(struct trs_mia_cfg *mia_cfg, u8 *msg, u32 len)
{
    struct trs_normal_cqsq_mailbox *tmp_msg = (struct trs_normal_cqsq_mailbox *)msg;

    tmp_msg->app_type = TRS_MBOX_SEND_FROM_DEVICE;
    tmp_msg->sq_cq_side = (TRS_CHAN_DEV_MEM << TRS_CHAN_SQ_MEM_OFFSET) |
        (TRS_CHAN_DEV_MEM << TRS_CHAN_CQ_MEM_OFFSET);
    tmp_msg->fid = mia_cfg->vfid;

    return 0;
}

static int trs_mia_core_ops_logic_cq_alloc_msg_fill(struct trs_mia_cfg *mia_cfg, u8 *msg, u32 len)
{
    struct trs_logic_cq_mbox *tmp_msg = (struct trs_logic_cq_mbox *)msg;

    tmp_msg->mb_alloc.app_flag = TRS_MBOX_SEND_FROM_DEVICE;
    tmp_msg->mb_alloc.vfid = mia_cfg->vfid;
    return 0;
}

static int trs_mia_core_ops_logic_cq_free_msg_fill(struct trs_mia_cfg *mia_cfg, u8 *msg, u32 len)
{
    struct trs_logic_cq_mbox *tmp_msg = (struct trs_logic_cq_mbox *)msg;

    tmp_msg->mb_free.vfid = mia_cfg->vfid;

    return 0;
}

static int trs_mia_core_ops_recycle_check_msg_fill(struct trs_mia_cfg *mia_cfg, u8 *msg, u32 len)
{
    struct recycle_proc_msg *tmp_msg = (struct recycle_proc_msg *)msg;

    tmp_msg->proc_info.plat_type = TRS_MBOX_SEND_FROM_DEVICE;
    tmp_msg->proc_info.fid = mia_cfg->vfid;

    return 0;
}

static int trs_mia_core_ops_res_map_msg_fill(struct trs_mia_cfg *mia_cfg, u8 *msg, u32 len)
{
    struct trs_res_map_msg *tmp_msg = (struct trs_res_map_msg *)msg;

    tmp_msg->vf_id = mia_cfg->vfid;
    tmp_msg->phy_id = tmp_msg->id;

    return 0;
}

static int trs_mia_core_ops_reset_notify(struct trs_mia_cfg *mia_cfg, u8 *msg, u32 len)
{
    struct trs_notify_msg *tmp_msg = (struct trs_notify_msg *)msg;

    tmp_msg->fid = mia_cfg->vfid;
    tmp_msg->plat_type = TRS_MBOX_SEND_FROM_DEVICE;
    tmp_msg->phy_notifyId = tmp_msg->notifyId;

    return 0;
}

static int (* const trs_mia_core_ops_fill_mbox_msg[TRS_MBOX_CMD_MAX])
    (struct trs_mia_cfg *mia_cfg, u8 *msg, u32 len) = {
    [TRS_MBOX_CREATE_CB_CQ] = trs_mia_core_ops_cbcq_msg_fill,
    [TRS_MBOX_RELEASE_CB_CQ] = trs_mia_core_ops_cbcq_msg_fill,
    [TRS_MBOX_CREATE_CQSQ_CALC] = trs_mia_core_ops_sqcq_msg_fill,
    [TRS_MBOX_RELEASE_CQSQ_CALC] = trs_mia_core_ops_sqcq_msg_fill,
    [TRS_MBOX_LOGIC_CQ_ALLOC] = trs_mia_core_ops_logic_cq_alloc_msg_fill,
    [TRS_MBOX_LOGIC_CQ_FREE] = trs_mia_core_ops_logic_cq_free_msg_fill,
    [TRS_MBOX_RECYCLE_PID] = trs_mia_core_ops_recycle_check_msg_fill,
    [TRS_MBOX_RECYCLE_CHECK] = trs_mia_core_ops_recycle_check_msg_fill,
    [TRS_MBOX_RES_MAP] = trs_mia_core_ops_res_map_msg_fill,
    [TRS_MBOX_RESET_NOTIFY] = trs_mia_core_ops_reset_notify
};

static int trs_mia_core_ops_notice_ts(struct trs_id_inst *inst, u8 *msg, u32 len)
{
    struct trs_mb_header *header = (struct trs_mb_header *)msg;
    struct trs_mia_cfg *mia_cfg = NULL;
    int ret;

    /* stars not need res map */
    if (header->cmd_type == TRS_MBOX_RES_MAP) {
        return 0;
    }

    mia_cfg = trs_mia_cfg_get(inst);
    if (mia_cfg == NULL) {
        return -EINVAL;
    }

    if (trs_mia_core_ops_fill_mbox_msg[header->cmd_type] != NULL) {
        ret = trs_mia_core_ops_fill_mbox_msg[header->cmd_type](mia_cfg, msg, len);
        if (ret != 0) {
            trs_mia_cfg_put(mia_cfg);
            return ret;
        }
    }

    ret = trs_mbox_send(&mia_cfg->pm_inst, 0, (void *)msg, (size_t)len, TRS_MIA_CORE_MBOX_TIMEOUT);
    trs_mia_cfg_put(mia_cfg);
    return ret;
}

static int trs_mia_device_get_ssid(struct trs_id_inst *inst, int *ssid)
{
    struct trs_mia_cfg *mia_cfg = NULL;
    int ret = -EINVAL;

    mia_cfg = trs_mia_cfg_get(inst);
    if (mia_cfg != NULL) {
        ret = trs_device_get_ssid(&mia_cfg->pm_inst, ssid);
        trs_mia_cfg_put(mia_cfg);
    }

    return ret;
}

static int trs_mia_core_ops_res_id_ctrl(struct trs_id_inst *inst, u32 type, u32 sqid, u32 cmd)
{
    struct trs_mia_cfg *mia_cfg = NULL;
    int ret = -EINVAL;

    mia_cfg = trs_mia_cfg_get(inst);
    if (mia_cfg != NULL) {
        ret = trs_stars_res_id_ctrl(&mia_cfg->pm_inst, type, sqid, cmd);
        trs_mia_cfg_put(mia_cfg);
    }

    return ret;
}

int trs_mia_core_ops_get_support_proc_num(struct trs_id_inst *inst, u32 *proc_num)
{
    u32 max_id, total_num, pm_proc_num;
    int ret;

    ret = trs_core_ops_get_support_proc_num(inst, &pm_proc_num);
    if (ret != 0) {
        return ret;
    }

    /*
     * Proc number is related to pf/vf capacity
     * Use stream id to calculate vf capacity
     */
    ret = trs_id_get_max_id(inst, TRS_STREAM_ID, &max_id);
    ret |= trs_id_get_total_num(inst, TRS_STREAM_ID, &total_num);
    if (ret != 0) {
        trs_err("Get stream id info fail. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }
    if (max_id != total_num) {
        if ((max_id / total_num) <= 8) { /* 1/8 capacity */
            *proc_num = pm_proc_num / 2; /* not less than 1/8 capacity, support 1/2 pm_proc_num */
        } else {
            *proc_num = pm_proc_num / 8; /* less than 1/8 capacity, support 1/8 pm_proc_num */
        }
    }

    return 0;
}
EXPORT_SYMBOL(trs_mia_core_ops_get_support_proc_num);

int trs_mia_core_ops_get_ts_inst_status(struct trs_id_inst *inst, u32 *status)
{
    struct res_inst_info res_inst;
    u32 pf_id, vf_id;
    int ret;

    ret = vmngd_get_pfvf_id_by_devid(inst->devid, &pf_id, &vf_id);
    if (ret != 0) {
        trs_err("Get pf devid failed. (devid=%u; ret=%d)\n", inst->devid, ret);
        return ret;
    }

    soc_resmng_inst_pack(&res_inst, pf_id, TS_SUBSYS, inst->tsid);
    ret = soc_resmng_get_ts_status(&res_inst, status);
    if (ret != 0) {
        trs_err("Get ts status failed. (devid=%u; ret=%d)\n", inst->devid, ret);
    }

    return ret;
}

static int trs_mia_core_ops_sqcq_reg_map(struct trs_id_inst *inst, struct trs_sqcq_reg_map_para *para)
{
    struct trs_mia_cfg *mia_cfg = NULL;
    int ret = -ENOSPC;
    phys_addr_t pa;
    size_t size;
    u64 va;

    mia_cfg = trs_mia_cfg_get(inst);
    if (mia_cfg == NULL) {
        trs_err("Mia cfg get fail. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return ret;
    }

    ret = trs_core_ops_get_sq_reg_paddr(&mia_cfg->pm_inst, para->sqid, &pa, &size);
    trs_mia_cfg_put(mia_cfg);
    if (ret != 0) {
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
    }
    return ret;
}

static int trs_mia_core_ops_sqcq_reg_unmap(struct trs_id_inst *inst, struct trs_sqcq_reg_map_para *para)
{
    return trs_core_ops_sqcq_reg_unmap(inst, para);
}

struct trs_core_adapt_ops trs_mia_device_core_ops = {
    .owner = THIS_MODULE,
    .ssid_query = trs_mia_device_get_ssid,                          /* call os_svm, need pm_inst */
    .get_res_surport_proc_num = trs_mia_core_ops_get_support_proc_num,  /* related to ability */
    .get_sq_id_head_from_hw_cqe = trs_core_ops_get_sq_id_head_from_hw_cqe,
    .get_stream_id_from_hw_cqe = trs_core_ops_get_stream_from_cqe,  /* parse cqe, no need pm_inst */
    .hw_cqe_to_logic_cqe = trs_core_ops_cqe_to_logic_cqe,           /* parse cqe, no need pm_inst */
    .notice_ts = trs_mia_core_ops_notice_ts,                        /* use pm mailbox */
    .res_id_ctrl = trs_mia_core_ops_res_id_ctrl,                /* operate stars, need pm_inst */
    .get_res_reg_offset = trs_core_ops_get_res_reg_offset,          /* no need pm_inst */
    .get_res_reg_total_size = trs_core_ops_get_res_reg_total_size,  /* no need pm_inst */
    .sqcq_reg_map = trs_mia_core_ops_sqcq_reg_map,
    .sqcq_reg_unmap = trs_mia_core_ops_sqcq_reg_unmap,
    .get_ts_inst_status = trs_mia_core_ops_get_ts_inst_status,
    .trace_cqe_fill = trs_core_ops_trace_cqe_fill,
    .trace_sqe_fill = trs_core_ops_trace_sqe_fill
};

static int trs_sq_trigger_msg_send(struct trs_id_inst *inst, u32 db, u32 hwirq)
{
    struct trs_mia_cfg *mia_cfg = NULL;
    struct trs_sq_trigger_msg msg;
    int ret;

    mia_cfg = trs_mia_cfg_get(inst);
    if (mia_cfg == NULL) {
        return -EINVAL;
    }

    trs_mbox_init_header(&msg.header, TRS_MBOX_NOTICE_SQ_TRIGGER);
    msg.db = db;
    msg.irq = hwirq;
    msg.vfid = mia_cfg->vfid;
    ret = trs_mbox_send(&mia_cfg->pm_inst, 0, &msg, sizeof(struct trs_sq_trigger_msg), 3000); /* timeout 3000 ms */
    if ((ret != 0) || (msg.header.result != 0)) {
        trs_mia_cfg_put(mia_cfg);
        trs_err("Mbox send fail. (ret=%d; result=%d)\n", ret, msg.header.result);
        return -EFAULT;
    }
    trs_mia_cfg_put(mia_cfg);

    return 0;
}

static int trs_mia_set_sq_send_trigger_irq(struct trs_id_inst *inst, u32 irq, u32 hwirq)
{
    struct res_inst_info res_inst;
    u32 devid = inst->devid;
    u32 tsid = inst->tsid;
    int ret;

    soc_resmng_inst_pack(&res_inst, devid, TS_SUBSYS, tsid);
    ret = soc_resmng_set_irq_num(&res_inst, TS_SQ_SEND_TRIGGER_IRQ, 1);
    if (ret != 0) {
        trs_err("Set trigger irq_num failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    ret = soc_resmng_set_irq_by_index(&res_inst, TS_SQ_SEND_TRIGGER_IRQ, 0, irq);
    if (ret != 0) {
        trs_err("Set trigger irq failed. (devid=%u; tsid=%u; irq=%u)\n", devid, tsid, irq);
        return ret;
    }
    ret = soc_resmng_set_hwirq(&res_inst, TS_SQ_SEND_TRIGGER_IRQ, irq, hwirq);
    if (ret != 0) {
        trs_err("Set trigger hwirq failed. (devid=%u; tsid=%u; irq=%u; hwirq=%u)\n", devid, tsid, irq, hwirq);
        return ret;
    }

    return 0;
}

static int trs_mia_sq_send_trigger_db_init(struct trs_id_inst *inst)
{
    u32 start, end, irq, hwirq, pf_id, vf_id;
    struct res_inst_info res_inst;
    int ret;

    ret = trs_get_trigger_sq_db_cfg(inst, &start, &end);
    if (ret != 0) {
        trs_info("Not support UIO mode. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    ret = vmngd_get_pfvf_id_by_devid(inst->devid, &pf_id, &vf_id);
    if (ret != 0) {
        trs_err("Get pf vf id failed. (devid=%u; pf_id=%u; vf_id=%u)\n", inst->devid, pf_id, vf_id);
        return ret;
    }

    ret = trs_ts_db_init(inst, TRS_DB_TRIGGER_SQ, start, end);
    if (ret != 0) {
        trs_err("Trs db init fail. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    soc_resmng_inst_pack(&res_inst, pf_id, TS_SUBSYS, inst->tsid);
    ret = soc_resmng_get_irq_by_index(&res_inst, TS_SQ_SEND_TRIGGER_IRQ, vf_id, &irq);
    ret |= soc_resmng_get_hwirq(&res_inst, TS_SQ_SEND_TRIGGER_IRQ, irq, &hwirq);
    if (ret != 0) {
        trs_ts_db_uninit(inst, TRS_DB_TRIGGER_SQ);
        trs_err("Get irq fail. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    ret = trs_sq_trigger_msg_send(inst, start, hwirq);
    if (ret != 0) {
        trs_ts_db_uninit(inst, TRS_DB_TRIGGER_SQ);
        trs_err("Send trigger msg fail. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    ret = trs_mia_set_sq_send_trigger_irq(inst, irq, hwirq);
    if (ret != 0) {
        trs_ts_db_uninit(inst, TRS_DB_TRIGGER_SQ);
        trs_err("Set trigger sq irq fail. (devid=%u; tsid=%u; irq=%u; hwirq=%u; ret=%d)\n",
            inst->devid, inst->tsid, irq, hwirq, ret);
        return ret;
    }

    trs_info("Trigger init ok. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
    return 0;
}

static void trs_mia_sq_send_trigger_db_uninit(struct trs_id_inst *inst)
{
    u32 start, end;
    if (trs_get_trigger_sq_db_cfg(inst, &start, &end)) {
        return;
    }
    trs_ts_db_uninit(inst, TRS_DB_TRIGGER_SQ);
}

int trs_mia_device_core_config(struct trs_id_inst *inst)
{
    int ret;

    ret = trs_mia_sq_send_trigger_db_init(inst);
    if (ret == 0) {
        trs_mia_device_core_ops.get_sq_trigger_irq = trs_core_ops_get_sq_trigger_irq;
        trs_mia_device_core_ops.get_trigger_sqid = trs_core_ops_get_trigger_sqid;
        trs_mia_device_core_ops.set_trigger_irq_affinity = trs_core_ops_set_trigger_irq_affinity;
    }

    return trs_core_ts_inst_register(inst, trs_get_hw_type(), &trs_mia_device_core_ops);
}

void trs_mia_device_core_deconfig(struct trs_id_inst *inst)
{
    trs_mia_sq_send_trigger_db_uninit(inst);
    trs_core_ts_inst_unregister(inst);
}

