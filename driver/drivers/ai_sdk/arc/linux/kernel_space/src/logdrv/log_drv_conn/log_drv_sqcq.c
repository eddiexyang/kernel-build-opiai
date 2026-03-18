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
#include <linux/types.h>
#include "log_drv_sqcq.h"
#include "log_drv_res_common.h"
#include "devdrv_functional_cqsq_api.h"
#ifndef AOS_LLVM_BUILD
#include "tsdrv_hwinfo.h"
#endif
#include "log_drv_chan.h"
#ifdef CFG_SOC_PLATFORM_MDC_V51
#include "tsdrv_ts_node.h"
#include "log_xpc_server.h"
#endif

#ifdef CFG_SOC_PLATFORM_MDC_V51
#ifdef CFG_SOC_MDC_V51_LITE
#define LOG_DRV_TS_MDC_NUM 1
#else
#define LOG_DRV_TS_MDC_NUM 2
#endif
bool log_is_ts_valid(void)
{
    int ts_num = devdrv_get_ts_node_num();
    return (ts_num == LOG_DRV_TS_MDC_NUM) ? true : false;
}
#endif

#ifndef AOS_LLVM_BUILD
void log_cq0_report(u32 device_id, u32 tsid, const u8 *cq_buf, u8 *sq_buf);
void log_cq1_callback(u32 device_id, u32 tsid, const u8 *cq_buf, u8 *sq_buf);

STATIC s32 log_cqsq_init(struct log_device_ctx *device_ctx, u32 tsid);
STATIC void log_cqsq_uninit(struct log_device_ctx *device_ctx, u32 tsid);

STATIC s32 judge_chan_is_ready(struct log_device_ctx *device_ctx, const struct log_channel_info *channel_info)
{
    const struct log_channel_desc *log_chan_desc = channel_info->log_desc;
    const struct log_device_info *device_info = &device_ctx->device_info;
    s32 tsid = log_chan_desc->reserve;

    if ((tsid < 0) || (tsid > LOG_MDC_TS_ID)) {
        slog_drv_err("Tsid is invalid. (device_id=%d; tsid=%d)\n", device_info->device_id, tsid);
        return LOG_RET_ERROR;
    }

    if (log_chan_desc->channel_conn != (s32)LOG_CHANNEL_CONN_SQCQ) {
        slog_drv_err("Channel connect type was unmatched. "
            "(device_id=%d; channel_conn=%d)\n", device_info->device_id, log_chan_desc->channel_conn);
        return LOG_RET_ERROR;
    }

    if (device_info->device_state[tsid] == DEV_UNUSED) {
        slog_drv_err("Log cqsq is not ready. (device_id=%d; tsid=%d)\n", device_info->device_id, tsid);
        return LOG_RET_ERROR;
    }
    return LOG_RET_OK;
}

/*
 * cmd msg to ts, include create, delete, set level
 * these function will be attached to log_channel_desc
 */
STATIC s32 log_send_cmd_ts_cqsq(struct log_device_ctx *device_ctx, struct log_sq_scheduler *sq_info,
    u32 tsid, u32 channel_id)
{
    struct log_channel_info *channel_info = NULL;
    struct log_device_info *device_info = NULL;
    struct log_cqsq_info *cqsq_info = NULL;
    s32 set_level_channel_type;
    s32 device_id;
    s32 ret;

    device_id = device_ctx->device_info.device_id;
    device_info = &device_ctx->device_info;
    cqsq_info = &device_info->cqsq_info[tsid];
    ret = log_shift_cmd_to_ts((u32)device_id, sq_info, &set_level_channel_type);
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("Unknown channel_id. (device_id=%d; channel_id=%u)\n",
            device_id, sq_info->channel_id);
        return LOG_RET_ERROR;
    }

    sema_init(&(cqsq_info->cq0_wait_sema[set_level_channel_type]), 0);

#ifdef CFG_TRS_REFACTOR_FEATURE
    ret = log_sqcq_send(device_id, tsid, (u8 *)sq_info);
#else
#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (!log_is_ts_valid()) {
        ret = log_xpc_send_cmd((u32)device_id, tsid, cqsq_info->sq_0_index, sq_info);
    } else {
        ret = devdrv_functional_send_sq((u32)device_id, tsid, cqsq_info->sq_0_index,
            (u8 *)sq_info, sizeof(struct log_sq_scheduler));
    }
