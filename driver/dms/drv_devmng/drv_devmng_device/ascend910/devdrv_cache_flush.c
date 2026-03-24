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

#include <linux/types.h>
#include <asm/barrier.h>
#include "devdrv_manager.h"

void devdrv_flush_cache(u64 base, u32 len)
{
    uint64_t addr_loop, addr_end;

    addr_loop = base & (~DEVDRV_CACHELINE_MASK);
    addr_end = (base + len) & (~DEVDRV_CACHELINE_MASK);

    asm volatile("dsb st"
                 :
                 :
                 : "memory");
    for (; addr_loop < addr_end;) {
        asm volatile("DC CIVAC ,%x0" ::"r"(addr_loop));
        mb();
        addr_loop += DEVDRV_CACHELINE_SIZE;
    }
    asm volatile("dsb st"
                 :
                 :
                 : "memory");
}

u64 devdrv_read_cntpct(void)
{
    u64 cntpct;
    asm volatile("mrs %0, cntpct_el0"
                 : "=r"(cntpct));
    return cntpct;
}
