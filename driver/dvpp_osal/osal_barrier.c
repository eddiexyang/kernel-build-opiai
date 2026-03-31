#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/barrier.h>

#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#endif

void osal_mb(void)
{
    mb();
}
EXPORT_SYMBOL(osal_mb);
void osal_rmb(void)
{
    rmb();
}
EXPORT_SYMBOL(osal_rmb);
void osal_wmb(void)
{
    wmb();
}
EXPORT_SYMBOL(osal_wmb);
void osal_smp_mb(void)
{
    smp_mb();
}
EXPORT_SYMBOL(osal_smp_mb);
void osal_smp_rmb(void)
{
    smp_rmb();
}
EXPORT_SYMBOL(osal_smp_rmb);
void osal_smp_wmb(void)
{
    smp_wmb();
}
EXPORT_SYMBOL(osal_smp_wmb);
void osal_isb(void)
{
    isb();
}
EXPORT_SYMBOL(osal_isb);
void osal_dsb(void)
{
#ifdef AOS_LLVM_BUILD
    dsb(sy);
#else // #ifdef AOS_LLVM_BUILD
#ifdef CONFIG_64BIT
    dsb(sy);
#else
    dsb();
#endif
#endif // #ifdef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_dsb);
void osal_dmb(void)
{
#ifdef AOS_LLVM_BUILD
    dmb(sy);
#else // #ifdef AOS_LLVM_BUILD
#ifdef CONFIG_64BIT
    dmb(sy);
#else
    dmb();
#endif
#endif // #ifdef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_dmb);
