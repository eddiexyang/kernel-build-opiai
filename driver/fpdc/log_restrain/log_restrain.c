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
#include <linux/version.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>

#include "fpdc_define.h"
#include "log_restrain/log_restrain.h"

static inline unsigned long get_monotonic_sec(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    struct timespec64 ts = {0};
    ktime_get_raw_ts64(&ts);
    return (ts.tv_sec + (ts.tv_nsec / NSEC_PER_SEC));
#else
    struct timespec ts = {0};
    getrawmonotonic(&ts);
    return (ts.tv_sec + (ts.tv_nsec / NSEC_PER_SEC));
#endif
}

int log_restrain(struct log_res *restrain, unsigned long period, unsigned int max_log_cnt, const char *desc)
{
    unsigned long cur = get_monotonic_sec();
    if (cur - restrain->start_time > period) { /* out of time window, reset */
        restrain->start_time = cur;
        restrain->log_cnt = 1;
        if (restrain->restrain_cnt) {
            fpdc_err("Log restrain recover. (restrain_cnt=%u; module=\"%s\")\n", restrain->restrain_cnt, desc);
            restrain->restrain_cnt = 0;
        }
        return 0;
    }

    if (restrain->log_cnt < max_log_cnt) {
        restrain->log_cnt++;
        return 0;
    }

    restrain->restrain_cnt++;
    return 1;
}

