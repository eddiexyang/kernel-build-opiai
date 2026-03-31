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


#ifndef __DEVDRV_FLASH_CONFIG_H
#define __DEVDRV_FLASH_CONFIG_H
#include <linux/module.h>
#include <linux/platform_device.h>
#define DEVDRV_UC_DATT_LEN_MIN 4
/* process white list */
#define WHITE_LIST_PROCESS_NUM 1
#define PROCESS_NAME_DMP "dmp_daemon"

int devdrv_flash_user_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_flash_root_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);
#endif