#else
    ret = devdrv_functional_send_sq((u32)device_id, tsid, cqsq_info->sq_0_index,
        (u8 *)sq_info, sizeof(struct log_sq_scheduler));
#endif
#endif
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("Send sq error. "
            "(device_id=%d; channel_id=%u; cmd_verify=%u; channel_cmd=%u; tsid=%u; ret=%d)\n",
            device_id, sq_info->channel_id, sq_info->cmd_verify, sq_info->channel_cmd, tsid, ret);
        device_info->cmd_verify++;

        return ret;
    }

    ret = down_timeout(&cqsq_info->cq0_wait_sema[set_level_channel_type], TS2DRV_TIMEOUT);
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("Waiting for TS to reply cq0 timeout. "
            "(device_id=%d; channel_id=%u; cmd_verify=%u; channel_cmd=%u; tsid=%u; ret=%d)\n",
            device_id, sq_info->channel_id, sq_info->cmd_verify, sq_info->channel_cmd, tsid, ret);
        device_info->cmd_verify++;

        return LOG_RET_TIMEOUT;
    }

    channel_info = &device_ctx->channels_info[channel_id];
    if (channel_info->ret_val != (s32)LOG_RET_OK) {
        slog_drv_err("TS return an error. (ret_val=%d)\n", channel_info->ret_val);
        return channel_info->ret_val;
    }

    return LOG_RET_OK;
}

STATIC s32 log_send_create_cmd_ts_cqsq(struct log_device_ctx *device_ctx, struct log_channel_info *channel_info)
{
    const struct log_channel_desc *log_chan_desc = channel_info->log_desc;
    const struct log_device_info *device_info = NULL;
    struct log_sq_scheduler sq_slot = {0};
    s32 ret;

    device_info = &device_ctx->device_info;

    LOG_DEBUG_CHANINFO_SHOW(channel_info);

    if (judge_chan_is_ready(device_ctx, channel_info) != LOG_RET_OK) {
        return LOG_RET_ERROR;
    }

    sq_slot.cmd_verify = device_info->cmd_verify;
    sq_slot.channel_id = (u32)(log_chan_desc->channel_ids);
    sq_slot.channel_cmd = LOG_CHANNEL_MSG_CREATE;
    sq_slot.buf_len = (u32)(log_chan_desc->buf_size - sizeof(struct log_channel_buf_head));
    sq_slot.phy_addr = channel_info->phy_addr;
    sq_slot.log_level = (u32)channel_info->log_level;
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    sq_slot.phy_addr &= 0xFFFFFFFFFF; /* a55 can only access 40bit address, die1 or p1/2/3... use ADDR SLIDING WINDOW */
#endif
    LOG_DEBUG_SQSLOT(&sq_slot, log_chan_desc->reserve);

    ret = log_send_cmd_ts_cqsq(device_ctx, &sq_slot, (u32)log_chan_desc->reserve,
        (u32)log_chan_desc->channel_ids);
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("Log_send_cmd_ts_cqsq failed. (device_id=%d; channel_conn=%d; ret=%d)\n",
            device_info->device_id, channel_info->channel_conn, ret);
        return LOG_RET_ERROR;
    }

    return LOG_RET_OK;
}

STATIC s32 log_send_delete_cmd_ts_cqsq(struct log_device_ctx *device_ctx, struct log_channel_info *channel_info)
{
    const struct log_channel_desc *log_chan_desc = channel_info->log_desc;
    const struct log_device_info *device_info = NULL;
    struct log_sq_scheduler sq_slot = {0};
    s32 ret;

    LOG_DEBUG_CHANINFO_SHOW(channel_info);

    device_info = &device_ctx->device_info;

    if (judge_chan_is_ready(device_ctx, channel_info) != LOG_RET_OK) {
        return LOG_RET_ERROR;
    }

    sq_slot.cmd_verify = device_info->cmd_verify;
    sq_slot.channel_id = (u32)log_chan_desc->channel_ids;
    sq_slot.channel_cmd = LOG_CHANNEL_MSG_DELETE;
    LOG_DEBUG_SQSLOT(&sq_slot, log_chan_desc->reserve);

    ret = log_send_cmd_ts_cqsq(device_ctx, &sq_slot, (u32)log_chan_desc->reserve,
        (u32)log_chan_desc->channel_ids);
#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (log_is_ts_valid()) {
        log_cqsq_uninit(device_ctx, (u32)log_chan_desc->reserve);
    } else {
        log_xpc_cqsq_uninit(device_ctx, (u32)log_chan_desc->reserve);
    }
