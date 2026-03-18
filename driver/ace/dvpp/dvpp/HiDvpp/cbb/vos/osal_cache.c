#include "hi_osal.h"

/*
 * 当前vos未提供cache相关接口，本源文件中相关接口暂未在vos侧业务中调用，暂时空实现
 */

void osal_cpuc_flush_dcache_area(const void *addr, hi_s32 size)
{
    HI_UNUSED(addr);
    HI_UNUSED(size);
}

void osal_flush_dcache_area(void *kvirt, hi_ulong phys_addr, hi_ulong length)
{
    HI_UNUSED(phys_addr);
    if (kvirt == NULL) {
        return;
    }
}

void osal_invalid_dcache_area(void *kvirt, hi_ulong phys_addr, hi_ulong length)
{
    HI_UNUSED(phys_addr);
    if (kvirt == NULL) {
        return;
    }
}
