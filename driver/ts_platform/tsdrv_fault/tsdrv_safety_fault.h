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

#ifndef TSDRV_SAFETY_FAULT_H
#define TSDRV_SAFETY_FAULT_H

#ifdef CFG_SOC_PLATFORM_MINIV2

#include "drvfault_user_common.h"
#include "tsdrv_fault_init.h"
#include <linux/types.h>

int tsdrv_chk_safety_param(const struct safety_fault_info *safety_fault, unsigned int *event_num);

const struct ras_fault_converge_item *tsdrv_safety_converge(
    const struct ras_fault_converge_item *items, u32 items_num,
    u32 section_type, u64 ras_code);

int  tsdrv_register_safety_irq(u32 devid);
void tsdrv_unregister_safety_irq(u32 devid);

#else

static inline int  tsdrv_register_safety_irq(u32 devid)
{
    return 0;
}

static inline void tsdrv_unregister_safety_irq(u32 devid)
{
}

#endif

#endif