#else
    log_cqsq_uninit(device_ctx, (u32)log_chan_desc->reserve);
#endif
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("Log_send_cmd_ts_cqsq failed. (device_id=%d; channel_conn=%d; ret=%d)\n",
            device_info->device_id, channel_info->channel_conn, ret);
        return LOG_RET_ERROR;
    }

    LOG_DEBUG_CHANINFO_SHOW(channel_info);

    return LOG_RET_OK;
}

STATIC s32 log_send_setlevel_cmd_ts_cqsq(struct log_device_ctx *device_ctx,
    struct log_channel_info *channel_info, s32 log_level)
{
    const struct log_channel_desc *log_chan_desc = channel_info->log_desc;
    const struct log_device_info *device_info = NULL;
    struct log_sq_scheduler sq_slot = {0};
    s32 ret;

    device_info = &device_ctx->device_info;

    LOG_DEBUG_CHANINFO_SHOW(channel_info);

    if (judge_chan_is_ready(device_ctx, channel_info) != LOG_RET_OK) {
        return LOG_RET_ERROR;
    }

    sq_slot.cmd_verify = device_info->cmd_verify;
    sq_slot.channel_id = (u32)log_chan_desc->channel_ids;
    sq_slot.channel_cmd = LOG_CHANNEL_MSG_SET;
    sq_slot.log_level = (u32)log_level;
    LOG_DEBUG_SQSLOT(&sq_slot, log_chan_desc->reserve);

    ret = log_send_cmd_ts_cqsq(device_ctx, &sq_slot, (u32)log_chan_desc->reserve,
        (u32)log_chan_desc->channel_ids);
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("Log_send_cmd_ts_cqsq failed. (device_id=%d; channel_conn=%d; ret=%d)\n",
            device_info->device_id, channel_info->channel_conn, ret);
        return LOG_RET_ERROR;
    }

    channel_info->log_level = log_level;
    slog_drv_info("Log_send_setlevel_cmd_ts_cqsq succeeded. (device_id=%d; channel_id=%d; log_level=%d)\n",
        device_info->device_id, log_chan_desc->channel_ids, log_level);

    return LOG_RET_OK;
}

STATIC s32 log_init_channel_head_ts_cqsq(struct log_device_ctx *device_ctx, struct log_channel_info *channel_info)
{
    s32 ret;
    struct log_channel_buf_head *buf_ptr = (struct log_channel_buf_head *)(channel_info->vir_addr);
    const struct log_channel_desc *log_chan_desc = channel_info->log_desc;
    const struct log_device_info *device_info = &device_ctx->device_info;
    s32 tsid = log_chan_desc->reserve;

    if (buf_ptr == NULL) {
        slog_drv_err("Buf_ptr is NULL.\n");
        return LOG_RET_ERROR;
    }

#ifndef CFG_FEATURE_HISTORY_CONFIG
    buf_ptr->buf_read = 0;
    buf_ptr->buf_write = 0;
#endif
    buf_ptr->buf_len = channel_info->buf_size;

    LOG_DEBUG_BUFFHEAD_SHOW(buf_ptr);

    if ((tsid < 0) || (tsid > LOG_MDC_TS_ID)) {
        slog_drv_err("Tsid is invalid. (device_id=%d; tsid=%d)\n", device_info->device_id, tsid);
        return LOG_RET_ERROR;
    }
#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (log_is_ts_valid()) {
        ret = log_cqsq_init(device_ctx, (u32)tsid);
    } else {
        ret = log_xpc_cqsq_init(device_ctx, (u32)tsid);
    }
#else
    ret = log_cqsq_init(device_ctx, (u32)tsid);
#endif
    if (ret != LOG_RET_OK) {
        slog_drv_err("Log cqsq init failed. (device_id=%d; tsid=%d)\n", device_info->device_id, tsid);
        return LOG_RET_ERROR;
    }
    return LOG_RET_OK;
}

