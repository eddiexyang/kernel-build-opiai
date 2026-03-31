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

#ifndef __DEVDRV_PCIE_H
#define __DEVDRV_PCIE_H

#include "devdrv_common.h"
#include "devdrv_interface.h"
#include "devdrv_manager.h"

#define DEVDRV_HOST_MAX_DEV_NUM 64
#define DEVDRV_DEVICE_MAX_DEV_NUM 4

int devdrv_manager_get_host_phy_mach_flag(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_get_local_devid_by_host_devid(struct file *filep, unsigned int cmd, unsigned long arg);
bool devdrv_manager_is_pf_device(unsigned int dev_id);

#endif /* __DEVDRV_PCIE_H */
