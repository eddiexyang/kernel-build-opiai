/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: Queue processing function
 * Author: huawei
 * Create: 2020-09-13
 */
#ifndef EVENT_ESCHED_INTERFACE_H
#define EVENT_ESCHED_INTERFACE_H

#if defined(__arm__) || defined(__aarch64__)
#else
#include <time.h>
#endif

#include "ascend_hal.h"

#define USEC_PER_SEC 1000000
#define NSEC_PER_USEC 1000
#ifdef USER_EVENT_SCHED_UT
#define SCHED_GET_CUR_SYSTEM_COUNTER(cnt) (cnt = 1000000)
#define SCHED_GET_SYSTEM_FREQ(cnt) (cnt = 1000000)
#else
#define SCHED_GET_CUR_SYSTEM_COUNTER(cnt) asm volatile("mrs %0, CNTVCT_EL0" : "=r"(cnt) :)
#define SCHED_GET_SYSTEM_FREQ(cnt) asm volatile("mrs %0, CNTFRQ_EL0" : "=r"(cnt) :)
#endif

struct event_res {
    unsigned int gid;
    unsigned int tid;
    unsigned int event_id;
    unsigned int subevent_id : 16; /* same as struct event_sync_msg */
};

enum sync_event_type {
    NORMAL_EVENT,
    QUEUE_EVENT,
    F2NF_EVENT,
    E2NE_EVENT,
    EVENT_TYPE_MAX
};

/* inline */
/* get cpu tick */
static inline unsigned long long eSchedGetCurCpuTick(void)
{
#if defined(__arm__) || defined(__aarch64__) || defined(USER_EVENT_SCHED_UT)
    unsigned long long cnt = 0;
    SCHED_GET_CUR_SYSTEM_COUNTER(cnt);
    return cnt;
#else
    struct timespec timestamp;
    (void)clock_gettime(CLOCK_MONOTONIC, &timestamp);
    return (unsigned long long)((timestamp.tv_sec * USEC_PER_SEC) + (timestamp.tv_nsec / NSEC_PER_USEC));
#endif
}

static inline unsigned long long eSchedGetCurCpuTimestamp(void)
{
    return eSchedGetCurCpuTick();
}

drvError_t halEschedSubmitEventEx(uint32_t devId, uint32_t dstDevId, struct event_summary *event);

int eSchedAllocEventRes(uint32_t devId, int32_t event_type, struct event_res *e_res);
void eSchedFreeEventRes(uint32_t devId, int32_t event_type, const struct event_res *e_res);
drvError_t eSchedQueryGrpType(uint32_t devId, uint32_t grpId, GROUP_TYPE *type);
drvError_t eschedCreateExtendGrp(uint32_t devId, unsigned int grpId, struct esched_grp_para *grpPara);

#ifndef EMU_ST
#include "sched_trace.h"
drvError_t RegisterEschedTraceRecordFunc(unsigned int grpId, unsigned int event_id,
    void (*finishFunc)(unsigned int grpId, unsigned int event_id, unsigned int subevent_id,
        SchedTraceTimeInfo *sched_trace_time_info));
#endif
#endif
