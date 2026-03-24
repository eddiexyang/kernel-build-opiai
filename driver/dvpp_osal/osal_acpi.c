/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * Description:
 * Author: huawei
 * Create: 2023-4-1
 */
#include "hi_osal.h"

#include <linux/acpi.h>
#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#endif
hi_s32 osal_acpi_dma_configure(void *dev, hi_s32 attr)
{
#ifndef AOS_LLVM_BUILD
    return acpi_dma_configure((struct device*)dev, (enum dev_dma_attr)attr);
#else
    return 0;
#endif
}
EXPORT_SYMBOL(osal_acpi_dma_configure);
