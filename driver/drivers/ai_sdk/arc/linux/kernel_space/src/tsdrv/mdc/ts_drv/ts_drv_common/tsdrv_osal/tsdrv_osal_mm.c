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

#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/gfp.h>

#include "tsdrv_osal_mm.h"

#ifndef AOS_LLVM_BUILD
static void tsdrv_make_alloc_exact(struct page *p, u32 order, size_t size)
{
    struct page *cur_page = NULL;
    u64 alloced_num, page_num, i;

    if (order == 0) {
        return;
    }
    split_page(p, order);
    alloced_num = 1ull << order;
    page_num = PAGE_ALIGN(size) / PAGE_SIZE;

    for (i = page_num; i < alloced_num; i++) {
        cur_page = nth_page(p, i);
        __free_page(cur_page);
    }
}
/**
 * alloc_pages_exact_nid not exprot, realize function as alloc_pages_exact_nid
 * - allocate an exact number of physically-contiguous pages on a node.
 * @nid: the preferred node ID where memory should be allocated
 * @size: the number of bytes to allocate
 * @gfp_mask: GFP flags for the allocation, must not contain __GFP_COMP and __GFP_ACCOUNT
 * __GFP_ACCOUNT:cgroup can not statistics page free num, euler integrating this patch
 */
void *tsdrv_alloc_pages_exact_nid(int nid, size_t size, gfp_t gfp_mask)
{
    u32 order = get_order(size);
    struct page *p;

    if (gfp_mask & __GFP_COMP) {
        gfp_mask &= ~__GFP_COMP;
    }

    p = alloc_pages_node(nid, gfp_mask | __GFP_ZERO, order);
    if (p == NULL) {
        return NULL;
    }

    tsdrv_make_alloc_exact(p, order, size);

    return (void *)page_address(p);
}
#endif
void *tsdrv_alloc_pages_exact(size_t size, gfp_t gfp_mask)
{
#ifndef AOS_LLVM_BUILD
    return alloc_pages_exact(size, gfp_mask | __GFP_ZERO);
#else
    return kmalloc(size, GFP_USER | __GFP_ACCOUNT);
#endif
}

void tsdrv_free_pages_exact(void *virt, size_t size)
{
#ifndef AOS_LLVM_BUILD
    free_pages_exact(virt, size);
#else
    kfree(virt);
#endif
}

