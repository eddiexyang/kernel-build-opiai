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

#define AGENTDRV_MAX_DIE_NUM 2

#define DEVDRV_SUPPORT_MAX_VF_NUM 8

#define DEVDRV_BASE_BUS_ID 0x10
#define DEVDRV_MAX_BUS_ID 0x2F
#define DEVDRV_MAX_CHIP_ID 8 /* max is 8p */

#define DEVDRV_DIFF_OS_CHIP_OFFSET_SIZE 0x80000000000ULL
#define DEVDRV_PEH_DIFF_OS_CHIP_OFFSET_SIZE 0x20000000000ULL

#define DEVDRV_CHIP_ID_INFO_ADDR 0x36EFD800 /* 0 is p0, 1 is p1,...7 is p7 */

#define DEVDRV_RX_ATU_NUM 32
#define DEVDRV_TX_ATU_NUM 16

#define DEVDRV_RM_PF_SUPPOT_ENABLE (0x801000000 + 0x30000 + 0x904)
#define DEVDRV_PF_ENABLE_MASK 0x3
#define DEVDRV_PF_ENABLE_SINGLE 1
#define DEVDRV_PF_ENABLE_DOUBLE 2

#define DEVDRV_PCIE_DDR_SIZE 0x10000000000 /* 1024G = 16 * 64G */
#define DEVDRV_P2P_SUPPORT_MAX_DEVICE 16
#define DEVDRV_H2D_SUPPORT_MAX_DEVICE 16

#define DEVDRV_SYSCTL_BASE_ADDR 0x80000000
#define DEVDRV_SYSCTL_SIZE 0x10000
#define DEVDRV_SYSCTL_PLTFORM_REG 0xfffc

#define AGENTDRV_SYSFS_LINK_STATUS_REG_OFFSET (0x70000 + 0x60)

#define AGENTDRV_PEH_BOARD 0xB2
#define AGENTDRV_PEH_MAIN_BOARD1 0x10
#define AGENTDRV_PEH_MAIN_BOARD2 0x1F

#define AGENTDRV_CACHELINE_SIZE           (128)
#define AGENTDRV_CACHELINE_MASK           (AGENTDRV_CACHELINE_SIZE - 1)

/* bios set, drv use */
typedef struct devdrv_hw_info {
    unsigned char chip_id;
    unsigned char multi_chip;
    unsigned char multi_die;
    unsigned char mainboard_id;
    unsigned short hccs_connect_status;
    unsigned short board_id;
    unsigned int reserved;
} devdrv_hw_info_t;

int agentdrv_get_chip_offset_by_devid(int devid, unsigned long long *chip_offset);
unsigned char agentdrv_get_mainboard_id_by_hw_info(void);

#include "apb_comm_drv.h"

#endif
