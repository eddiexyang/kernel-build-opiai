/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2023. All rights reserved.
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
 * Create: 2020-4-20
 */

#include "hi_osal.h"
#include "mmz_ext.h"

hi_s32 osal_mmz_init(void)
{
    mmz_init();
}

hi_s32 osal_mmz_exit(void)
{
    mmz_exit();
}

hi_u64 osal_mmz_malloc(const hi_char *mmz_name, const hi_char *buf_name, hi_ulong size)
{
    return mmz_malloc(mmz_name, buf_name, size);
}

hi_void osal_mmz_free(hi_u64 phy_addr, hi_void *vir_addr)
{
    mmz_free(phy_addr, vir_addr);
}

hi_s32 osal_mmz_is_valid(const hi_char *mmz_name, hi_u64 phy_addr)
{
    return mmz_is_valid(mmz_name, phy_addr);
}

void *osal_vmalloc_(hi_ulong size, const char *function)
{
    void *addr = HI_NULL;
    addr = (void*)osal_mmz_malloc("mmz_trusted", HI_NULL, size);
    return addr;
}

void osal_vfree_(const void *addr, const char *function)
{
    osal_mmz_free((hi_u64)addr, HI_NULL);
}

void *osal_kmalloc_(hi_ulong size, hi_u32 osal_gfp_flag, const char *function)
{
    return osal_vmalloc_(size, function);
}

void osal_kfree_(const void *addr, const char *function)
{
    osal_vfree_(addr, function);
}