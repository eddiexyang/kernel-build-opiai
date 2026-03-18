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
#ifndef LOG_DRV_RES_ARRAY_H
#define LOG_DRV_RES_ARRAY_H

#include "log_drv_res.h"
#include "log_drv_dev.h"

struct log_channel_desc log_channel_sets[] = {
    {   .channel_type = LOG_CHANNEL_TYPE_TS,
        .channel_conn = LOG_CHANNEL_CONN_SQCQ,
        .channel_ids = LOG_CHANNEL_TYPE_TS,
        .channel_ids_num = 1,
        .log_level_support = 1,
        .buf_phy_addr = 0,  // dynamic alloc
        .buf_size = LOG_TS_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_DEBUG,
        .log_type = LOG_TYPE_RUN,
        .reserve = 0
    },
    {   .channel_type = LOG_CHANNEL_TYPE_TS_DUMP,
        .channel_conn = LOG_CHANNEL_CONN_SQCQ,
        .channel_ids = LOG_CHANNEL_TYPE_TS_DUMP,
        .channel_ids_num = 1,
        .log_level_support = 1,
        .buf_phy_addr = 0,  // dynamic alloc
        .buf_size = LOG_TS_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_DEBUG,
        .log_type = LOG_TYPE_RUN,
        .reserve = 0
    },
    {   .channel_type = LOG_CHANNEL_TYPE_LPM3,
        .channel_conn = LOG_CHANNEL_CONN_IPC,
        .channel_ids = LOG_CHANNEL_TYPE_LPM3,
        .channel_ids_num = 1,
        .log_level_support = 1,
        .buf_phy_addr = 0xa00000,
        .buf_size = LOG_LPM3_BUFFER_LEN,
        .channel_state = LOG_CHANNEL_ENABLE,
        .log_level = LOG_CHANNEL_LEVEL_DEBUG,
        .log_type = LOG_TYPE_RUN
    },
};

static inline struct log_channel_desc *log_get_channel_desc_array(void)
{
    return log_channel_sets;
}

static inline s32 log_get_channel_num_array(void)
{
    return sizeof(log_channel_sets) / sizeof(struct log_channel_desc);
}

#endif

