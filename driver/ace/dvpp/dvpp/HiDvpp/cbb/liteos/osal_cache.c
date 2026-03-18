/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2016-2020.
 * Description: osal cache source file.
 * Author: Hisilicon multimedia software group
 * Create: 2016-11-11
 */

#include "hi_osal.h"
#include "arch/cache.h"

void osal_flush_dcache_area(void *kvirt, hi_ulong phys_addr, hi_ulong length)
{
    HI_UNUSED(phys_addr);
    ArchDCacheCleanByAddr((UINTPTR)kvirt, (UINTPTR)kvirt + length);
}

void osal_invalid_dcache_area(void *kvirt, hi_ulong phys_addr, hi_ulong length)
{
    HI_UNUSED(phys_addr);
    ArchDCacheInvByAddr((UINTPTR)kvirt, (UINTPTR)kvirt + length);
}
