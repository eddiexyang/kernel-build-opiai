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

#ifndef DEVDRV_MANAGER_LLC_H
#define DEVDRV_MANAGER_LLC_H

#include <linux/time.h>
#include <linux/hrtimer.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>

#include "drv_profile.h"
#include "ascend_platform.h"

#define L3T_TOTAL_NUM_MAX 4

/* L3T_DFX_CTRL register */
#define L3T_DFX_CTRL_OFFSET 0x0408
#define L3T_PERF_EVT_EN_BIT 16
#define L3T_PERF_EVT_EN_MASK 0x10000
#define L3T_PERF_DFX_ENABLE 0x1
#define L3T_PERF_DFX_DISABLE 0x0
#define L3T_DFX_CTRL_DEFAULT 0x00060000

/* L3T_EVENT_CTRL register */
#define L3T_EVT_CTRL_OFFSET 0x1C00
#define L3T_EVT_CTRL_SET 0xFF

/* L3T_EVT_TYPE0 register */
#define L3T_EVT_TYPE0_OFFSET 0x1D00
#define L3T_EVT_TYPE1_OFFSET 0x1D04
#define L3T_EVT_CNT0_TYPE_MASK 0x00000000000000FF
#define L3T_EVT_CNT1_TYPE_MASK 0x000000000000FF00
#define L3T_EVT_CNT2_TYPE_MASK 0x0000000000FF0000
#define L3T_EVT_CNT3_TYPE_MASK 0x00000000FF000000
#define L3T_EVT_CNT4_TYPE_MASK 0x000000FF00000000
#define L3T_EVT_CNT5_TYPE_MASK 0x0000FF0000000000
#define L3T_EVT_CNT6_TYPE_MASK 0x00FF000000000000
#define L3T_EVT_CNT7_TYPE_MASK 0xFF00000000000000

#define L3T_EVT_CNT0_TYPE_BIT 0
#define L3T_EVT_CNT1_TYPE_BIT 8
#define L3T_EVT_CNT2_TYPE_BIT 16
#define L3T_EVT_CNT3_TYPE_BIT 24
#define L3T_EVT_CNT4_TYPE_BIT 32
#define L3T_EVT_CNT5_TYPE_BIT 40
#define L3T_EVT_CNT6_TYPE_BIT 48
#define L3T_EVT_CNT7_TYPE_BIT 56

/* L3T EVT_CNT */
#define L3T_EVT_CNT0_L_OFFSET 0x1E00
#define L3T_EVT_CNT0_H_OFFSET 0x1E04
#define L3T_EVT_CNT1_L_OFFSET 0x1E08
#define L3T_EVT_CNT1_H_OFFSET 0x1E0C
#define L3T_EVT_CNT2_L_OFFSET 0x1E10
#define L3T_EVT_CNT2_H_OFFSET 0x1E14
#define L3T_EVT_CNT3_L_OFFSET 0x1E18
#define L3T_EVT_CNT3_H_OFFSET 0x1E1C
#define L3T_EVT_CNT4_L_OFFSET 0x1E20
#define L3T_EVT_CNT4_H_OFFSET 0x1E24
#define L3T_EVT_CNT5_L_OFFSET 0x1E28
#define L3T_EVT_CNT5_H_OFFSET 0x1E2C
#define L3T_EVT_CNT6_L_OFFSET 0x1E30
#define L3T_EVT_CNT6_H_OFFSET 0x1E34
#define L3T_EVT_CNT7_L_OFFSET 0x1E38
#define L3T_EVT_CNT7_H_OFFSET 0x1E3C

#define L3T_EVT_CNT_H_MASK 0x0000FFFF

#define L3T_EVT_NUM_MAX 0x8
#define L3T_EVT_REG_NUM 16
#define L3T_EVT_REG_BITS 32

/* L3T register op type, read or write */
#define L3T_REG_RD 0x0
#define L3T_REG_WR 0x1
#define L3T_MAP_SIZE (4096 * 2)
#define LLC_STAT_PERTAGE 100

#define LLC_PROF_TYPE_NONE 0
#define LLC_PROF_TYPE_RD_HIT 1
#define LLC_PROF_TYPE_WR_HIT 2
#define LLC_PROF_TYPE_MAX 3

#define LLC_EVT_TYPE0_RD_HIT 0x20340100
#define LLC_EVT_TYPE1_RD_HIT 0x02221336
#define LLC_EVT_TYPE0_WR_HIT 0x21350100
#define LLC_EVT_TYPE1_WR_HIT 0x03231437
#define LLC_EVT_TYPE0_MASK 0xFFFFFFFF
#define LLC_EVT_CNT_RDWR_OFFSET 0x2
#define LLC_EVT_CNT_RDWR_NUM 0x4

#define LLC_BIT_WIDTH_BYTE 64
#define LLC_TIME_STAMP_PER_SECOND 1000000UL
#define LLC_THROUGHPUT_DIV 1024   /* KB as unit */
#define LLC_DSMI_DFX_TIME 50000  /* 50 ms */
#define LLC_SLEEP_UINT_MS 10      /* 10ms */

