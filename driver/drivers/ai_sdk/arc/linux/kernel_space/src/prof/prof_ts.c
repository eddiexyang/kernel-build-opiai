/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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
* Create: 2019-10-15
*/

#ifndef PROF_UNIT_TEST
#ifdef AOS_LLVM_BUILD
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/atomic.h>
#include <linux/hrtimer.h>
#include <asm-generic/getorder.h>
#endif

#include <linux/types.h>

#include "prof_drv_dev.h"
#include "prof_def.h"
#include "prof_ts.h"
#include "prof_chan.h"
#include "tsdrv_hwinfo.h"
#include "devdrv_interface.h"
#include "dbl/chip_config.h"
#ifdef CFG_FEATURE_PROF_HWTS_THROUGH
#include "prof_hwts_init.h"
#endif
#define PROF_TS_ACK_OK                 0

prof_tscpu_buf_info_t prof_tscpu_common_buf_info[PROF_DEVICE_NUM_VALUE][PROF_CHANNEL_NUM] = {0};

prof_tscpu_buf_info_t *prof_tscpu_get_common_buf_info(u32 device_id, u32 channel_id)
{
    return &(prof_tscpu_common_buf_info[device_id][channel_id]);
}

#ifdef CFG_FEATURE_PROF_HWTS_THROUGH
STATIC bool prof_channel_is_hwts_through(u32 channel_id)
{
    if (channel_id == CHANNEL_AICORE || channel_id == CHANNEL_HWTS_LOG) {
        return true;
    }
    return false;
}
#endif

STATIC void prof_sq_mes_start_stop_init(struct prof_sub_channel_info *sub_channel_info, u32 cmd,
    struct prof_sq_scheduler *sq_send_cmd)
{
    prof_tscpu_buf_info_t *common_buf_info =
        prof_tscpu_get_common_buf_info(sub_channel_info->device_id, sub_channel_info->channel_id);

    sq_send_cmd->com_buf_phy_addr = common_buf_info->phy_addr;
    sq_send_cmd->com_buf_len= common_buf_info->len;
    sq_send_cmd->buf_num = prof_sq_init_get_buf_num(sub_channel_info->channel_id, sub_channel_info->vfid);
    sq_send_cmd->buf_phy_addr = prof_sq_init_get_buf_pa(sub_channel_info);
    sq_send_cmd->buf_len = prof_sq_init_get_buf_len(sub_channel_info);
    sq_send_cmd->collect_target_pid = sub_channel_info->proc_ctx->collect_target_pid;
}

STATIC void prof_sq_messages_init(struct prof_sub_channel_info *sub_channel_info, u32 cmd,
    struct prof_sq_scheduler *sq_send_cmd)
{
    u32 vfid;
#ifdef CFG_FEATURE_SRIOV
    u32 phy_device_id;

    (void)vmngd_get_pfvf_id_by_devid(sub_channel_info->device_id, &phy_device_id, &vfid);
#else
    vfid = sub_channel_info->vfid;
#endif

    sq_send_cmd->cmd_verify = sub_channel_info->ts_channel.cmd_verify;
    sq_send_cmd->channel_id = sub_channel_info->channel_id;
    sq_send_cmd->sub_channel_id = sub_channel_info->sub_channel_id;
    sq_send_cmd->vfid = vfid;
    sq_send_cmd->channel_cmd = cmd;

    if (cmd == TS_START || cmd == TS_STOP) {
        prof_sq_mes_start_stop_init(sub_channel_info, cmd, sq_send_cmd);
    }
}

STATIC int prof_start_sq_messages_init(struct prof_sub_channel_info *sub_channel_info, const char *ts_data,
    u32 ts_data_size, struct prof_sq_scheduler *sq_send_cmd)
{
    int ret;

    prof_sq_messages_init(sub_channel_info, TS_START, sq_send_cmd);

    sq_send_cmd->data_size = ts_data_size;
    if (ts_data_size != 0) {
        ret = memcpy_s((unsigned char *)sq_send_cmd->ts_cpu_data, PROF_TS_CPU_DATA, ts_data, ts_data_size);
        if (ret != EOK) {
            prof_err("Failed to invoke function [memcpy_s]. (device_id=%u; ret=%d)\n",
                sub_channel_info->device_id, ret);
            return ret;
        }
    }

    prof_debug("Profile SQ0 setting was success.(cmd_verify=%u; chann_id=%u; sub_channel_id=%u; chann_cmd=%u;"
        " data_size=%u; buff_num=%u; vfid=%u; com_buf_len=%u; channel_buf_len=%u; collect_target_pid=%u)\n",
        sq_send_cmd->cmd_verify, sq_send_cmd->channel_id, sq_send_cmd->sub_channel_id,
        sq_send_cmd->channel_cmd, sq_send_cmd->data_size, sq_send_cmd->buf_num,
        sq_send_cmd->vfid, sq_send_cmd->com_buf_len, sq_send_cmd->buf_len,
        sq_send_cmd->collect_target_pid);

    return PROF_OK;
}

STATIC int prof_check_ts_cq(const struct prof_cq_scheduler *cq_scheduler)
{
    u32 device_id = cq_scheduler->device_id;
    u32 vfid = cq_scheduler->vfid;
    u32 channel_id = cq_scheduler->channel_id;
    u32 sub_channel_id = cq_scheduler->sub_channel_id;
    int ret;

    ret = prof_check_device_state(device_id);
    if (ret != PROF_OK) {
        prof_err("Failed to check the state of device CQ. (device_id=%u; ret=%d)\n", device_id, ret);
        return PROF_ERROR;
    }

    if ((vfid >= PROF_VFID_NUM_MAX) || (prof_get_channel_type(channel_id) != PROF_TS_TYPE)) {
        prof_err("The variable [vfid] was invalid or the channel type was not PROF_TS_TYPE."
            "(dev id=%u; vfid=%u; chan id=%u; sub chan id=%u;)\n", device_id, vfid, channel_id, sub_channel_id);
        return PROF_ERROR;
    }

    if (sub_channel_id >= PROF_SUB_CHANNEL_NUM_MAX) {
        prof_err("The [sub_channel_id] was invalid. (sub_channel_id=%u)\n", sub_channel_id);
        return PROF_ERROR;
    }

    return PROF_OK;
}

STATIC int prof_cq_info_shift(struct prof_cq_scheduler *cq_scheduler)
{
#ifdef CFG_FEATURE_SRIOV
    u32 tmp_device_id;
    int ret;

    ret = vmngd_get_devid_by_pfvf_id(cq_scheduler->device_id, cq_scheduler->vfid, &tmp_device_id);
    if (ret != PROF_OK) {
        prof_err("Failed to get true_dev_id. (device_id=%u; vfid=%u; ret=%d)\n",
            cq_scheduler->device_id, cq_scheduler->vfid, ret);
        return ret;
    }
    cq_scheduler->device_id = tmp_device_id;
    cq_scheduler->vfid = 0;
#endif
    return PROF_OK;
}

STATIC int prof_cq_info_op(struct prof_cq_scheduler *cq_scheduler)
{
    int ret;

    if (cq_scheduler == NULL) {
        prof_err("Parameter [cq_scheduler] was [NULL].\n");
        return PROF_ERROR;
    }

    PROF_CQ_INFO_SHOW(cq_scheduler);
    ret = prof_cq_info_shift(cq_scheduler);
    if (ret != PROF_OK) {
        prof_err("Failed to shift cq info. (ret=%d)\n", ret);
        return ret;
    }
    PROF_CQ_INFO_SHOW(cq_scheduler);

    ret = prof_check_ts_cq(cq_scheduler);
    if (ret != PROF_OK) {
        prof_err("Failed to check the CQ of TS. (ret=%d)\n", ret);
        return ret;
    }

    return ret;
}

