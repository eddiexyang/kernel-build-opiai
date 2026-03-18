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
 * Create: 2023-06-05
 */

#include <linux/slab.h>
#include <asm/io.h>
#include <linux/types.h>

#include "fpdc_define.h"
#include "drv_ras_common.h"

static DEFINE_MUTEX(g_ras_int_umask_mutex);
int ras_int_unmask_reg_write(u64 reg_addr, u32 bit_mask, u32 val)
{
    void __iomem *vaddr = NULL;
    u32 read_val, mask_val, write_val;

    mutex_lock(&g_ras_int_umask_mutex);
    vaddr = ioremap(reg_addr, sizeof(u32));
    if (vaddr == NULL) {
        mutex_unlock(&g_ras_int_umask_mutex);
        fpdc_err("Remap register failed.\n");
        return -ENOMEM;
    }

    read_val = readl_relaxed(vaddr);
    read_val &= ~(bit_mask);
    mask_val = val&bit_mask;
    write_val = read_val | mask_val;
    writel_relaxed(write_val, vaddr);
    iounmap(vaddr);
    vaddr = NULL;
    mutex_unlock(&g_ras_int_umask_mutex);
    return 0;
}

EXPORT_SYMBOL(ras_int_unmask_reg_write);