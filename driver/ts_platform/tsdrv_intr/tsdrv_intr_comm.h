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

#ifndef TSDRV_INTR_COMM_H
#define TSDRV_INTR_COMM_H

#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>

#include "devdrv_user_common.h"
#include "devdrv_common.h"
#include "tsdrv_pdata.h"

#define IRQ_DIVISOR 16
#define IRQ_DIVISOR_OFFSET 1

#define IRQ_ENABLE_DIVISOR 32
#define IRQ_DISABLE_DIVISOR 32

#define GICD_IROUTER_REG_OFFSET 3
#define GICD_ICFGR_REG_OFFSET1 4
#define GICD_ICFGR_REG_OFFSET2 2

#define GICD_ENABLE_REG_OFFSET1 5
#define GICD_ENABLE_REG_OFFSET2 2

#define GICD_DISABLE_REG_OFFSET1 5
#define GICD_DISABLE_REG_OFFSET2 2

#define TSDRV_WAIT_GICD_RWP_TIMEOUT 1000000

enum tsdrv_irq_trigger_type {
    TSDRV_IRQ_LEVEL_TRIGGER = 0x1, /* do not change this value */
    TSDRV_IRQ_EDGE_TRIGGER = 0x3, /* do not change this value */
    TSDRV_IRQ_NONE_TRIGGER
};

enum tsdrv_irq_enable {
    TSDRV_IRQ_DISABLE,
    TSDRV_IRQ_ENABLE
};

struct tsdrv_intr_config {
    u32 devid;
    enum tsdrv_cpu_type cpu_type; /* target cpu type for irq */
    u32 cpu_index; /* cpu index for current cpu type, e.g. */
    char *irq_name;
    u32 irq;
    enum tsdrv_irq_trigger_type irq_trigger_type;
    enum tsdrv_irq_enable enable;
    u32 chipid;
    u32 dieid;
};

#define TSDRV_MAX_INTR_ROUTE_NUM    128U

struct tsdrv_intr_info {
    void __iomem *gicv3_vbase;
    size_t gicv3_size;
};

extern struct tsdrv_intr_config *tsdrv_get_intr_config(u32 devid, u32 chipid, u32 dieid);
extern u64 tsdrv_get_irq_affinity(u32 chipid, u32 dieid, u32 sclid, u32 cluster, u32 cpuid);

#endif /* __TSDRV_INTR_COMM_H */
