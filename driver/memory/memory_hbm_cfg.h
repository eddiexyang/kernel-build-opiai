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

#ifndef DRV_MEMORY_HBM_CFG_H
#define DRV_MEMORY_HBM_CFG_H

#include <linux/time.h>
#include "drv_profile.h"

#define HBMC_CHAN_REG_BASE_OFFSET 0x0000002000

/* HBM_CTRL_PERF register */
#define HBMC_CTRL_PERF_OFFSET 0x010
#define HBMC_PERF_EN_MASK 0x1
#define HBMC_PERF_EN_SET (1 << 0)

/* HBMC_HISI_FLUX_WR register */
#define HBMC_HISI_FLUX_WR_OFFSET 0x380
#define HBMC_HISI_FLUX_RD_OFFSET 0x384
#define HBMC_HISI_FLUXID_WR_OFFSET 0x390
#define HBMC_HISI_FLUXID_RD_OFFSET 0x394

/* HBMC mask ID configure value */
#define HBMC_CFG_STAID 0x270
#define HBMC_CFG_STAIDMSK 0x274

#define HBMC_CFG_STAID_SET 0x10000
#define HBMC_CFG_STAIDMSK_SET 0x3f800
#define HBMC_CFG_STAID_OFFSET 11

/* HBMC number */
#define HBM_TOTAL_NUM 4
#define HBM_MAX_CHAN_NUM 16

/* HBMC register op type, read or write */
#define HBMC_REG_WR 0x0
#define HBMC_REG_RD 0x1

#define HBMC_CHAN_NUM_MAX 16
#define HBMC_FLUX_STAT_TIME 100 // 100 ms
#define HBMC_FLUX_STAT_TIMEOUT 500
#define HBMC_MAP_SIZE (4096 * 4)
#define HBMC_STAT_PERTAGE 100

#define HBMC_MASTER_ID_ALL 0xFFFFFFFF
#define HBMC_FLUX_MB_DIV 32768
#define HBMC_TIME_STAMP_PER_SECOND 1000000

#define HBMC_BANDWIDTH_PER_CHAN 19661

#define MAX_CHIP_NUM 4
#define MAX_EVENT_NUM 2
#define HBMC_INFO_ITEM_NUM 4

#define UINT_TYPE_MAX 0xFFFFFFFF
#define HBM_PROF_EVENT_RD 0
#define HBM_PROF_EVENT_WR 1

#define US_PER_SECOND 1000000UL

#ifndef NSEC_PER_USEC
#define NSEC_PER_USEC 1000L
#endif

#define HBMC_REG_CLR_NUM 2
#define RD_FLUX_INDEX 0
#define WR_FLUX_INDEX 1
#define ID_RD_FLUX_INDEX 2
#define ID_WR_FLUX_INDEX 3


/* get timestamp flags */
#define HBM_TIMESTAMP_BEFORE_READ 0
#define HBM_TIMESTAMP_AFTER_READ 1

#define COMMON_MASTER 2

/* DDR profile cfg data */
typedef struct _hbm_prof_cfg {
	unsigned int period;			   // sample period
	unsigned int master_id;			// CPU ID
	unsigned int event_num;			// read or write event number
	unsigned int event[MAX_EVENT_NUM]; // event id, 0 read , 1 write
} hbm_prof_cfg_t;

/* profiling data return to user */
typedef struct _hbm_prof_profile_data {
	unsigned long long timestamp; // record time for calculate
	unsigned long long count;	 // flux count, the same as flux register unit
	unsigned int event;		   // record event type
	unsigned int hbm_id;		  // HBM ID
} hbm_prof_data_t;

/* HBM hardware info */
typedef struct _hbm_hw_info {
	unsigned int hbm_num;
	unsigned int chan_num;
	unsigned long reg_base[HBM_TOTAL_NUM][HBM_MAX_CHAN_NUM];
	void __iomem *reg_vir_addr[HBM_TOTAL_NUM][HBM_MAX_CHAN_NUM];
} hbm_hw_info_t;

/* HBM flux info record */
typedef struct _hbm_flux_info {
	unsigned int flux_rd[HBM_TOTAL_NUM][HBM_MAX_CHAN_NUM];
	unsigned int flux_wr[HBM_TOTAL_NUM][HBM_MAX_CHAN_NUM];
	unsigned int id_flux_rd[HBM_TOTAL_NUM][HBM_MAX_CHAN_NUM];
	unsigned int id_flux_wr[HBM_TOTAL_NUM][HBM_MAX_CHAN_NUM];
	struct timespec64 time_stamp[HBM_TOTAL_NUM];
} hbm_flux_info_t;

struct mata_readonce_info {
	unsigned int readonce;
};

typedef union {
	uint32_t	  value;
	struct {
		uint32_t  reg_lucwt_read_en		   : 1;
		uint32_t  reg_lucwt_static_en		 : 1;
		uint32_t  reg_lucwt_dynamic_en		: 1;
		uint32_t  reg_luc_nofwd_en			: 1;
		uint32_t  reg_luc_stashfwd			: 1;
		uint32_t  reg_luc_seqrd			   : 1;
		uint32_t  reg_luc_compfwd			 : 1;
		uint32_t  reg_luc_rduse_alloc		 : 1;
		uint32_t  reg_luc_wruse_alloc		 : 1;
		uint32_t  reg_luc_hitorder			: 1;
		uint32_t  reg_luc_cache_en			: 1;
		uint32_t  reg_luc_allocopt			: 1;
		uint32_t  reg_luc_remoteaic_wrbyp_dis : 1;
		uint32_t  reserved_0				  : 1;
		uint32_t  reg_luc_persist_id		  : 3;
		uint32_t  reg_luc_intlv_mode		  : 2;
		uint32_t  reg_luc_intlv_remap0		: 2;
		uint32_t  reg_luc_intlv_remap1		: 2;
		uint32_t  reg_luc_replace_type		: 2;
		uint32_t  reserved_1				  : 7;
	} reg;
} MATA_MATA_LUC_CTRL_UNION;

#ifdef CFG_MEMORY_HBM_STUB
#define HBM_BANDWIDTH_DEFAULT_RATE 17
#endif
#endif
