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

#ifndef __DRV_NOT_H__
#define __DRV_NOT_H__

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#ifndef AOS_LLVM_BUILD
#include <linux/irq.h>
#endif
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/securec.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/string.h>
#include <linux/atomic.h>
#include <linux/version.h>
#include <linux/types.h>

#include "drv_notify_cmd.h"
#include "drv_log.h"

#define MODULE_DRV_NOTIFY "centre_notify"

#define notify_err(fmt, ...) drv_err(MODULE_DRV_NOTIFY, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define notify_warn(fmt, ...) drv_warn(MODULE_DRV_NOTIFY, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define notify_info(fmt, ...) drv_info(MODULE_DRV_NOTIFY, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define notify_notice(fmt, ...) drv_event(MODULE_DRV_NOTIFY, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)
#define notify_debug(fmt, ...) drv_pr_debug(MODULE_DRV_NOTIFY, \
    "<%s:%d:%d> " fmt, current->comm, current->tgid, current->pid, ##__VA_ARGS__)

#define DRV_NOTIFY_STATUS_VALID 1

#define DRV_NOTIFY_WORKMODE_INVALID -1

#define DRV_NOTIFY_CENTRE_NUMBER 1024
#define DRV_NOTIFY_SLOTID_INDEX (DRV_NOTIFY_CENTRE_NUMBER - 1)

#ifdef AOS_LLVM_BUILD
    #define DRV_NOTIFY_SLOTID_GPIO_NUM 262  // GPIO262 AOSCORE use physical pin numbers
#else
    #if (defined(CFG_SOC_PLATFORM_MDC_V51))
        #define DRV_NOTIFY_SLOTID_GPIO_NUM 338  // ascend610 get slot_id by GPIO262
    #elif (defined(CFG_SOC_PLATFORM_MDC_V11))
        #define DRV_NOTIFY_SLOTID_GPIO_NUM 36   // as31xm1 get slot_id by GPIO1-4
    #else
        #define DRV_NOTIFY_SLOTID_GPIO_NUM 338  // stub for other chips compile
    #endif
#endif

#define DRV_NOTIFY_CDEV_COUNT 1

#define NOTIFY_BOARDID_LENGTH 4
#define NOTIFY_BOARDID_900 900
#define NOTIFY_BOARDID_999 999
#define NOTIFY_BOARDID_MASK_1000 1000
#define NOTIFY_BOARDID_MASK_100 100
#define NOTIFY_BOARDID_MASK_10 10
#define NOTIFY_BOARDID_INDEX_2 2
#define NOTIFY_BOARDID_INDEX_3 3

struct drv_notify_centre_node {
    int flag;
    atomic_t value;
};

struct drv_notify_centre_ctrl {
    int is_valid;
    struct drv_notify_centre_node node_info[DRV_NOTIFY_CENTRE_NUMBER];
};

struct drv_notify_cdev {
    struct cdev cdev;
    dev_t dev_no;
    struct class *cdev_class;
    struct device *dev;
};

/**
* @ingroup driver
* @brief set the value by index
* @attention null
* @param [in]  int index : which index you want to set(0-1022)
*                          index 1023 is the slotid.
* @param [in]  int value : the valve you want to set
* @return 0, -EINVAL
*/
int centre_notify_set_val(int index, int value);

/**
* @ingroup driver
* @brief get the value by index
* @attention null
* @param [in]  int index : which index you want to get(0-1023)
* @param [out] int *value  : the valve you want to get
* @return 0, -EINVAL, -EPERM
*/
int centre_notify_get_val(int index, int *value);

#endif
