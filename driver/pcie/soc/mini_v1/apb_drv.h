/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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

#ifndef _APB_DRV_H_
#define _APB_DRV_H_

#define DEVDRV_RX_ATU_NUM 13
#define DEVDRV_TX_ATU_NUM 6 /* mini not use tx atu */

#define DEVDRV_SUPPORT_MAX_VF_NUM 0

#define DEVDRV_PCIE_DDR_SIZE 0x4000000000
#define DEVDRV_P2P_SUPPORT_MAX_DEVICE 8
#define DEVDRV_H2D_SUPPORT_MAX_DEVICE 8

#define DEVDRV_SYSCTL_BASE_ADDR 0x1100c0000
#define DEVDRV_SYSCTL_SIZE 0x10000
#define DEVDRV_SYSCTL_PLTFORM_REG 0xfffc

#define AGENTDRV_SYSFS_LINK_STATUS_REG_OFFSET 0x107060

#include "apb_comm_drv.h"

#endif
