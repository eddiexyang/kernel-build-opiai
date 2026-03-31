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

#ifndef DRV_MEMORY_DDR_REG_310_H
#define DRV_MEMORY_DDR_REG_310_H

/* DDRC_CTRL_PERF register */
#define DDRC_CTRL_PERF_OFFSET 0x4010

/* DDRC_HISI_FLUX_WR register */
#define DDRC_HISI_FLUX_WR_OFFSET 0x4380
#define DDRC_HISI_FLUX_RD_OFFSET 0x4384
#define DDRC_HISI_FLUXID_WR_OFFSET 0x4390
#define DDRC_HISI_FLUXID_RD_OFFSET 0x4394

/* DDRC mask ID configure value */
#define DDRC_CFG_STAID 0x4270
#define DDRC_CFG_STAIDMSK 0x4274

#define DDRC_CFG_STAID_SET 0x10000
#define DDRC_CFG_STAIDMSK_SET 0x3f800
#define DDRC_CFG_STAID_OFFSET 11

#define DDRC_FLUX_STAT_TIME 100 // 100 ms
#define DDRC_FLUX_STAT_TIMEOUT 500
#define DDRC_MAP_SIZE (4096 * 5)
#define DDRC_STAT_PERTAGE 100

#define DDRC_FLUX_MB_DIV 65536

#define DDRC_BANDWIDTH_CHAN_8 52429 // 51.2GB/s(51.2*1024MB/s)
#define DDRC_BANDWIDTH_CHAN_4 26214 // 25.6GB/s(25.6*1024MB/s)
#define DDRC_BANDWIDTH_CHAN_2 13107 // 12.8GB/s(12.8*1024MB/s)

#define DDR_PROF_TEST_BUFF_NUM 128

#define DDRC_CHIP_REG_BASE_OFFSET 0 // adapt for other chips

/* DDRC max channel number */
#define DDR_CHAN_BITMAP 0x00FFFFFF // default value, should update by dts file
#define DDRC_BANDWIDTH_PER_CHAN 6554 // 6.5GB/s(6.4*1024MB/s)

#endif