void prof_sq_report_profile(u32 device_id_in, u32 tsid,
    const unsigned char *cq_buf, unsigned char *sq_buf)
{
    u32 device_id;
    u32 vfid;
    u32 channel_id;
    u32 sub_channel_id;
    struct prof_cq_scheduler *cq_scheduler = NULL;
    struct prof_sub_channel_info *sub_channel_info = NULL;
    int ret;

    cq_scheduler = (struct prof_cq_scheduler *)cq_buf;
    ret = prof_cq_info_op(cq_scheduler);
    if (ret != PROF_OK) {
        prof_err("Failed to prof_cq_info_op. (ret=%d)\n", ret);
        return;
    }

    device_id = cq_scheduler->device_id;
    if (prof_device_info_get(device_id) == NULL) {
        prof_err("Device not valid. (device_id=%u)\n", device_id);
        return;
    }

    vfid = cq_scheduler->vfid;
    channel_id = cq_scheduler->channel_id;
    sub_channel_id = cq_scheduler->sub_channel_id;
    sub_channel_info = prof_get_sub_channel_info_from_index(device_id, vfid, channel_id, sub_channel_id);
    if (sub_channel_info == NULL) {
        prof_warn("The channel was disabled to report profile. (device_id=%u; vfid=%u; channel_id=%u;"
            " sub_channel_id=%u)\n", device_id, vfid, channel_id, sub_channel_id);
        return;
    }

    if (sub_channel_info->ts_channel.cmd_verify != cq_scheduler->cmd_verify) {
        prof_err("Failed to verify the channel command. (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u;"
            " channel_cmd=%u; channel_state=%u; channel.cmd_verify=%u; cmd_verify=%u)\n", device_id, vfid, channel_id,
            sub_channel_id, cq_scheduler->channel_cmd, sub_channel_info->channel_state,
            sub_channel_info->ts_channel.cmd_verify, cq_scheduler->cmd_verify);
        return;
    }

    if ((cq_scheduler->channel_cmd == TS_START) && (sub_channel_info->channel_state == (u32)PROF_CHANNEL_DISABLE)) {
        sub_channel_info->channel_state = (u32)PROF_CHANNEL_STARTING;
        sub_channel_info->ts_channel.ret_val = cq_scheduler->ret_val;
        wake_up(&sub_channel_info->channel_wq);
        return;
    }

    if ((cq_scheduler->channel_cmd == TS_STOP) && (sub_channel_info->channel_state == (u32)PROF_CHANNEL_STOP_WAIT_TS)) {
        sub_channel_info->channel_state = (u32)PROF_CHANNEL_ENABLE;
        sub_channel_info->ts_channel.ret_val = cq_scheduler->ret_val;
        wake_up(&sub_channel_info->channel_wq);
        return;
    }

    // do not use cq here, because ts modify it to 0.
    if ((cq_scheduler->channel_cmd == TS_SYNC_READ_PTR) || (cq_scheduler->channel_cmd == TS_SYNC_WRITE_PTR)) {
        if ((sub_channel_info->channel_state == (u32)PROF_CHANNEL_FLUSH) &&
            (cq_scheduler->channel_cmd == TS_SYNC_WRITE_PTR)) {
            sub_channel_info->channel_state = (u32)PROF_CHANNEL_ENABLE;
            wake_up(&sub_channel_info->channel_wq);
        }
        return;
    }
    prof_warn("Do not wake up nothing. (device_id=%u; vfid=%u; channel_id=%u; channel_cmd=%u; channel_state=%u).\n",
        device_id, vfid, channel_id, cq_scheduler->channel_cmd, sub_channel_info->channel_state);

    if (sub_channel_info->channel_state == (u32)PROF_CHANNEL_DISABLE) {
        prof_err("The channel was disabled. (device_id=%u; vfid=%u; channel_id=%u)\n", device_id, vfid, channel_id);
        return;
    }
}

void prof_cq_callback_profile(u32 device_id_in, u32 tsid,
    const unsigned char *cq_buf, unsigned char *sq_buf)
{
    struct prof_cq_scheduler *cq_scheduler = NULL;
    struct prof_sub_channel_info *sub_channel_info = NULL;
    u32 channel_id;
    u32 device_id;
    u32 sub_channel_id;
    u32 vfid;
    int ret;

    cq_scheduler = (struct prof_cq_scheduler *)cq_buf;
    ret = prof_cq_info_op(cq_scheduler);
    if (ret != PROF_OK) {
        prof_err("Failed to prof_cq_info_op. (ret=%d)\n", ret);
        return;
    }

    device_id = cq_scheduler->device_id;
    if (prof_device_info_get(device_id) == NULL) {
        prof_err("Device not valid. (device_id=%u)\n", device_id);
        return;
    }

    vfid = cq_scheduler->vfid;
    channel_id = cq_scheduler->channel_id;
    sub_channel_id = cq_scheduler->sub_channel_id;
    sub_channel_info = prof_get_sub_channel_info_from_index(device_id, vfid, channel_id, sub_channel_id);
    if (sub_channel_info == NULL) {
        return;
    }

    sub_channel_info->prof_dfx.prof_cq1_counts_from_ts++;
    if (sub_channel_info->channel_state == (u32)PROF_CHANNEL_DISABLE ||
        sub_channel_info->channel_state == (u32)PROF_CHANNEL_IDLE) {
        return;
    }

    (void)prof_wake_up_poll_wq(sub_channel_info);

    return;
}

/* AICPU channel is still used with TS, keep it for expansion */
void prof_cq_callback_aicpu(u32 device_id, u32 tsid,
    const unsigned char *cq_buf, unsigned char *sq_buf)
{
    return;
}

#ifndef CFG_TRS_REFACTOR_FEATURE
STATIC int prof_create_cqsq(u32 device_id, u32 tsid)
{
    struct prof_device_info *prof_device = prof_device_info_get(device_id);
    struct devdrv_mailbox_cqsq mailbox_cqsq = {0};
    struct prof_cqsq_info *cqsq_info = &prof_device->cqsq_info[tsid];
    int ret;

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    /* a55 can only access 40bit address, die1 or p1/2/3... use ADDR SLIDING WINDOW */
    cqsq_info->sq_0_addr &= 0xFFFFFFFFFF;
    cqsq_info->cq_0_addr &= 0xFFFFFFFFFF;
    cqsq_info->cq_1_addr &= 0xFFFFFFFFFF;
    cqsq_info->cq_2_addr &= 0xFFFFFFFFFF;
#endif
    /* only use cq2 for profile; log and debug don't use cq2 for driver and TS */
    mailbox_cqsq.cq_irq = (u16)tsdrv_get_dfx_cq_irq_vector(device_id, tsid);
    mailbox_cqsq.cmd_type = PROFILE_CQSQ_CREATE;
    mailbox_cqsq.sq_index = cqsq_info->sq_0_index;
    mailbox_cqsq.sq_addr = cqsq_info->sq_0_addr;
    mailbox_cqsq.cq0_index = cqsq_info->cq_0_index;
    mailbox_cqsq.cq0_addr = cqsq_info->cq_0_addr;
    mailbox_cqsq.cq1_index = cqsq_info->cq_1_index;
    mailbox_cqsq.cq1_addr = cqsq_info->cq_1_addr;
    mailbox_cqsq.cq2_index = cqsq_info->cq_2_index;
    mailbox_cqsq.cq2_addr = cqsq_info->cq_2_addr;
    ret = devdrv_mailbox_send_cqsq(device_id, tsid, &mailbox_cqsq);
    if (ret != 0) {
        prof_err("Failed to send CQ and SQ messages by mailbox. (device_id=%u; ret=%d)\n",
            device_id, ret);
    }
    return ret;
}

STATIC int prof_destroy_cqsq(u32 device_id, u32 tsid)
{
    struct prof_device_info *prof_device = prof_device_info_get(device_id);
    struct prof_cqsq_info *cqsq_info = &prof_device->cqsq_info[tsid];
    struct devdrv_mailbox_cqsq mailbox_cqsq = {0};
    int ret;

    mailbox_cqsq.cmd_type = PROFILE_CQSQ_RELEASE;
    mailbox_cqsq.sq_index = (u16)cqsq_info->sq_0_index;
    mailbox_cqsq.cq0_index = (u16)cqsq_info->cq_0_index;
    mailbox_cqsq.cq1_index = (u16)cqsq_info->cq_1_index;
    mailbox_cqsq.cq2_index = (u16)cqsq_info->cq_2_index;

    ret = devdrv_mailbox_send_cqsq(device_id, tsid, &mailbox_cqsq);
    if (ret != 0) {
        prof_err("Failed to send messages to CQ and SQ by mailbox. (device_id=%u; ret=%d)\n",
            device_id, ret);
    }
    return ret;
}
#endif

