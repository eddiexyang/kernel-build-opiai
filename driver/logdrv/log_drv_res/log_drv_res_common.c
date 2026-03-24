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
#include "log_drv_ipc.h"
#include "log_drv_shm.h"
#include "log_drv_sqcq.h"
#include "log_drv_res.h"
#include "log_drv_res_array.h"
#include "log_drv_res_common.h"
#include "devdrv_interface.h"

struct log_channel_desc *log_get_channel_desc(void)
{
    return log_get_channel_desc_array();
}

s32 log_get_channel_num(void)
{
    return log_get_channel_num_array();
}

s32 log_is_channel_valid(s32 channel_id)
{
    s32 i;
    s32 channel_sets_num = log_get_channel_num();
    struct log_channel_desc *log_chan_desc = log_get_channel_desc();

    if ((channel_id < 0) || (channel_id >= (s32)LOG_CHANNEL_TYPE_MAX) || (channel_sets_num == 0)) {
        slog_drv_err("Channel_id out of range or channel_sets_num is invalid. "
            "(channel_id=%d; Channel_id_range=\"0-%d\"; channel_sets_num=%d)\n",
            channel_id, LOG_CHANNEL_TYPE_MAX, channel_sets_num);
        return LOG_RET_ERROR;
    }

    for (i = 0; i < channel_sets_num; i++) {
        if (channel_id == log_chan_desc[i].channel_ids) {
            return LOG_RET_OK;
        }
    }

    slog_drv_err("Channel_id was unmatched. (channel_id=%d; channel_sets_num=%d)\n", channel_id, channel_sets_num);
    return LOG_RET_ERROR;
}

void log_init_channel_desc(void)
{
    s32 i;
    s32 channel_sets_num = log_get_channel_num();
    struct log_channel_desc *log_chan_desc = log_get_channel_desc();

    if (channel_sets_num == 0) {
        slog_drv_err("Channel_num is zero.\n");
        return;
    }

    slog_drv_info("Channel number. (channel_sets_num=%d)\n", channel_sets_num);
    for (i = 0; i < channel_sets_num; i++) {
        switch (log_chan_desc[i].channel_conn) {
            case LOG_CHANNEL_CONN_IPC:
                log_init_conn_func_ipc(&log_chan_desc[i]);
                break;
            case LOG_CHANNEL_CONN_SHAREMEM:
                log_init_conn_func_shm(&log_chan_desc[i]);
                break;
            case LOG_CHANNEL_CONN_SQCQ:
                log_init_conn_func_sqcq(&log_chan_desc[i]);
                break;
            default:
                slog_drv_err("Show channel connect type. (channel_conn=%d; i=%d)\n",
                    log_chan_desc[i].channel_conn, i);
                break;
        }
    }
}

s32 log_channel_id_shift(u32 device_id, u32 tsid, s32 channel_id, s32 *set_level_channel_type)
{
    u32 chip_type;
    enum log_channel_type tmp_channel_id = (enum log_channel_type)channel_id;

    chip_type = uda_get_chip_type(device_id);
    if ((tmp_channel_id != LOG_CHANNEL_TYPE_TS) && (tmp_channel_id != LOG_CHANNEL_TYPE_TS_DUMP)) {
        return (s32)LOG_CHANNEL_TYPE_MAX;
    }

    switch (tmp_channel_id) {
        case LOG_CHANNEL_TYPE_TS:
            *set_level_channel_type = LOG_TS_CMD_TYPE;
            if ((chip_type == HISI_MINI_V2) && (tsid == LOG_DC_TS_ID)) {
                tmp_channel_id = LOG_CHANNEL_TYPE_TS0;
            } else if ((chip_type == HISI_MINI_V2) && (tsid == LOG_MDC_TS_ID)) {
                tmp_channel_id = LOG_CHANNEL_TYPE_TS1;
            }
            break;
        case LOG_CHANNEL_TYPE_TS_DUMP:
            *set_level_channel_type = LOG_TS_DUMP_CMD_TYPE;
            if ((chip_type == HISI_MINI_V2) && (tsid == LOG_DC_TS_ID)) {
                tmp_channel_id = LOG_CHANNEL_TYPE_TS0_DUMP;
            } else if ((chip_type == HISI_MINI_V2) && (tsid == LOG_MDC_TS_ID)) {
                tmp_channel_id = LOG_CHANNEL_TYPE_TS1_DUMP;
            }
            break;
        default:
            slog_drv_err("Unknown channel_id. (channel_id=%u; tsid=%u)\n", channel_id, tsid);
            tmp_channel_id = LOG_CHANNEL_TYPE_MAX;
            break;
    }

    return (s32)tmp_channel_id;
}

STATIC s32 log_shift_cmd_to_ts_normal(const struct log_sq_scheduler *sq_info, s32 *set_level_channel_type)
{
    s32 ret = LOG_RET_OK;

    switch (sq_info->channel_id) {
        case LOG_CHANNEL_TYPE_TS:
            *set_level_channel_type = LOG_TS_CMD_TYPE;
            break;
        case LOG_CHANNEL_TYPE_TS_DUMP:
            *set_level_channel_type = LOG_TS_DUMP_CMD_TYPE;
            break;
        default:
            ret = (s32)LOG_RET_ERROR;
            slog_drv_err("Unknown channel_id. (channel_id=%u)\n", sq_info->channel_id);
            break;
    }

    return ret;
}

STATIC s32 log_shift_cmd_to_ts_mini_v2(struct log_sq_scheduler *sq_info, s32 *set_level_channel_type)
{
    s32 ret = LOG_RET_OK;
    switch (sq_info->channel_id) {
        case LOG_CHANNEL_TYPE_TS0:
            sq_info->channel_id = LOG_CHANNEL_TYPE_TS;
            *set_level_channel_type = LOG_TS_CMD_TYPE;
            break;
        case LOG_CHANNEL_TYPE_TS1:
            sq_info->channel_id = LOG_CHANNEL_TYPE_TS;
            *set_level_channel_type = LOG_TS_CMD_TYPE;
            break;
        case LOG_CHANNEL_TYPE_TS0_DUMP:
            sq_info->channel_id = LOG_CHANNEL_TYPE_TS_DUMP;
            *set_level_channel_type = LOG_TS_DUMP_CMD_TYPE;
            break;
        case LOG_CHANNEL_TYPE_TS1_DUMP:
            sq_info->channel_id = LOG_CHANNEL_TYPE_TS_DUMP;
            *set_level_channel_type = LOG_TS_DUMP_CMD_TYPE;
            break;
        default:
            ret = (s32)LOG_RET_ERROR;
            slog_drv_err("Unknown channel_id. (channel_id=%u)\n", sq_info->channel_id);
            break;
    }
    return ret;
}

s32 log_shift_cmd_to_ts(u32 device_id, struct log_sq_scheduler *sq_info, s32 *set_level_channel_type)
{
    u32 chip_type;
    chip_type = uda_get_chip_type(device_id);
    if (chip_type == HISI_MINI_V2) {
        return log_shift_cmd_to_ts_mini_v2(sq_info, set_level_channel_type);
    } else {
        return log_shift_cmd_to_ts_normal(sq_info, set_level_channel_type);
    }
}

