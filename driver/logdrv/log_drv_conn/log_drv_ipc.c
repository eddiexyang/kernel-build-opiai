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

#include "log_drv_dev.h"
#include "drv_ipc.h"
#include "log_drv_ipc.h"

/*
 * communicate with other subsystem adapt
 */
static inline void log_set_level_in_icmdrv_ipc_msg(union log_ipc_msg *msg, u8 level)
{
    msg->log_icm_ipc_msg.data[0] = level;
}

static inline void log_set_level_in_devdrv_ipc_lpm3(union log_ipc_msg *msg, u8 level)
{
    msg->log_devdrv_ipc_msg.cmd_para0 = level;
}

static inline u32 log_get_icmfd_by_devid(u8 devid, s32 mailbox)
{
    return ICM_FD_BUILD(devid, (u32)mailbox);
}

static inline u32 log_get_icmfd(u8 devid, s32 mailbox)
{
    UNUSED(devid);
    return ICM_FD_BUILD(0, (u32)mailbox);
}

static inline u32 log_get_devdrv_ipc_msg_len(void)
{
    return sizeof(struct devdrv_ipc_lpm3) / sizeof(int);
}

static inline u32 log_get_icmdrv_ipc_msg_len(void)
{
    return sizeof(struct icmdrv_ipc_msg) / sizeof(int);
}

static inline s32 log_get_mailbox_id(void)
{
    s32 mailbox_id = 0;
#ifdef CFG_SOC_PLATFORM_MINI
    mailbox_id = (s32)HISI_RPROC_TX_LPM3;
#elif defined (CFG_SOC_PLATFORM_MINIV2)
    mailbox_id = (s32)HISI_RPROC_LP_Q_TX_RPID4_ACPU0;
#elif defined (CFG_SOC_PLATFORM_CLOUD_V2)
    mailbox_id = (s32)HISI_RPROC_TX_IMU_MBX28;
#elif defined (CFG_SOC_PLATFORM_MINIV3)
    mailbox_id = (s32)HISI_RPROC_TX_LP_ACPU0;
#endif
    return mailbox_id;
}

static struct log_ipc_cfg log_ipc_cfg_info[HISI_CHIP_NUM] = {
    [HISI_MINI_V1] = {
        .is_support = true,
        .msg.log_devdrv_ipc_msg = {0, LPM3_LOGLEVEL_CMD, LPM3_LOG_ID, LPM3_ID},
        .get_ipc_len = log_get_devdrv_ipc_msg_len,
        .get_ipc_fd = log_get_icmfd,
        .set_log_level = log_set_level_in_devdrv_ipc_lpm3,
    },
    [HISI_CLOUD_V1] = {0},
    [HISI_MINI_V2] = {
        .is_support = true,
        .msg.log_devdrv_ipc_msg = {LPR52_TYPE1, LPR52_LOGLEVEL_CMD, LPR52_TARGET_ID, LPR52_SOURECE_ID},
        .get_ipc_len = log_get_devdrv_ipc_msg_len,
        .get_ipc_fd = log_get_icmfd,
        .set_log_level = log_set_level_in_devdrv_ipc_lpm3,
    },
    [HISI_CLOUD_V2] = {
        .is_support = true,
        .msg.log_icm_ipc_msg = {
            ICM_SUB_CMD_LOG, ICM_MAIN_CMD_LP_SET, OBJ_CMD_LP, OBJ_CMD_CCPU, MSGTYPE_DRIVER_SEND, 1
        },
        .get_ipc_len = log_get_icmdrv_ipc_msg_len,
        .get_ipc_fd = log_get_icmfd_by_devid,
        .set_log_level = log_set_level_in_icmdrv_ipc_msg,
    },
    [HISI_MINI_V3] = {
        .is_support = true,
        .msg.log_icm_ipc_msg = {
            ICM_SUB_CMD_LOG, ICM_MAIN_CMD_LP_SET, OBJ_CMD_LP, OBJ_CMD_CCPU, MSGTYPE_DRIVER_SEND, 1
        },
        .get_ipc_len = log_get_icmdrv_ipc_msg_len,
        .get_ipc_fd = log_get_icmfd_by_devid,
        .set_log_level = log_set_level_in_icmdrv_ipc_msg,
    },
};