#ifdef CFG_TRS_REFACTOR_FEATURE
int prof_cqsq_init(u32 device_id, u32 tsid)
{
    struct prof_device_info *prof_device = prof_device_info_get(device_id);
    struct prof_cqsq_info *cqsq_info = &prof_device->cqsq_info[tsid];
    int ret;

    if (device_id >= PROF_DEVICE_NUM_VALUE) {
        prof_err("Failed to initialize the CQ and SQ. (device_id=%u)\n", device_id);
        return PROF_ERROR;
    }

    ret = prof_sqcq_alloc(device_id, tsid, cqsq_info);
    if (ret) {
        prof_err("Alloc sqcq fail. (device_id=%u; tsid=%u)\n", device_id, tsid);
        prof_device->device_state = DEV_UNUSED;
        return ret;
    }

    return 0;
}

void prof_cqsq_uninit(u32 device_id, u32 tsid)
{
    struct prof_device_info *prof_device = prof_device_info_get(device_id);
    struct prof_cqsq_info *cqsq_info = &prof_device->cqsq_info[tsid];

    prof_sqcq_free(device_id, tsid, cqsq_info);
}

#else
int prof_cqsq_init(u32 device_id, u32 tsid)
{
    struct prof_device_info *prof_device = prof_device_info_get(device_id);
    struct prof_cqsq_info *cqsq_info = &prof_device->cqsq_info[tsid];
    int ret;
    u32 sq_0_index;
    u32 cq_0_index;
    u32 cq_1_index;
    u32 cq_2_index;
    unsigned long long sq_0_addr = 0;
    unsigned long long cq_0_addr = 0;
    unsigned long long cq_1_addr = 0;
    unsigned long long cq_2_addr = 0;
#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (!prof_is_ts_valid()) {
        return PROF_OK;
    }
#endif
    if (device_id >= PROF_DEVICE_NUM_VALUE) {
        prof_err("Failed to initialize the CQ and SQ. (device_id=%u)\n", device_id);
        return PROF_ERROR;
    }

    ret = devdrv_create_functional_sq(device_id, tsid, PROF_SQ_BUF_LEN, &sq_0_index,
        (unsigned long long *)&sq_0_addr);
    if (ret != PROF_OK) {
        prof_err("Failed to make profile driver to send the SQ0. (device_id=%u; ret=%d)\n",
            device_id, ret);
        return ret;
    }

    /* cq0's length is 128 Byte */
    ret = devdrv_create_functional_cq(device_id, tsid, PROF_SQ_BUF_LEN, report_hs_cq,
        prof_sq_report_profile, &cq_0_index, (unsigned long long *)&cq_0_addr);
    if (ret != PROF_OK) {
        prof_err("Failed to make profile driver to create the CQ0. (device_id=%u; ret=%d)\n",
            device_id, ret);
        devdrv_destroy_functional_sq(device_id, tsid, sq_0_index);
        return ret;
    }

    ret = devdrv_create_functional_cq(device_id, tsid, PROF_CQ_BUF_LEN, callback_hs_cq,
        prof_cq_callback_profile, &cq_1_index, (unsigned long long *)&cq_1_addr);
    if (ret != PROF_OK) {
        prof_err("Failed to make profile driver to create the CQ1. (device_id=%u; ret=%d)\n",
            device_id, ret);
        devdrv_destroy_functional_sq(device_id, tsid, sq_0_index);
        devdrv_destroy_functional_cq(device_id, tsid, cq_0_index);
        return ret;
    }

    ret = devdrv_create_functional_cq(device_id, tsid, PROF_CQ_BUF_LEN, callback_cq,
        prof_cq_callback_aicpu, &cq_2_index, (unsigned long long *)&cq_2_addr);
    if (ret != PROF_OK) {
        prof_err("Failed to make profile driver to create the CQ2. (device_id=%u; ret=%d)\n",
            device_id, ret);
        devdrv_destroy_functional_sq(device_id, tsid, sq_0_index);
        devdrv_destroy_functional_cq(device_id, tsid, cq_0_index);
        devdrv_destroy_functional_cq(device_id, tsid, cq_1_index);
        return ret;
    }

    cqsq_info->sq_0_index = sq_0_index;
    cqsq_info->cq_0_index = cq_0_index;
    cqsq_info->cq_1_index = cq_1_index;
    cqsq_info->cq_2_index = cq_2_index;
    cqsq_info->sq_0_addr  = sq_0_addr;
    cqsq_info->cq_0_addr  = cq_0_addr;
    cqsq_info->cq_1_addr  = cq_1_addr;
    cqsq_info->cq_2_addr  = cq_2_addr;

    ret = prof_create_cqsq(device_id, tsid);
    if (ret != PROF_OK) {
        prof_err("Failed to create the CQ and SQ. (device_id=%u; ret=%d)\n",
            device_id, ret);
        devdrv_destroy_functional_sq(device_id, tsid, sq_0_index);
        devdrv_destroy_functional_cq(device_id, tsid, cq_0_index);
        devdrv_destroy_functional_cq(device_id, tsid, cq_1_index);
        devdrv_destroy_functional_cq(device_id, tsid, cq_2_index);
        cqsq_info->sq_0_index = 0;
        cqsq_info->cq_0_index = 0;
        cqsq_info->cq_1_index = 0;
        cqsq_info->cq_2_index = 0;
        cqsq_info->sq_0_addr  = 0;
        cqsq_info->cq_0_addr  = 0;
        cqsq_info->cq_1_addr  = 0;
        cqsq_info->cq_2_addr  = 0;
        return ret;
    }
    PROF_CQSQ_INFO_SHOW(tsid, cqsq_info);
    prof_info("Profile SQ and CQ initialization was success. (device_id=%u)\n", device_id);

    return ret;
}

void prof_cqsq_uninit(u32 device_id, u32 tsid)
{
    struct prof_device_info *prof_device = prof_device_info_get(device_id);
    struct prof_cqsq_info *cqsq_info = &prof_device->cqsq_info[tsid];

#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (!prof_is_ts_valid()) {
        return;
    }
#endif

    (void)prof_destroy_cqsq(device_id, tsid);
    devdrv_destroy_functional_sq(device_id, tsid, cqsq_info->sq_0_index);
    devdrv_destroy_functional_cq(device_id, tsid, cqsq_info->cq_0_index);
    devdrv_destroy_functional_cq(device_id, tsid, cqsq_info->cq_1_index);
    devdrv_destroy_functional_cq(device_id, tsid, cqsq_info->cq_2_index);

    return;
}
#endif

STATIC int prof_check_tscpu_start_para_valid(struct prof_ioctl_para *para)
{
    u32 device_id = para->device_id;
    u32 vfid = para->vfid;
    u32 channel_id = para->channel_id;
    int ret;

    ret = prof_check_device_state(device_id);
    if (ret != PROF_OK) {
        prof_err("Failed to check the state of device. (device_id=%u; ret=%d)\n", device_id, ret);
        return PROF_ERROR;
    }

    if (para->user_data_size > PROF_TS_CPU_DATA) {
        prof_err("Parameter [user_data_size] was invalid."
            " (device_id=%u; vfid=%u; channel_id=%u; ts_data_size=%u; PROF_TS_CPU_DATA=%d)\n",
            device_id, vfid, channel_id, para->user_data_size, PROF_TS_CPU_DATA);
        return PROF_ERROR;
    }

    return PROF_OK;
}

STATIC int prof_tscpu_sq_send(u32 device_id, u32 tsid, unsigned char *sq_send_cmd)
{
    struct prof_device_info *prof_device = prof_device_info_get(device_id);
    struct prof_cqsq_info *cqsq_info = NULL;
    int ret;
#ifdef CFG_FEATURE_SRIOV
    u32 phy_device_id, vfid;

    (void)vmngd_get_pfvf_id_by_devid(device_id, &phy_device_id, &vfid);
    device_id = phy_device_id;
#endif

    cqsq_info = &prof_device->cqsq_info[tsid];
    mutex_lock(&cqsq_info->sq_mutex);
#ifdef CFG_TRS_REFACTOR_FEATURE
    ret = prof_sqcq_send(device_id, tsid, sq_send_cmd);
#else
    ret = devdrv_functional_send_sq(device_id, tsid, cqsq_info->sq_0_index,
        sq_send_cmd, sizeof(struct prof_sq_scheduler));
#endif
    mutex_unlock(&cqsq_info->sq_mutex);

    return ret;
}

