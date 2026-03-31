/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * Create: 2023-08-12
 */

#ifndef TRS_TIMESTAMP_H
#define TRS_TIMESTAMP_H
#include <linux/ktime.h>
#include <linux/version.h>
#include <linux/jiffies.h>
static inline u64 trs_get_us_timestamp(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    struct timespec64 timestamp;
    ktime_get_ts64(&timestamp);
    return ((u64)timestamp.tv_sec * (u64)USEC_PER_SEC) + (u64)timestamp.tv_nsec / 1000ULL; // 1000 ns
#else
    struct timeval timestamp;
    do_gettimeofday(&timestamp);
    return ((u64)timestamp.tv_sec * (u64)USEC_PER_SEC) + (u64)timestamp.tv_usec;
#endif
}

static inline u64 trs_get_s_timestamp(void)
{
    return (jiffies / HZ);
}
#endif
