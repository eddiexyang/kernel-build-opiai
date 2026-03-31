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
* Create: 2023-1-11
*/

#ifndef TSDRV_AIV_SAFETY_H
#define TSDRV_AIV_SAFETY_H

#ifdef CFG_SOC_PLATFORM_MDC_V51

#include "drvfault_user_common.h"
#include "fpdc.h"

/**
 * register fault irq info to fault drv
 *
 * when where are safety interrupt has occur in modules, fault driver
 * will call notifier function @soc_misc_safety_notifier.
 */
int tsdrv_register_aiv_safety_irq(u32 devid);

/**
 * unregister fault irq info to fault drv
 *
 * this function be called when driver exit.
 */
void tsdrv_unregister_aiv_safety_irq(u32 devid);

/**
 * safety irq notifier.
 *
 * this function will be registered to fault driver.
 */
int tsdrv_aiv_safety_handler(struct safety_fault_info *fault_info,
    unsigned int *event_num, struct safety_event **event_list);


#else
static inline int tsdrv_register_aiv_safety_irq(u32 devid)
{
    return 0;
}

static inline void tsdrv_unregister_aiv_safety_irq(u32 devid)
{
}
#endif

const struct ras_fault_converge_item *ts_aiv_safety_converge(u32 section_type, u64 ras_code);
#endif