int prof_tscpu_send_sync_rw_ptr_cmd(struct prof_sub_channel_info *sub_channel_info, int refresh_type)
{
    u32 device_id = sub_channel_info->device_id;
    u32 vfid = sub_channel_info->vfid;
    u32 channel_id = sub_channel_info->channel_id;
    u32 tsid = sub_channel_info->ts_channel.tsid;
    struct prof_sq_scheduler sq_send_cmd = {0};
    int ret;

    ret = prof_check_device_state(device_id);
    if (ret != PROF_OK) {
        prof_err("Failed to check the state of device. (device_id=%u; ret=%d)\n", device_id, ret);
        return PROF_ERROR;
    }

    prof_sq_messages_init(sub_channel_info, refresh_type, &sq_send_cmd);

    ret = prof_tscpu_sq_send(device_id, tsid, (unsigned char *)&sq_send_cmd);
    if (ret != PROF_OK) {
        prof_err("Failed to send messages SQ messages."
            " (device_id=%u; vfid=%u; ret=%d; channel_id=%u; cmd_verify=%u)\n",
            device_id, vfid, ret, channel_id, sub_channel_info->ts_channel.cmd_verify);
    }

    return ret;
}

STATIC int prof_tscpu_sync_rw_ptr(struct prof_sub_channel_info *sub_channel_info, int refresh_type)
{
    int ret = PROF_OK;

#ifdef CFG_FEATURE_PROF_HWTS_THROUGH
    if (prof_channel_is_hwts_through(sub_channel_info->channel_id)) {
        return prof_refresh_rw_ptr(sub_channel_info, refresh_type);
    }
#endif

    if (!prof_channel_need_sync_rw_ptr(sub_channel_info)) {
        ret = prof_tscpu_send_sync_rw_ptr_cmd(sub_channel_info, refresh_type);
        if (ret != PROF_OK) {
            return ret;
        }
    }

    return ret;
}

int prof_tscpu_alloc_channel_memory(struct prof_sub_channel_info *sub_channel_info)
{
    u32 memory_type = prof_get_mem_type(sub_channel_info->channel_id);
    if (memory_type == MEM_RESERVE) {
            sub_channel_info->vir_addr = (unsigned char *)ioremap_wc(sub_channel_info->phy_addr,
                sub_channel_info->buf_len);
            if (sub_channel_info->vir_addr == NULL) {
                prof_err("Failed to invoke function [ioremap_wc]."
                    " (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u)\n",
                    sub_channel_info->device_id, sub_channel_info->vfid,
                    sub_channel_info->channel_id, sub_channel_info->sub_channel_id);
                return PROF_ERROR;
            }
    } else {
        sub_channel_info->vir_addr = prof_alloc_node_memory(sub_channel_info->device_id,
            sub_channel_info->buf_len, PROF__GFP_THISNODE | __GFP_ZERO | PROF_GFP_HIGHUSER_MOVABLE, TS_NODE_MEM_TYPE);
        if (sub_channel_info->vir_addr == NULL) {
            prof_err("Failed to invoke function [prof_tscpu_alloc_memory] to apply for memory."
                " (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u; buf_len=%u)\n",
                sub_channel_info->device_id, sub_channel_info->vfid,
                sub_channel_info->channel_id, sub_channel_info->sub_channel_id, sub_channel_info->buf_len);
            return PROF_ERROR;
        }
        sub_channel_info->phy_addr = virt_to_phys(sub_channel_info->vir_addr);
    }

    return PROF_OK;
}

void prof_tscpu_free_channel_memory(struct prof_sub_channel_info *sub_channel_info)
{
    u32 memory_type = prof_get_mem_type(sub_channel_info->channel_id);

    prof_platform_ts_buff_free_flag(&sub_channel_info->ts_channel.ts_buff_free_flag);
    if (sub_channel_info->ts_channel.ts_buff_free_flag == TS_BUFF_NOT_FREE) {
        prof_platform_ts_buff_not_free_op(sub_channel_info);
        return;
    }

    if (memory_type == MEM_RESERVE) {
        iounmap(sub_channel_info->vir_addr);
        sub_channel_info->vir_addr = NULL;
    } else {
        free_pages((unsigned long)sub_channel_info->vir_addr, (u32)get_order(sub_channel_info->buf_len));
        sub_channel_info->phy_addr = 0;
        sub_channel_info->vir_addr = NULL;
    }
}

STATIC void prof_ts_ack_error_analysis(int ret_val)
{
/* Profile TS_Ack definition */
#define PROF_TS_ACK_INVALID_CMDTYPE    1
#define PROF_TS_ACK_INVALID_CHANNEL    2
#define PROF_TS_ACK_INVALID_BUFF       3
#define PROF_TS_ACK_PARA_ERROR         4
#define PROF_TS_ACK_VIR_NOT_SUPPORT    5
#define PROF_TS_ACK_INVALID_SUBCHANNEL 7 /* Current Profile TS_Ack: no 6 */

    switch (ret_val) {
        case PROF_TS_ACK_INVALID_CMDTYPE:
            prof_err("Profile TS_ACK error. The cmd_type was invalid. (TS_ACK=%d)\n", ret_val);
            break;
        case PROF_TS_ACK_INVALID_CHANNEL:
            prof_err("Profile TS_ACK error. The channel_id was invalid. (TS_ACK=%d)\n", ret_val);
            break;
        case PROF_TS_ACK_INVALID_BUFF:
            prof_err("Profile TS_ACK error. The buffer_info was invalid. (TS_ACK=%d)\n", ret_val);
            break;
        case PROF_TS_ACK_PARA_ERROR:
            prof_err("Profile TS_ACK error. The parameter member was invalid. (TS_ACK=%d)\n", ret_val);
            break;
        case PROF_TS_ACK_VIR_NOT_SUPPORT:
            prof_err("Profile TS_ACK error. Virtualization was not supported. (TS_ACK=%d)\n", ret_val);
            break;
        case PROF_TS_ACK_INVALID_SUBCHANNEL:
            prof_err("Profile TS_ACK error. The sub_channel_id was not invalid. (TS_ACK=%d)\n", ret_val);
            break;
        default:
            break;
    }
}

