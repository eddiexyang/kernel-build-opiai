/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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

#ifndef __TSMNG_COMMON_H__
#define __TSMNG_COMMON_H__

#include "tsmng_interface.h"

#define IPC_RETRY_TIME  3

int tsmng_ipc_chan_proc(u32 cmd_type, void *data, u32 dev_id);
int tsmng_ipc_handler_register(u32 dev_id, u32 cmd_type, tsmng_handler handler);
void tsmng_ipc_handler_unregister(u32 dev_id, u32 cmd_type);

#endif  /* __TSMNG_COMMON_H__ */
