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
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/io.h>
#include <linux/irqchip/arm-gic-v3.h>

#include "devdrv_common.h"
#include "devdrv_platform.h"
#include "devdrv_pm.h"
#include "devdrv_manager.h"
#include "devdrv_platform_resource.h"
#include "devdrv_driver_pm.h"
#include "devdrv_interface.h"
#include "devdrv_parse_pdata.h"

u32 devdrv_get_freq(void)
{
    return 0;
}

unsigned long devdrv_manager_get_wait_time(struct devdrv_info *dev_info)
{
    int boardid = devdrv_get_boardid();
    unsigned long timeout;

    /*
    * try to inform host ai subsystem is ready,
    * if env type is EMU or boardid equals 1000, 2000, 3004 , 4004, set timeout to 50
    */
    if (dev_info->env_type == DEVDRV_PLAT_TYPE_EMU) {
        timeout = msecs_to_jiffies(DEVDRV_WAIT_TIME_SHORT_TIME);
    } else if (dev_info->env_type == DEVDRV_PLAT_TYPE_FPGA) {
        timeout = msecs_to_jiffies(DEVDRV_WAIT_TIME_FPGA_DC);
    } else if (dev_info->env_type == DEVDRV_PLAT_TYPE_ESL) {
        timeout = msecs_to_jiffies(DEVDRV_WAIT_TIME_FPGA_DC);
    } else {
        devdrv_drv_info("wait time = %d\n", DEVDRV_WAIT_TIME_NORMAL);
        timeout = msecs_to_jiffies(DEVDRV_WAIT_TIME_NORMAL);
    }

#ifdef CFG_SOC_PLATFORM_HELPER
    timeout = msecs_to_jiffies(DEVDRV_WAIT_TIME_SHORT_TIME);
#endif

    devdrv_drv_info("boardid = %d, timeout=%lu\n", boardid, timeout);

    return timeout;
}

int devdrv_manager_nfe_irq_register(struct devdrv_info *dev_info)
{
    return 0;
}


void devdrv_free_tsfw_mem(struct device *dev, int size, void *addr, dma_addr_t ts_dma_handle)
{
    if (addr != NULL) {
        iounmap(addr);
    }
}

int devdrv_get_l2_buffer(u32 devid, u64 *base, u64 *len)
{
    return 0;
}
EXPORT_SYMBOL(devdrv_get_l2_buffer);
#else
int devdrv_manager_nfe_irq_register(struct devdrv_info *dev_info)
{
    return 0;
}
#endif


