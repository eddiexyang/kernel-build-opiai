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

#ifndef SOC_MISC_BOARD_INFO_H
#define SOC_MISC_BOARD_INFO_H

#include "soc_misc_info.h"

#define DEV_BOARDID_PCIE_VA 1
#define DEV_BOARDID_PCIE_VC 3
#define DEV_BOARDID_PCIE_VD 5
#define DEV_BOARDID_PCIE_MAX_ID 299
#define DEV_BOARDID_PCIE_MIN_ID 200

#define DEV_BOARDID_MDC_L4 2
#define DEV_BOARDID_MDC_L3 6
#define DEV_BOARDID_PCIE_LOW_LIMIT 200
#define DEV_BOARDID_PCIE_UPPER_LIMIT 299
#define DEV_BOARDID_EVB_LOW_LIMIT 900
#define DEV_BOARDID_EVB_UPPER_LIMIT 949

#define SLOT_GPIO_SIZE  4
#define BOARD_GPIO_SIZE 14

#define AK_CLOUD_BOARDID          0x0020
#define AK_CLOUD_BOARDID_AG_280T  0x0270
#define AK_CLOUD_BOARDID_MASK     0xFFF0
#define CLOUD_MODULE_BOARID_SHIFT 9
#define CLOUD_MODULE_BOARDID_MASK 0x1

#define SLOT_ID_BUFFER_KERNEL_LEN 4096
#define SLOT_ID_STRLEN 7

#define DTS_BOARD_ID_ARRAY_SIZE 4

#define THOUSANDS_PLACE 1000
#define HUNDREDS_PLACE  100
#define TENS_PLACE      10

#define BOARD_ID_PROPNAME  "hisi,boardid"
#define GPIO_BOARD_ID_NAME "board"
#define GPIO_SLOT_ID_NAME  "slot"
#define CMDLINE_LOCATION   "/proc/cmdline"

#endif
