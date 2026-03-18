#include "hi_osal.h"
#include "securec.h"

void *osal_ioremap(hi_ulong phys_addr, hi_ulong size)
{
    HI_UNUSED(size);
    return (void *)phys_addr;
}

void *osal_ioremap_nocache(hi_ulong phys_addr, hi_ulong size)
{
    return osal_ioremap(phys_addr, size);
}

void osal_iounmap(void *addr)
{
    return;
}

void *osal_ioremap_wc(hi_ulong phys_addr, hi_ulong size)
{
    return osal_ioremap(phys_addr, size);
}

hi_ulong osal_copy_from_user(void *to, const void *from, hi_ulong n)
{
    return (memcpy_s(to, n, from, n) != 0) ? n : 0;
}

hi_ulong osal_copy_to_user(void *to, const void *from, hi_ulong n)
{
    return (memcpy_s(to, n, from, n) != 0) ? n : 0;
}

hi_s32 osal_access_ok(hi_s32 type, const void *addr, hi_ulong size)
{
    // 是否直接返回成功？
    HI_UNUSED(type);
    HI_UNUSED(addr);
    HI_UNUSED(size);
    return 1;
}

hi_u32 osal_reg_rd(hi_ulong vir_addr)
{
    volatile hi_u32 *p_dst = (volatile hi_u32 *)vir_addr;
    hi_u32 val = 0;

    if (p_dst != NULL) {
        val = *p_dst;
    }

    return val;
}

void osal_reg_wr(hi_ulong vir_addr, hi_u32 val)
{
    volatile hi_u32 *p_dst = (volatile hi_u32 *)vir_addr;
    if (p_dst != NULL) {
        *p_dst = val;
    }
}

void *osal_symbol_get(const char *symbol)
{
    return NULL;
}

void osal_symbol_put(const char *symbol)
{
    return;
}