/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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

#include <linux/slab.h>
#include <linux/types.h>

#include "tsdrv_delay_recycle.h"
#include "tsdrv_kernel_common.h"
#include "tsdrv_log.h"

#include <linux/list.h>

#ifndef TSDRV_KERNEL_UT

#include "tsdrv_osal_mm.h"

struct mem_delay_free_info {
    void *virt_addr;
    u32 size;
    struct list_head list;
};

struct tsdrv_mem_delay_free {
    struct list_head mem_list;
    spinlock_t spinlock;
};

static struct tsdrv_mem_delay_free delay_free[TSDRV_MAX_DAVINCI_NUM];

void tsdrv_add_mem_delay_free(u32 devid, void *vaddr, u32 size)
{
#ifndef TSDRV_UT
    struct mem_delay_free_info *mem_info = NULL;

    mem_info = kzalloc(sizeof(struct mem_delay_free_info), GFP_KERNEL);
    if (mem_info == NULL) {
        return;
    }

    mem_info->virt_addr = vaddr;
    mem_info->size = size;
    TSDRV_PRINT_DEBUG("devid=%u, vaddr=%pK\n", devid, (void *)(uintptr_t)vaddr);
    spin_lock(&delay_free[devid].spinlock);
    list_add(&mem_info->list, &delay_free[devid].mem_list);
    spin_unlock(&delay_free[devid].spinlock);
#endif
}

void tsdrv_delay_free_mem(u32 devid)
{
    struct mem_delay_free_info *mem_info = NULL;
    struct list_head *pos = NULL, *n = NULL;

    spin_lock(&delay_free[devid].spinlock);
    if (list_empty_careful(&delay_free[devid].mem_list) != 0) {
        spin_unlock(&delay_free[devid].spinlock);
        return;
    }
#ifndef TSDRV_UT
    list_for_each_safe(pos, n, &delay_free[devid].mem_list) {
        mem_info = list_entry(pos, struct mem_delay_free_info, list);
        list_del(&mem_info->list);

        if (mem_info->virt_addr != NULL) {
            TSDRV_PRINT_DEBUG("devid=%u, vaddr=%pK\n", devid, (void *)(uintptr_t)mem_info->virt_addr);
            if (mem_info->size != 0) {
                tsdrv_free_pages_exact(mem_info->virt_addr, mem_info->size);
            } else {
                kfree(mem_info->virt_addr);
            }
            mem_info->virt_addr = NULL;
        }
        kfree(mem_info);
    }
    spin_unlock(&delay_free[devid].spinlock);
#endif
}

void tsdrv_mem_delay_free_init(u32 devid)
{
    INIT_LIST_HEAD(&delay_free[devid].mem_list);
    spin_lock_init(&delay_free[devid].spinlock);
}

#else
void tsdrv_mem_delay_free_init(u32 devid)
{
}
#endif
