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

#include <linux/types.h>
#include <linux/module.h>
#include <linux/delay.h>
#ifndef AOS_LLVM_BUILD
#include <linux/irqchip/arm-gic-v3.h>
#endif
#include "tsdrv_intr_comm.h"
#include "tsdrv_log.h"

/* maximum device number is 2 for mini_v2 */
STATIC struct tsdrv_intr_config miniv2_intr_config[DEVDRV_MAX_DAVINCI_NUM][TSDRV_MAX_INTR_ROUTE_NUM] = {
    {
        /* end of array */
        {0, TSDRV_INVALID, -1, NULL, -1, TSDRV_IRQ_NONE_TRIGGER, TSDRV_IRQ_DISABLE},
    },
    {
        /* end of array */
        {1, TSDRV_INVALID, -1, NULL, -1, TSDRV_IRQ_NONE_TRIGGER, TSDRV_IRQ_DISABLE},
    }
};

struct tsdrv_intr_config *tsdrv_get_intr_config(u32 devid, u32 chipid, u32 dieid)
{
    return miniv2_intr_config[devid];
}

u64 tsdrv_get_irq_affinity(u32 chipid, u32 dieid, u32 sclid, u32 cluster, u32 cpuid)
{
#ifndef TSDRV_UT
    return 0;
#endif
}
