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

#ifndef TS_DRV_CMD_H
#define TS_DRV_CMD_H

#include "ascend_hal_define.h"

#define TS_CHAR_DEV_NAME "ts_aisle"
#define TS_CHAR_DEV_FULL_NAME "/dev/ts_aisle"

#define TS_CMD_MAX_NR                  (10)
#define TS_ID_MAGIC 'W'
#define TS_SEND_MSG_TO_TS_ASYNC_ID _IOWR_BAD(TS_ID_MAGIC, 1, sizeof(struct ts_ioctl_para))
#define AICPU_RECORD_MSG_ID _IOR_BAD(TS_ID_MAGIC, 2, sizeof(struct ts_ioctl_para))

typedef struct ts_aicpu_record {
    unsigned int record_type;
    unsigned int record_id;
} ts_aicpu_record;

typedef struct ts_aicpu_msg {
    unsigned int msg_len;
    char *msg;
} ts_aicpu_msg;

struct ts_ioctl_para {
    unsigned int dev_id;
    unsigned int ts_id;
    union {
        ts_aicpu_record aicpu_record;
        ts_aicpu_msg aicpu_msg;
    }u;
};

#endif
