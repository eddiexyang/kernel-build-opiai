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
* Create: 2022-11-15
*/

#ifndef SOC_MISC_GIC_AO_H
#define SOC_MISC_GIC_AO_H

#include "drvfault_user_common.h"
#include "soc_misc_safety_conf.h"

/*
 * src id, from EMU_SUBS sheet, interrupt number
 */
#define GIC_AO_INT_SAFETY_BIT_ID 4
#define GIC_AO_ERR_SAFETY_BIT_ID 20

#define GIC_AO_REG_BASE (0x85100000U)

/**
 * safety irq notifier.
 *
 * this function will be registered to fault driver.
 */
int soc_gic_ao_safety_handler(struct safety_fault_info *fault_info,
    unsigned int *event_num, struct safety_event **event_list);

extern struct soc_misc_safety_cfg g_gic_ao_safety_cfg;

#endif // SOC_MISC_GIC_AO_H
