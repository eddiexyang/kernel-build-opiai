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

#ifndef __DEVDRV_OS_POWER_H
#define __DEVDRV_OS_POWER_H

#include <linux/platform_device.h>

#define DEVDRV_SUB_OS_MAX        3
#define DEVDRV_SUB_OS_POWER_NUM  2
#define DEVDRV_SUB_OS_BOOT_MAGIC 0x626F6F74
#define DEVDRV_SUB_OS_SHUT_MAGIC 0x73687574
#define DEVDRV_SUB_OS_ONLINE     0
#define DEVDRV_SUB_OS_OFFLINE    1
#define DEVDRV_SUB_OS_BITMAP     0x7
#define DEVDRV_SUB_OS_VALID      1
#define DEVDRV_SUB_OS_INVALID    0

/* sub os type def*/
typedef enum {
    SUB_OS_SD = 0,
    SUB_OS_CLUSTER = 1,
    SUB_OS_COMIS = 2,
    SUB_OS_ALL = 0xFE,
    SUB_OS_INVALID = 0xFF,
} DEVDRV_SUB_OS_TYPE;

struct devdrv_power_pdata {
    u32 os_id;
    u32 valid;
    void __iomem *boot_vaddr;
    void __iomem *shut_vaddr;
    u64 boot_paddr;
    u64 shut_paddr;
    size_t boot_addr_size;
    size_t shut_addr_size;
};

struct devdrv_sub_os_info {
    u32 os_num;
    struct devdrv_power_pdata pw_pdata[DEVDRV_SUB_OS_MAX];
};

struct devdrv_sub_os_info *devdrv_get_sub_os_info(void);
int devdrv_sub_os_init(struct platform_device *pdev);
void devdrv_sub_os_uninit(void);

#endif /*  __DEVDRV_OS_POWER_H  */
