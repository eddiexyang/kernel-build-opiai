/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
* Create: 2023-05-25
*/

#ifndef MEMORY_COMMON_H
#define MEMORY_COMMON_H

#include "memory_ddr_cfg.h"
#include "devdrv_ipc.h"
#include "devdrv_manager_common.h"

int32_t memory_dev_get_msg_from_lp(uint32_t dev_id, struct devdrv_ipc_imu *ipc, struct devdrv_ipc_imu *msg_ret,
	uint32_t msg_len);
int32_t memory_dev_check_lp_msg_validity(struct devdrv_ipc_imu *lp_msg);

#endif
