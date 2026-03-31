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
#ifndef TS_AISLE_API_H
#define TS_AISLE_API_H

#include <linux/types.h>
#include <linux/platform_device.h>

#include "ts_aisle_queue.h"

#define TS_IPC_MSG_MAX_LEN 24 /* 4 bytes as unit */
#define MSG_TYPE_B0 0x1E
#define MSG_TYPE_B1 0x71
#define MSG_TYPE_B2 0
#define MSG_TYPE_B3 0
#define MSG_HEAD_LEN 4 /* 1 byte as unit */

int send_msg_to_ts_async(int dev_id, int ts_id, unsigned int len, const void *msg);
int aisle_init(struct platform_device *pdev, int node_id);
void aisle_uninit(int node_id);

#endif