#ifdef CFG_TRS_REFACTOR_FEATURE
STATIC int log_cqsq_init(struct log_device_ctx *device_ctx, u32 tsid)
{
    struct log_device_info *device_info = &device_ctx->device_info;
    struct log_cqsq_info *cqsq_info = &device_info->cqsq_info[tsid];
    u32 devid = device_info->device_id;
    int ret;

    if (device_info->device_state[tsid] == DEV_USED) {
        slog_drv_info("Device has initialized sqcq. (devid=%d)\n", devid);
        return LOG_RET_OK;
    }

    device_info->device_state[tsid] = DEV_USED;

    ret = log_sqcq_alloc(devid, tsid, &cqsq_info->sq_0_index, &cqsq_info->cq_0_index, &cqsq_info->cq_1_index);
    if (ret != 0) {
        slog_drv_err("Alloc sqcq fail. (devid=%d)\n", devid);
        device_info->device_state[tsid] = DEV_UNUSED;
        return ret;
    }

    return 0;
}

STATIC void log_cqsq_uninit(struct log_device_ctx *device_ctx, u32 tsid)
{
    struct log_device_info *device_info = &device_ctx->device_info;
    struct log_cqsq_info *cqsq_info = &device_info->cqsq_info[tsid];
    u32 devid = device_info->device_id;

    if (device_info->device_state[tsid] == DEV_UNUSED) {
        slog_drv_err("Device state is DEV_UNUSED. (device_id=%d; DEV_UNUSED=%d)\n", devid, DEV_UNUSED);
        return;
    }

    device_info->device_state[tsid] = DEV_UNUSED;
    log_sqcq_free(devid, tsid, cqsq_info->sq_0_index, cqsq_info->cq_0_index, cqsq_info->cq_1_index);
}

#else
STATIC int log_cqsq_init(struct log_device_ctx *device_ctx, u32 tsid)
{
    u32 sq_0_index = 0;
    u32 cq_0_index = 0;
    u32 cq_1_index = 0;
    u64 sq_0_addr = 0;
    u64 cq_0_addr = 0;
    u64 cq_1_addr = 0;
    struct log_device_info *device_info = NULL;
    struct devdrv_mailbox_cqsq mailbox_cqsq = {0};
    struct log_cqsq_info *cqsq_info = NULL;
    s32 device_id;
    s32 ret;

    if (device_ctx == NULL) {
        slog_drv_info("Device_ctx is NULL.\n");
        return LOG_RET_ERROR;
    }

    device_info = &device_ctx->device_info;
    device_id = device_info->device_id;
    cqsq_info = &device_info->cqsq_info[tsid];

    if (device_info->device_state[tsid] == DEV_USED) {
        slog_drv_info("Device has initialized sqcq. (device_id=%d)\n", device_id);
        return LOG_RET_OK;
    }

    device_info->device_state[tsid] = DEV_USED;

    ret = devdrv_create_functional_sq((u32)device_id, tsid, LOG_SQ_BUF_LEN, &sq_0_index,
        (u64 *)&sq_0_addr);
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("Devdrv_create_functional_sq failed. (device_id=%d; ret=%d)\n", device_id, ret);
        device_info->device_state[tsid] = DEV_UNUSED;
        return ret;
    }

    /* the first cq length is 128 Byte; and the other cqs are 32 Byte; */
    ret = devdrv_create_functional_cq((u32)device_id, tsid, LOG_SQ_BUF_LEN, report_cq,
        log_cq0_report, &cq_0_index, (u64 *)&cq_0_addr);
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("Devdrv_create_functional_cq failed. (device_id=%d; ret=%d)\n", device_id, ret);
        devdrv_destroy_functional_sq((u32)device_id, tsid, sq_0_index);
        device_info->device_state[tsid] = DEV_UNUSED;
        return ret;
    }

    ret = devdrv_create_functional_cq((u32)device_id, tsid, LOG_CQ_BUF_LEN, callback_cq,
        log_cq1_callback, &cq_1_index, (u64 *)&cq_1_addr);
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("Devdrv_create_functional_cq failed. (device_id=%d; ret=%d)\n", device_id, ret);
        devdrv_destroy_functional_sq((u32)device_id, tsid, sq_0_index);
        devdrv_destroy_functional_cq((u32)device_id, tsid, cq_0_index);
        device_info->device_state[tsid] = DEV_UNUSED;
        return ret;
    }
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    /* a55 can only access 40bit address, die1 or p1/2/3... use ADDR SLIDING WINDOW */
    sq_0_addr &= 0xFFFFFFFFFF;
    cq_0_addr &= 0xFFFFFFFFFF;
    cq_1_addr &= 0xFFFFFFFFFF;
