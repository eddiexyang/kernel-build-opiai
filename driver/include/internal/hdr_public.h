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

#ifndef HDR_PUBLIC_H
#define HDR_PUBLIC_H

#include "hdr_log.h"
#include "hdr_status.h"

/**
 *  the whole space is 512k, used for history data record
 *  the struct distribution is as follows:
 *  +-----------------------+
 *  | hdr log head(1k)      |
 *  +-----------------------+
 *  | boot log region(300k) |
 *  +-----------------------+
 *  | run log region(150k)  |
 *  +-----------------------+
 *  | hdr status head(1k)   |
 *  +-----------------------+
 *  | status region(30k)    |
 *  +-----------------------+
 *  | reserved(30k)         |
 *  +-----------------------+
 */

#define HDR_LOG_HEAD_OFFSET                 0x0U
#define HDR_LOG_HEAD_SIZE                   0x400U   // 1KB
#define HDR_LOG_BOOT_DATA_OFFSET            (HDR_LOG_HEAD_OFFSET + HDR_LOG_HEAD_SIZE)
#define HDR_LOG_BOOT_DATA_SIZE              0x4B000U // 300KB
#define HDR_LOG_RUN_DATA_OFFSET             (HDR_LOG_BOOT_DATA_OFFSET + HDR_LOG_BOOT_DATA_SIZE)
#define HDR_LOG_RUN_DATA_SIZE               0x25800U // 150KB
#define HDR_STATUS_HEAD_OFFSET              (HDR_LOG_RUN_DATA_OFFSET + HDR_LOG_RUN_DATA_SIZE)
#define HDR_STATUS_HEAD_SIZE                0x400U   // 1KB
#define HDR_STATUS_DATA_OFFSET              (HDR_STATUS_HEAD_OFFSET + HDR_STATUS_HEAD_SIZE)
#define HDR_STATUS_DATA_SIZE                0x7800U  // 30KB
#define HDR_RESERVE_OFFSET                  (HDR_STATUS_DATA_OFFSET + HDR_STATUS_DATA_SIZE)
#define HDR_RESERVE_SIZE                    0x7800U  // 30KB

struct hdr_log_data {
    struct hdr_log_head head;
    u8 padding[HDR_LOG_HEAD_SIZE - sizeof(struct hdr_log_head)];
    char boot_region[HDR_LOG_BOOT_DATA_SIZE];
    char run_region[HDR_LOG_RUN_DATA_SIZE];
};

struct hdr_status_data {
    struct hdr_status_head head;
    u8 padding[HDR_STATUS_HEAD_SIZE - sizeof(struct hdr_status_head)];
    struct status_block block[STATUS_AREA_MAX_NUM][STATUS_BLOCK_MAX_NUM];
};

struct hdr_data {
    struct hdr_log_data log_data;
    struct hdr_status_data status;
    char reserve[HDR_RESERVE_SIZE];
};

#endif

