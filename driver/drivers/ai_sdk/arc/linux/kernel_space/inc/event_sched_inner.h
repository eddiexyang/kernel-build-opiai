/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#ifndef EVENT_SCHED_INNER_H
#define EVENT_SCHED_INNER_H

#define SCHED_SUCCESS 0

#define SCHED_MAX_DEFAULT_GRP_NUM 32
#define SCHED_MAX_EX_GRP_NUM  32
#define SCHED_MAX_GRP_NUM (SCHED_MAX_DEFAULT_GRP_NUM + SCHED_MAX_EX_GRP_NUM)

#define SCHED_INVALID_DEVID 0xFFFFFFFFU
#define SCHED_INVALID_TID 0xFFFFFFFFU
#define SCHED_INVALID_GID 0xFFFFFFFFU
#ifdef ESCHED_HOST
#define SCHED_DEFAULT_THREAD_NUM_IN_GRP 256
#else
#define SCHED_DEFAULT_THREAD_NUM_IN_GRP 16
#endif
#define SCHED_MAX_THREAD_NUM_IN_GRP     1024

#ifdef STATIC_SKIP
#  define STATIC
#else
#  define STATIC    static
#endif

#define SCHED_THREAD_SWAPOUT   (-2)
#define SCHED_WAIT_TIMEOUT_GIVEUP   SCHED_THREAD_SWAPOUT
#define SCHED_WAIT_TIMEOUT_SWAPOUT  SCHED_THREAD_SWAPOUT    /* GIVEUP and SWAPOUT is same */
#define SCHED_WAIT_TIMEOUT_ALWAYS_WAIT (-1)
#define SCHED_WAIT_TIMEOUT_NO_WAIT 0

struct sched_published_event_info {
    unsigned int dst_engine;
    unsigned int policy;
    int pid;
    unsigned int gid;
    unsigned int tid;
    unsigned int event_id;
    unsigned int subevent_id;
    unsigned int dst_devid;
    unsigned int msg_len;
    char *msg;
    unsigned int event_num;
    unsigned long long publish_timestamp;
    unsigned long long publish_timestamp_of_day;
    void *priv; /* Private event info for ack/finish. */
};

struct sched_published_event_func {
    int (*event_ack_func)(unsigned int devid, unsigned int subevent_id, const char *msg,
        unsigned int msg_len, void *priv);
    void (*event_finish_func)(unsigned int devid, unsigned int subevent_id, const char *msg,
        unsigned int msg_len, void *priv);
};

struct sched_published_event {
    struct sched_published_event_info event_info;
    struct sched_published_event_func event_func;
};

struct sched_subscribed_event {
    int pid;
    int host_pid;
    unsigned int gid;
    unsigned int event_id;
    unsigned int subevent_id;
    unsigned int msg_len; /* input msg buff size */
    char *msg;
    unsigned long long publish_timestamp;
    unsigned long long publish_wakeup_timestamp;
    unsigned long long subscribe_timestamp;
};

int sched_submit_event(unsigned int chip_id, struct sched_published_event *event);
int sched_query_local_task_gid(unsigned int chip_id, int pid, const char *grp_name, unsigned int *gid);

typedef int (*sched_event_pre_proc)(unsigned int devid, struct sched_published_event_info *event_info,
    struct sched_published_event_func *event_func);
int sched_register_event_pre_proc_handle(unsigned int event_id, sched_event_pre_proc handle);
void sched_unregister_event_pre_proc_handle(unsigned int event_id);

#endif

