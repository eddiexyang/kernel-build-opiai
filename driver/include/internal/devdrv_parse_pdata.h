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
#ifndef DEVDRV_PARSE_PDATA_H
#define DEVDRV_PARSE_PDATA_H

#include <linux/platform_device.h>

#include "devdrv_interface.h"
#include "devdrv_platform_register.h"
#include "devdrv_platform_resource.h"

#define CPU_INFO_CONFIGED 0x5A5A

struct devdrv_cpu_info {
    int set_flag;
    struct agentdrv_cpu_info cpu_info;
};

u32 devdrv_get_cpu_number(u32 dev_id, u32 cpu_type);

#endif /* __DEVDRV_PARSE_PDATA_H__ */
