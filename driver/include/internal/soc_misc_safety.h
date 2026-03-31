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

#ifndef SOC_MISC_SAFETY_H
#define SOC_MISC_SAFETY_H

/**
 * there are 4 cup cluster in MDC
 */
#define SOC_CPU_CLUSTER_0     0
#define SOC_CPU_CLUSTER_1     1
#define SOC_CPU_CLUSTER_2     2
#define SOC_CPU_CLUSTER_3     3

#ifdef CFG_FEATURE_SAFETY_MANAGER
#include "drvfault_common.h"
#include "fpdc.h"

#define BIT_CLEAR_WITH_0 0
#define BIT_CLEAR_WITH_1 1
#define BIT_MASK_WITH_0 0
#define BIT_MASK_WITH_1 1
#define BIT_ENABLE_WITH_0 0
#define BIT_ENABLE_WITH_1 1

struct soc_safety_map {
    unsigned int irq_status;
    u8 safety_code;
};

#define SOC_MISC_IRQ_STATUS(src_id, bit_id) ((src_id << 0x10) & bit_id)

/**
 * register fault irq info to fault drv
 *
 * when where are safety interrupt has occur in modules, fault driver
 * will call notifier function @soc_misc_safety_notifier.
 */
int soc_misc_init_safety_irq(void);

void soc_misc_uninit_safety_irq(void);

/**
 * safety fault irq handler
 *
 * This interface is registered with the faultmng. After receiving the fault
 * information, the faultmng reads the fault status register and sends the
 * value of the fault status register to the driver through this interface.
 */
int soc_misc_subsys_irq_handler(struct safety_fault_info *fault_info,
    unsigned int *event_num, struct safety_event **event_list);

u8 soc_misc_get_node_type(u8 sensor_id);

#else
static inline int soc_misc_init_safety_irq(void)
{
    return 0;
}

static inline void soc_misc_uninit_safety_irq(void)
{
}
#endif

void soc_misc_report_ras_fault(u32 devid, u8 cluster_id, u8 sensor_id, u32 ras_ierr);

#endif
