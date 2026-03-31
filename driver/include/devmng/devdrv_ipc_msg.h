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


#ifndef __DEVDRV_MANAGER_MSG__HOST_H
#define __DEVDRV_MANAGER_MSG__HOST_H

#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/fs.h>

#include "devdrv_common.h"
#include "drv_ipc.h"

void devdrv_ipc_aicpu_register(u32 dev_id);
int devdrv_ipc_msg_send_recv(struct devdrv_info *dev_info, unsigned long arg);

int devdrv_ipc_msg_send_async(struct devdrv_info *dev_info, unsigned long arg);


#endif
