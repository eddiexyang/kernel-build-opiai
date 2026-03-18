/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2016-2020.
 * Description: osal vmalloc source file.
 * Author: Hisilicon multimedia software group
 * Create: 2016-11-11
 */

#include "hi_osal.h"
#include "los_memory.h"

static hi_void *g_dvpp_heap;

int osal_heap_init(hi_void *base, hi_u64 size)
{
    UINT32 ret = LOS_MemInit(base, size);
    if (ret != LOS_OK) {
        HI_TRACE_OSAL(HI_DBG_ERR, "los mem init fail, ret=0x%x!\n", ret);
        return HI_FAILURE;
    }
    g_dvpp_heap = base;
    return HI_SUCCESS;
}

hi_void osal_heap_exit(hi_void)
{
    if (g_dvpp_heap != NULL) {
        // not support deinit pool
        g_dvpp_heap = NULL;
    }
}

void *osal_vmalloc_(unsigned long size, const char *function)
{
    HI_UNUSED(function);
    return osal_kmalloc(size, 0);
}

void osal_vfree_(const void *addr, const char *function)
{
    HI_UNUSED(function);
    osal_kfree((void *)addr);
}

void *osal_kmalloc_(unsigned long size, unsigned int osal_gfp_flag, const char *function)
{
    HI_UNUSED(osal_gfp_flag);
    HI_UNUSED(function);
    return LOS_MemAlloc((void *)g_dvpp_heap, size);
}

void osal_kfree_(const void *addr, const char *function)
{
    HI_UNUSED(function);
    (void)LOS_MemFree((void *)g_dvpp_heap, (void *)addr);
}
