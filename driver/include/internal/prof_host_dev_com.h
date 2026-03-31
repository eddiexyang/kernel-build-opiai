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

#ifndef PROF_HOST_DEV_COM_H
#define PROF_HOST_DEV_COM_H
#include <linux/types.h>
#include "drv_type.h"

typedef struct prof_dev_dfx_info {
    uint32_t data_buf_len;
    uint32_t read_ptr;
    uint32_t write_ptr;
    uint64_t prof_in_total_size;
    uint64_t prof_out_total_size;
    uint32_t prof_cq1_counts_from_ts;
    uint32_t prof_wake_up_poll_count;
    uint32_t prof_read_count;
    uint32_t prof_read_flipped_count;
    uint32_t buf_not_enough_count;
    /* Avoid compatibility issues caused by struct length changing */
    __kernel_long_t reserved[3];
} prof_dev_dfx_info_t;
#endif
