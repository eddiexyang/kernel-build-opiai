/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2016-2020.
 * Description: osal address source file.
 * Author: Hisilicon multimedia software group
 * Create: 2016-11-11
 */

#include "hi_osal.h"
#include "securec.h"

static inline void *ioremap_nocache(unsigned long physAddr, unsigned long size)
{
    return (void *)physAddr;
}

static inline void iounmap(void *addr)
{
}

void *osal_ioremap(hi_ulong phys_addr, hi_ulong size)
{
    return ioremap_nocache(phys_addr, size);
}

void *osal_ioremap_nocache(hi_ulong phys_addr, hi_ulong size)
{
    return ioremap_nocache(phys_addr, size);
}

void *osal_ioremap_wc(hi_ulong phys_addr, hi_ulong size)
{
    return ioremap_nocache(phys_addr, size);
}

void osal_iounmap(void *addr)
{
    iounmap(addr);
}

hi_ulong osal_copy_from_user(void *to, const void *from, hi_ulong n)
{
    return (memcpy_s(to, n, from, n) != 0) ? 0 : n;
}

hi_ulong osal_copy_to_user(void *to, const void *from, hi_ulong n)
{
    return (memcpy_s(to, n, from, n) != 0) ? 0 : n;
}

hi_s32 osal_access_ok(hi_s32 type, const void *addr, hi_ulong size)
{
    HI_UNUSED(type);
    HI_UNUSED(addr);
    HI_UNUSED(size);
    HI_TRACE_OSAL(HI_DBG_ERR, "not supported!\n");
    return 0;
}
