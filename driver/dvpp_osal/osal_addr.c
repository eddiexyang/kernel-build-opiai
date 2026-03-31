#include "hi_osal.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/kern_levels.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/share_pool.h>
#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#include <linux/kallsyms.h>
#else
#include <linux/sched/mm.h>
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)

#ifndef CONFIG_64BIT
#include <mach/io.h>
#endif

#endif

void *osal_ioremap(hi_ulong phys_addr, hi_ulong size)
{
    return ioremap(phys_addr, size);
}
EXPORT_SYMBOL(osal_ioremap);

void *osal_ioremap_nocache(hi_ulong phys_addr, hi_ulong size)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
    return ioremap_nocache(phys_addr, size);
#else
    return ioremap(phys_addr, size);
#endif
}
EXPORT_SYMBOL(osal_ioremap_nocache);

void osal_iounmap(void *addr)
{
    iounmap(addr);
}
EXPORT_SYMBOL(osal_iounmap);

void *osal_ioremap_wc(hi_ulong phys_addr, hi_ulong size)
{
    return ioremap_wc(phys_addr, size);
}
EXPORT_SYMBOL(osal_ioremap_wc);

hi_ulong osal_copy_from_user(void *to, const void *from, hi_ulong n)
{
    return copy_from_user(to, from, n);
}
EXPORT_SYMBOL(osal_copy_from_user);

hi_ulong osal_copy_to_user(void *to, const void *from, hi_ulong n)
{
    return copy_to_user(to, from, n);
}
EXPORT_SYMBOL(osal_copy_to_user);

hi_s32 osal_access_ok(hi_s32 type, const void *addr, hi_ulong size)
{
#ifndef AOS_LLVM_BUILD
    HI_UNUSED(type);
    return access_ok(addr, size);
#else
    return 1;
#endif
}
EXPORT_SYMBOL(osal_access_ok);

hi_u32 osal_reg_rd(hi_ulong vir_addr)
{
    volatile hi_u32 *p_dst = (volatile hi_u32 *)vir_addr;
    hi_u32 val = 0;

    if (likely(p_dst != NULL)) {
        val = *p_dst;
    }

    return val;
}
EXPORT_SYMBOL(osal_reg_rd);

void osal_reg_wr(hi_ulong vir_addr, hi_u32 val)
{
    volatile hi_u32 *p_dst = (volatile hi_u32 *)vir_addr;
    if (likely(p_dst != NULL)) {
        *p_dst = val;
    }
}
EXPORT_SYMBOL(osal_reg_wr);

void *osal_symbol_get(const char *symbol)
{
    if (symbol == NULL) {
        return NULL;
    }
#ifndef AOS_LLVM_BUILD
    return __symbol_get(symbol);
#else
    return kallsyms_lookup_name(symbol);
#endif
}
EXPORT_SYMBOL(osal_symbol_get);

void osal_symbol_put(const char *symbol)
{
    if (symbol == NULL) {
        return;
    }
#ifndef AOS_LLVM_BUILD
    __symbol_put(symbol);
#endif
}
EXPORT_SYMBOL(osal_symbol_put);

hi_void *osal_find_vma(hi_void *mm, hi_ulong addr)
{
#ifndef AOS_LLVM_BUILD
    if (mm == NULL) {
        return NULL;
    }
    return find_vma((struct mm_struct *)mm, addr);
#else
    return NULL;
#endif
}
EXPORT_SYMBOL(osal_find_vma);

hi_void osal_down_read_mmap_sem(hi_void *mm)
{
    if (mm == NULL) {
        return;
    }
#ifndef AOS_LLVM_BUILD
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
    down_read(&((struct mm_struct *)mm)->mmap_lock);
#else
    down_read(&((struct mm_struct *)mm)->mmap_sem);
#endif
#endif
}
EXPORT_SYMBOL(osal_down_read_mmap_sem);

hi_void osal_up_read_mmap_sem(hi_void *mm)
{
    if (mm == NULL) {
        return;
    }
#ifndef AOS_LLVM_BUILD
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
    up_read(&((struct mm_struct*)mm)->mmap_lock);
#else
    up_read(&((struct mm_struct*)mm)->mmap_sem);
#endif
#endif
}
EXPORT_SYMBOL(osal_up_read_mmap_sem);

hi_bool osal_is_sharepool_addr(hi_ulong addr)
{
    return mg_is_sharepool_addr(addr);
}
EXPORT_SYMBOL(osal_is_sharepool_addr);

hi_s32 osal_mg_sp_id_of_current(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0) && !defined AOS_LLVM_BUILD
    return mg_sp_id_of_current();
#else
    return 0;
#endif
}
EXPORT_SYMBOL(osal_mg_sp_id_of_current);

hi_void *osal_mg_sp_make_share_k2u(hi_ulong kva, hi_ulong size,
    hi_ulong sp_flags, hi_s32 tgid, hi_s32 spg_id)
{
    return mg_sp_make_share_k2u(kva, size, sp_flags, tgid, spg_id);
}
EXPORT_SYMBOL(osal_mg_sp_make_share_k2u);

int osal_mg_sp_unshare(hi_ulong va, hi_ulong size, int spg_id)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
    return mg_sp_unshare(va, size, spg_id);
#else // #if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
#ifdef SHARE_POOL_GROUP_ID
    return mg_sp_unshare(va, size, spg_id);
#else // #ifdef SHARE_POOL_GROUP_ID
    HI_UNUSED(spg_id);
    return mg_sp_unshare(va, size);
#endif // #ifdef SHARE_POOL_GROUP_ID
#endif // #if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
}
EXPORT_SYMBOL(osal_mg_sp_unshare);

void *osal_mg_sp_make_share_u2k(hi_ulong uva, hi_ulong size, int tgid)
{
    return mg_sp_make_share_u2k(uva, size, tgid);
}
EXPORT_SYMBOL(osal_mg_sp_make_share_u2k);

void osal_mmput(void *mm)
{
    mmput((struct mm_struct *)mm);
}
EXPORT_SYMBOL(osal_mmput);

void osal_mmget(void *mm)
{
    mmget((struct mm_struct *)mm);
}
EXPORT_SYMBOL(osal_mmget);
