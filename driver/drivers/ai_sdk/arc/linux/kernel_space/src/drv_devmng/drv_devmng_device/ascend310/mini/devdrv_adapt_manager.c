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

#ifdef CONFIG_GENERIC_BUG
#undef CONFIG_GENERIC_BUG
#endif
#ifdef CONFIG_BUG
#undef CONFIG_BUG
#endif

#include <linux/types.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>

#include "devdrv_common.h"
#include "devdrv_platform.h"
#include "devdrv_pm.h"
#include "devdrv_manager.h"
#include "devdrv_platform_resource.h"
#include "devdrv_platform_register.h"
#include "devdrv_adapt_manager.h"

#define DEVDRV_SYSCTL_MAP_SIZE 0x10
u8 __iomem *g_sysctl = NULL;
extern void devdrv_mn_mutex_lock(int dev_id);
extern void devdrv_mn_mutex_unlock(int dev_id);
extern struct semaphore *devdrv_get_core_info_sema(u32 dev_id);
extern void devdrv_get_active_core(struct devdrv_info *dev_info);

void devdrv_free_tsfw_mem(struct device *dev, int size, void *addr, dma_addr_t ts_dma_handle)
{
    if (dev == NULL || addr == NULL || ts_dma_handle == 0) {
        return;
    }
    dma_free_coherent(dev, DEVDRV_TS_MEMORY_SIZE, addr, ts_dma_handle);
}
EXPORT_SYMBOL(devdrv_free_tsfw_mem);

u32 devdrv_get_freq(void)
{
    struct devdrv_manager_info *manager_info = devdrv_get_manager_info();
    struct devdrv_platform_data *pdata = NULL;
    u8 __iomem *sysctl = NULL;
    u64 sysctrl_paddr;
    u32 value;
    u32 mode = 4;
    u32 freq;

    if (manager_info == NULL || manager_info->dev_info[CHIP0_ID] == NULL) {
        devdrv_drv_err("dev_manager_info is invalid.\n");
        return 0;
    }
    pdata = (struct devdrv_platform_data *)manager_info->dev_info[CHIP0_ID]->pdata;
    sysctrl_paddr = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_SYSCTL_INDEX];
    if (g_sysctl == NULL) {
        sysctl = devm_ioremap(manager_info->dev, sysctrl_paddr + DEVDRV_SYSCTL_PROFILE_RATE, DEVDRV_SYSCTL_MAP_SIZE);
        if (IS_ERR_OR_NULL(sysctl)) {
            devdrv_drv_err("devm_ioremap failed. \n");
           return -ENOMEM;
        }

        g_sysctl = sysctl;
    }

    value = readl_relaxed(g_sysctl);
    mode = value & 0xFF;

    switch (mode) {
        case 0:
            freq = 600;
            break;
        case 1:
        case 2:
            freq = 1200;
            break;
        case 3:
        case 4:
            freq = 1600;
            break;
        default:
            freq = 1600;
    };

    return freq;
}
