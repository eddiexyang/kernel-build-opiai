/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
* Create: 2023-1-13
*/
#ifndef __TSDRV_FAULT_PLATFORM_H__
#define __TSDRV_FAULT_PLATFORM_H__

#define RAS_L2BUF_SRAM_MULTI_BIT_ECC_UER 0x0106
#define RAS_L2BUF_SRAM_SINGLE_BIT_ECC_CE 0x0206
#define RAS_L2BUF_CFG_0  0x0301 /* UEO */
#define RAS_L2BUF_CFG_1  0x0401 /* UEU */
#define RAS_L2BUF_CFG_2  0x0501 /* UEU */
#define RAS_L2BUF_CFG_3  0x0601 /* UEU */
#define RAS_L2BUF_CFG_4  0x0701 /* UEU */
#define RAS_L2BUF_CFG_5  0x0801 /* UEO */
#define RAS_L2BUF_CFG_6  0x090D /* UER */
#define RAS_L2BUF_CFG_7  0x0A01 /* UER */
#define RAS_L2BUF_CFG_8  0x0B01 /* UER */
#define RAS_L2BUF_CFG_9  0x0C01 /* UEO */
#define RAS_L2BUF_CFG_10 0x0D01 /* UER */

#define L2BUFF_NUM 8U
#define TSCPU_NUM  1U
#endif