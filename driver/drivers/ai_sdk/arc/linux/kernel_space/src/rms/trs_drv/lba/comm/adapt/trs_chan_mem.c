/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
* Create: 2022-7-15
*/
#include <linux/opiai_vendor_compat.h>

#include "trs_chan.h"
#include "trs_rsv_mem.h"
#include "trs_chan_mem.h"

static void trs_chan_mem_make_exact(struct page *p, u32 order, size_t size)
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

void *trs_chan_mem_alloc_ddr(struct trs_id_inst *inst, int nid, size_t size, phys_addr_t *paddr)
{
    u32 order = get_order(size);
    struct page *p = NULL;

    if (nid == NUMA_NO_NODE) {
        p = alloc_pages(__GFP_ZERO | __GFP_ACCOUNT | GFP_HIGHUSER_MOVABLE | __GFP_NOWARN, order);
    } else {
        p = alloc_pages_node(nid, __GFP_THISNODE | __GFP_ZERO | __GFP_ACCOUNT | GFP_HIGHUSER_MOVABLE | __GFP_NOWARN,
            order);
    }
    if (p == NULL) {
        return NULL;
    }
    trs_chan_mem_make_exact(p, order, size);
    *paddr = page_to_phys(p);

    return (void *)page_address(p);
}

void trs_chan_mem_free_ddr(struct trs_id_inst *inst, void *vaddr, size_t size)
{
    if (vaddr != NULL) {
        free_pages_exact(vaddr, size);
    }
}

void *trs_chan_mem_alloc_rsv(struct trs_id_inst *inst, int type, size_t size, phys_addr_t *paddr, u32 flag)
{
    void *vaddr = NULL;
    int ret;

    vaddr = trs_rsv_mem_alloc(inst, type, size, flag);
    if (vaddr == NULL) {
        trs_err("Alloc rsv mem fail. (devid=%u; tsid=%u; type=%d; size=0x%lu)\n", inst->devid, inst->tsid, type, size);
        return NULL;
    }
    ret = trs_rsv_mem_v2p(inst, type, vaddr, paddr);
    if (ret != 0) {
        trs_rsv_mem_free(inst, type, vaddr, size);
        trs_err("Rcv mem v2p fail. (devid=%u; tsid=%u; type=%d; size=0x%lu; ret=%d)\n",
                inst->devid, inst->tsid, type, size, ret);
        return NULL;
    }

    return vaddr;
}

void trs_chan_mem_free_rsv(struct trs_id_inst *inst, int type, void *sq_addr, size_t size)
{
    trs_rsv_mem_free(inst, type, sq_addr, size);
}
