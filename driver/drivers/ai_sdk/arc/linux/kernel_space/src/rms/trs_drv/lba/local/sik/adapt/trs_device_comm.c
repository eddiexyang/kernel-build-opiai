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
* Create: 2023-8-3
*/
#include <linux/compiler_types.h>
#include <linux/export.h>
#include <asm/barrier.h>

#include "trs_pub_def.h"
#include "trs_device_comm.h"
#include "trs_core_smmu.h"

#define TRS_CACHELINE_SIZE           (64)
#define TRS_CACHELINE_MASK           (TRS_CACHELINE_SIZE - 1)

int trs_device_get_ssid(struct trs_id_inst *inst, int *ssid)
{
    return trs_core_ops_get_ssid(inst, ssid);
}
EXPORT_SYMBOL(trs_device_get_ssid);

void trs_flush_cache(u64 base, u32 len)
{
#ifdef __aarch64__
    u64 addr_loop, addr_end;

    addr_loop = base & (~TRS_CACHELINE_MASK);
    addr_end = (base + len + TRS_CACHELINE_MASK) & (~TRS_CACHELINE_MASK);

    asm volatile("dsb st"
                 :
                 :
                 : "memory");
    for (; addr_loop < addr_end;) {
        asm volatile("DC CIVAC ,%x0" ::"r"(addr_loop));
        mb();
        addr_loop += TRS_CACHELINE_SIZE;
    }
    asm volatile("dsb st"
                 :
                 :
                 : "memory");
#endif
}
EXPORT_SYMBOL(trs_flush_cache);

void trs_invalid_cache(u64 base, u32 len)
{
#ifdef __aarch64__
    u64 addr_loop, addr_end;

    addr_loop = base & (~TRS_CACHELINE_MASK);
    addr_end = (base + len + TRS_CACHELINE_MASK) & (~TRS_CACHELINE_MASK);

    asm volatile("dsb st"
                 :
                 :
                 : "memory");
    for (; addr_loop < addr_end;) {
        asm volatile("DC CIVAC ,%x0" ::"r"(addr_loop));
        mb();
        addr_loop += TRS_CACHELINE_SIZE;
    }
    asm volatile("dsb st"
                 :
                 :
                 : "memory");
#endif
}
EXPORT_SYMBOL(trs_invalid_cache);

static u32 feature_mode = ALL_FEATURE_MODE;

void trs_set_feature_mode(void)
{
    feature_mode = PART_FEATURE_MODE;
}
EXPORT_SYMBOL(trs_set_feature_mode);

u32 trs_get_feature_mode(void)
{
    return feature_mode;
}
EXPORT_SYMBOL(trs_get_feature_mode);

