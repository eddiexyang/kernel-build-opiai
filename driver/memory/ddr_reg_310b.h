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

#ifndef DRV_MEMORY_DDR_REG_310B_H
#define DRV_MEMORY_DDR_REG_310B_H

/* DDRC register */
#define DDRC_CTRL_PERF_OFFSET 0x930
#define DDRC_HISI_FLUX_WR_OFFSET 0x948
#define DDRC_HISI_FLUX_RD_OFFSET 0x94C
#define DDRC_HISI_FLUXID_WR_OFFSET 0x958
#define DDRC_HISI_FLUXID_RD_OFFSET 0x95C
#define DDRC_CFG_STAID 0x938
#define DDRC_CFG_STAID1 0x93C
#define DDRC_CFG_STAIDMSK 0x940
#define DDRC_CFG_STAIDMSK1 0x944

#define DDR_CHAN_BITMAP 0x3F
#define DDRC_FLUX_MB_DIV 65536
#define DDRC_DQ_NUM 96
#define DDRC_FREQ_DOUBLE 2

#define DDRC_CHIP_REG_BASE_OFFSET 0x4000000000

#define DDRC_FLUX_STAT_TIME 100 /* 100ms */
#define DDRC_FLUX_STAT_TIMEOUT 500
#define DDRC_MAP_SIZE 4096
#define DDRC_STAT_PERTAGE 100

#define DDRC_BANDWIDTH_PER_CHAN 6554 /* 12.8GB/s(12.8*1024MB/s) */
#define DDRC_BPS_PER_BYTE 8 /* 8 bps per byte/s */

#define MEMORY_SHAREMEM_BASE_ADDR  0x90317280
#define DDR_CHAN_MASK_IOREMAP_ADDR    0x9031728CU

#endif
