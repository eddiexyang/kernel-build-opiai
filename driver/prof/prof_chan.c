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
* Create: 2022-7-26
*/
#ifndef PROF_UNIT_TEST
#include "drv_type.h"
#include "prof_def.h"
#include "prof_ts.h"
#include "prof_drv_dev.h"

#include "soc_res.h"
#include "trs_chan.h"
#include "trs_pub_def.h"
#include "trs_mailbox_def.h"
#include "prof_chan.h"

#define PROF_SQCQ_DEPTH 1024

#define PROF_MAX_TS_NUM 2

enum prof_chan_type {
    CHAN_0 = 0,
    CHAN_1,
    CHAN_2,
};

int chan_0_id[PROF_DEVICE_NUM_VALUE][PROF_MAX_TS_NUM];
int chan_1_id[PROF_DEVICE_NUM_VALUE][PROF_MAX_TS_NUM];
int chan_2_id[PROF_DEVICE_NUM_VALUE][PROF_MAX_TS_NUM];

bool prof_cqe_is_valid(void *cqe, u32 round)
{
    struct devdrv_functional_cq_report *report = (struct devdrv_functional_cq_report *)cqe;

    return (report->phase == ((round + 1) & 0x1));
}

static void prof_get_sq_head_in_cqe(void *cqe, u32 *sq_head)
{
    struct devdrv_functional_cq_report *report = (struct devdrv_functional_cq_report *)cqe;

    *sq_head = report->sq_head;
}

static int prof_cq_0_recv(struct trs_id_inst *inst, u32 cqid, void *cqe)
{
    u8 *cq_slot = (u8 *)cqe + PROF_CQ0_OFFSET;

    prof_sq_report_profile(inst->devid, inst->tsid, cq_slot, NULL);
    return CQ_RECV_FINISH;
}

static int prof_cq_1_recv(struct trs_id_inst *inst, u32 cqid, void *cqe)
{
    u8 *cq_slot = (u8 *)cqe + PROF_CQ1_OFFSET;

    prof_cq_callback_profile(inst->devid, inst->tsid, cq_slot, NULL);
    return CQ_RECV_FINISH;
}

static void prof_set_chan_create_para(struct trs_chan_para *para, enum prof_chan_type type)
{
    para->flag = (0x1 << CHAN_FLAG_ALLOC_CQ_BIT);
    para->types.type = CHAN_TYPE_MAINT;
    para->types.sub_type = CHAN_SUB_TYPE_MAINT_PROF;
    para->cq_para.cq_depth = PROF_SQCQ_DEPTH;
    para->ops.cqe_is_valid = prof_cqe_is_valid;
    para->ops.get_sq_head_in_cqe = prof_get_sq_head_in_cqe;

    if (type == CHAN_0) {
        para->flag |= (0x1 << CHAN_FLAG_ALLOC_SQ_BIT);
        para->flag |= (0x1 << CHAN_FLAG_AUTO_UPDATE_SQ_HEAD_BIT);
        para->sq_para.sq_depth = PROF_SQCQ_DEPTH;
        para->sq_para.sqe_size = PROF_SQ_BUF_LEN;
        para->cq_para.cqe_size = PROF_SQ_BUF_LEN;
        para->ops.cq_recv = prof_cq_0_recv;
        para->ops.abnormal_proc = NULL;
    } else if (type == CHAN_1) {
        para->cq_para.cqe_size = PROF_CQ_BUF_LEN;
        para->ops.cq_recv = prof_cq_1_recv;
        para->ops.abnormal_proc = NULL;
    } else {
        para->flag |= 0x1 << CHAN_FLAG_RECV_BLOCK_BIT;
        para->cq_para.cqe_size = PROF_CQ_BUF_LEN;
        para->ops.cq_recv = NULL;
        para->ops.abnormal_proc = NULL;
    }
}

