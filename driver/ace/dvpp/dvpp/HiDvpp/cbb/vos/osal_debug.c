/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2016-2020.
 * Description: osal debug source file.
 * Author: Hisilicon multimedia software group
 * Create: 2016-11-11
 */
#include "hi_osal.h"

int osal_printk(const char *fmt, ...)
{
    HI_UNUSED(fmt);
    return 0;
}

#ifdef HI_DEBUG
void osal_panic(const char *fmt, const char *fun, int line, const char *cond)
{
    // vos 不支持panic
    osal_printk(fmt, fun, line, cond);
}
#endif
