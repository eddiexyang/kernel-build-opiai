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
#include <linux/sched.h>

#include "trs_pm_adapt.h"
#include "trs_chan_mem.h"
#include "securec.h"
#include "soc_res.h"
#include "trs_chan_mbox.h"

#define TRS_SQCQ_INFO_LEN   ((size_t)(SQCQ_INFO_LENGTH) * (sizeof(u32)))
#define TRS_DEVICE_CHAN_MBOX_TIMEOUT_MS    3000

typedef int (*chan_mbox_func)(struct trs_id_inst *, struct trs_chan_info *, struct trs_chan_adapt_info *);

static const u16 chan_create_mbox_cmd[CHAN_TYPE_MAX][CHAN_SUB_TYPE_MAX] = {
    [CHAN_TYPE_HW][CHAN_SUB_TYPE_HW_RTS] = TRS_MBOX_CREATE_CQSQ_CALC,
    [CHAN_TYPE_HW][CHAN_SUB_TYPE_HW_TOPIC_SCHED] = TRS_MBOX_CREATE_TOPIC_SQCQ,
    [CHAN_TYPE_HW][CHAN_SUB_TYPE_HW_DVPP] = TRS_MBOX_CREATE_KERNEL_SQCQ,
    [CHAN_TYPE_HW][CHAN_SUB_TYPE_HW_TS] = TRS_MBOX_NOTICE_TS_SQCQ_CREATE,
    [CHAN_TYPE_HW][CHAN_SUB_TYPE_HW_RSV_TS] = TRS_MBOX_NOTICE_TS_SQCQ_CREATE,
    [CHAN_TYPE_SW][CHAN_SUB_TYPE_SW_CTRL] = TRS_MBOX_CREATE_CTRL_CQSQ,
    [CHAN_TYPE_MAINT][CHAN_SUB_TYPE_MAINT_HB] = TRS_MBOX_CREATE_HB_SQCQ,
    [CHAN_TYPE_MAINT][CHAN_SUB_TYPE_MAINT_DBG] = TRS_MBOX_DBG_CQSQ_CREATE,
    [CHAN_TYPE_TASK_SCHED][CHAN_SUB_TYPE_TASK_SCHED_ASYNC_CB] = TRS_MBOX_CREATE_TASKSCHED_SQCQ,
};

static const u16 chan_release_mbox_cmd[CHAN_TYPE_MAX][CHAN_SUB_TYPE_MAX] = {
    [CHAN_TYPE_HW][CHAN_SUB_TYPE_HW_RTS] = TRS_MBOX_RELEASE_CQSQ_CALC,
    [CHAN_TYPE_HW][CHAN_SUB_TYPE_HW_TOPIC_SCHED] = TRS_MBOX_RELEASE_TOPIC_SQCQ,
    [CHAN_TYPE_HW][CHAN_SUB_TYPE_HW_DVPP] = TRS_MBOX_RELEASE_KERNEL_SQCQ,
    [CHAN_TYPE_HW][CHAN_SUB_TYPE_HW_TS] = TRS_MBOX_NOTICE_TS_SQCQ_FREE,
    [CHAN_TYPE_HW][CHAN_SUB_TYPE_HW_RSV_TS] = TRS_MBOX_NOTICE_TS_SQCQ_FREE,
    [CHAN_TYPE_SW][CHAN_SUB_TYPE_SW_CTRL] = TRS_MBOX_RELEASE_CTRL_CQSQ,
    [CHAN_TYPE_MAINT][CHAN_SUB_TYPE_MAINT_HB] = TRS_MBOX_RELEASE_HB_SQCQ,
    [CHAN_TYPE_MAINT][CHAN_SUB_TYPE_MAINT_DBG] = TRS_MBOX_DBG_CQSQ_RELEASE,
    [CHAN_TYPE_TASK_SCHED][CHAN_SUB_TYPE_TASK_SCHED_ASYNC_CB] = TRS_MBOX_RELEASE_TASKSCHED_SQCQ,
};

static u16 trs_chan_ops_get_mbox_cmd(u32 op, u32 type, u32 sub_type)
{
    return (op != 0) ? chan_create_mbox_cmd[type][sub_type] : chan_release_mbox_cmd[type][sub_type];
}