static int proc_send_sqcq_create_info_to_ts(struct trs_id_inst *id_inst, struct trs_chan_sq_info *sq_info,
    struct trs_chan_cq_info *cq_0_info, struct trs_chan_cq_info *cq_1_info, struct trs_chan_cq_info *cq_2_info)
{
    struct trs_maint_sqcq_mbox mbox_data = {0};
    struct res_inst_info res_inst;
    u32 hwirq;
    int ret;

    trs_mbox_init_header(&mbox_data.header, PROFILE_CQSQ_CREATE);

    soc_resmng_inst_pack(&res_inst, id_inst->devid, TS_SUBSYS, id_inst->tsid);
    ret = soc_resmng_get_hwirq(&res_inst, (u32)TS_FUNC_CQ_IRQ, (u32)cq_0_info->irq, &hwirq);
    if (ret != 0) {
        prof_err("Get hw irq failed. (devid=%u; tsid=%u; irq=%d)\n", id_inst->devid, id_inst->tsid, cq_0_info->irq);
        return ret;
    }

    mbox_data.cq_irq = (u16)hwirq;

    mbox_data.sq_index = sq_info->sqid;
    mbox_data.sq_addr = sq_info->sq_phy_addr;

    mbox_data.cq0_index = cq_0_info->cqid;
    mbox_data.cq1_index = cq_1_info->cqid;
    mbox_data.cq2_index = cq_2_info->cqid;

    mbox_data.cq0_addr = cq_0_info->cq_phy_addr;
    mbox_data.cq1_addr = cq_1_info->cq_phy_addr;
    mbox_data.cq2_addr = cq_2_info->cq_phy_addr;

    ret = trs_mbox_send(id_inst, 0, &mbox_data, sizeof(struct trs_maint_sqcq_mbox), -1);
    if ((ret != 0) || (mbox_data.header.result != 0)) {
        prof_err("Failed to send create sqcq info to ts. (devid=%u; tsid=%u; ret=%d)\n",
            id_inst->devid, id_inst->tsid, ret);
    }

    return ret;
}

static void proc_send_sqcq_release_info_to_ts(struct trs_id_inst *id_inst, struct prof_cqsq_info *cqsq_info)
{
    struct trs_maint_sqcq_mbox mbox_data = {0};
    int ret;

    trs_mbox_init_header(&mbox_data.header, PROFILE_CQSQ_RELEASE);

    mbox_data.sq_index = cqsq_info->sq_0_index;
    mbox_data.cq0_index = cqsq_info->cq_0_index;
    mbox_data.cq1_index = cqsq_info->cq_1_index;
    mbox_data.cq2_index = cqsq_info->cq_2_index;

    ret = trs_mbox_send(id_inst, 0, &mbox_data, sizeof(struct trs_maint_sqcq_mbox), -1);
    if ((ret != 0) || (mbox_data.header.result != 0)) {
        prof_err("Failed to send create sqcq info to ts. (devid=%u; tsid=%u; ret=%d)\n",
            id_inst->devid, id_inst->tsid, ret);
    }
}

static void prof_set_cqsq_info_outpara(struct trs_chan_sq_info *sq_info, struct trs_chan_cq_info *cq_0_info,
    struct trs_chan_cq_info *cq_1_info, struct trs_chan_cq_info *cq_2_info, struct prof_cqsq_info *cqsq_info)
{
    cqsq_info->sq_0_index = sq_info->sqid;
    cqsq_info->sq_0_addr  = sq_info->sq_phy_addr;
    cqsq_info->cq_0_index = cq_0_info->cqid;
    cqsq_info->cq_1_index = cq_1_info->cqid;
    cqsq_info->cq_2_index = cq_2_info->cqid;
    cqsq_info->cq_0_addr  = cq_0_info->cq_phy_addr;
    cqsq_info->cq_1_addr  = cq_1_info->cq_phy_addr;
    cqsq_info->cq_2_addr  = cq_2_info->cq_phy_addr;
}

