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

#include <linux/spinlock.h>

#include "devdrv_common.h"
#include "devdrv_manager_common.h"
#include "devdrv_manager_freq.h"
#include "devdrv_pm.h"
#include "devdrv_platform_resource.h"
#include "tsmng_interface.h"

DEFINE_SPINLOCK(freq_manager_lock);
STATIC struct devdrv_ddr_freq_manager freq_manager = {
    .ticket_pool = 0,
    .last_ipc_cmd = DDR_FREQ_ORIGIN,
    .inited = DDR_FREQ_ORIGIN,
};

STATIC u64 devdrv_freq_clear_bit(u64 pool, u32 bit)
{
    return (pool & (~(1ULL << bit)));
}

STATIC u64 devdrv_freq_set_bit(u64 pool, u32 bit)
{
    return (pool | (1ULL << bit));
}

STATIC void devdrv_ddr_freq_manager_init(void)
{
    unsigned long flags;
    u64 pool;
    u32 i;

    spin_lock_irqsave(&freq_manager_lock, flags);

    if (freq_manager.inited == DDR_FREQ_INITED) {
        spin_unlock_irqrestore(&freq_manager_lock, flags);
        return;
    }

    pool = 0;
    for (i = 0; i < DEVDRV_FREQ_MAX; i++) {
        pool = devdrv_freq_set_bit(pool, i);
    }

    freq_manager.ticket_pool = pool;
    freq_manager.last_ipc_cmd = DDR_FREQ_ORIGIN;
    freq_manager.inited = DDR_FREQ_INITED;

    spin_unlock_irqrestore(&freq_manager_lock, flags);

    devdrv_drv_info("ticket_pool: 0x%pK, last cmd: %d\n", (void *)(uintptr_t)freq_manager.ticket_pool,
                    freq_manager.last_ipc_cmd);
}

int devdrv_upper_ddr_freq(u32 devid, u32 module_id)
{
    unsigned long flags;
    int ret;

    devdrv_ddr_freq_manager_init();

    if (devid >= MAX_CHIP_NUM) {
        return -EINVAL;
    }

    if (module_id >= DEVDRV_FREQ_MAX) {
        return -EINVAL;
    }

    spin_lock_irqsave(&freq_manager_lock, flags);
    freq_manager.ticket_pool = devdrv_freq_set_bit(freq_manager.ticket_pool, module_id);
    if (freq_manager.last_ipc_cmd == DDR_FREQ_LAST_IS_UP) {
        spin_unlock_irqrestore(&freq_manager_lock, flags);
        return 0;
    }
    spin_unlock_irqrestore(&freq_manager_lock, flags);

    ret = devdrv_inform_lpm3_upper_ddr_freq(devid);
    spin_lock_irqsave(&freq_manager_lock, flags);
    if (ret) {
        freq_manager.ticket_pool = devdrv_freq_clear_bit(freq_manager.ticket_pool, module_id);
        spin_unlock_irqrestore(&freq_manager_lock, flags);
        return ret;
    }
    freq_manager.last_ipc_cmd = DDR_FREQ_LAST_IS_UP;

    spin_unlock_irqrestore(&freq_manager_lock, flags);

    return 0;
}
EXPORT_SYMBOL(devdrv_upper_ddr_freq);

int devdrv_lower_ddr_freq(u32 devid, u32 module_id)
{
    unsigned long flags;
    int ret;

    devdrv_ddr_freq_manager_init();

    if (devid >= MAX_CHIP_NUM) {
        return -EINVAL;
    }

    if (module_id >= DEVDRV_FREQ_MAX) {
        return -EINVAL;
    }

    spin_lock_irqsave(&freq_manager_lock, flags);
    freq_manager.ticket_pool = devdrv_freq_clear_bit(freq_manager.ticket_pool, module_id);
    if (freq_manager.ticket_pool != 0) {
        spin_unlock_irqrestore(&freq_manager_lock, flags);
        return 0;
    }

    if (freq_manager.last_ipc_cmd == DDR_FREQ_LAST_IS_DOWN) {
        spin_unlock_irqrestore(&freq_manager_lock, flags);
        return 0;
    }
    spin_unlock_irqrestore(&freq_manager_lock, flags);

    ret = devdrv_inform_lpm3_lower_ddr_freq(devid);
    spin_lock_irqsave(&freq_manager_lock, flags);
    if (ret) {
        freq_manager.ticket_pool = devdrv_freq_set_bit(freq_manager.ticket_pool, module_id);
        spin_unlock_irqrestore(&freq_manager_lock, flags);
        return ret;
    }

    freq_manager.last_ipc_cmd = DDR_FREQ_LAST_IS_DOWN;
    spin_unlock_irqrestore(&freq_manager_lock, flags);

    return 0;
}
EXPORT_SYMBOL(devdrv_lower_ddr_freq);

STATIC int devdrv_control_ts_upper_ddr_freq(void *data, u32 dev_id)
{
    (void)data;

    return devdrv_upper_ddr_freq(dev_id, DEVDRV_FREQ_TS);
}

STATIC int devdrv_control_ts_lower_ddr_freq(void *data, u32 dev_id)
{
    (void)data;

    return devdrv_lower_ddr_freq(dev_id, DEVDRV_FREQ_TS);
}

void devdrv_freq_control_register(u32 dev_id)
{
    int ret;

    devdrv_drv_info("Dev %u register freq control handle.\n", dev_id);

    ret = tsmng_handler_register_upper_ddrfreq(dev_id, devdrv_control_ts_upper_ddr_freq);
    if (ret != 0) {
        devdrv_drv_err("Register upper ddr freq handler failed.\n");
        return;
    }

    ret = tsmng_handler_register_lower_ddrfreq(dev_id, devdrv_control_ts_lower_ddr_freq);
    if (ret != 0) {
        tsmng_handler_unregister_ddrfreq(dev_id);
        devdrv_drv_err("Register lower ddr freq handler failed.\n");
    }
}

