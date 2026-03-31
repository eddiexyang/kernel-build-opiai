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

#ifndef ASCEND_CTL_H
#define ASCEND_CTL_H

#ifdef STATIC_SKIP
#define STATIC
#else
#define STATIC static
#endif

#define MODULE_DAVINCI "drv_ascend_ctl"
#define DRV_ASCEND_CTL_PRINTK(level, module, fmt, ...) \
        printk(level " [%s] [%s %d] " fmt, module, __func__, __LINE__, ##__VA_ARGS__)

#define ASCEND_CTL_ERR(fmt, ...) do { \
    DRV_ASCEND_CTL_PRINTK(KERN_ERR, MODULE_DAVINCI, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
} while (0);
#define ASCEND_CTL_WARN(fmt, ...) do { \
    DRV_ASCEND_CTL_PRINTK(KERN_WARNING, MODULE_DAVINCI, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
} while (0);
#define ASCEND_CTL_INFO(fmt, ...) do { \
    DRV_ASCEND_CTL_PRINTK(KERN_INFO, MODULE_DAVINCI, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
} while (0);

#endif
