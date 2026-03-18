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

#ifndef PROF_MEM_SAMPLE_H
#define PROF_MEM_SAMPLE_H

#include <linux/types.h>
#include <linux/time64.h>

#define PROF_MEM_SAMPLE_PROC_MODE     0
#define PROF_MEM_SAMPLE_DEV_MODE      1

#define PROF_MEM_SAMPLE_MAX_DEV_NUM   64
#define PROF_MEM_SAMPLE_MAX_VF_NUM    32

struct  prof_mem_sample_data {
    u32 timestamp;
    u32 event;
    u64 rsv;
    u64 ddr_used_size;
    u64 hbm_used_size;
};

static inline u64 prof_get_time_interval(struct timespec64 *start, struct timespec64 *end)
{
    return ((end->tv_sec - start->tv_sec) * USEC_PER_SEC + (end->tv_nsec - start->tv_nsec) / NSEC_PER_USEC);
}

#endif
