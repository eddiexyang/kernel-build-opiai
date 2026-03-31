#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#endif // #ifdef AOS_LLVM_BUILD
void osal_yield(void)
{
    cond_resched();
}
EXPORT_SYMBOL(osal_yield);
