/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Description:
 * Author: huawei
 * Create: 2019-10-15
 */

#ifndef __DRV_LOG_FLOW_H__
#define __DRV_LOG_FLOW_H__
#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>

static unsigned long g_log_last_jiffies = 0;  /* ms */
static unsigned long g_log_count = 0;

#define LOG_MS_PER_SECOND 1000
#define LOG_US_PER_MSECOND 1000
#define LOG_LIMIT_RATE 30

/*lint -e551*/
static inline bool print_timed_ratelimit(unsigned long *caller_jiffies, unsigned long interval_msecs)
{
    unsigned long timer, time_now;
    struct timeval now;

    (void)gettimeofday(&now, NULL);
    time_now = (unsigned long)(now.tv_sec * LOG_MS_PER_SECOND + now.tv_usec / LOG_US_PER_MSECOND);
    timer =  time_now - *caller_jiffies;
    if ((*caller_jiffies != 0) && (timer <= interval_msecs)) {
        return false;
    }
    *caller_jiffies = time_now;
    return true;
}

/*lint +e551*/
static inline bool drv_log_rate_limit(int *count, int branch_rate, unsigned long limit_time)
{
    if (print_timed_ratelimit(&g_log_last_jiffies, limit_time)) {
        *count = 0;
        g_log_count = 0;
        return false;
    } else {
        (*count)++;
        g_log_count++;
        return  ((*count >= branch_rate)  || (g_log_count  >=  LOG_LIMIT_RATE));
    }
}
#endif
