/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2016-2020.
 * Description: osal barrier source file.
 * Author: Hisilicon multimedia software group
 * Create: 2016-11-11
 */

#include "hi_osal.h"
#include "los_hw.h"

void osal_isb(void)
{
    isb();
}

void osal_dsb(void)
{
    dsb();
}

void osal_dmb(void)
{
    dmb();
}
