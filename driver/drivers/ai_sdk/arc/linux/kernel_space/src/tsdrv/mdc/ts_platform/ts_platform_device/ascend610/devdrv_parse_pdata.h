/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
#include "devdrv_platform_register.h"
#include "devdrv_platform_resource.h"

#include "devdrv_interface.h"

#define DEVDRV_TSCPU_CORE_NUM 1
#define DEVDRV_PLAT_TYPE_ASIC_AICORE_NUM 10
#define DEVDRV_SLOT_ID_BIT 0x1

#define CPU_INFO_RETRY_TIME 1000
#define DEFAULT_CCPU_CORE_NUM 1
#define DEFAULT_DCPU_CORE_NUM 0
#define DEFAULT_AICPU_CORE_NUM 7
#define CPU_INFO_CONFIGED 0x5A5A


#define SLOT_TYPE_4P        0
#define SLOT_TYPE_NON_4P    1
#define SLOT_ID_OFFSET      1
#ifndef AOS_LLVM_BUILD
#define GPIO_258    334
#define GPIO_262    338
#define GPIO_266    342
#else
#define GPIO_258    258
#define GPIO_262    262
#define GPIO_266    266
#endif

#define MIN_BOARD_ID 900
#define MAX_BOARD_ID 999

struct devdrv_cpu_info {
    int set_flag;
    struct agentdrv_cpu_info cpu_info;
};

u32 devdrv_get_cpu_number(u32 dev_id, u32 cpu_type);
int get_devdrv_affinity_cpuid(u32 dev_id);
int tsdrv_get_specification(struct platform_device *pdev, enum tsdrv_hwts_spec *spec);
#endif /* __DEVDRV_PARSE_PDATA_H__ */
