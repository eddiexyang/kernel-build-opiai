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


#include <linux/irq.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#ifndef DEVMNG_UT
#include "devdrv_common.h"
#include "devdrv_manager.h"
#include "devdrv_manager_common.h"
#include "devdrv_manager_msg.h"
#include "devdrv_ipc_msg.h"
#include "devdrv_platform_resource.h"
#include "devdrv_platform_drvops.h"

#define DEVDRV_DOORBELL_OFFLINE_SEND_OFFSET 510
#define DEVDRV_DOORBELL_OFFLINE_READ_OFFSET 511
#define DEVDRV_DOORBELL_ONLINE_SEND_OFFSET (510 - 48)
#define DEVDRV_DOORBELL_IRQ_TRIGGER_VALUE 0x3A


STATIC int devdrv_manager_device_irq_trigger(u32 dev_id, u32 irq_num)
{
    struct devdrv_info *dev_info = devdrv_manager_get_devdrv_info(dev_id);
    struct devdrv_platform_data *pdata = NULL;
    static u32 read_done_value = 0;
    static u32 value = 0;
    u32 *doorbell = NULL;
    u8 *addr = NULL;

    if (dev_info == NULL) {
        devdrv_drv_err("dev info is null!\n");
        return -EINVAL;
    }

    pdata = dev_info->pdata;
    if (irq_num == DEVDRV_MAILBOX_SEND_OFFLINE_IRQ) {
        addr = pdata->ts_pdata[0].doorbell_vaddr;
        addr = addr + (unsigned long)((DEVDRV_DOORBELL_OFFLINE_SEND_OFFSET + DEVDRV_TS_DOORBELL_SQ_NUM) *
            DEVDRV_TS_DOORBELL_STRIDE);
        doorbell = (u32 *)addr;
        *doorbell = value;
        value++;
    } else if (irq_num == DEVDRV_MAILBOX_SEND_ONLINE_IRQ) {
        addr = pdata->ts_pdata[0].doorbell_vaddr;
        addr = addr + (unsigned long)((DEVDRV_DOORBELL_ONLINE_SEND_OFFSET + DEVDRV_TS_DOORBELL_SQ_NUM) *
            DEVDRV_TS_DOORBELL_STRIDE);
        doorbell = (u32 *)addr;
        *doorbell = value;
        value++;
    } else if (irq_num == DEVDRV_MAILBOX_READ_DONE_IRQ) {
        addr = pdata->ts_pdata[0].doorbell_vaddr;
        addr = addr + (unsigned long)((DEVDRV_DOORBELL_OFFLINE_READ_OFFSET + DEVDRV_TS_DOORBELL_SQ_NUM) *
            DEVDRV_TS_DOORBELL_STRIDE);
        doorbell = (u32 *)addr;
        *doorbell = read_done_value;
        read_done_value++;
    }
    mb();
    return 0;
}

struct tsdrv_drv_ops devdrv_platform_drv_ops = {
    .flush_cache = devdrv_flush_cache,
#ifndef DEVDRV_MAILBOX_SPI_IRQ
    .irq_trigger = devdrv_manager_device_irq_trigger,
#endif

    .ipc_msg_recv_async = devdrv_ipc_msg_send_recv,
    .ipc_msg_send_async = devdrv_ipc_msg_send_async,
    .memcpy_to_device_sq = NULL,
};
#else
int devdrv_manager_device_irq_trigger(u32 dev_id, u32 irq_num)
{
    return 0;
}
#endif
