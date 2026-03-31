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

#ifndef __DAVINCI_INTF_INFO_H__
#define __DAVINCI_INTF_INFO_H__

#include <linux/types.h>
#include "devdrv_interface.h"

int davinci_intf_get_cpu_info(u32 dev_id, struct agentdrv_cpu_info *cpu_info);

#endif