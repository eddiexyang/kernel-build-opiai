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
 * Create: 2023-11-11
 */

#ifndef __KERNEL_CGROUP_MEM_ADAPT_H
#define __KERNEL_CGROUP_MEM_ADAPT_H

#ifndef AOS_LLVM_BUILD
#include <linux/version.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/gfp.h>
#if !defined(EMU_ST) && !defined(TSDRV_UT)
#include <linux/page-flags.h>
#include <linux/cgroup.h>
#include <linux/memcontrol.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
#ifdef CONFIG_MEMCG
static inline void ka_set_page_memcg(struct page *page, u64 pfn)
{
    struct page *cur_page = pfn_to_page(pfn);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
    if (page_memcg_check(cur_page) != NULL) {
        return;
    }
    cur_page->memcg_data = page->memcg_data | MEMCG_DATA_KMEM;
#else
    if (cur_page->mem_cgroup != NULL) {
        return;
    }
    cur_page->mem_cgroup = page->mem_cgroup;
    __SetPageKmemcg(cur_page);
#endif
}
#endif
#endif

static inline void ka_split_page_memcg(struct page *page, unsigned int order)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
#ifdef CONFIG_MEMCG
    u64 pfn, i;
    /*
     * split_page do not change mem_cgroup of page[1-n]
     * if app run in docker can not statistics free page num
     * set page[0] mem_cgroup to page[1-n]
     */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
    if (page_memcg_check(page) == NULL) {
        return;
    }
#else
    if (page->mem_cgroup == NULL) {
        return;
    }
#endif
    for (i = 1, pfn = page_to_pfn(page) + 1; i < (1ULL << order); i++, pfn++) {
        ka_set_page_memcg(page, pfn);
    }
#endif
#endif
}

static inline void ka_make_alloc_exact_free(struct page *page, unsigned int order, size_t size, unsigned long addr)
{
    unsigned long alloc_end = addr + (PAGE_SIZE << order);
    unsigned long used = addr + PAGE_ALIGN(size);

    if (order != 0) {
        split_page(page, order);
        ka_split_page_memcg(page, order);
    }

    while (used < alloc_end) {
        free_page(used);
        used += PAGE_SIZE;
    }
}

static inline void *ka_make_alloc_exact(struct page *page, unsigned int order, size_t size)
{
    unsigned long addr;

    addr = (unsigned long)page_address(page);
    if (addr != 0) {
        ka_make_alloc_exact_free(page, order, size, addr);
    }

    return (void *)addr;
}

static inline void *ka_alloc_pages_exact(size_t size, gfp_t gfp_mask)
{
    struct page *page;
    unsigned int order = (u32)get_order(size);

    page = alloc_pages(gfp_mask, order);
    if (page == NULL) {
        return NULL;
    }

    return ka_make_alloc_exact(page, order, size);
}

static inline struct page *ka_alloc_pages_node(int nid, gfp_t gfp_mask, u32 order)
{
    struct page *page;

    page = alloc_pages_node(nid, gfp_mask, order);
    if (page == NULL) {
        return NULL;
    }

    if (order != 0) {
        split_page(page, order);
        ka_split_page_memcg(page, order);
    }

    return page;
}
#else
static inline void *ka_alloc_pages_exact(size_t size, gfp_t gfp_mask)
{
    return alloc_pages_exact(size, gfp_mask);
}

static inline struct page *ka_alloc_pages_node(int nid, gfp_t gfp_mask, u32 order)
{
    return alloc_pages_node(nid, gfp_mask, order);
}
#endif
#endif
#endif  /* __KERNEL_CGROUP_MEM_ADAPT_H */