int prof_sqcq_alloc(u32 devid, u32 tsid, struct prof_cqsq_info *cqsq_info)
{
    struct trs_id_inst id_inst = {.devid = devid, .tsid = tsid};
    struct trs_chan_cq_info cq_0_info, cq_1_info, cq_2_info;
    struct trs_chan_sq_info sq_info;
    struct trs_chan_para para = {0};
    int chan_id_0, chan_id_1, chan_id_2;
    int ret;

    prof_set_chan_create_para(&para, CHAN_0);
    ret = hal_kernel_trs_chan_create(&id_inst, &para, &chan_id_0);
    if (ret != 0) {
        prof_err("Create chan 0 fail. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    prof_set_chan_create_para(&para, CHAN_1);
    ret = hal_kernel_trs_chan_create(&id_inst, &para, &chan_id_1);
    if (ret != 0) {
        prof_err("Create chan 1 fail. (devid=%u; tsid=%u)\n", devid, tsid);
        goto create_chan_1_fail;
    }

    prof_set_chan_create_para(&para, CHAN_2);
    ret = hal_kernel_trs_chan_create(&id_inst, &para, &chan_id_2);
    if (ret != 0) {
        prof_err("Create chan 2 fail. (devid=%u; tsid=%u)\n", devid, tsid);
        goto create_chan_2_fail;
    }

    (void)trs_chan_get_sq_info(&id_inst, chan_id_0, &sq_info);
    (void)trs_chan_get_cq_info(&id_inst, chan_id_0, &cq_0_info);
    (void)trs_chan_get_cq_info(&id_inst, chan_id_1, &cq_1_info);
    (void)trs_chan_get_cq_info(&id_inst, chan_id_2, &cq_2_info);

    ret = proc_send_sqcq_create_info_to_ts(&id_inst, &sq_info, &cq_0_info, &cq_1_info, &cq_2_info);
    if (ret != 0) {
        goto send_fail;
    }

    chan_0_id[devid][tsid] = chan_id_0;
    chan_1_id[devid][tsid] = chan_id_1;
    chan_2_id[devid][tsid] = chan_id_2;
    prof_set_cqsq_info_outpara(&sq_info, &cq_0_info, &cq_1_info, &cq_2_info, cqsq_info);
    return 0;

send_fail:
    hal_kernel_trs_chan_destroy(&id_inst, chan_id_2);
create_chan_2_fail:
    hal_kernel_trs_chan_destroy(&id_inst, chan_id_1);
create_chan_1_fail:
    hal_kernel_trs_chan_destroy(&id_inst, chan_id_0);
    return ret;
}

void prof_sqcq_free(u32 devid, u32 tsid, struct prof_cqsq_info *cqsq_info)
{
    struct trs_id_inst id_inst = {.devid = devid, .tsid = tsid};

    proc_send_sqcq_release_info_to_ts(&id_inst, cqsq_info);

    hal_kernel_trs_chan_destroy(&id_inst, chan_0_id[devid][tsid]);
    hal_kernel_trs_chan_destroy(&id_inst, chan_1_id[devid][tsid]);
    hal_kernel_trs_chan_destroy(&id_inst, chan_2_id[devid][tsid]);
}

int prof_sqcq_send(u32 devid, u32 tsid, u8 *sqe)
{
    struct trs_id_inst id_inst = {.devid = devid, .tsid = tsid};
    struct trs_chan_send_para para;

    para.sqe = sqe;
    para.sqe_num = 1;
    para.timeout = -1;
    return hal_kernel_trs_chan_send(&id_inst, chan_0_id[devid][tsid], &para);
}

int prof_sqcq_recv(u32 devid, u32 tsid, u8 *cqe, int timeout)
{
    struct trs_id_inst id_inst = {.devid = devid, .tsid = tsid};
    struct trs_chan_recv_para para;

    para.cqe = cqe;
    para.cqe_num = 1;
    para.timeout = timeout;
    return hal_kernel_trs_chan_recv(&id_inst, chan_0_id[devid][tsid], &para);
}
#else
void prof_chan_stub_test(void)
{
}
#endif
