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

#ifndef PROF_USER_KER_COM_H
#define PROF_USER_KER_COM_H
#include "ascend_hal_define.h"
#define PROF_USER_DATA_LEN 128U
typedef struct prof_ioctl_para {
    uint32_t device_id;
    uint32_t vfid;           /* vfid = 0, is physical machine; vfid = 1~16, is virtual machine */
    uint32_t channel_id;
    uint32_t cmd;
    uint32_t buf_len;
    uint32_t sample_period;  /**< Sampling period */
    int ret_val;
    int timeout;
    int poll_number; /**< channel number */
    uint32_t user_data_size;
    uint32_t use_mode;
    char user_data[PROF_USER_DATA_LEN];
    void *out_buf; /**< save return info */
} prof_ioctl_para_t;

enum prof_cmd_type {
    PROF_GET_PLATFORM = 201,
    PROF_GET_DEVNUM,
    PROF_GET_DEVIDS,  // for reserve
    PROF_START,
    PROF_STOP,
    PROF_READ,
    PROF_POLL,
    PROF_GET_CHANNEL_LIST,
    PROF_DATA_FLUSH,
    PROF_CMD_MAX
};

#define PROF_POLL_DEPTH 512U
enum channel_poll_flag {
    POLL_INVALID,
    POLL_VALID
};

/* period time / ms */
#define PROF_PERIOD_MIN 10U    /* 10ms */
#define PROF_PERIOD_MAX 10000U /* 10s */
#endif
