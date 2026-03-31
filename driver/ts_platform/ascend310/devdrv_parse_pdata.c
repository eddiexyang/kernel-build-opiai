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
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/nodemask.h>

#include "devdrv_user_common.h"
#include "devdrv_parse_pdata.h"
#include "devdrv_common.h"
#include "devdrv_platform.h"
#include "devdrv_pm.h"
#include "devdrv_manager.h"
#include "tsdrv_kernel_common.h"
#include "tsdrv_intr.h"
#include "devdrv_mailbox.h"
#include "tsdrv_parse.h"
#include "tsdrv_log.h"

#define DEVDRV_DEFAULT_AICORE_NUM 2

int tsdrv_get_plat_info(struct devdrv_platform_data *pdata)
{
    u32 __iomem *version_base = NULL;
    u32 reg_val = 0;
    u32 type = 0;
    u32 ver;
    u32 num = 0;
    int ret = 0;

    version_base = (u32 __iomem *)pdata->platform_info.sysctl_base;
    if (version_base == NULL) {
        devdrv_drv_err("sysctl base is NULL.\n");
        return -ENOMEM;
    }
    reg_val = readl_relaxed((void __iomem *)((u64)(uintptr_t)version_base + DEVDRV_SYSCTL_VERSION_REG_OFFSET));
    if (reg_val == 0) {
        pdata->env_type = DEVDRV_PLAT_TYPE_ASIC;
        pdata->ai_core_num = DEVDRV_DEFAULT_AICORE_NUM;
        ret = 0;
        goto out;
    }

    type = (reg_val & DEVDRV_PLAT_MASK) >> 16;
    ver = reg_val & DEVDRV_PLAT_VERSION_MASK;
    num = reg_val & DEVDRV_PLAT_AI_CORE_NUM_MASK;

    if (type == DEVDRV_PLAT_TYPE_EMU) {
        pdata->env_type = DEVDRV_PLAT_TYPE_EMU;
    } else if (type == DEVDRV_PLAT_TYPE_ESL) {
        pdata->env_type = DEVDRV_PLAT_TYPE_ESL;
    } else if (type == DEVDRV_PLAT_TYPE_FPGA) {
        pdata->env_type = DEVDRV_PLAT_TYPE_FPGA;
    } else {
        devdrv_drv_err("invalid plat type, type = %u.\n", type);
        ret = -EINVAL;
        goto out;
    }

    if (num == DEVDRV_PLAT_AI_CORE_NUM_1)
        pdata->ai_core_num = 1;
#ifndef DEVDRV_MANAGER_HOST_UT_TEST
    else if (num == DEVDRV_PLAT_AI_CORE_NUM_2)
        pdata->ai_core_num = DEVDRV_DEFAULT_AICORE_NUM;
#endif
    else {
        devdrv_drv_err("invalid ai core num. num = %u.\n", num);
        ret = -EINVAL;
        goto out;
    }
    ret = 0;

out:
    return ret;
}

u32 devdrv_get_cpu_number(u32 dev_id, u32 cpu_type)
{
    u32 cpu_num = 0;

    switch (cpu_type) {
        case CPU_TYPE_OF_TS:
            /* there is 1 ts cpucore for mini */
            cpu_num = 1;
            break;
        default:
            devdrv_drv_err("invalid cpu type, cpu_type = %u\n", cpu_type);
            break;
    }
    return cpu_num;
}

int tsdrv_get_aicpu_occupy_bitmap(u32 devid, u32 plat_type, u32 *aicpu_bitmap)
{
    struct platform_device *pdev = NULL;

    pdev = tsdrv_get_platform_device(devid);
    if (pdev == NULL) {
        TSDRV_PRINT_ERR("Get platform device failed. (devid=%u)\n", devid);
        return -ENODEV;
    }

    switch (plat_type) {
        case DEVDRV_PLAT_TYPE_EMU:
            return of_property_read_u32(pdev->dev.of_node,
                "aicpu,emu_occupy_bitmap", aicpu_bitmap);
        case DEVDRV_PLAT_TYPE_ESL:
            return of_property_read_u32(pdev->dev.of_node,
                "aicpu,esl_occupy_bitmap", aicpu_bitmap);
        case DEVDRV_PLAT_TYPE_FPGA:
            return of_property_read_u32(pdev->dev.of_node,
                "aicpu,fpga_occupy_bitmap", aicpu_bitmap);
        case DEVDRV_PLAT_TYPE_ASIC:
            return of_property_read_u32(pdev->dev.of_node,
                "aicpu,asic_occupy_bitmap", aicpu_bitmap);
        default:
            TSDRV_PRINT_ERR("Plat type is invalid. (devid=%u; type=%u)\n",
                devid, plat_type);
            return -EINVAL;
    }
}

void tsdrv_dfx_cq_irq_bind_core(struct devdrv_platform_data *pdata)
{
    u32 cpuid;
    u32 ai_cpu_num;
    u32 ctrl_cpu_num;
    u32 irq;

    irq = pdata->ts_pdata[0].irq_functional_cq;
    ai_cpu_num = __fls(pdata->platform_info.occupy_bitmap) - __ffs(pdata->platform_info.occupy_bitmap) + 1;
    ctrl_cpu_num = num_possible_cpus() - ai_cpu_num;
    cpuid = ctrl_cpu_num - 1;

    /* Set irq affinity. */
    (void)devdrv_set_irq_affinity(irq, get_cpu_mask(cpuid));

    return;
}

int devdrv_get_base_addr_info(struct platform_device *pdev, struct devdrv_platform_data *pdata)
{
    struct resource *res = NULL;
    u32 i;

    for (i = 0; i < DEVDRV_DTS_MAX_RESOURCE_NODE; i++) {
        res = platform_get_resource(pdev, IORESOURCE_MEM, i);
        if (res == NULL) {
            devdrv_drv_err("platform_get_resource failed i = %u.\n", i);
            return -EINVAL;
        }
        pdata->platform_info.devdrv_addr_base[i] = res->start;
    }

    return 0;
}
