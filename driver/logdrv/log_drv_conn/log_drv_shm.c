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
#include "log_drv_shm.h"
#include "log_drv_dev.h"

/*
 * cmd msg to IMP, SAFE ISLAND, include create, delete, set level
 * these function will be attached to log_channel_desc
 */

STATIC s32 log_send_setlevel_cmd_shm(struct log_device_ctx *device_ctx,
    struct log_channel_info *channel_info, s32 log_level)
{
    const struct log_channel_desc *log_chan_desc = channel_info->log_desc;
    s32 device_id = device_ctx->device_info.device_id;
    struct log_channel_buf_head *buf_ptr = NULL;

    if (log_chan_desc->channel_conn != (s32)LOG_CHANNEL_CONN_SHAREMEM) {
        slog_drv_err("Channel connection was unmatched. "
            "(device_id=%d; channel_conn=%d)\n", device_id, log_chan_desc->channel_conn);
        return LOG_RET_ERROR;
    }

    buf_ptr = (struct log_channel_buf_head *)(channel_info->vir_addr);
    if (buf_ptr == NULL) {
        slog_drv_err("Buf_ptr is NULL.\n");
        return LOG_RET_ERROR;
    }

    buf_ptr->log_level = (u32)log_level;
    channel_info->log_level = log_level;
    slog_drv_info("Log_user_shm_set_level succeeded. (device_id=%d; channel_id=%d; log_level=%d)\n",
        device_id, log_chan_desc->channel_ids, log_level);

    LOG_DEBUG_BUFFHEAD_SHOW(buf_ptr);
    LOG_DEBUG_CHANINFO_SHOW(channel_info);
    return LOG_RET_OK;
}

STATIC s32 log_init_channel_head_shm(struct log_device_ctx *device_ctx, struct log_channel_info *channel_info)
{
    const struct log_channel_desc *desc = channel_info->log_desc;
    struct log_channel_buf_head *buf_ptr = (struct log_channel_buf_head *)channel_info->vir_addr;

    if (buf_ptr == NULL) {
        slog_drv_err("Buf_ptr is NULL.\n");
        return LOG_RET_ERROR;
    }

    if (desc->channel_type == LOG_CHANNEL_TYPE_ISP) {
        buf_ptr->buf_read = 0;
        buf_ptr->buf_write = 0;
        buf_ptr->buf_len = channel_info->buf_size;
        buf_ptr->log_level = (u32)(channel_info->log_level);
        buf_ptr->rev[0] = ISP_START_FLAG;
    }
    LOG_DEBUG_BUFFHEAD_SHOW(buf_ptr);

    return LOG_RET_OK;
}

void log_init_conn_func_shm(struct log_channel_desc * log_chan_desc)
{
    if (log_chan_desc == NULL) {
        slog_drv_err("Log_chan_desc is NULL.\n");
        return;
    }

    log_chan_desc->create = NULL;
    log_chan_desc->del = NULL;
    log_chan_desc->set_level = log_chan_desc->log_level_support ? log_send_setlevel_cmd_shm : NULL;
    log_chan_desc->init = log_init_channel_head_shm;
}

