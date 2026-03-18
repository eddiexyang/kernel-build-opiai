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

#ifndef TSDRV_HEART_BEAT_H
#define TSDRV_HEART_BEAT_H

#include <linux/types.h>
#ifdef AOS_LLVM_BUILD
#include <linux/atomic.h>
#include <linux/sched.h>
#endif
#include "tsdrv_heartbeat_dfx.h"

#define TSDRV_HEARTBEAT_STOP       0
#define TSDRV_HEARTBEAT_WORK       1

#define TSDRV_HEARTBEAT_UNINIT 0
#define TSDRV_HEARTBEAT_INITED 1
#define TSDRV_HEARTBEAT_MAX_TS_NUM DEVDRV_MAX_TS_NUM

struct ts_instance {
    u32 devid;
    u32 tsid;
};

struct tsdrv_heart_beat_msg_chan {
    u32 sq_index;
    u32 cq_index;
};

struct tsdrv_heart_beat_resp_msg {
    u32 sn;
};

#ifdef CFG_FEATURE_TS_HB_DFX
#define TS_STATUS_ERR_SHOW 0
#define TS_HEART_BEAT_LOST_SHOW 1
struct tsdrv_heart_beat_tsfw_dfx_reg {
    unsigned char *heart_beart_dotting;
    unsigned char *main_thread_dotting;
    unsigned char *doorbell_dotting;
};
#endif

struct tsdrv_heart_beat_info {
    struct tsdrv_heart_beat_msg_chan msg_chan;
    struct tsdrv_heart_beat_resp_msg resp_msg;
    struct ts_instance instance;
    struct task_struct *tsk;
    atomic_t work_state;
    u8 init_state;
    u8 lost_count;
    struct mutex lock;
    u32 msg_sn;
#ifdef CFG_FEATURE_TS_HB_DFX
    struct tsdrv_heart_beat_tsfw_dfx_reg tsfw_reg;
#endif
    int chan_id;
};

int tsdrv_heart_beat_init(u32 devid);
void tsdrv_heart_beat_exit(u32 devid);
void tsdrv_heart_beat_set_work_state(u32 devid, u32 tsid, u8 state);
struct tsdrv_heart_beat_info *tsdrv_hb_get_heart_beat_info(u32 devid, u32 tsid);
#ifdef CFG_FEATURE_TS_HB_DFX
void tsdrv_hb_tsfw_dfx_reg_show(u32 ts_status, struct tsdrv_heart_beat_info *heart_beat, u32 flag);
#endif

void tsdrv_hb_cq_callback(u32 devid, u32 tsid, const u8 *cq_slot, u8 *sq_slot);

#endif
