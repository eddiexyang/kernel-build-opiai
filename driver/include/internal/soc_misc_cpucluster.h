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

#ifndef SOC_MISC_CPUCLUSTER_SAFETY_H
#define SOC_MISC_CPUCLUSTER_SAFETY_H

#include "drvfault_user_common.h"
#include "soc_misc_safety_conf.h"
/*
 * src id, from EMU_SUBS sheet, interrupt number
 */
#define CPU_CLUSTER_INT_SAFETY_BIT_ID 7
#define CPU_CLUSTER_ERR_SAFETY_BIT_ID 23

#define CPU_CLUSTER0_REG_BASE (0x811B0000U)
#define CPU_CLUSTER1_REG_BASE (0x811C0000U)
#define CPU_CLUSTER2_REG_BASE (0x811D0000U)
#define CPU_CLUSTER3_REG_BASE (0x811E0000U)

/**
 * safety irq notifier.
 *
 * this function will be registered to fault driver.
 */
int soc_cpucluster_safety_handler(struct safety_fault_info *fault_info,
    unsigned int *event_num, struct safety_event **event_list);

extern struct soc_misc_safety_cfg g_cpucluster_safety_cfg;

#endif
