/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
#ifndef PROF_HDC_MSG_DEF_H
#define PROF_HDC_MSG_DEF_H

#include "prof_user_ker_com.h"

enum prof_hdc_msg_type {
    PROF_HDC_CMD_GET_CHANNEL,
    PROF_HDC_CMD_START,
    PROF_HDC_CMD_STOP,
    PROF_HDC_DATA,
    PROF_HDC_CLOSE_SESSION,
    PROF_HDC_DATA_FLUSH,
    PROF_HDC_CMD_MAX
};

struct prof_hdc_start_para {
    uint32_t channel_type;          /* for ts and other device */
    uint32_t buf_len;               /* buffer size */
    uint32_t sample_period;
    char user_data[PROF_USER_DATA_LEN]; /* ts data */
    uint32_t user_data_size;        /* user data's size */
};

struct prof_hdc_msg {
    int msg_type;
    int ret_val;
    uint32_t channel_id;
    uint32_t data_len;
    unsigned char data[0];
};

#endif