STATIC int prof_chan_sync_msg_send(struct prof_sub_channel_info *sub_channel_info,
    unsigned char *sq_send_buf, enum prof_channel_state target_state)
{
    u32 devid = sub_channel_info->device_id;
    u32 channel_id = sub_channel_info->channel_id;
    u32 sub_channel_id = sub_channel_info->sub_channel_id;
    u32 tsid = sub_channel_info->ts_channel.tsid;
    u32 vfid = sub_channel_info->vfid;
    int ret;

    ret = prof_tscpu_sq_send(devid, tsid, sq_send_buf);
    if (ret != PROF_OK) {
        prof_err("Chan send fail. (devid=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    prof_debug("Send cmd to TS. (devid=%u; channel_id=%u; sub_channel_id=%u; tsid=%u; vfid=%u)\n",
        devid, channel_id, sub_channel_id, tsid, vfid);

    ret = wait_event_interruptible_timeout(sub_channel_info->channel_wq,
        sub_channel_info->channel_state == (u32)target_state, TS2DRV_TIMEOUT);
    if (ret == PROF_WAIT_NOTHING) {
        sub_channel_info->ts_channel.cmd_verify++;
        prof_err("Waiting for TS to synchronize the command response was timeout."
            " (devid=%u; vfid=%u; tsid=%u; channel_id=%u; sub_channel_id=%u; cmd_verify=%u)\n",
            devid, vfid, tsid, channel_id, sub_channel_id, sub_channel_info->ts_channel.cmd_verify);
        return PROF_TIMEOUT;
    } else if (ret == -ERESTARTSYS) {
        prof_event("Invoked function [wait_event_interruptible_timeout] had been awakened "
            "by the value returned from interruption function. (ret=%d)\n", ret);
        return PROF_ERROR;
    } else if (ret < 0) {
        prof_err("Failed to invoked function [wait_event_interruptible_timeout]. (ret=%d)\n", ret);
        return PROF_ERROR;
    } else {
        /* do nothing */
    }

    return 0;
}

int prof_tscpu_start(struct prof_proc_ctx *proc_ctx, struct prof_ioctl_para *para)
{
    struct prof_sub_channel_info *sub_channel_info = NULL;
    struct prof_sq_scheduler sq_send_cmd = {0};
    u32 device_id = para->device_id;
    u32 vfid = para->vfid;
    u32 channel_id = para->channel_id;
    u32 sub_channel_id, tsid;
    int poll_fd_num, ret;
    para->ret_val = PROF_ERROR;

    if (prof_check_tscpu_start_para_valid(para) != 0) {
        prof_err("Parameter [para] had invalid members."
            " (device_id=%u; vfid=%u; channel_id=%u; buf_len=%u)\n",
            device_id, vfid, channel_id, para->buf_len);
        return PROF_ERROR;
    }

    sub_channel_info = prof_alloc_sub_channel_resource(para, proc_ctx);
    if (sub_channel_info == NULL) {
        if (para->ret_val == PROF_STARTED_ALREADY) {
            return PROF_OK;
        }
        prof_err("Failed to allocate the subordinate channel. (device_id=%u; vfid=%u; channel_id=%u)\n",
            device_id, vfid, channel_id);
        return PROF_ERROR;
    }

    mutex_lock(&sub_channel_info->state_mutex);
    sub_channel_id = sub_channel_info->sub_channel_id;
    prof_init_dfx_info(sub_channel_info);

    ret = prof_tscpu_init_channel_info(sub_channel_info);
    if (ret != PROF_OK) {
        mutex_unlock(&sub_channel_info->state_mutex);
        prof_free_sub_channel_resource(sub_channel_info);
        prof_err("Failed to initialize the TS channel. (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u)\n",
            device_id, vfid, channel_id, sub_channel_id);
        return PROF_ERROR;
    }

    ret = prof_start_sq_messages_init(sub_channel_info, para->user_data, para->user_data_size, &sq_send_cmd);
    if (ret != PROF_OK) {
        prof_err("Failed to set the SQ messages. (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u)\n",
            device_id, vfid, channel_id, sub_channel_id);
        goto prof_tscpu_start_err;
    }

    tsid = sub_channel_info->ts_channel.tsid;
    PROF_SQ_CMD_SHOW(tsid, &sq_send_cmd);

    ret = prof_chan_sync_msg_send(sub_channel_info, (unsigned char *)&sq_send_cmd, PROF_CHANNEL_STARTING);
    if (ret != PROF_OK) {
        if (ret == PROF_TIMEOUT) {
            sub_channel_info->ts_channel.cmd_verify++;
            sub_channel_info->ts_channel.ts_buff_free_flag = TS_BUFF_NOT_FREE;
        }
        goto prof_tscpu_start_err;
    }

    /* check the result from ts */
    if (sub_channel_info->ts_channel.ret_val != PROF_TS_ACK_OK) {
        ret = sub_channel_info->ts_channel.ret_val;
        prof_ts_ack_error_analysis(sub_channel_info->ts_channel.ret_val);
        prof_err("The value received from TS was invalid."
            " (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u; ret=%d)\n",
            device_id, vfid, channel_id, sub_channel_id, sub_channel_info->ts_channel.ret_val);
        goto prof_tscpu_start_err;
    }

    sub_channel_info->ts_channel.cmd_verify++;
    sub_channel_info->channel_state = (u32)PROF_CHANNEL_ENABLE;
    poll_fd_num = atomic_inc_return(&proc_ctx->poll.fd_num);

    sub_channel_info->prof_dfx.data_buf_len = sub_channel_info->buf_len - BUFF_HEAD_SIZE;
    para->ret_val = PROF_OK;
    mutex_unlock(&sub_channel_info->state_mutex);

    prof_info("The TS channel starting was success. (device_id=%u; vfid=%u; channel_id=%u; "
        "sub_channel_id=%u; poll_fd_num=%d)\n", device_id, vfid, channel_id, sub_channel_id, poll_fd_num);

    return PROF_OK;
prof_tscpu_start_err:
    if (sub_channel_info->vir_addr != NULL) {
        prof_tscpu_free_channel_memory(sub_channel_info);
    }
    sub_channel_info->channel_state = (u32)PROF_CHANNEL_DISABLE;
    mutex_unlock(&sub_channel_info->state_mutex);
    prof_free_sub_channel_resource(sub_channel_info);
    para->ret_val = ret;
    return ret;
}

u32 prof_tscpu_get_one_zone_data_len(unsigned char *base, u32 channel_group_data_len)
{
    prof_data_head_t *data_head = (prof_data_head_t *)base;
    u32 data_len;
    u32 write_ptr;
    u32 read_ptr;

#ifdef __aarch64__
    invalidate_cache((unsigned char *)data_head, BUFF_HEAD_SIZE);
#endif

    write_ptr = data_head->write_ptr;
    read_ptr = data_head->read_ptr;

    if (write_ptr >= read_ptr) {
        data_len = write_ptr - read_ptr;
    } else {
        data_len = (channel_group_data_len - read_ptr) + write_ptr;
    }
    prof_info("Profile showed buffer information. (write_ptr=%u; read_ptr=%u; data_len=%u;"
        " channel_group_data_len=%u)\n", write_ptr, read_ptr, data_len, channel_group_data_len);

    return data_len;
}

void prof_tscpu_stop_dfx_update(struct prof_sub_channel_info *sub_channel_info)
{
    prof_data_head_t *data_head = (struct prof_data_head *)((uintptr_t)sub_channel_info->vir_addr);
#ifdef __aarch64__
    invalidate_cache((unsigned char *)data_head, BUFF_HEAD_SIZE);
#endif
    sub_channel_info->prof_dfx.read_ptr = data_head->read_ptr;
    sub_channel_info->prof_dfx.write_ptr = data_head->write_ptr;
}

void prof_tscpu_stop_dfx_op(struct prof_sub_channel_info *sub_channel_info, struct prof_ioctl_para *para)
{
    prof_tscpu_stop_dfx_update(sub_channel_info);
    prof_show_dfx_info(sub_channel_info);
    prof_dev_dfx_load(&sub_channel_info->prof_dfx, para);
}

int prof_tscpu_stop(struct prof_proc_ctx *proc_ctx, struct prof_ioctl_para *para)
{
    struct prof_sub_channel_info *sub_channel_info = NULL;
    struct prof_sq_scheduler sq_send_cmd = {0};
    u32 tsid;
    int ret;

    para->ret_val = PROF_ERROR;

    ret = prof_check_device_state(para->device_id);
    if (ret != PROF_OK) {
        prof_err("Failed to check the status of device. (device_id=%u; ret=%d)\n", para->device_id, ret);
        return PROF_ERROR;
    }

    sub_channel_info = prof_get_sub_channel_info_from_ctx(proc_ctx, para->device_id,
        para->vfid, para->channel_id);
    if (sub_channel_info == NULL) {
        para->ret_val = PROF_STOPPED_ALREADY;
        prof_warn("The TS channel had been stopped. (device_id=%u; vfid=%u; channel_id=%u)\n",
            para->device_id, para->vfid, para->channel_id);
        return PROF_OK;
    }

    tsid = sub_channel_info->ts_channel.tsid;
    mutex_lock(&sub_channel_info->state_mutex);
    ret = prof_stop_channel_status_check(sub_channel_info, proc_ctx);
    if (ret == PROF_STOPPED_ALREADY) {
        mutex_unlock(&sub_channel_info->state_mutex);
        para->ret_val = PROF_STOPPED_ALREADY;
        return PROF_OK;
    } else if (ret != PROF_OK) {
        mutex_unlock(&sub_channel_info->state_mutex);
        para->ret_val = ret;
        return PROF_ERROR;
    }

    prof_sq_messages_init(sub_channel_info, TS_STOP, &sq_send_cmd);
    sub_channel_info->channel_state = (u32)PROF_CHANNEL_STOP_WAIT_TS;
    ret = prof_chan_sync_msg_send(sub_channel_info, (unsigned char *)&sq_send_cmd, PROF_CHANNEL_ENABLE);
    if (ret != PROF_OK) {
        para->ret_val = ret;
        sub_channel_info->ts_channel.ts_buff_free_flag = TS_BUFF_NOT_FREE;
        if (ret == PROF_TIMEOUT) {
            sub_channel_info->ts_channel.cmd_verify++;
        }
        goto end;
    }
    sub_channel_info->ts_channel.cmd_verify++;

    /* check the result from ts */
    if (sub_channel_info->ts_channel.ret_val != PROF_TS_ACK_OK) {
        para->ret_val = sub_channel_info->ts_channel.ret_val;
        sub_channel_info->ts_channel.ts_buff_free_flag = TS_BUFF_NOT_FREE;
        prof_ts_ack_error_analysis(sub_channel_info->ts_channel.ret_val);
        prof_err("Received the invalid value returned from TS. The buffer of TS was not free."
            " (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u; ret_val=%d)\n",
            para->device_id, para->vfid, para->channel_id, sub_channel_info->sub_channel_id, para->ret_val);
        goto end;
    }

    ret = prof_stop_wait_buff_read_over(sub_channel_info, PROF_TS_TYPE, proc_ctx);
    if (ret != PROF_OK) {
        mutex_unlock(&sub_channel_info->state_mutex);
        para->ret_val = PROF_STOPPED_ALREADY;
        return PROF_OK;
    }

    prof_tscpu_stop_dfx_op(sub_channel_info, para);
    para->ret_val = PROF_OK;
    prof_info("The TS channel had been stopped successfully. (dev_id=%u; vfid=%u; chan_id=%u; sub_chan_id=%u)\n",
        para->device_id, para->vfid, para->channel_id, sub_channel_info->sub_channel_id);

end:
    prof_channel_info_free(sub_channel_info, PROF_TS_TYPE);
    mutex_unlock(&sub_channel_info->state_mutex);
    prof_free_sub_channel_resource(sub_channel_info);

    return PROF_OK;
}

STATIC void prof_functional_send_sq_to_ts(struct prof_sub_channel_info *sub_channel_info,
    struct prof_cqsq_info *cqsq_info, u32 cmd)
{
    struct prof_sq_scheduler sq_send_cmd = {0};

    prof_sq_messages_init(sub_channel_info, cmd, &sq_send_cmd);
    sub_channel_info->channel_state = (u32)PROF_CHANNEL_STOP_WAIT_TS;
    (void)prof_chan_sync_msg_send(sub_channel_info, (unsigned char *)&sq_send_cmd, PROF_CHANNEL_ENABLE);
}

STATIC void prof_tscpu_sub_channel_stop(struct prof_sub_channel_info *sub_channel_info)
{
    struct prof_device_info *prof_device = prof_device_info_get(sub_channel_info->device_id);
    struct prof_cqsq_info *cqsq_info = NULL;
    u32 tsid = 0;

    mutex_lock(&sub_channel_info->state_mutex);
    if (sub_channel_info->channel_state == (u32)PROF_CHANNEL_DISABLE ||
        sub_channel_info->channel_state == (u32)PROF_CHANNEL_IDLE) {
        mutex_unlock(&sub_channel_info->state_mutex);
        return;
    }

    sub_channel_info->channel_state = (u32)PROF_CHANNEL_DISABLE;
    sub_channel_info->poll_flag = (int)POLL_INVALID;
    tsid = sub_channel_info->ts_channel.tsid;
    cqsq_info = &prof_device->cqsq_info[tsid];

    prof_functional_send_sq_to_ts(sub_channel_info, cqsq_info, TS_STOP);

    if (sub_channel_info->vir_addr != NULL)
        prof_tscpu_free_channel_memory(sub_channel_info);

    sub_channel_info->ts_channel.cmd_verify++;
    sub_channel_info->channel_state = (u32)PROF_CHANNEL_DISABLE;
    sub_channel_info->poll_flag = (int)POLL_INVALID;
    mutex_unlock(&sub_channel_info->state_mutex);
    prof_free_sub_channel_resource(sub_channel_info);
}

STATIC void prof_tscpu_channel_stop(u32 device_id, u32 vfid, u32 channel_id)
{
    struct prof_sub_channel_info *sub_channel_info = NULL;
    u32 i;

    for (i = 0; i < PROF_SUB_CHANNEL_NUM_MAX; i++) {
        sub_channel_info = prof_get_sub_channel_info_from_index(device_id, vfid, channel_id, i);
        if (sub_channel_info == NULL) {
            continue;
        }
        prof_tscpu_sub_channel_stop(sub_channel_info);
    }
}

void prof_dev_tscpu_all_stop(u32 device_id)
{
    u32 j, k;

    for (j = 0; j < PROF_VFID_NUM_MAX; j++) {
        for (k = CHANNEL_TSCPU; k < CHANNEL_TSCPU_MAX; k++) {
            prof_tscpu_channel_stop(device_id, j, k);
        }
    }
}

STATIC int prof_tscpu_read_ringbuffer(struct prof_data_head *data_head, struct prof_ioctl_para *para,
    u32 could_read_size, u32 ch_data_buf_len, void *out_buf)
{
    unsigned char *data_base = NULL;
    u32 read_ptr = data_head->read_ptr;

    if (read_ptr >= ch_data_buf_len) {
        prof_err("Invalid read_ptr. (device_id=%u; vfid=%u; channel_id=%u; read_ptr=%u; ch_data_len=%u)\n",
            para->device_id, para->vfid, para->channel_id, read_ptr, ch_data_buf_len);
        return -EINVAL;
    }

    data_base = (unsigned char *)(uintptr_t)data_head + BUFF_HEAD_SIZE + read_ptr;
#ifdef __aarch64__
    invalidate_cache(data_base, could_read_size);
#endif

    if (prof_send_data(out_buf, data_base, could_read_size, para) != 0) {
        prof_err("Failed to make profile send data. (device_id=%u; vfid=%u; channel_id=%u; "
            "could_read_size=%u\n",
            para->device_id, para->vfid, para->channel_id, could_read_size);
        return PROF_ERROR;
    }

    data_head->read_ptr = (read_ptr + could_read_size) % ch_data_buf_len;
#ifdef __aarch64__
    flush_cache((unsigned char *)data_head, BUFF_HEAD_SIZE);
#endif

    return PROF_OK;
}

int prof_tscpu_ringbuffer(unsigned char *base, struct prof_ioctl_para *para,
    struct prof_dev_dfx_info *prof_dfx, u32 sub_channel_id)
{
    u32 ch_data_buf_len = prof_get_buff_data_len(para->device_id, para->channel_id,
        para->vfid, sub_channel_id);
    struct prof_data_head *data_head = (struct prof_data_head *)base;
    u32 could_read_size, write_ptr, read_ptr;
    void *out_buf = para->out_buf + para->ret_val;
    int ret;

#ifdef __aarch64__
    invalidate_cache((unsigned char *)data_head, BUFF_HEAD_SIZE);
#endif
    read_ptr = data_head->read_ptr;
    write_ptr = data_head->write_ptr;
    if (read_ptr == write_ptr) {
        return PROF_OK;
    }

    if (read_ptr >= ch_data_buf_len || write_ptr >= ch_data_buf_len) {
        prof_err("The read_ptr or writer_ptr was invalid. (device_id=%u; vfid=%u; channel_id=%u; "
            "ch_data_buf_len=%u; read_ptr=%u, write_ptr=%u\n",
            para->device_id, para->vfid, para->channel_id,
            ch_data_buf_len, read_ptr, write_ptr);
        return PROF_ERROR;
    }

    if (write_ptr > read_ptr) {
        could_read_size = ((write_ptr - read_ptr) > para->buf_len) ? para->buf_len : (write_ptr - read_ptr);

        ret = prof_tscpu_read_ringbuffer(data_head, para, could_read_size, ch_data_buf_len, out_buf);
        if (ret != PROF_OK) {
            prof_err("Failed to read the ringbuffer. (ret=%d)\n", ret);
            return ret;
        }

        para->ret_val += could_read_size;
        para->buf_len -= could_read_size;
        prof_dfx->prof_out_total_size += could_read_size;
        prof_dfx->prof_read_count++;

        return PROF_OK;
    } else {
        could_read_size = ((ch_data_buf_len - read_ptr) > para->buf_len) ?
            para->buf_len : (ch_data_buf_len - read_ptr);

        ret = prof_tscpu_read_ringbuffer(data_head, para, could_read_size, ch_data_buf_len, out_buf);
        if (ret != PROF_OK) {
            prof_err("Failed to read the ringbuffer. (ret=%d)\n", ret);
            return ret;
        }

        para->ret_val += could_read_size;
        para->buf_len -= could_read_size;
        prof_dfx->prof_out_total_size += could_read_size;
        if (data_head->read_ptr == 0) {
            prof_dfx->prof_read_flipped_count++;
        }

        if (write_ptr == 0 || para->buf_len == 0) {
            prof_dfx->prof_read_count++;
            return PROF_OK;
        }
    }

    out_buf = out_buf + could_read_size;
    could_read_size = (write_ptr > para->buf_len) ? para->buf_len : write_ptr;
    ret = prof_tscpu_read_ringbuffer(data_head, para, could_read_size, ch_data_buf_len, out_buf);
    if (ret != PROF_OK) {
        prof_err("Failed to read the ringbuffer. (ret=%d)\n", ret);
        return ret;
    }

    para->ret_val += could_read_size;
    para->buf_len -= could_read_size;
    prof_dfx->prof_out_total_size += could_read_size;
    prof_dfx->prof_read_count++;

    return PROF_OK;
}

STATIC int prof_tscpu_read_sub_channel(struct prof_ioctl_para *para,
    struct prof_sub_channel_info *sub_channel_info)
{
    int ret;
    para->ret_val = 0;

    ret = prof_tscpu_platform_read_sub_ch(para, sub_channel_info);
    if (ret == PROF_OK && para->buf_len != 0) {
        if (sub_channel_info->channel_state == (u32)PROF_CHANNEL_STOPPING) {
            sub_channel_info->channel_state = (u32)PROF_CHANNEL_ENABLE;
            wake_up(&sub_channel_info->channel_wq);
            prof_debug("Reading buffer had been finished. Stop to wake up the channel."
                " (device_id=%u; vfid=%u; channel_id=%u; sub_ch_id=%u)", sub_channel_info->device_id,
                sub_channel_info->vfid, sub_channel_info->channel_id, sub_channel_info->sub_channel_id);
        }

        if (sub_channel_info->channel_state == (u32)PROF_CHANNEL_FLUSH) {
            sub_channel_info->channel_state = (u32)PROF_CHANNEL_ENABLE;
            wake_up(&sub_channel_info->channel_wq);
            prof_debug("Reading buffer had been finished. The channel was flushed."
                "(device_id=%u; vfid=%u; channel_id=%u; sub_ch_id=%u)\n", sub_channel_info->device_id,
                sub_channel_info->vfid, sub_channel_info->channel_id, sub_channel_info->sub_channel_id);
        }
    }

    return ret;
}

STATIC int prof_tscpu_read_check_channel(struct prof_sub_channel_info *sub_channel_info,
    struct prof_ioctl_para *para, struct prof_proc_ctx *proc_ctx)
{
    if (sub_channel_info->channel_state == (u32)PROF_CHANNEL_DISABLE ||
        sub_channel_info->channel_state == (u32)PROF_CHANNEL_IDLE) {
        prof_warn("Reading channel had been stopped."
            " (device_id=%u; vfid=%u: sub_channel_id=%u; sub_channel_id=%u; channel_state=%u)\n",
            para->device_id, para->vfid, para->channel_id,
            sub_channel_info->sub_channel_id, sub_channel_info->channel_state);
        return PROF_STOPPED_ALREADY;
    }

    spin_lock_bh(&sub_channel_info->spinlock);
    if (sub_channel_info->proc_ctx != proc_ctx) {
        spin_unlock_bh(&sub_channel_info->spinlock);
        prof_err("Could not read other channels of process CTX."
            " (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u)\n",
            sub_channel_info->device_id, para->vfid, para->channel_id, sub_channel_info->sub_channel_id);
        return PROF_ERROR;
    }
    spin_unlock_bh(&sub_channel_info->spinlock);

    if (sub_channel_info->vir_addr == NULL) {
        prof_err("Parameter [vir_addr] was invalid."
            " (vir_addr=NULL; device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u)\n",
            para->device_id, para->vfid, para->channel_id, sub_channel_info->sub_channel_id);
        return PROF_ERROR;
    }

    return PROF_OK;
}

int prof_tscpu_read(struct prof_proc_ctx *proc_ctx, struct prof_ioctl_para *para)
{
    struct prof_sub_channel_info *sub_channel_info = NULL;
    int ret;

    sub_channel_info = prof_get_sub_channel_info_from_ctx(proc_ctx, para->device_id,
        para->vfid, para->channel_id);
    if (sub_channel_info == NULL) {
        prof_warn("The subordinate channel had been stopped. (device_id=%u; vfid=%u; channel_id=%u)\n",
            para->device_id, para->vfid, para->channel_id);
        para->ret_val = PROF_STOPPED_ALREADY;
        return PROF_OK;
    }

    mutex_lock(&sub_channel_info->state_mutex);
    ret = prof_tscpu_read_check_channel(sub_channel_info, para, proc_ctx);
    if (ret == PROF_STOPPED_ALREADY) {
        mutex_unlock(&sub_channel_info->state_mutex);
        para->ret_val = ret;
        return PROF_OK;
    } else if (ret != PROF_OK) {
        mutex_unlock(&sub_channel_info->state_mutex);
        return ret;
    }

    ret = prof_tscpu_sync_rw_ptr(sub_channel_info, (int)TS_SYNC_WRITE_PTR);
    if (ret != PROF_OK) {
        mutex_unlock(&sub_channel_info->state_mutex);
        prof_err("Failed to refresh the writing synchronous pointer of TS."
            " (device_id=%u; vfid=%u; channel_id=%u)\n",
            sub_channel_info->device_id, sub_channel_info->vfid, sub_channel_info->channel_id);
        return ret;
    }

    ret = prof_tscpu_read_sub_channel(para, sub_channel_info);
    if (ret != PROF_OK) {
        mutex_unlock(&sub_channel_info->state_mutex);
        return ret;
    }

    ret = prof_tscpu_sync_rw_ptr(sub_channel_info, (int)TS_SYNC_READ_PTR);
    if (ret != PROF_OK) {
        mutex_unlock(&sub_channel_info->state_mutex);
        prof_err("Failed to refresh the reading synchronous pointer of TS."
            " (device_id=%u; vfid=%u; channel_id=%u)\n",
            sub_channel_info->device_id, sub_channel_info->vfid, sub_channel_info->channel_id);
        return ret;
    }
    mutex_unlock(&sub_channel_info->state_mutex);

    return ret;
}

STATIC int prof_data_len_flush_para_check(struct prof_ioctl_para *prof_para)
{
    if (prof_is_valid_channel(prof_para->channel_id) == false) {
        prof_err("Parameter [channel_id] was invalid. (channel_id=%u)\n",
            prof_para->channel_id);
        return PROF_ERROR;
    }

    if (prof_para->out_buf == NULL) {
        prof_err("Parameter [out_buf] was invalid. Failed to flush data length."
            " (out_buf=NULL; device_id=%u; channel_id=%u)\n",
            prof_para->device_id, prof_para->channel_id);
        return PROF_ERROR;
    }

    return PROF_OK;
}

STATIC int prof_data_flush_channel_info_check(struct prof_sub_channel_info *sub_channel_info,
    struct prof_proc_ctx *proc_ctx)
{
    int ret;

    if (sub_channel_info->channel_state == (u32)PROF_CHANNEL_DISABLE ||
        sub_channel_info->channel_state == (u32)PROF_CHANNEL_IDLE ||
        sub_channel_info->channel_state == (u32)PROF_CHANNEL_STARTING) {
        prof_err("The channel was disabled. Failed to flush data length."
            " (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u)\n",
            sub_channel_info->device_id, sub_channel_info->vfid,
            sub_channel_info->channel_id, sub_channel_info->sub_channel_id);
        return PROF_STOPPED_ALREADY;
    }

    if (sub_channel_info->channel_state == (u32)PROF_CHANNEL_STOPPING ||
        sub_channel_info->channel_state == (u32)PROF_CHANNEL_STOP_WAIT_TS) {
        prof_err("The channel was stopped. Failed to flush data length."
            " (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u)",
            sub_channel_info->device_id, sub_channel_info->vfid,
            sub_channel_info->channel_id, sub_channel_info->sub_channel_id);
        return PROF_BUSY;
    }

    ret = prof_data_flush_support_check(sub_channel_info->channel_id);
    if (ret != PROF_OK) {
        prof_err("Flushing data was not supported. (device_id=%u; channel_id=%u)\n",
            sub_channel_info->device_id, sub_channel_info->channel_id);
        return ret;
    }

    spin_lock_bh(&sub_channel_info->spinlock);
    if (sub_channel_info->proc_ctx != proc_ctx) {
        spin_unlock_bh(&sub_channel_info->spinlock);
        prof_err("Failed to flush other channel of the process."
            " (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u)\n",
            sub_channel_info->device_id, sub_channel_info->vfid,
            sub_channel_info->channel_id, sub_channel_info->sub_channel_id);
        return PROF_ERROR;
    }
    spin_unlock_bh(&sub_channel_info->spinlock);

    if (sub_channel_info->channel_state == (u32)PROF_CHANNEL_FLUSH) {
        prof_err("The channel was flushed. Failed to flush data length."
            " (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u)\n",
            sub_channel_info->device_id, sub_channel_info->vfid,
            sub_channel_info->channel_id, sub_channel_info->sub_channel_id);
        return PROF_BUSY;
    }

    return PROF_OK;
}

STATIC int prof_flush_sync_write_ptr(struct prof_sub_channel_info *sub_channel_info)
{
    u32 device_id = sub_channel_info->device_id;
    u32 vfid = sub_channel_info->vfid;
    u32 channel_id = sub_channel_info->channel_id;
    int ret;

#ifdef CFG_FEATURE_PROF_HWTS_THROUGH
    if (prof_channel_is_hwts_through(channel_id)) {
        return PROF_OK;
    }
#endif

    ret = prof_channel_need_sync_rw_ptr(sub_channel_info);
    if (ret != PROF_OK) {
        return PROF_OK;
    }

    ret = prof_tscpu_send_sync_rw_ptr_cmd(sub_channel_info, (int)TS_SYNC_WRITE_PTR);
    if (ret != PROF_OK) {
        prof_err("Failed to refresh the writing synchronous pointer of TS. "
            "(device_id=%u; vfid=%u; channel_id=%u)\n", device_id, vfid, channel_id);
        return PROF_ERROR;
    }

    ret = wait_event_interruptible_timeout(sub_channel_info->channel_wq,
        sub_channel_info->channel_state == (u32)PROF_CHANNEL_ENABLE, TS2DRV_TIMEOUT);
    if (ret == PROF_WAIT_NOTHING) {
        sub_channel_info->ts_channel.cmd_verify++;
        prof_err("Waiting for TS to flush the command response was timeout."
            " (device_id=%u; vfid=%u; ret=%d; channel_id=%u; cmd_verify=%u)\n",
            device_id, vfid, ret, channel_id, sub_channel_info->ts_channel.cmd_verify);
        return PROF_TIMEOUT;
    } else if (ret == -ERESTARTSYS) {
        prof_event("Invoked function [wait_event_interruptible_timeout] had been awakened "
            "by the value returned from interruption function. (ret=%d)\n", ret);
        return PROF_ERROR;
    } else if (ret < 0) {
        prof_err("Failed to invoked function [wait_event_interruptible_timeout]. (ret=%d)\n", ret);
        return PROF_ERROR;
    }

    sub_channel_info->ts_channel.cmd_verify++;
    return PROF_OK;
}

STATIC void prof_flush_wait_buff_read_over(struct prof_sub_channel_info *sub_channel_info, u32 data_len,
    u32 mode)
{
    if (data_len == 0) {
        return;
    }

    (void)prof_wake_up_poll_wq(sub_channel_info);
    /* host collect mode need wait data read over */
    if (mode == PROF_MODE_KERNEL) {
        sub_channel_info->channel_state = (u32)PROF_CHANNEL_FLUSH;
        mutex_unlock(&sub_channel_info->state_mutex);
        if (prof_wait_read_last_data(sub_channel_info) != PROF_OK) {
            prof_err("Failed to wait for reading the last data. Failed to flush data length."
                " (device_id=%u; vfid=%u; channel_id=%u)\n",
                sub_channel_info->device_id, sub_channel_info->vfid, sub_channel_info->channel_id);
        }
        mutex_lock(&sub_channel_info->state_mutex);
    }
}

int prof_data_len_flush(struct prof_proc_ctx *proc_ctx, struct prof_ioctl_para *prof_para)
{
    struct prof_sub_channel_info *sub_channel_info = NULL;
    u32 data_len;
    int ret;

    ret = prof_data_len_flush_para_check(prof_para);
    if (ret != PROF_OK) {
        prof_para->ret_val = PROF_ERROR;
        return ret;
    }

    sub_channel_info = prof_get_sub_channel_info_from_ctx(proc_ctx, prof_para->device_id,
        prof_para->vfid, prof_para->channel_id);
    if (sub_channel_info == NULL) {
        prof_para->ret_val = PROF_STOPPED_ALREADY;
        prof_err("The subordinate channel had been stopped. (device_id=%u; vfid=%u; channel_id=%u)\n",
            prof_para->device_id, prof_para->vfid, prof_para->channel_id);
        return PROF_ERROR;
    }
    mutex_lock(&sub_channel_info->state_mutex);

    ret = prof_data_flush_channel_info_check(sub_channel_info, proc_ctx);
    if (ret != PROF_OK) {
        mutex_unlock(&sub_channel_info->state_mutex);
        prof_para->ret_val = ret;
        return ret;
    }

    sub_channel_info->channel_state = (u32)PROF_CHANNEL_FLUSH;
    /* sync rw ptr with ts */
    ret = prof_flush_sync_write_ptr(sub_channel_info);
    if (ret != PROF_OK) {
        sub_channel_info->channel_state = (u32)PROF_CHANNEL_ENABLE;
        mutex_unlock(&sub_channel_info->state_mutex);
        prof_err("Failed to synchronize the reading and writing pointer of TS. Failed to flush data length."
            " (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u)\n",
            sub_channel_info->device_id, sub_channel_info->vfid, sub_channel_info->channel_id,
            sub_channel_info->sub_channel_id);
        prof_para->ret_val = ret;
        return ret;
    }

    /* get data len in data_buff */
    data_len = prof_tscpu_get_data_len(sub_channel_info);
    /* return data_len */
    ret = prof_send_data(prof_para->out_buf, &data_len, sizeof(u32), prof_para);
    if (ret != PROF_OK) {
        sub_channel_info->channel_state = (u32)PROF_CHANNEL_ENABLE;
        mutex_unlock(&sub_channel_info->state_mutex);
        prof_err("Failed to make profile send data. (device_id=%u; vfid=%u; channel_id=%u)\n",
            prof_para->device_id, prof_para->vfid, prof_para->channel_id);
        prof_para->ret_val = PROF_ERROR;
        return PROF_ERROR;
    }

    prof_flush_wait_buff_read_over(sub_channel_info, data_len, prof_para->use_mode);
    sub_channel_info->channel_state = (u32)PROF_CHANNEL_ENABLE;
    mutex_unlock(&sub_channel_info->state_mutex);

    prof_para->ret_val = ret;
    return ret;
}
#else
int prof_ts_ut_test(void)
{
    return 0;
}
module_init(prof_ts_ut_test);
#endif
