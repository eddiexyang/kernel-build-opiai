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

#ifndef __DEVDRV_MANAGER_MSG_H
#define __DEVDRV_MANAGER_MSG_H
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/workqueue.h>
#include <linux/list.h>

#include "devdrv_manager_common.h"
#include "devdrv_common.h"
#include "devdrv_manager.h"
#include "devdrv_interface.h"
#include "devdrv_pm.h"

extern void *devdrv_manager_msg_chan[DEVDRV_MAX_DAVINCI_NUM];

typedef struct devdrv_core_utilization {
    u32 dev_id;
    u32 vfid;
    u32 core_type; /* 0: aicore  1: aivector 2:aicpu */
    u32 utilization;
} devdrv_core_utilization_t;

typedef enum devdrv_core_type{
    DEV_DRV_TYPE_AICORE = 0,
    DEV_DRV_TYPE_AIVECTOR,
    DEV_DRV_TYPE_AICPU,
    DEV_DRV_TYPE_MAX,
} devdrv_core_type_t;

int devdrv_manager_get_devinfo_from_host(u32 devid, void *msg, u32 in_len, u32 *ack_len);
int devdrv_gicd_irq_cpuid_config(struct devdrv_info *dev_info, u32 irq_num, u32 cpuid);
int devdrv_manager_h2d_sync_low_power(u32 devid, void *msg, u32 in_len, u32 *ack_len);
int devdrv_manager_get_core_utilization_from_host(u32 dev_id, void *msg, u32 in_len, u32 *ack_len);


#endif /* __DEVDRV_MANAGER_MSG_H */
