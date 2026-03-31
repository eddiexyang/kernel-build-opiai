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

#ifndef DEVDRV_PCIE_H
#define DEVDRV_PCIE_H

#include <linux/slab.h>
#include <linux/cred.h>

#include "devdrv_manager_container.h"
#include "devdrv_pm.h"
#include "devdrv_interface.h"
#include "devdrv_common.h"

#ifdef CFG_FEATURE_OLD_ALL_DEVICE_RESET_FLAG
#define DEVDRV_RESET_ALL_DEVICE_ID 0xff
#else
#define DEVDRV_RESET_ALL_DEVICE_ID 0xffffffff
#endif

#ifndef CAP_SYS_ADMIN
#define CAP_SYS_ADMIN 21
#endif

int devdrv_manager_check_permission(void);
#ifndef CFG_FEATURE_RC_MODE
int devdrv_manager_pcie_pre_reset(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_pcie_rescan(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_pcie_hot_reset_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);
int drv_pcie_bbox_imu_ddr_read(struct file *filep, unsigned int cmd, unsigned long arg);
int drv_pcie_read(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_p2p_attr_op(struct file *filep, unsigned int cmd, unsigned long arg);
int drv_dma_mmap(struct file *filep, unsigned int cmd, unsigned long arg);
int drv_get_device_boot_status(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_get_host_phy_mach_flag(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_get_master_device_in_the_same_os(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_get_p2p_capability(struct file *filep, unsigned int cmd, unsigned long arg);
bool devdrv_manager_is_pf_device(unsigned int dev_id);
bool devdrv_manager_is_mdev_vm_mode(unsigned int dev_id);
bool devdrv_manager_is_mdev_vf_vm_mode(unsigned int dev_id);
int devdrv_manager_is_pm_boot_mode(unsigned int dev_id, bool *is_pm_boot);
#endif
bool devdrv_manager_is_sriov_support(unsigned int dev_id);

#endif /* __DEVDRV_PCIE_H */
