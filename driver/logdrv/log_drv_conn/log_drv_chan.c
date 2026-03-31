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
 * Create: 2022-08-13
*/
#ifndef  LOG_UT
#include "drv_type.h"
#include "log_drv_dev.h"

#include "soc_res.h"
#include "trs_chan.h"
#include "trs_pub_def.h"
#include "trs_mailbox_def.h"

#define LOG_SQCQ_DEPTH 1024

#define LOG_CQ0_OFFSET 8
#define LOG_CQ1_OFFSET 4

#define LOG_MAX_TS_NUM 2

struct devdrv_functional_cq_report {
    u8 phase;
    u8 reserved[3];
    u16 sq_index;
    u16 sq_head;
};

enum log_chan_type {
    CHAN_0 = 0,
    CHAN_1
};

int chan_0_id[LOG_DEVICE_ID_MAX][LOG_MAX_TS_NUM];
int chan_1_id[LOG_DEVICE_ID_MAX][LOG_MAX_TS_NUM];

static bool log_cqe_is_valid(void *cqe, u32 round)
{
    struct devdrv_functional_cq_report *report = (struct devdrv_functional_cq_report *)cqe;

    return (report->phase == ((round + 1) & 0x1));
}

static void log_get_sq_head_in_cqe(void *cqe, u32 *sq_head)
{
    struct devdrv_functional_cq_report *report = (struct devdrv_functional_cq_report *)cqe;

    *sq_head = report->sq_head;
}

void log_cq0_report(u32 device_id, u32 tsid, const u8 *cq_buf, const u8 *sq_buf);
void log_cq1_callback(u32 device_id, u32 tsid, const u8 *cq_buf, u8 *sq_buf);
static int log_cq_0_recv(struct trs_id_inst *inst, u32 cqid, void *cqe)
{
    u8 *cq_slot = (u8 *)cqe + LOG_CQ0_OFFSET;

    log_cq0_report(inst->devid, inst->tsid, cq_slot, NULL);
    return CQ_RECV_FINISH;
}

static int log_cq_1_recv(struct trs_id_inst *inst, u32 cqid, void *cqe)
{
    u8 *cq_slot = (u8 *)cqe + LOG_CQ1_OFFSET;

    log_cq1_callback(inst->devid, inst->tsid, cq_slot, NULL);
    return CQ_RECV_FINISH;
}

static void log_set_chan_create_para(struct trs_chan_para *para, enum log_chan_type type)
{
    para->flag = (0x1 << CHAN_FLAG_ALLOC_CQ_BIT) | (0x1 << CHAN_FLAG_RECV_BLOCK_BIT);
    para->types.type = CHAN_TYPE_MAINT;
    para->types.sub_type = CHAN_SUB_TYPE_MAINT_LOG;
    para->cq_para.cq_depth = LOG_SQCQ_DEPTH;
    para->ops.cqe_is_valid = log_cqe_is_valid;

    if (type == CHAN_0) {
        para->flag |= (0x1 << CHAN_FLAG_ALLOC_SQ_BIT);
        para->flag |= (0x1 << CHAN_FLAG_AUTO_UPDATE_SQ_HEAD_BIT);
        para->sq_para.sq_depth = LOG_SQCQ_DEPTH;
        para->sq_para.sqe_size = LOG_SQ_BUF_LEN;
        para->cq_para.cqe_size = LOG_SQ_BUF_LEN;
        para->ops.cq_recv = log_cq_0_recv;
        para->ops.abnormal_proc = NULL;
        para->ops.get_sq_head_in_cqe = log_get_sq_head_in_cqe;
    } else {
        para->cq_para.cqe_size = LOG_CQ_BUF_LEN;
        para->ops.cq_recv = log_cq_1_recv;
        para->ops.abnormal_proc = NULL;
    }
}

static int log_send_sqcq_create_info_to_ts(struct trs_id_inst *id_inst, struct trs_chan_sq_info *sq_info,
    struct trs_chan_cq_info *cq_0_info, struct trs_chan_cq_info *cq_1_info)
{
    struct trs_maint_sqcq_mbox mbox_data = {0};
    struct res_inst_info res_inst;
    u32 hwirq;
    int ret;

    trs_mbox_init_header(&mbox_data.header, LOG_CQSQ_CREATE);

    soc_resmng_inst_pack(&res_inst, id_inst->devid, TS_SUBSYS, id_inst->tsid);
    ret = soc_resmng_get_hwirq(&res_inst, (u32)TS_FUNC_CQ_IRQ, (u32)cq_0_info->irq, &hwirq);
    if (ret != 0) {
        slog_drv_err("Get hw irq failed. (devid=%u; tsid=%u; irq=%d)\n", id_inst->devid, id_inst->tsid, cq_0_info->irq);
        return ret;
    }

