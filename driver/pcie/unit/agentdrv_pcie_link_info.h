/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#ifndef AGENTDRV_PCIE_LINK_INFO_H
#define AGENTDRV_PCIE_LINK_INFO_H

#include "drv_type.h"

typedef enum {
    AGENTDRV_PCIE_LINK_STATUS_OK          = 0,
    AGENTDRV_PCIE_LINK_STATUS_DOWN        = 1,
    AGENTDRV_PCIE_LINK_STATUS_CHANNEL_ERR = 2,
} AGENTDRV_PCIE_LINK_STATUS;

typedef enum {
    AGENTDRV_PCIE_INIT_OK = 0,
    AGENTDRV_PCIE_INIT_ERR  = 1,
} AGENTDRV_PCIE_INIT_STATUS;

/*                  PCIE mac link                                                          */
/********************************************************************************************/
#define PCIE_HIPCIEC_MAC_REG_LINK_INFO 0x60
#define PCIE_MAC_REG_LINK_LTSSM_ST_OFFSET   24U   /* MAC_REG_LINK_INFO.mac_ltssm_st bit[24-29] */
#define PCIE_MAC_REG_LINK_LTSSM_L0 16U
#define PCIE_MAC_REG_LINK_SPEED_OFFSET   8U   /* MAC_REG_LINK_INFO.mac_cur_link_speed bit[8-11] */

void devdrv_set_pcie_channel_status(u32 value);

#endif