int trs_chan_ops_get_hw_irq(struct trs_id_inst *inst, int irq_type, u32 irq, u32 *hwirq)
{
    struct res_inst_info res_inst;

    soc_resmng_inst_pack(&res_inst, inst->devid, TS_SUBSYS, inst->tsid);
    return soc_resmng_get_hwirq(&res_inst, (u32)irq_type, irq, hwirq);
}

static int trs_chan_ops_hw_create_mbox_send(struct trs_id_inst *inst, struct trs_chan_info *chan_info,
    struct trs_chan_adapt_info *adapt)
{
    struct trs_normal_cqsq_mailbox mbox_data;
    u32 cq_irq;
    u16 cmd;
    int ret;

    cmd = trs_chan_ops_get_mbox_cmd(chan_info->op, chan_info->types.type, chan_info->types.sub_type);
    trs_mbox_init_header(&mbox_data.header, cmd);

    ret = trs_chan_ops_get_hw_irq(inst, chan_info->irq_type, chan_info->cq_info.irq, &cq_irq);
    if (ret != 0) {
        trs_err("Get hwirq fail. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    mbox_data.cq_irq = (u16)cq_irq;

    mbox_data.ssid = chan_info->ssid;
    mbox_data.app_type = adapt->app_type;
    mbox_data.sw_reg_flag = (cmd == TRS_MBOX_CREATE_CTRL_CQSQ) ? 1 : 0;
    mbox_data.pid = chan_info->pid;
    mbox_data.master_pid_flag = chan_info->master_pid_flag;
    mbox_data.sq_index = chan_info->sq_info.sqid;
    mbox_data.sq_addr = chan_info->sq_info.sq_phy_addr;
    mbox_data.sqesize = chan_info->sq_info.sq_para.sqe_size;
    mbox_data.sqdepth = chan_info->sq_info.sq_para.sq_depth;

    mbox_data.cq0_index = chan_info->cq_info.cqid;
    mbox_data.cq0_addr = chan_info->cq_info.cq_phy_addr;
    mbox_data.cqdepth = chan_info->cq_info.cq_para.cq_depth;
    mbox_data.cqesize = chan_info->cq_info.cq_para.cqe_size;
    mbox_data.sq_cq_side = ((adapt->sq_side << TRS_CHAN_SQ_MEM_OFFSET) | (adapt->cq_side << TRS_CHAN_CQ_MEM_OFFSET));

    mbox_data.fid = adapt->fid;

    ret = memcpy_s(mbox_data.info, TRS_SQCQ_INFO_LEN, chan_info->msg, TRS_SQCQ_INFO_LEN);
    if (ret != EOK) {
        trs_err("Memcpy fail. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    ret = trs_mbox_send(inst, 0, &mbox_data, sizeof(struct trs_normal_cqsq_mailbox), TRS_DEVICE_CHAN_MBOX_TIMEOUT_MS);
    if (ret != 0 || mbox_data.header.result != 0) {
        trs_err("Mbox send fail. (devid=%u; tsid=%u; cmd=%u; result=%u; ret=%d)\n",
            inst->devid, inst->tsid, cmd, mbox_data.header.result, ret);
        ret = -EFAULT;
    }
    return ret;
}

static int trs_chan_ops_hw_release_mbox_send(struct trs_id_inst *inst, struct trs_chan_info *chan_info,
    struct trs_chan_adapt_info *adapt)
{
    struct trs_normal_cqsq_mailbox mbox_data;
    u16 cmd;
    int ret;

    cmd = trs_chan_ops_get_mbox_cmd(chan_info->op, chan_info->types.type, chan_info->types.sub_type);
    trs_mbox_init_header(&mbox_data.header, cmd);

    mbox_data.pid = chan_info->pid;
    mbox_data.sq_index = chan_info->sq_info.sqid;
    mbox_data.sq_addr = 0;
    mbox_data.cq0_index = chan_info->cq_info.cqid;
    mbox_data.cq0_addr = 0;
    mbox_data.app_type = adapt->app_type;
    mbox_data.fid = adapt->fid;

    ret = memcpy_s(mbox_data.info, TRS_SQCQ_INFO_LEN, chan_info->msg, TRS_SQCQ_INFO_LEN);
    if (ret != EOK) {
        trs_err("Memcpy fail. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    ret = trs_mbox_send(inst, 0, &mbox_data, sizeof(struct trs_normal_cqsq_mailbox), TRS_DEVICE_CHAN_MBOX_TIMEOUT_MS);
    if (ret != 0 || mbox_data.header.result != 0) {
        if (ret == -ENXIO) {
            trs_warn("Mb chan is not exist. (devid=%u; tsid=%u; cmd=%u; ret=%d)\n", inst->devid, inst->tsid, cmd, ret);
        } else {
            trs_err("Mbos send fail. (devid=%u; tsid=%u; cmd=%u; result=%u; ret=%d)\n",
                inst->devid, inst->tsid, cmd, mbox_data.header.result, ret);
            ret = -EFAULT;
        }
    }
    return ret;
}

static int trs_chan_ops_maint_create_mbox_send(struct trs_id_inst *inst, struct trs_chan_info *chan_info,
    struct trs_chan_adapt_info *adapt)
{
    struct trs_maint_sqcq_mbox mbox_data;
    u32 cq_irq;
    int ret;
    u16 cmd;

    ret = trs_chan_ops_get_hw_irq(inst, chan_info->irq_type, chan_info->cq_info.irq, &cq_irq);
    if (ret != 0) {
        trs_err("Get hwirq fail. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    mbox_data.cq_irq = (u16)cq_irq;

    cmd = trs_chan_ops_get_mbox_cmd(chan_info->op, chan_info->types.type, chan_info->types.sub_type);
    trs_mbox_init_header(&mbox_data.header, cmd);

    mbox_data.sq_index = chan_info->sq_info.sqid;
    mbox_data.sq_addr = chan_info->sq_info.sq_phy_addr;

    mbox_data.cq0_index = chan_info->cq_info.cqid;
    mbox_data.cq0_addr = chan_info->cq_info.cq_phy_addr;
    mbox_data.plat_type = adapt->app_type;

    ret = trs_mbox_send(inst, 0, &mbox_data, sizeof(struct trs_maint_sqcq_mbox), TRS_DEVICE_CHAN_MBOX_TIMEOUT_MS);
    if (ret != 0 || mbox_data.header.result != 0) {
        trs_err("Mbos send fail. (devid=%u; tsid=%u; cmd=%u; result=%u; ret=%d)\n",
            inst->devid, inst->tsid, cmd, mbox_data.header.result, ret);
        ret = -EFAULT;
    }
    return ret;
}

static int trs_chan_ops_maint_release_mbox_send(struct trs_id_inst *inst, struct trs_chan_info *chan_info,
    struct trs_chan_adapt_info *adapt)
{
    struct trs_maint_sqcq_mbox mbox_data;
    int ret;
    u16 cmd;

    cmd = trs_chan_ops_get_mbox_cmd(chan_info->op, chan_info->types.type, chan_info->types.sub_type);
    trs_mbox_init_header(&mbox_data.header, cmd);

    mbox_data.sq_index = chan_info->sq_info.sqid;
    mbox_data.sq_addr = 0;

    mbox_data.cq0_index = chan_info->cq_info.cqid;
    mbox_data.cq0_addr = 0;
    mbox_data.plat_type = adapt->app_type;

    ret = trs_mbox_send(inst, 0, &mbox_data, sizeof(struct trs_maint_sqcq_mbox), TRS_DEVICE_CHAN_MBOX_TIMEOUT_MS);
    if (ret != 0 || mbox_data.header.result != 0) {
        trs_err("Mbos send fail. (devid=%u; tsid=%u; cmd=%u; result=%u; ret=%d)\n",
            inst->devid, inst->tsid, cmd, mbox_data.header.result, ret);
        ret = -EFAULT;
    }
    return ret;
}

static int trs_chan_ops_hw_ts_mbox_send(struct trs_id_inst *inst, struct trs_chan_info *chan_info,
    struct trs_chan_adapt_info *adapt)
{
    struct trs_ts_sqcq_mbox mbox_data;
    u16 cmd;
    int ret;

    cmd = trs_chan_ops_get_mbox_cmd(chan_info->op, chan_info->types.type, chan_info->types.sub_type);
    trs_mbox_init_header(&mbox_data.header, cmd);
    mbox_data.sqid = chan_info->sq_info.sqid;
    mbox_data.cqid = chan_info->cq_info.cqid;
    mbox_data.pid = chan_info->pid;
    mbox_data.vfid = adapt->fid;
    mbox_data.ssid = chan_info->ssid;

    ret = memcpy_s(mbox_data.info, TRS_SQCQ_INFO_LEN, chan_info->msg, TRS_SQCQ_INFO_LEN);
    if (ret != EOK) {
        trs_err("Memcpy fail. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    ret = trs_mbox_send(inst, 0, &mbox_data, sizeof(struct trs_ts_sqcq_mbox), TRS_DEVICE_CHAN_MBOX_TIMEOUT_MS);
    if (ret != 0 || mbox_data.header.result != 0) {
        trs_err("Mbox send fail. (devid=%u; tsid=%u; cmd=%u; result=%u; ret=%d)\n",
            inst->devid, inst->tsid, cmd, mbox_data.header.result, ret);
        ret = -EFAULT;
    }
    return ret;
}

static int trs_chan_ops_task_sched_create_mbox_send(struct trs_id_inst *inst, struct trs_chan_info *chan_info,
    struct trs_chan_adapt_info *adapt)
{
    struct trs_task_sched_sqcq_alloc_mbox mbox_data;
    u16 cmd;
    int ret;

    cmd = trs_chan_ops_get_mbox_cmd(chan_info->op, chan_info->types.type, chan_info->types.sub_type);
    trs_mbox_init_header(&mbox_data.header, cmd);

    ret = trs_chan_ops_get_hw_irq(inst, chan_info->irq_type, chan_info->cq_info.irq, &mbox_data.cq_irq);
    if (ret != 0) {
        trs_err("Get hwirq fail. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }

    mbox_data.plat_type = adapt->app_type;

    mbox_data.sq_index = chan_info->sq_info.sqid;
    mbox_data.sq_addr = chan_info->sq_info.sq_phy_addr;
    mbox_data.sqe_size = chan_info->sq_info.sq_para.sqe_size;
    mbox_data.sq_depth = chan_info->sq_info.sq_para.sq_depth;

    mbox_data.cq_index = chan_info->cq_info.cqid;
    mbox_data.cq_addr = chan_info->cq_info.cq_phy_addr;
    mbox_data.cqe_size = chan_info->cq_info.cq_para.cqe_size;
    mbox_data.cq_depth = chan_info->cq_info.cq_para.cq_depth;

    ret = trs_mbox_send(inst, 0, &mbox_data, sizeof(mbox_data), TRS_DEVICE_CHAN_MBOX_TIMEOUT_MS);
    if (ret != 0 || mbox_data.header.result != 0) {
        trs_err("Mbox send fail. (devid=%u; tsid=%u; cmd=%u; result=%u; ret=%d)\n",
            inst->devid, inst->tsid, cmd, mbox_data.header.result, ret);
        ret = -EFAULT;
    }

    return ret;
}

static int trs_chan_ops_task_sched_release_mbox_send(struct trs_id_inst *inst, struct trs_chan_info *chan_info,
    struct trs_chan_adapt_info *adapt)
{
    struct trs_task_sched_sqcq_free_mbox mbox_data;
    u16 cmd;
    int ret;

    cmd = trs_chan_ops_get_mbox_cmd(chan_info->op, chan_info->types.type, chan_info->types.sub_type);
    trs_mbox_init_header(&mbox_data.header, cmd);

    mbox_data.plat_type = adapt->app_type;
    mbox_data.sq_index = chan_info->sq_info.sqid;
    mbox_data.cq_index = chan_info->cq_info.cqid;

    ret = trs_mbox_send(inst, 0, &mbox_data, sizeof(mbox_data), TRS_DEVICE_CHAN_MBOX_TIMEOUT_MS);
    if (ret != 0 || mbox_data.header.result != 0) {
        trs_err("Mbos send fail. (devid=%u; tsid=%u; cmd=%u; result=%u; ret=%d)\n",
            inst->devid, inst->tsid, cmd, mbox_data.header.result, ret);
        ret = -EFAULT;
    }

    return ret;
}

static const chan_mbox_func chan_create_mbox_func[CHAN_TYPE_MAX][CHAN_SUB_TYPE_MAX] = {
    [CHAN_TYPE_HW][CHAN_SUB_TYPE_HW_RTS] = trs_chan_ops_hw_create_mbox_send,
    [CHAN_TYPE_HW][CHAN_SUB_TYPE_HW_TOPIC_SCHED] = trs_chan_ops_hw_create_mbox_send,
    [CHAN_TYPE_HW][CHAN_SUB_TYPE_HW_DVPP] = trs_chan_ops_hw_create_mbox_send,
    [CHAN_TYPE_HW][CHAN_SUB_TYPE_HW_TS] = trs_chan_ops_hw_ts_mbox_send,
    [CHAN_TYPE_HW][CHAN_SUB_TYPE_HW_RSV_TS] = trs_chan_ops_hw_ts_mbox_send,
    [CHAN_TYPE_SW][CHAN_SUB_TYPE_SW_CTRL] = trs_chan_ops_hw_create_mbox_send,
    [CHAN_TYPE_MAINT][CHAN_SUB_TYPE_MAINT_HB] = trs_chan_ops_maint_create_mbox_send,
    [CHAN_TYPE_MAINT][CHAN_SUB_TYPE_MAINT_DBG] = trs_chan_ops_maint_create_mbox_send,
    [CHAN_TYPE_TASK_SCHED][CHAN_SUB_TYPE_TASK_SCHED_ASYNC_CB] = trs_chan_ops_task_sched_create_mbox_send,
};

static const chan_mbox_func chan_release_mbox_func[CHAN_TYPE_MAX][CHAN_SUB_TYPE_MAX] = {
    [CHAN_TYPE_HW][CHAN_SUB_TYPE_HW_RTS] = trs_chan_ops_hw_release_mbox_send,
    [CHAN_TYPE_HW][CHAN_SUB_TYPE_HW_TOPIC_SCHED] = trs_chan_ops_hw_release_mbox_send,
    [CHAN_TYPE_HW][CHAN_SUB_TYPE_HW_DVPP] = trs_chan_ops_hw_release_mbox_send,
    [CHAN_TYPE_HW][CHAN_SUB_TYPE_HW_TS] = trs_chan_ops_hw_ts_mbox_send,
    [CHAN_TYPE_HW][CHAN_SUB_TYPE_HW_RSV_TS] = trs_chan_ops_hw_ts_mbox_send,
    [CHAN_TYPE_SW][CHAN_SUB_TYPE_SW_CTRL] = trs_chan_ops_hw_release_mbox_send,
    [CHAN_TYPE_MAINT][CHAN_SUB_TYPE_MAINT_HB] = trs_chan_ops_maint_release_mbox_send,
    [CHAN_TYPE_MAINT][CHAN_SUB_TYPE_MAINT_DBG] = trs_chan_ops_maint_release_mbox_send,
    [CHAN_TYPE_TASK_SCHED][CHAN_SUB_TYPE_TASK_SCHED_ASYNC_CB] = trs_chan_ops_task_sched_release_mbox_send,
};

int trs_chan_mbox_send(struct trs_id_inst *inst, struct trs_chan_info *chan_info, struct trs_chan_adapt_info *adapt)
{
    u32 sub_type = chan_info->types.sub_type;
    u32 type = chan_info->types.type;
    chan_mbox_func mbox_func;
    int ret = 0;

    if (chan_info->op != 0) {
        mbox_func = chan_create_mbox_func[type][sub_type];
    } else {
        mbox_func = chan_release_mbox_func[type][sub_type];
    }

    if (mbox_func != NULL) {
        ret = mbox_func(inst, chan_info, adapt);
    }

    return ret;
}
EXPORT_SYMBOL(trs_chan_mbox_send);
