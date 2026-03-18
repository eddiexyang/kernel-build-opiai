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
 * Create: 2023-01-03
 */

#ifndef SOC_MISC_CPUCORE_H
#define SOC_MISC_CPUCORE_H

#include <linux/types.h>
#include "dms_node_type.h"
#include "dms_define.h"

int soc_misc_cpucore_register(u32 devid);
void soc_misc_cpucore_unregister(void);

#endif