static s32 log_send_ipc(struct log_ipc_cfg *ipc_cfg, s32 device_id, u8 log_level)
{
    union log_ipc_msg *ipc_msg = &(ipc_cfg->msg);
    u32 ipc_len, ipc_fd;
    s32 mailbox_id;

    mailbox_id = log_get_mailbox_id();
    ipc_fd = ipc_cfg->get_ipc_fd(device_id, mailbox_id);
    ipc_len = ipc_cfg->get_ipc_len();
    ipc_cfg->set_log_level(ipc_msg, log_level);

    return icm_msg_send_async(ipc_fd, (rproc_msg_t *)ipc_msg, ipc_len);
}

s32 log_send_setlevel_cmd_ipc(struct log_device_ctx *device_ctx,
    struct log_channel_info *channel_info, s32 log_level)
{
    const struct log_channel_desc *log_chan_desc = channel_info->log_desc;
    s32 device_id = device_ctx->device_info.device_id;
    u32 chip_type;
    s32 ret;

    chip_type = uda_get_chip_type((u32)device_id);
    if (chip_type >= HISI_CHIP_NUM) {
        slog_drv_err("Unknown chip type. (chip_type=%u, device_id=%d)\n", chip_type, device_id);
        return LOG_RET_ERROR;
    }

    if (log_ipc_cfg_info[chip_type].is_support == false) {
        slog_drv_err("No support ipc. (chip_type=%u)\n", chip_type);
        return LOG_RET_ERROR;
    }

    if (log_chan_desc->channel_conn != LOG_CHANNEL_CONN_IPC) {
        slog_drv_err("Connect type was unmatched. "
            "(device_id=%d; channel_conn=%d)\n", device_id, log_chan_desc->channel_conn);
        return LOG_RET_ERROR;
    }

    LOG_DEBUG_CHANINFO_SHOW(channel_info);
    ret = log_send_ipc(&log_ipc_cfg_info[chip_type], device_id, (u8)log_level);
    if (ret != LOG_RET_OK) {
        slog_drv_err("Ipc message send failed. (device_id=%d)\n", device_id);
        return LOG_RET_ERROR;
    }

    channel_info->log_level = log_level;
    slog_drv_info("Log_send_setlevel_cmd_ipc succeeded. (device_id=%d; channel_id=%d; log_level=%d)\n",
        device_id, log_chan_desc->channel_ids, log_level);

    return LOG_RET_OK;
}

#if (!defined (CFG_SOC_PLATFORM_CLOUD))
STATIC s32 log_init_channel_head_ipc(struct log_device_ctx *device_ctx, struct log_channel_info *channel_info)
{
    struct log_channel_buf_head *buf_ptr = (struct log_channel_buf_head *)(channel_info->vir_addr);

    if (buf_ptr == NULL) {
        slog_drv_err("Buf_ptr is NULL.\n");
        return LOG_RET_ERROR;
    }

#ifndef CFG_FEATURE_HISTORY_CONFIG
    buf_ptr->buf_read = 0;
#endif
    buf_ptr->buf_len = channel_info->buf_size;
    LOG_DEBUG_BUFFHEAD_SHOW(buf_ptr);

    return LOG_RET_OK;
}
#endif

void log_init_conn_func_ipc(struct log_channel_desc * log_chan_desc)
{
    if (log_chan_desc == NULL) {
        slog_drv_err("Log_chan_desc is NULL.\n");
        return;
    }

    log_chan_desc->create = NULL;
    log_chan_desc->del = NULL;

#if defined (CFG_SOC_PLATFORM_CLOUD)
    log_chan_desc->set_level = NULL;
    log_chan_desc->init = NULL;
#else
    log_chan_desc->set_level = log_chan_desc->log_level_support ? log_send_setlevel_cmd_ipc : NULL;
    log_chan_desc->init = log_init_channel_head_ipc;
#endif
}
