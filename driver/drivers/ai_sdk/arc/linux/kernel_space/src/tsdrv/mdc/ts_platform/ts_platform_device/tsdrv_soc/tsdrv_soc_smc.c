/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
 * Create: 2019-10-15
 */
#include "tsdrv_soc_smc.h"

#ifndef TSDRV_KERNEL_UT
#ifndef AOS_LLVM_BUILD
u64 atf_write_reg_el3(u64 addr, u32 value)
{
    u64 res;
    struct tf_smc_param param = { 0 };

    param.a0 = DEVDRV_REG_WRITE;
    param.a1 = addr;
    param.a2 = value;
    res = smc_jump_to_atf(&param);
    return res;
}

u64 atf_read_reg_el3(u64 addr, u32 *value)
{
    u64 res = 0;
    struct tf_smc_param param = { 0 };

    if (value != NULL) {
        param.a0 = DEVDRV_REG_READ;
        param.a1 = addr;
        param.a2 = (u64)((uintptr_t)value);
        res = smc_jump_to_atf(&param);
        *value = param.a1;
    }

    return res;
}
#endif
#else
u64 atf_write_reg_el3(u64 addr, u32 value)
{
    return 0;
}
#endif
