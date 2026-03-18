#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/interrupt.h>
#include <linux/of_irq.h>
#include <linux/version.h>
#include <linux/cpumask.h>

#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#endif // #ifdef AOS_LLVM_BUILD
hi_s32 osal_request_irq(hi_u32 irq, osal_irq_handler_t handler, osal_irq_handler_t thread_fn, const hi_char *name,
                        void *dev)
{
    hi_ulong flags = 0x20; // IRQF_DISABLED;

    return request_threaded_irq(irq, (irq_handler_t)handler, (irq_handler_t)thread_fn, flags, name, dev);
}
EXPORT_SYMBOL(osal_request_irq);
void osal_free_irq(hi_u32 irq, void *dev)
{
    (void)free_irq(irq, dev);
}
EXPORT_SYMBOL(osal_free_irq);

#ifndef AOS_LLVM_BUILD
hi_s32 osal_request_irq_ex(hi_u32 irq, osal_irq_handler_t handler, osal_irq_handler_t thread_fn, const hi_char *name,
                           void *dev)
{
    hi_ulong flags = 0x20; // IRQF_DISABLED;

    return request_threaded_irq(irq, (irq_handler_t)handler, (irq_handler_t)thread_fn,
                                flags | IRQF_NOBALANCING, name, dev);
}
EXPORT_SYMBOL(osal_request_irq_ex);
#endif

hi_s32 osal_in_interrupt(void)
{
#ifndef AOS_LLVM_BUILD
    return in_interrupt();
#else
    return 0;
#endif
}
EXPORT_SYMBOL(osal_in_interrupt);

hi_u32 osal_irq_of_parse_and_map(void *dev, hi_s32 idx)
{
    return irq_of_parse_and_map((struct device_node*)dev, idx);
}
EXPORT_SYMBOL(osal_irq_of_parse_and_map);

hi_s32 osal_irq_force_affinity(hi_u32 irq, const void *cpumask)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0) && !defined AOS_LLVM_BUILD
    if (cpumask == HI_NULL) {
        return irq_force_affinity(irq, get_cpu_mask(0));
    }
    return irq_force_affinity(irq, (const struct cpumask *)cpumask);
#else
    return irq_set_affinity_hint(irq, (const struct cpumask *)cpumask);
#endif
}
EXPORT_SYMBOL(osal_irq_force_affinity);

hi_s32 osal_irq_set_affinity_hint(hi_u32 irq, const void *cpumask)
{
    return irq_set_affinity_hint(irq, (const struct cpumask *)cpumask);
}
EXPORT_SYMBOL(osal_irq_set_affinity_hint);

const void *osal_get_cpu_mask(hi_u32 cpu)
{
    return get_cpu_mask(cpu);
}
EXPORT_SYMBOL(osal_get_cpu_mask);
