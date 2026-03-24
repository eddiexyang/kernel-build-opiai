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

#ifndef DRV_MEMORY_DDR_REG_910_H
#define DRV_MEMORY_DDR_REG_910_H

#define DDRC_CHIP_REG_BASE_OFFSET 0x200000000000
#define DDRC_CTRL_PERF_OFFSET 0x010
#define DDRC_HISI_FLUX_WR_OFFSET 0x380
#define DDRC_HISI_FLUX_RD_OFFSET 0x384
#define DDRC_HISI_FLUXID_WR_OFFSET 0x390
#define DDRC_HISI_FLUXID_RD_OFFSET 0x394
#define DDRC_CFG_STAID 0x270
#define DDRC_CFG_STAIDMSK 0x274
#define DDR_CHAN_BITMAP 0x3
#define DDRC_FLUX_MB_DIV 32768

#define DDRC_FLUX_STAT_TIME 100 // 100 ms
#define DDRC_FLUX_STAT_TIMEOUT 500
#define DDRC_MAP_SIZE 4096
#define DDRC_STAT_PERTAGE 100

#define DDRC_BANDWIDTH_PER_CHAN 26214 // 12.8GB/s(12.8*1024MB/s)

/* sharemem baseaddr for 910B */
#define SHAREMEM_BASE_ADDR  0x29220000

#endif
