/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2016-2020.
 * Description: osal interrupt source file.
 * Author: Hisilicon multimedia software group
 * Create: 2016-11-11
 */

#include "hi_osal.h"
#include "los_base.h"
#include "los_hwi.h"
#include "arch/mmu.h"
#include "los_task.h"

int osal_request_irq(unsigned int irq, osal_irq_handler_t handler,
                     osal_irq_handler_t thread_fn, const char *name, void *dev)
{
    HI_UNUSED(thread_fn);

    UINT32 ret;

    HWI_IRQ_PARAM_S irqParam;
    irqParam.swIrq = (INT32)irq;
    irqParam.pDevId = dev;
    irqParam.pName = name;

    ret = LOS_HwiCreate(irq, 0, 0, (HWI_PROC_FUNC)handler, &irqParam);
    if (ret == LOS_OK) {
        HalIrqSetAffinity(irq, CPUID_TO_AFFI_MASK(ArchCurrCpuid()));
        HalIrqUnmask(irq);
        return 0;
    }

    HI_TRACE_OSAL(HI_DBG_ERR, "los create hwi, ret = 0x%x!\n", ret);
    return -1;
}

void osal_free_irq(unsigned int irq, void *dev)
{
    HWI_IRQ_PARAM_S irqParam;
    irqParam.swIrq = (INT32)irq;
    irqParam.pDevId = dev;

    (hi_void)LOS_HwiDelete(irq, &irqParam);
    return;
}

int osal_in_interrupt(void)
{
    return OS_INT_ACTIVE;
}
