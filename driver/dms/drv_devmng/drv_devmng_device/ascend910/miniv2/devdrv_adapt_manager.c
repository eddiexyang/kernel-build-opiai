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

#ifndef DEVMNG_UT
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
#include "devdrv_driver_pm.h"
#include "devdrv_parse_pdata.h"

u32 devdrv_get_freq(void)
{
    struct devdrv_manager_info *manager_info = devdrv_get_manager_info();
    struct devdrv_platform_data *pdata = NULL;
    u8 __iomem *sysctl = NULL;
    u8 __iomem *reg = NULL;
    u64 sysctl_paddr;
    u32 value;
    u32 mode = 4; // default mode 4
    u32 freq;

    if (manager_info == NULL || manager_info->dev_info[CHIP0_ID] == NULL) {
        devdrv_drv_err("dev_manager_info is invalid.\n");
        return 0;
    }
    pdata = (struct devdrv_platform_data *)manager_info->dev_info[CHIP0_ID]->pdata;
    sysctl_paddr = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_SYSCTL_INDEX];
    sysctl = devm_ioremap(manager_info->dev, sysctl_paddr, DEVDRV_SYSCTL_REG_SIZE);
    if (sysctl != NULL) {
        reg = sysctl + DEVDRV_SYSCTL_PROFILE_RATE;
        value = readl_relaxed(reg);
        mode = value & 0xFF;

        devdrv_drv_debug("profile mode: %d.\n", mode);

        devm_iounmap(manager_info->dev, sysctl);
        sysctl = NULL;
    }

    switch (mode) {
        /* mode 0 */
        case 0:
            freq = 600; /* Frequency is 600 */
            break;
        /* mode 1 */
        case 1:
        /* mode 2 */
        case 2:
            freq = 1200; /* Frequency is 1200 */
            break;
        /* mode 3 */
        case 3:
        /* mode 4 */
        case 4:
            freq = 1600; /* Frequency is 1600 */
            break;
        default:
            freq = 1600; /* Frequency is 1600 */
    };

    return freq;
}

unsigned long devdrv_manager_get_wait_time(struct devdrv_info *dev_info)
{
    unsigned long timeout;

    /*
    * try to inform host ai subsystem is ready,
    * if env type is EMU or boardid equals 1000, 2000, 3004 , 4004, set timeout to 50
    */
    if (dev_info->env_type == DEVDRV_PLAT_TYPE_EMU) {
        timeout = msecs_to_jiffies(DEVDRV_WAIT_TIME_SHORT_TIME);
    } else if (dev_info->env_type == DEVDRV_PLAT_TYPE_FPGA) {
#ifdef CFG_SOC_PLATFORM_MDC_V51
        timeout = msecs_to_jiffies(DEVDRV_WAIT_TIME_SHORT_TIME);
#else
        timeout = msecs_to_jiffies(DEVDRV_WAIT_TIME_FPGA_DC);
#endif
    } else if (dev_info->env_type == DEVDRV_PLAT_TYPE_ESL) {
#ifdef CFG_SOC_PLATFORM_MINIV3
        timeout = msecs_to_jiffies(DEVDRV_WAIT_TIME_NORMAL);
#else
        timeout = msecs_to_jiffies(DEVDRV_WAIT_TIME_SHORT_TIME);
#endif
    } else {
#ifdef CFG_SOC_PLATFORM_MDC_V51
        devdrv_drv_info("mdc wait time = %u(ms)\n", DEVDRV_WAIT_TIME_SHORT_TIME);
        timeout = msecs_to_jiffies(DEVDRV_WAIT_TIME_SHORT_TIME);
#else
        devdrv_drv_info("dc wait time = %u(ms)\n", DEVDRV_WAIT_TIME_NORMAL);
        timeout = msecs_to_jiffies(DEVDRV_WAIT_TIME_NORMAL);
#endif
    }

#ifdef CFG_SOC_PLATFORM_HELPER
    timeout = msecs_to_jiffies(DEVDRV_WAIT_TIME_SHORT_TIME);
#endif

    return timeout;
}

int devdrv_manager_nfe_irq_register(struct devdrv_info *dev_info)
{
    return 0;
}

void devdrv_free_tsfw_mem(struct device *dev, int size, void *addr, dma_addr_t dma_addr)
{
    DRV_CHECK_PTR(dev, return, "dev is NULL\n");
    DRV_CHECK_PTR(addr, return, "addr is NULL\n");
    devm_iounmap(dev, addr);
    return;
}

int devdrv_get_l2_buffer(u32 devid, u64 *base, u64 *len)
{
    return 0;
}
EXPORT_SYMBOL(devdrv_get_l2_buffer);
#else
int devdrv_gicd_irq_cpuid_config(struct devdrv_info *dev_info, unsigned int irq_num, unsigned int cpuid)
{
    return 0;
}
#endif

