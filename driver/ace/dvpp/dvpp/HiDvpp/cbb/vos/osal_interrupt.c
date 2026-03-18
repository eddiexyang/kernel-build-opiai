#include "hi_osal.h"

hi_s32 osal_request_irq(hi_u32 irq, osal_irq_handler_t handler, osal_irq_handler_t thread_fn, const hi_char *name,
                        void *dev)
{
    return 0;
}

void osal_free_irq(hi_u32 irq, void *dev)
{
}

hi_s32 osal_request_irq_ex(hi_u32 irq, osal_irq_handler_t handler, osal_irq_handler_t thread_fn, const hi_char *name,
                           void *dev)
{
    return 0;
}

hi_s32 osal_in_interrupt(void)
{
    return 0;
}

hi_u32 osal_irq_of_parse_and_map(void *dev, hi_s32 idx)
{
    return 0;
}

hi_s32 osal_irq_force_affinity(hi_u32 irq, const void *cpumask)
{
    return 0;
}

hi_s32 osal_irq_set_affinity_hint(hi_u32 irq, const void *cpumask)
{
    return 0;
}