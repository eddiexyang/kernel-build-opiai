#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#ifndef AOS_LLVM_BUILD
#include <asm/cacheflush.h>
#include <linux/dma-direction.h>
#endif

#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#endif
void osal_cpuc_flush_dcache_area(const void *addr, hi_s32 size)
{
    HI_UNUSED(addr);
    HI_UNUSED(size);
}
EXPORT_SYMBOL(osal_cpuc_flush_dcache_area);

void osal_flush_dcache_area(void *kvirt, hi_ulong phys_addr, hi_ulong length)
{
    HI_UNUSED(phys_addr);
    if (kvirt == NULL) {
        return;
    }
}
EXPORT_SYMBOL(osal_flush_dcache_area);

void osal_invalid_dcache_area(void *kvirt, hi_ulong phys_addr, hi_ulong length)
{
    HI_UNUSED(phys_addr);
    if (kvirt == NULL) {
        return;
    }
}
EXPORT_SYMBOL(osal_invalid_dcache_area);