    mbox_data.cq_irq = (u16)hwirq;

    mbox_data.sq_addr = sq_info->sq_phy_addr;
    mbox_data.cq0_addr = cq_0_info->cq_phy_addr;
    mbox_data.cq1_addr = cq_1_info->cq_phy_addr;

    mbox_data.sq_index = sq_info->sqid;
    mbox_data.cq0_index = cq_0_info->cqid;
    mbox_data.cq1_index = cq_1_info->cqid;

    mbox_data.plat_type = 0;

    ret = trs_mbox_send(id_inst, 0, &mbox_data, sizeof(struct trs_maint_sqcq_mbox), -1);
    if ((ret != 0) || (mbox_data.header.result != 0)) {
        slog_drv_err("Failed to send create sqcq info to ts. (devid=%u; tsid=%u; ret=%d)\n",
            id_inst->devid, id_inst->tsid, ret);
    }

    return ret;
}

static void log_send_sqcq_release_info_to_ts(struct trs_id_inst *id_inst, u32 sq_id, u32 cq_0_id, u32 cq_1_id)
{
    struct trs_maint_sqcq_mbox mbox_data = {0};
    int ret;

    trs_mbox_init_header(&mbox_data.header, LOG_CQSQ_RELEASE);

    mbox_data.sq_index = sq_id;
    mbox_data.cq0_index = cq_0_id;
    mbox_data.cq1_index = cq_1_id;

    ret = trs_mbox_send(id_inst, 0, &mbox_data, sizeof(struct trs_maint_sqcq_mbox), -1);
    if ((ret != 0) || (mbox_data.header.result != 0)) {
        slog_drv_err("Send mailbox fail. (devid=%u; tsid=%u)\n", id_inst->devid, id_inst->tsid);
    }
}

int log_sqcq_alloc(u32 devid, u32 tsid, u32 *sq_id, u32 *cq_0_id, u32 *cq_1_id)
{
    struct trs_id_inst id_inst = {.devid = devid, .tsid = tsid};
    struct trs_chan_cq_info cq_0_info, cq_1_info;
    struct trs_chan_sq_info sq_info;
    struct trs_chan_para para = {0};
    int chan_id_0, chan_id_1;
    int ret;

    log_set_chan_create_para(&para, CHAN_0);
    ret = hal_kernel_trs_chan_create(&id_inst, &para, &chan_id_0);
    if (ret != 0) {
        slog_drv_err("Create chan 0 fail. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    log_set_chan_create_para(&para, CHAN_1);
    ret = hal_kernel_trs_chan_create(&id_inst, &para, &chan_id_1);
    if (ret != 0) {
        slog_drv_err("Create chan 0 fail. (devid=%u; tsid=%u)\n", devid, tsid);
        goto create_chan_1_fail;
    }

    (void)trs_chan_get_sq_info(&id_inst, chan_id_0, &sq_info);
    (void)trs_chan_get_cq_info(&id_inst, chan_id_0, &cq_0_info);
    (void)trs_chan_get_cq_info(&id_inst, chan_id_1, &cq_1_info);

    ret = log_send_sqcq_create_info_to_ts(&id_inst, &sq_info, &cq_0_info, &cq_1_info);
    if (ret != 0) {
        goto send_fail;
    }

    *sq_id = sq_info.sqid;
    *cq_0_id = cq_0_info.cqid;
    *cq_1_id = cq_1_info.cqid;
    chan_0_id[devid][tsid] = chan_id_0;
    chan_1_id[devid][tsid] = chan_id_1;
    return 0;

send_fail:
    hal_kernel_trs_chan_destroy(&id_inst, chan_id_1);
create_chan_1_fail:
    hal_kernel_trs_chan_destroy(&id_inst, chan_id_0);
    return ret;
}

void log_sqcq_free(u32 devid, u32 tsid, u32 sq_id, u32 cq_0_id, u32 cq_1_id)
{
    struct trs_id_inst id_inst = {.devid = devid, .tsid = tsid};

    log_send_sqcq_release_info_to_ts(&id_inst, sq_id, cq_0_id, cq_1_id);

    hal_kernel_trs_chan_destroy(&id_inst, chan_0_id[devid][tsid]);
    hal_kernel_trs_chan_destroy(&id_inst, chan_1_id[devid][tsid]);
}

int log_sqcq_send(u32 devid, u32 tsid, u8 *sqe)
{
    struct trs_id_inst id_inst = {.devid = devid, .tsid = tsid};
    struct trs_chan_send_para para;

    para.sqe = sqe;
    para.sqe_num = 1;
    para.timeout = -1;
    return hal_kernel_trs_chan_send(&id_inst, chan_0_id[devid][tsid], &para);
}
#else
void log_drv_chan_stub_test(void)
{
}
#endif
