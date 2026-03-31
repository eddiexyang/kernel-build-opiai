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

#ifndef _PROFILING_DRV_H_
#define _PROFILING_DRV_H_

#include "nvme_drv.h"

/* --- profiling needs dfx registers while cloud use core 1, port 0 --- */
/* profiling control register */
#define BW_CTRL_ENABLE_CMD 0x7A00100
#define BW_CTRL_DISABLE_CMD 0x6000000
#define LAN_CTRL_ENABLE_CMD 0x200100
#define LAN_CTRL_DISABLE_CMD 0x0
#define TX_TABLE_BASE_ADDR 0x0
#define AGENTDRV_TX_NP_LAN_CTRL (TX_TABLE_BASE_ADDR + 0x2780)
#define AGENTDRV_TX_P_BW_CTRL (TX_TABLE_BASE_ADDR + 0x2784)
#define AGENTDRV_TX_NP_BW_CTRL (TX_TABLE_BASE_ADDR + 0x2788)
#define AGENTDRV_TX_CPL_BW_CTRL (TX_TABLE_BASE_ADDR + 0x278c)
#define TX_P_BW_MIN (TX_TABLE_BASE_ADDR + 0x2800 + (0x10 * AGENTDRV_CORE_NUM))
#define TX_P_BW_MAX (TX_TABLE_BASE_ADDR + 0x2804 + (0x10 * AGENTDRV_CORE_NUM))
#define TX_P_BW_AVERAGE (TX_TABLE_BASE_ADDR + 0x2808 + (0x10 * AGENTDRV_CORE_NUM))
#define TX_NP_BW_MIN (TX_TABLE_BASE_ADDR + 0x2840 + (0x10 * AGENTDRV_CORE_NUM))
#define TX_NP_BW_MAX (TX_TABLE_BASE_ADDR + 0x2844 + (0x10 * AGENTDRV_CORE_NUM))
#define TX_NP_BW_AVERAGE (TX_TABLE_BASE_ADDR + 0x2848 + (0x10 * AGENTDRV_CORE_NUM))
#define TX_CPL_BW_MIN (TX_TABLE_BASE_ADDR + 0x2880 + (0x10 * AGENTDRV_CORE_NUM))
#define TX_CPL_BW_MAX (TX_TABLE_BASE_ADDR + 0x2884 + (0x10 * AGENTDRV_CORE_NUM))
#define TX_CPL_BW_AVERAGE (TX_TABLE_BASE_ADDR + 0x2888 + (0x10 * AGENTDRV_CORE_NUM))
#define TX_NP_LAN_MIN (TX_TABLE_BASE_ADDR + 0x2900 + (0x10 * AGENTDRV_CORE_NUM))
#define TX_NP_LAN_MAX (TX_TABLE_BASE_ADDR + 0x2904 + (0x10 * AGENTDRV_CORE_NUM))
#define TX_NP_LAN_AVERAGE (TX_TABLE_BASE_ADDR + 0x2908 + (0x10 * AGENTDRV_CORE_NUM))

#define RX_BW_ENABLE_CMD 0x10101
#define RX_BW_DISABLE_CMD 0x0
#define RX_BW_CLEAR_CMD 0x7
#define RX_TABLE_BASE_ADDR 0x6400
#define AGENTDRV_RX_BW_CTRL_REG (RX_TABLE_BASE_ADDR + 0x380 + (0x400 * AGENTDRV_CORE_NUM))
#define AGENTDRV_RX_BW_CLR_REG (RX_TABLE_BASE_ADDR + 0x384 + (0x400 * AGENTDRV_CORE_NUM))
#define RX_NP_BW_CUR (RX_TABLE_BASE_ADDR + 0x390 + (0x400 * AGENTDRV_CORE_NUM))
#define RX_NP_BW (RX_TABLE_BASE_ADDR + 0x394 + (0x400 * AGENTDRV_CORE_NUM))
#define RX_P_BW_CUR (RX_TABLE_BASE_ADDR + 0x398 + (0x400 * AGENTDRV_CORE_NUM))
#define RX_P_BW (RX_TABLE_BASE_ADDR + 0x39C + (0x400 * AGENTDRV_CORE_NUM))
#define RX_CPL_BW_CUR (RX_TABLE_BASE_ADDR + 0x3A0 + (0x400 * AGENTDRV_CORE_NUM))
#define RX_CPL_BW (RX_TABLE_BASE_ADDR + 0x3A4 + (0x400 * AGENTDRV_CORE_NUM))


#endif
