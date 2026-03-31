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

#ifndef DRV_MEMORY_DDR_CFG_H
#define DRV_MEMORY_DDR_CFG_H

#include <linux/time.h>
#include "drv_profile.h"
#include "memory_ddr_reg.h"

/* DDRC_CTRL_PERF register */
#define DDRC_PERF_EN_MASK 0x1
#define DDRC_PERF_EN_SET (1 << 0)

/* DDRC mask ID configure value */
#define DDRC_CFG_STAID_SET 0x10000
#define DDRC_CFG_STAIDMSK_SET 0x3f800
#define DDRC_CFG_STAID_OFFSET 11

/* DDRC register op type, read or write */
#define DDRC_REG_WR 0x0
#define DDRC_REG_RD 0x1

#define DDRC_MASTER_ID_ALL 0xFFFFFFFF
#define DDRC_TIME_STAMP_PER_SECOND 1000000

#define MAX_CHIP_NUM 4
#define MAX_EVENT_NUM 2
#define DDRC_INFO_ITEM_NUM 4
#define DDRC_BW_ITEM_NUM 2
#define DDRC_CLEAR_ITEM_NUM 2

#define UINT_TYPE_MAX 0xFFFFFFFF
#define DDR_PROF_EVENT_RD 0
#define DDR_PROF_EVENT_WR 1

#ifndef NSEC_PER_USEC
#define NSEC_PER_USEC 1000L
#endif
#define US_PER_SECOND 1000000UL
#define US_PER_MSECOND 1000L
#define SAMPLE_FIRST 0
#define SAMPLE_SECOND 1

#define COMMON_MASTER 2

/* DDR profile cfg data */
typedef struct _ddr_prof_cfg {
	unsigned int period;			   /* sample period */
	unsigned int master_id;			/* CPU ID */
	unsigned int event_num;			/* read or write event number */
	unsigned int event[MAX_EVENT_NUM]; /* event id, 0 read , 1 write */
} ddr_prof_cfg_t;

/* profiling data return to user */
typedef struct _ddr_prof_profile_data {
	unsigned int timestamp; /* record time for calculate */
	unsigned int count;	 /* flux count, the same as flux register unit */
	unsigned int event;	 /* record event type */
	unsigned int ddr_id;	/* DDR channel ID */
	unsigned int master_id; /* CPU ID 0xff if not set */
} ddr_prof_data_t;

/* DDR flux info record */
typedef union _ddr_flux_info {
	struct {
		unsigned int flux_rd[DDR_MAX_CHAN_NUM];
		unsigned int flux_wr[DDR_MAX_CHAN_NUM];
		unsigned int id_flux_rd[DDR_MAX_CHAN_NUM];
		unsigned int id_flux_wr[DDR_MAX_CHAN_NUM];
		struct timespec64 time_stamp[DDR_MAX_CHAN_NUM];
	};
	/* bs9sx1a flux info record */
	struct {
		unsigned long long total_time_stamp;
		unsigned long long total_flux_rd;
		unsigned long long total_flux_wr;
	};
} ddr_flux_info_t;

struct memory_info {
	unsigned int dev_id;
	unsigned int type; // DDR or HBM
	unsigned int chan_num;
	unsigned int bitmap;
	unsigned int ecc_sigle_bit;
	unsigned int ecc_double_bit;
};

#ifdef CFG_MEMORY_DDR_STUB
#define DDR_BANDWIDTH_DEFAULT_RATE 17
#endif
#endif
