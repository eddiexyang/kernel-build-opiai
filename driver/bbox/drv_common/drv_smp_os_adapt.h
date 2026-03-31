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
* Create: 2023-2-28
*/

#ifndef DRV_SMP_OS_ADAPT_H
#define DRV_SMP_OS_ADAPT_H

#include <linux/hashtable.h>

#ifndef AOS_LLVM_BUILD
#include <linux/smp.h>
#endif

static inline u32 drv_get_cur_processor_id(void)
{
#ifndef AOS_LLVM_BUILD
    return raw_smp_processor_id();
#else
    return (u32)smp_processor_id();
#endif
}

#endif