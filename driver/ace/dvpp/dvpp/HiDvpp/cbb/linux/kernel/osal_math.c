#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/random.h>
#include <linux/version.h>
#include "hi_osal.h"
#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#include <linux/math64.h>
#endif // #ifdef AOS_LLVM_BUILD
/* the result of u64/u32. */
hi_u64 osal_div_u64(hi_u64 dividend, hi_u32 divisor)
{
    return div_u64(dividend, divisor);
}
EXPORT_SYMBOL(osal_div_u64);

/* the result of s64/s32. */
hi_s64 osal_div_s64(hi_s64 dividend, hi_s32 divisor)
{
#ifndef AOS_LLVM_BUILD
    return div_s64(dividend, divisor);
#else
    return 0LL;
#endif
}
EXPORT_SYMBOL(osal_div_s64);

/* the result of u64/u64. */
hi_u64 osal_div64_u64(hi_u64 dividend, hi_u64 divisor)
{
#ifndef AOS_LLVM_BUILD
    return div64_u64(dividend, divisor);
#else
    return 0ULL;
#endif
}
EXPORT_SYMBOL(osal_div64_u64);

/* the result of s64/s64. */
hi_s64 osal_div64_s64(hi_s64 dividend, hi_s64 divisor)
{
#ifndef AOS_LLVM_BUILD
    return div64_s64(dividend, divisor);
#else
    return 0LL;
#endif
}
EXPORT_SYMBOL(osal_div64_s64);

/* the remainder of u64/u32. */
hi_u64 osal_div_u64_rem(hi_u64 dividend, hi_u32 divisor)
{
    hi_u32 rem = 0;
#ifndef AOS_LLVM_BUILD
    (void)div_u64_rem(dividend, divisor, &rem);
#endif
    return rem;
}
EXPORT_SYMBOL(osal_div_u64_rem);

/* the remainder of s64/s32. */
hi_s64 osal_div_s64_rem(hi_s64 dividend, hi_s32 divisor)
{
    hi_s32 rem = 0;
#ifndef AOS_LLVM_BUILD
    (void)div_s64_rem(dividend, divisor, &rem);
#endif
    return rem;
}
EXPORT_SYMBOL(osal_div_s64_rem);

/* the remainder of u64/u64. */
hi_u64 osal_div64_u64_rem(hi_u64 dividend, hi_u64 divisor)
{
    hi_u64 rem = 0;
#ifndef AOS_LLVM_BUILD
    (void)div64_u64_rem(dividend, divisor, &rem);
#endif
    return rem;
}
EXPORT_SYMBOL(osal_div64_u64_rem);

hi_u32 osal_random(void)
{
#ifndef AOS_LLVM_BUILD
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
    return random32();
#else
    return get_random_u32();
#endif
#else
    return 0;
#endif
}
EXPORT_SYMBOL(osal_random);
