/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
#ifndef TSDRV_KERNEL_UT
#include <linux/slab.h>
#include "devdrv_common.h"
#include "devdrv_id.h"
#include "devdrv_cqsq.h"
#include "tsdrv_drvops.h"
#include "tsdrv_device.h"
#include "tsdrv_hwinfo.h"
#include "tsdrv_id.h"
#include "tsdrv_sync.h"

STATIC void tsdrv_device_flush_cache(u64 base, u32 len)
{
#ifdef __aarch64__
    uint64_t addr_loop, addr_end;

    addr_loop = base & (~DEVDRV_CACHELINE_MASK);
    addr_end = (base + len + DEVDRV_CACHELINE_MASK) & (~DEVDRV_CACHELINE_MASK);

    asm volatile("dsb st"
                 :
                 :
                 : "memory");
    for (; addr_loop < addr_end;) {
        asm volatile("DC CIVAC ,%x0" ::"r"(addr_loop));
        mb();
        addr_loop += DEVDRV_CACHELINE_SIZE;
    }
    asm volatile("dsb st"
                 :
                 :
                 : "memory");
#endif
}

STATIC int tsdrv_device_irq_trigger(u32 dev_id, u32 tsid)
{
    struct tsdrv_db_hwinfo_t *db_hwinfo = NULL;
    enum tsdrv_env_type env_type;
    static u32 value = 0;
    u32 *doorbell = NULL;

    env_type = tsdrv_get_env_type();
    if (env_type == TSDRV_ENV_OFFLINE) {
        db_hwinfo = tsdrv_get_db_hwinfo_t(dev_id, tsid, TSDRV_OFFLINE_MBOX_SEND_DB);
        doorbell = (u32 *)db_hwinfo->dbs;
        *doorbell = value;  // 0x3A;
        value++;
        TSDRV_PRINT_DEBUG("Offline doorbell. (devid=%u; dbs=%pK; value=%u)\n", dev_id, db_hwinfo->dbs, *doorbell);
    } else if (env_type == TSDRV_ENV_ONLINE) {
#ifndef TSDRV_UT
        db_hwinfo = tsdrv_get_db_hwinfo_t(dev_id, tsid, TSDRV_ONLINE_MBOX_SEND_DB);
        doorbell = (u32 *)db_hwinfo->dbs;
        *doorbell = value;  // 0x3A;
        value++;
#endif
    }
    mb();

    return 0;
}

struct tsdrv_drv_ops tsdrv_device_drv_ops = {
    .flush_cache = tsdrv_device_flush_cache,
#ifndef DEVDRV_MAILBOX_SPI_IRQ
    .irq_trigger = tsdrv_device_irq_trigger,
#endif
    .add_ipc_event_msg_chan = devdrv_add_ipc_event_msg_chan,

    .add_notify_msg_chan = devdrv_add_notify_msg_chan,
    .add_id_msg_chan = devdrv_add_id_msg_chan,
};

struct tsdrv_drv_ops *tsdrv_get_drv_ops(void)
{
    return &tsdrv_device_drv_ops;
}
TSDRV_EXPORT_SYMBOL(tsdrv_get_drv_ops);
#else
int tsdrv_drvops(void)
{
    return 0;
}
#endif

