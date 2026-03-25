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

#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/property.h>
#include <linux/of_irq.h>
#include <linux/acpi.h>

#include "devdrv_platform.h"
#include "tsdrv_log.h"
#include "tsdrv_pdata.h"

#define ADDR_MAX_INDEX 8

static const char *acpi_addr_name[ADDR_MAX_INDEX] = {
    "GICV3_base",
    "Dispatch_base",
    "SYSCTL_base",
    "MN_base",
    "LPM3_STATUS_base",
    "FW_CORE_RESET_base",
    "FW_CL_SUBCTRL_base",
    "TS_FW_CORE_RESET_base",
};

static const char *acpi_size_name[ADDR_MAX_INDEX] = {
    "GICV3_size",
    "Dispatch_size",
    "SYSCTL_size",
    "MN_size",
    "LPM3_STATUS_size",
    "FW_CORE_RESET_size",
    "FW_CL_SUBCTRL_size",
    "TS_FW_CORE_RESET_size",
};

struct platform_device *g_platform_dev[DEVDRV_MAX_DAVINCI_NUM] = { NULL };

void tsdrv_set_platform_device(u32 devid, struct platform_device *pdev)
{
    if (devid >= DEVDRV_MAX_DAVINCI_NUM) {
        TSDRV_PRINT_ERR("Device id is invalid. (devid=%u)\n", devid);
        return;
    }

    g_platform_dev[devid] = pdev;
}

struct platform_device *tsdrv_get_platform_device(u32 devid)
{
    if (devid >= DEVDRV_MAX_DAVINCI_NUM) {
        TSDRV_PRINT_ERR("Device id is invalid. (devid=%u)\n", devid);
        return NULL;
    }

    return g_platform_dev[devid];
}

int tsdrv_get_acpi_addr_by_index(u32 devid, u32 addr_index,
    phys_addr_t *paddr, size_t *size)
{
#ifndef TSDRV_UT
    int ret;
    u64 read_size;
    struct platform_device *pdev = NULL;

    if (addr_index >= ADDR_MAX_INDEX) {
        TSDRV_PRINT_ERR(" index is invalid. (idx=%u).\n", addr_index);
        return -EINVAL;
    }

    pdev = tsdrv_get_platform_device(devid);
    if (pdev == NULL) {
        TSDRV_PRINT_ERR("Get platform device failed. (devid=%u)\n", devid);
        return -ENODEV;
    }

    ret = device_property_read_u64(&pdev->dev, acpi_addr_name[addr_index], paddr);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Read device property failed. (ret=%d; index=%u)\n", ret, addr_index);
        return ret;
    }

    ret = device_property_read_u64(&pdev->dev, acpi_size_name[addr_index], &read_size);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Read device property failed. (ret=%d; index=%u)\n", ret,  addr_index);
        return ret;
    }

    *size = (size_t)read_size;
    TSDRV_PRINT_DEBUG("Get addr info. (devid=%u; name=%s; addr=%pK; size=%llx)\n",
        devid, acpi_addr_name[addr_index], (void *)*paddr, read_size);
#endif
    return 0;
}

int tsdrv_get_dts_addr_by_index(u32 devid, u32 addr_index,
    phys_addr_t *paddr, size_t *size)
{
    struct resource *res = NULL;
    struct platform_device *pdev = NULL;

    pdev = tsdrv_get_platform_device(devid);
    if (pdev == NULL) {
        TSDRV_PRINT_ERR("Get platform device failed. (devid=%u)\n", devid);
        return -ENODEV;
    }

    res = platform_get_resource(pdev, IORESOURCE_MEM, addr_index);
    if (res == NULL) {
        TSDRV_PRINT_ERR("Get platform resource failed.\n");
        return -EINVAL;
    }

    *paddr = res->start;
    *size = res->end - res->start + 1;
    TSDRV_PRINT_DEBUG("Get addr info. (devid=%u; addr_index=%u; paddr=%pK; size=0x%lx). \n",
        devid, addr_index, (void *)*paddr, *size);
    return 0;
}

int tsdrv_get_addr_by_index(u32 devid, u32 addr_index,
    phys_addr_t *paddr, size_t *size)
{
    if (!acpi_disabled) {
        return tsdrv_get_acpi_addr_by_index(devid, addr_index, paddr, size);
    } else {
        return tsdrv_get_dts_addr_by_index(devid, addr_index, paddr, size);
    }
}

u32 tsdrv_get_cpu_sclid(u32 devid)
{
    return DEVDRV_SCLID;
}

u32 tsdrv_get_cpu_cluster(u32 devid, enum tsdrv_cpu_type cpu_type)
{
    switch (cpu_type) {
        case TSDRV_CCPU:
            return DEVDRV_CCPU_CLUSTER;
        case TSDRV_AICPU:
            return DEVDRV_AICPU_CLUSTER;
        case TSDRV_TSCPU:
            return DEVDRV_TSCPU_CLUSTER;
        default:
            TSDRV_PRINT_ERR("Cpu type is invalid. (type=%u)\n", cpu_type);
            break;
    }

    return TSDRV_INVALID_CPU_CLUSTER;
}
#ifndef AOS_LLVM_BUILD
STATIC struct irq_data *get_top_parent_irq_data(u32 irq)
{
    struct irq_data *irq_data = NULL;
    struct irq_data *parent = NULL;

    irq_data = irq_get_irq_data(irq);
    if (irq_data == NULL) {
        TSDRV_PRINT_ERR("Irq is invalid. (irq=%u)\n", irq);
        return NULL;
    }
    parent = irq_data->parent_data;

    while (parent != NULL) {
        irq_data = parent;
        parent = irq_data->parent_data;
    }
    return irq_data;
}
#endif

int tsdrv_get_hwirq_from_irq(u32 irq, u32 *hwirq)
{
#ifndef AOS_LLVM_BUILD
    struct irq_data *data = NULL;

    data =  get_top_parent_irq_data(irq);
    if (data == NULL) {
        TSDRV_PRINT_ERR("Get irq data failed. (irq=%u)\n", irq);
        return -EINVAL;
    }

    *hwirq = (u32)data->hwirq;
    TSDRV_PRINT_DEBUG("Irq to hwirq. (irq=%u; hwirq=%lu). \n", irq, data->hwirq);
#else
    *hwirq = irq; // AOS-CORE������һ��
#endif
    return 0;
}
