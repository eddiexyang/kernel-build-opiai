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
#ifndef TSDRV_HANDSHAKE_H
#define TSDRV_HANDSHAKE_H

#include <linux/types.h>
#include "devdrv_common.h"

int tsdrv_handshake_init(u32 devid, int (*hanshake_handler)(u32 devid, struct tsdrv_msg_info *msg, u32 *ack_len));
void tsdrv_handshake_exit(u32 devid);

#endif /* __TSDRV_HANDSHAKE_H */
