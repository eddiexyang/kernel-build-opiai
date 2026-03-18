/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Create: 2022-08-13
 */

#include "bbox_sys_api.h"
#include <linux/rtc.h>
#include <linux/time.h>
#include <linux/syscalls.h>
#include <linux/timekeeping.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/module.h>


/*
 * @brief       : get the monotonic clock
 * @param [in]  : bbox_time *tm     time
 * @return      : NA
 */
void bbox_get_monotonic_clock(struct bbox_time *tm)
{
    if (tm != NULL) {
        struct timespec64 ts;
        ktime_get_ts64(&ts);
        tm->tv_sec = (u64)ts.tv_sec;
        tm->tv_nsec = (u64)ts.tv_nsec;
    }
}

#ifdef DEBUG
#define dsb_st()
#define dc_civac(addr)
#else
#define dsb_st() do { asm volatile("dsb st" : : : "memory"); } while (0)
#define dc_civac(addr) do { asm volatile("DC CIVAC ,%x0" ::"r"(addr)); } while (0)
#endif

void bbox_flush_cache(const u8 *base, u32 len)
{
    u32 i;
    const u32 blck = 64;  // 64B align
    const u32 num = (len + (blck - 1U)) / blck;
    const u8 *start = (u8 *)(uintptr_t)ALIGN(((uintptr_t)base - (blck - 1U)), blck);

    dsb_st();
    for (i = 0; i < num; i++) {
        dc_civac(start + (i * blck));
        mb();
    }
    dsb_st();
}

