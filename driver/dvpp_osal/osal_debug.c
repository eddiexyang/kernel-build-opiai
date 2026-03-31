#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#endif

hi_s32 osal_printk(const hi_char *fmt, ...)
{
    va_list args;
    hi_s32 r;

    va_start(args, fmt);
    r = vprintk(fmt, args);
    va_end(args);

    return r;
}
EXPORT_SYMBOL(osal_printk);

void osal_panic(const hi_char *fmt, const hi_char *fun, hi_s32 line, const hi_char *cond)
{
    panic(fmt, fun, line, cond);
}
EXPORT_SYMBOL(osal_panic);
