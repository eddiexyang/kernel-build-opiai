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
#ifndef TSDRV_UT
#include "devdrv_functional_cqsq.h"
#include "tsdrv_heartbeat.h"
#include "trs_chan.h"

#define HB_SQ_DEPTH 1024
#define HB_CQ_DEPTH 1024
#define HB_SQE_SIZE 128
#define HB_CQE_SIZE 128

#define HB_MAX_DEV_NUM 64
#define HB_MAX_TS_NUM 2

#define HB_CQ_OFFSET 8

static bool hb_cqe_is_valid(void *cqe, u32 round)
{
    struct devdrv_functional_cq_report *report = (struct devdrv_functional_cq_report *)cqe;

    return (report->phase == ((round + 1) & 0x1));
}

static void hb_get_sq_head_in_cqe(void *cqe, u32 *sq_head)
{
    struct devdrv_functional_cq_report *report = (struct devdrv_functional_cq_report *)cqe;

    *sq_head = report->sq_head;
}

static int hb_cq_recv(struct trs_id_inst *inst, u32 cqid, void *cqe)
{
    u8 *cq_slot = (u8 *)cqe + HB_CQ_OFFSET;

    tsdrv_hb_cq_callback(inst->devid, inst->tsid, cq_slot, NULL);
    return CQ_RECV_FINISH;
}

void hb_set_chan_create_para(struct trs_chan_para *para)
{
    para->flag = (0x1 << CHAN_FLAG_ALLOC_SQ_BIT) | (0x1 << CHAN_FLAG_ALLOC_CQ_BIT) |
        (0x1 << CHAN_FLAG_NOTICE_TS_BIT) | (0x1 << CHAN_FLAG_AUTO_UPDATE_SQ_HEAD_BIT);
    para->types.type = CHAN_TYPE_MAINT;
    para->types.sub_type = CHAN_SUB_TYPE_MAINT_HB;
    para->sq_para.sq_depth = HB_SQ_DEPTH;
    para->sq_para.sqe_size = HB_SQE_SIZE;
    para->cq_para.cq_depth = HB_CQ_DEPTH;
    para->cq_para.cqe_size = HB_CQE_SIZE;
    para->ops.cqe_is_valid = hb_cqe_is_valid;
    para->ops.get_sq_head_in_cqe = hb_get_sq_head_in_cqe;
    para->ops.cq_recv = hb_cq_recv;
    para->ops.abnormal_proc = NULL;
}

int hb_sqcq_alloc(u32 devid, u32 tsid, int *chan_id)
{
    struct trs_id_inst id_inst = {.devid = devid, .tsid = tsid};
    struct trs_chan_sq_info sq_info;
    struct trs_chan_cq_info cq_info;
    struct trs_chan_para para = {0};
    int tmp_chan_id;
    int ret;

    hb_set_chan_create_para(&para);
    ret = hal_kernel_trs_chan_create(&id_inst, &para, &tmp_chan_id);
    if (ret != 0) {
        return ret;
    }

    (void)trs_chan_get_sq_info(&id_inst, tmp_chan_id, &sq_info);
    (void)trs_chan_get_cq_info(&id_inst, tmp_chan_id, &cq_info);

    *chan_id = tmp_chan_id;
    return 0;
}

void hb_sqcq_free(u32 devid, u32 tsid, int chan_id)
{
    struct trs_id_inst id_inst = {.devid = devid, .tsid = tsid};

    hal_kernel_trs_chan_destroy(&id_inst, chan_id);
}

int hb_sqcq_send(u32 devid, u32 tsid, int chan_id, u8 *sqe)
{
    struct trs_id_inst id_inst = {.devid = devid, .tsid = tsid};
    struct trs_chan_send_para para;

    para.sqe = sqe;
    para.sqe_num = 1;
    para.timeout = -1;
    return hal_kernel_trs_chan_send(&id_inst, chan_id, &para);
}
#else
void hb_sqcq_stub_test(void)
{
}
#endif
