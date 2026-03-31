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
#ifndef PROF_DRV_HDC_DEV_H
#define PROF_DRV_HDC_DEV_H

#include "prof_def.h"

#define PROF_HDC_EVENT_NUM_MAX 256
#define PROF_HDC_EVENT_NUM 32

#define HDCDRV_EPOLL_CTL_PARA_NUM 4

#define PROF_SESSION_MAX_NUM 32

/* same as HDC_EPOLL_OP_* */
#define HDC_EPOLL_CTL_ADD 0
#define HDC_EPOLL_CTL_DEL 1

/* same as HDC_EPOLL_* */
#define HDC_EPOLL_CONN_IN (0x1 << 0)
#define HDC_EPOLL_DATA_IN (0x1 << 1)
#define HDCDRV_EPOLL_FAST_DATA_IN (0x1 << 2)
#define HDCDRV_EPOLL_SESSION_CLOSE (0x1 << 3)

#define HDCDRV_TX_REMOTE_CLOSE (-15)
#define HDC_SERVICE_TYPE_PROF 11
#define HDCDRV_EPOLL_OP_ADD     0
#define HDCDRV_EPOLL_OP_DEL     1
#define PROF_SESSION_ID_LEN 32
#define HDC_SEGMENT_LEN (0x1 << 21)

#define PROF_HDC_CLOSE_WAIT_MAX_TIME 10 /* 10ms */
#define PROF_HDC_RECYCLE_GUARD_INTERVAL 10000 /* 10s */
#define PROF_AGENT_UNINIT_EPOLL_MAX_WAIT_CNT 100 /* 100 */
#define PROF_AGENT_UNINIT_WORK_MAX_WAIT_CNT 10 /* 10 */
#define PROF_AGENT_UNINIT_WORK_PER_WAIT_TIME 1000 /* 1s */

enum prof_poll_status {
    PROF_POLL_DISABLE = 0,
    PROF_POLL_ENABLE,
    PROF_POLL_IDLE,
    PROF_POLL_READABLE
};

struct prof_hdc_epoll {
    int epfd;
    int magic_num;
};

struct prof_hdc_server {
    int valid;
    u32 device_id;
    u32 server_type;
    struct mutex mutex;
    struct list_head session_list;
};

struct prof_hdc_session {
    struct list_head list;
    int device_id;
    int vfid;
    int session_fd;
    char session_id[PROF_SESSION_ID_LEN];
    struct prof_proc_ctx ctx;
    volatile int poll_task_run_status;
    volatile int poll_task_stop_flag;
    struct task_struct *poll_task;
};

struct prof_channel_hdc_session {
    struct mutex mutex;
    struct prof_hdc_session *prof_hdc_session_info[PROF_SUB_CHANNEL_NUM_MAX];
};

struct prof_vf_hdc_session {
    struct prof_channel_hdc_session prof_channel_hdc_session_info[PROF_CHANNEL_NUM];
};

struct prof_device_hdc_session {
    struct prof_vf_hdc_session prof_vf_hdc_session_info[PROF_VFID_NUM_MAX];
};

struct prof_hdc_recycle_list {
    struct list_head session_list;
    struct mutex mutex;
    struct delayed_work guard_work;
};

int prof_hdc_agent_init(void);
void prof_hdc_agent_uninit(void);
int prof_hdc_init_each_device(u32 device_id);
void prof_hdc_uninit_each_device(u32 device_id);
#endif
