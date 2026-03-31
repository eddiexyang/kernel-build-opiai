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

#ifndef PROF_DEF_H
#define PROF_DEF_H

#include <linux/wait.h>
#include <linux/securec.h>
#include <linux/semaphore.h>
#include <linux/types.h>

#include "prof_host_dev_com.h"
#include "prof_user_ker_com.h"
#include "prof.h"

#include "drv_profile.h"
#ifdef CFG_FEATURE_SRIOV
#define PROF_DEVICE_NUM_VALUE 64
#else
#define PROF_DEVICE_NUM_VALUE 4
#endif
#define PROF_CHANNEL_NUM   160
#define PROF_SQ_BUF_LEN  128

#ifdef __aarch64__
void flush_cache(unsigned char *base, u32 len);
void invalidate_cache(unsigned char *base, u32 len);
#endif

typedef struct prof_poll_box {
    u32 device_id;
    u32 channel_id;
} prof_poll_box_t;

typedef struct prof_poll_info_kernel {
    struct prof_poll_box *poll_box;
    spinlock_t spinlock; /* can used in the interruption */
    wait_queue_head_t poll_wq;
    u32 poll_head;
    u32 poll_tail;
    u32 status;
    atomic_t fd_num;
} prof_poll_info_kernel_t;

struct prof_sub_channel_res {
    u8 count[PROF_DEVICE_NUM_VALUE];
    u8 sub_channel_id[PROF_DEVICE_NUM_VALUE];
    struct mutex mutex;
};

struct prof_proc_ctx {
    u32 collect_target_pid;
    struct prof_poll_info_kernel poll;
    struct prof_sub_channel_res sub_channel_res; /* add for ascend910B */
};


enum prof_ts_buff_free_flag {
    TS_BUFF_FREE,
    TS_BUFF_NOT_FREE
};

struct ts_cpu_channel {
    u32 cmd_verify;
    u32 tsid;
    int ret_val;
    enum prof_ts_buff_free_flag ts_buff_free_flag;
};

/* ts and peripheral should't see this struct, need to optimize */
typedef struct prof_channel_info {
    u32 device_id;
    u32 vfid;
    u32 channel_id;
    struct mutex mutex;
    u32 channel_used_num_max;
    u32 channel_used_count;
    struct prof_sub_channel_info *sub_channel_info[PROF_SUB_CHANNEL_NUM_MAX];
} prof_channel_info_t;

struct prof_sample_hrtimer {
    struct hrtimer timer;
    ktime_t kt;
    struct semaphore sync_timer_sema;
};

#define PROF_PERIPHERAL_CACHE_NUM  10
typedef struct prof_peripheral_buff_head {
    u32 head;
    u32 tail;
    u32 buff_num;
    u32 one_buff_len;
    u32 data_len[PROF_PERIPHERAL_CACHE_NUM];
} prof_peripheral_buff_head_t;

typedef int (*prof_sample_userdata_handle)(struct prof_peri_para para);
typedef int (*prof_sample_handle)(struct prof_peri_para para);
typedef int (*prof_sample_stop_handle)(struct prof_peri_para para);
struct peripheral_channel {
    u32 sample_period;  /* sample period ; unit: ms */
    u32 user_data_size; /* user data's size */
    char user_data[PROF_USER_DATA_LEN];
    prof_sample_userdata_handle prof_sample_userdata_handle;
    prof_sample_handle prof_sample_handle;
    prof_sample_stop_handle prof_sample_stop_handle;
    int sample_thread_flag;
    struct prof_sample_hrtimer sample_hrtimer;
    struct task_struct *sample_thread;
    struct prof_peripheral_buff_head head;
};

struct prof_sub_channel_info {
    u32 channel_state;
    u32 device_id;
    u32 vfid;           /* vfid = 0, is physical machine; vfid = 1~16,  is virtual machine */
    u32 channel_id;
    u32 sub_channel_id;
    u32 buf_len;
    int poll_flag;
    unsigned long phy_addr;
    unsigned char *vir_addr;
    struct mutex state_mutex;
    struct prof_proc_ctx *proc_ctx;
    spinlock_t spinlock;
    wait_queue_head_t channel_wq;

    struct ts_cpu_channel ts_channel;
    struct peripheral_channel peri_channel;
    struct prof_dev_dfx_info prof_dfx;
};

#endif