#endif

    mailbox_cqsq.cq_irq = (u16)tsdrv_get_dfx_cq_irq_vector(device_id, tsid);
    mailbox_cqsq.cmd_type = LOG_CQSQ_CREATE;
    mailbox_cqsq.sq_index = (u16)sq_0_index;
    mailbox_cqsq.sq_addr = sq_0_addr;
    mailbox_cqsq.cq0_index = (u16)cq_0_index;
    mailbox_cqsq.cq0_addr = cq_0_addr;
    mailbox_cqsq.cq1_index = (u16)cq_1_index;
    mailbox_cqsq.cq1_addr = cq_1_addr;

    ret = devdrv_mailbox_send_cqsq((u32)device_id, tsid, &mailbox_cqsq);
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("Devdrv_mailbox_send_cqsq failed. (device_id=%d; ret=%d)\n", device_id, ret);
        devdrv_destroy_functional_sq((u32)device_id, tsid, sq_0_index);
        devdrv_destroy_functional_cq((u32)device_id, tsid, cq_0_index);
        devdrv_destroy_functional_cq((u32)device_id, tsid, cq_1_index);
        device_info->device_state[tsid] = DEV_UNUSED;
        return ret;
    }

    cqsq_info->sq_0_index = sq_0_index;
    cqsq_info->cq_0_index = cq_0_index;
    cqsq_info->cq_1_index = cq_1_index;
    LOG_CQSQ_INFO_SHOW(tsid, cqsq_info);
    slog_drv_info("Log_cqsq_init succeeded. (device_id=%d)\n", device_id);
    return ret;
}

STATIC void log_cqsq_uninit(struct log_device_ctx *device_ctx, u32 tsid)
{
    struct log_device_info *device_info = NULL;
    struct devdrv_mailbox_cqsq mailbox_cqsq = {0};
    const struct log_cqsq_info *cqsq_info = NULL;
    s32 device_id;
    s32 ret;

    if (device_ctx == NULL) {
        slog_drv_info("Device_ctx is NULL.\n");
        return;
    }

    device_info = &device_ctx->device_info;
    device_id = device_info->device_id;
    cqsq_info = &device_info->cqsq_info[tsid];
    if (device_id >= LOG_DEVICE_ID_MAX) {
        slog_drv_err("Device_id is invalid. (device_id=%d)\n", device_id);
        return;
    }

    if (device_info->device_state[tsid] == DEV_UNUSED) {
        slog_drv_err("Device state is DEV_UNUSED. (device_id=%d; DEV_UNUSED=%d)\n", device_id, DEV_UNUSED);
        return;
    }

    device_info->device_state[tsid] = DEV_UNUSED;

    mailbox_cqsq.cmd_type = LOG_CQSQ_RELEASE;
    mailbox_cqsq.sq_index = (u16)cqsq_info->sq_0_index;
    mailbox_cqsq.cq0_index = (u16)cqsq_info->cq_0_index;
    mailbox_cqsq.cq1_index = (u16)cqsq_info->cq_1_index;

    ret = devdrv_mailbox_send_cqsq((u32)device_id, tsid, &mailbox_cqsq);
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("Devdrv_mailbox_send_cqsq failed. (device_id=%d; ret=%d)\n", device_id, ret);
    }

    devdrv_destroy_functional_sq((u32)device_id, tsid, cqsq_info->sq_0_index);
    devdrv_destroy_functional_cq((u32)device_id, tsid, cqsq_info->cq_0_index);
    devdrv_destroy_functional_cq((u32)device_id, tsid, cqsq_info->cq_1_index);

    return;
}
#endif
void log_init_conn_func_sqcq(struct log_channel_desc * log_chan_desc)
{
    if (log_chan_desc == NULL) {
        slog_drv_err("Log_chan_desc is NULL.\n");
        return;
    }

    log_chan_desc->create = log_send_create_cmd_ts_cqsq;
    log_chan_desc->del = log_send_delete_cmd_ts_cqsq;
    log_chan_desc->set_level = log_chan_desc->log_level_support ? log_send_setlevel_cmd_ts_cqsq : NULL;
    log_chan_desc->init = log_init_channel_head_ts_cqsq;
}
#endif