#define NO_SAMPLE_EVENT -2
#define FAILED (-1)
#define OK 0
#define LLC_CNT_MAX 0xFFFFFFFFFFFF

#define DFX_STAT_NONE 0
#define DFX_STAT_DSMI_RUNNING 1
#define DFX_STAT_PROF_RUNNING 2
#define DFX_STAT_ALL_RUNNING (DFX_STAT_DSMI_RUNNING | DFX_STAT_PROF_RUNNING)

#define DFX_STAT_HRTIMER_PERIOD 2000000  /* 2000 us,ns as unit */
#define DFX_WAIT_HRTIMER_START_TIME     2 /* 2ms */

#define LLC_DFX_HOST_NONE 0x0
#define LLC_DFX_HOST_DSMI 0x1
#define LLC_DFX_HOST_PROF 0x2

/* Device ID */
#define DEVICE_ID_0 0x0
#define DEVICE_ID_1 0x1
#define DEVICE_ID_2 0x2
#define DEVICE_ID_3 0x3

#ifndef NSEC_PER_USEC
#define NSEC_PER_USEC 1000L
#endif

#ifndef USEC_PER_SEC
#define USEC_PER_SEC 1000000L
#endif

#define LLC_EVT_REG_NUM_PER_CHAN 2

#define LLC_EVT_TYPE_REG_NUM 2

#define LLC_GET_ERROR_NUM   3   /* write + read */

/* LLC profile cfg data */
typedef struct _llc_prof_cfg {
    u32 period;       /* sample period */
    u32 sample_type;  /* sample type */
} llc_prof_cfg_t;

/* profiling data return to user */
typedef struct _llc_prof_profile_data {
    u64 timestamp;  /* record time for calculate */
    u64 count;      /* LLC count, the same as flux register unit */
    u32 event_id;   /* record event type */
    u32 l3_tag_id;  /* LLC channel ID */
} llc_prof_data_t;

/* l3t hardware info */
typedef struct _llc_hw_info {
    u64 reg_base[L3T_TOTAL_NUM];
    void __iomem *regs[L3T_TOTAL_NUM];
} llc_hw_info_t;

/* L3T flux info record */
typedef struct _llc_evt_info {
    u64 event_type[L3T_TOTAL_NUM][L3T_EVT_NUM_MAX];
    u64 event_cnt[L3T_TOTAL_NUM][L3T_EVT_NUM_MAX];
    struct timespec64 time_stamp[L3T_TOTAL_NUM];
} llc_evt_info_t;

/* LLC DSMI performace para */
typedef struct _llc_perf_para {
    unsigned int dev_id;
    unsigned int wr_hit_rate;
    unsigned int rd_hit_rate;
    unsigned int throughput;
} llc_perf_para_t;

typedef struct _llc_sample_info {
    unsigned int sample_type;
    unsigned int triggering_cnt;
    unsigned long total_time;
    llc_evt_info_t evt_info;
} llc_sample_info_t;

/* llc counter info for calculate */
typedef struct _llc_cnt_info {
    u64 total_rd_cnt;
    u64 total_wr_cnt;
    u64 hit_rd_cnt;
    u64 hit_wr_cnt;
    u64 throughput_cnt;
} llc_cnt_info_t;

/* llc back up info */
typedef struct _llc_dsmi_backup_info {
    llc_perf_para_t last_result;
    unsigned int error_cnt;
} llc_dsmi_backup_info_t;

typedef struct _llc_prof_backup_info {
    llc_sample_info_t last_result;
    unsigned int error_cnt;
} llc_prof_backup_info_t;

/* llc related info for sample */
typedef struct _llc_info {
    llc_hw_info_t hw_info;
    llc_evt_info_t cur_evt_info;
    llc_evt_info_t bk_evt_info;
    llc_prof_cfg_t prof_cfg;
    llc_sample_info_t dsmi_info;
    llc_dsmi_backup_info_t dsmi_back;
    llc_sample_info_t prof_info;
    llc_prof_backup_info_t prof_back;
    struct mutex dsmi_lock;
    struct mutex dfx_lock;
    struct hrtimer dfx_timer;
    struct tasklet_struct tasklet;
    u32 dfx_state;
    u32 cur_sample_host;
    u32 dev_id;
} llc_info_t;

/* exports */
int devdrv_llc_prof_sample(struct prof_peri_para para);
int devdrv_llc_prof_sample_userdata(struct prof_peri_para para);
int devdrv_llc_get_perf_para(unsigned int dev_id, unsigned int para_type, llc_perf_para_t *para);
int devdrv_llc_prof_sample_stop(struct prof_peri_para para);
int devdrv_llc_init(unsigned int dev_id);
void devdrv_llc_exit(unsigned int dev_id);

#endif
