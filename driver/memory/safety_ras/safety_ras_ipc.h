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
* Create: 2023-9-18
*/

#ifndef SAFETY_RAS_IPC_H
#define SAFETY_RAS_IPC_H

#include <linux/types.h>
#include <linux/notifier.h>

// msg define
#define CMD_NOTIFY             3
#define LP_CPU_ID              4
#define TAISHAN_CPU_ID         0
#define CMD_TYPE_HEALTH_STATE  6

// EVENT ID bits
#define EVENT_ID_ASSERTION_BIT  28U
#define EVENT_ID_SERVERTY_BIT   25U
#define EVENT_ID_MODULE_BIT     17U
#define EVENT_ID_SENSOR_BIT     9U

/* 0:RESUME 1:OCCUR 2:ONE_TIME */
#define MEMORY_EVENT_RESUME     0x0
#define MEMORY_EVENT_OCCUR      0x1
#define MEMORY_EVENT_ONE_TIME   0x2

#define SIZE_CMDPARA 4

struct memory_ipc_info {
	uint32_t dev_id;
	struct notifier_block ipc_notifier;
};
#endif