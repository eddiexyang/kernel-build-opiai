/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Create: 2023-1-19
 */
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/pci.h>
#include <linux/types.h>
#include <linux/module.h>
#include "hns3_mirror_mem.h"

#ifdef MIRROR_MEMORY_SUPPORT

static u64 g_page_cnt = 0;

static void add_page_cnt(int order) {
    if (order >= 0) {
        g_page_cnt += (1 << order);
    }
}

static void del_page_cnt(int order) {
    if ((order >= 0) && (g_page_cnt >= (1 << order))) {
        g_page_cnt -= (1 << order);
    }
}

void *hns3_numa_dma_alloc_coherent(struct device *dev, size_t size, dma_addr_t *dma_handle, gfp_t flags)
{
    int nids[MAX_NODE_NUM] = {0};
    struct page *page = NULL;
    u32 i, cpu_id, dev_id;
    dma_addr_t dma_addr;
    int node_num;
    int order;

    if (dma_handle == NULL) {
        pr_err("hns3: dma_handle is NULL\n");
        return NULL;
    }

    if (!size) {
        pr_err("hns3: hns3_numa_dma_alloc_coherent size is 0\n");
        return NULL;
    }

    order = get_order(size);
    if (order >= MAX_ORDER) {
        pr_err("hns3: hns3_numa_dma_alloc_coherent fail, size %zu exceed to max order!\n", size);
        return NULL;
    }

    dbl_get_ctrl_nid_fun dbl_get_ctrl_nid =
        (dbl_get_ctrl_nid_fun)(uintptr_t)symbol_get(dbl_get_ctrl_nid);
    dbl_get_ai_nid_fun dbl_get_ai_nid =
        (dbl_get_ai_nid_fun)(uintptr_t)symbol_get(dbl_get_ai_nid);

    if (dbl_get_ctrl_nid == NULL || dbl_get_ai_nid == NULL) {
        pr_err("hns3: dbl_get_ctrl_nid [%pK] or dbl_get_ai_nid[%pK] is NULL!\n", dbl_get_ctrl_nid, dbl_get_ai_nid);
        return NULL;
    }

    cpu_id = smp_processor_id();
    dev_id = cpu_id / MAX_CPU_NUM;
    if (dev_id > 1) {
        dev_id = 0;
    }

    if (g_page_cnt < MAX_NETWORK_CTRL_MEM_PAGE_CNT) {
        node_num = dbl_get_ctrl_nid(dev_id, nids, MAX_NODE_NUM);
    } else {
        node_num = dbl_get_ai_nid(dev_id, nids, MAX_NODE_NUM);
    }

    if (dbl_get_ctrl_nid != NULL) {
        symbol_put(dbl_get_ctrl_nid);
    }

    if (dbl_get_ai_nid != NULL) {
        symbol_put(dbl_get_ai_nid);
    }

    if (node_num <= 0) {
        pr_err("hns3: dbl_get_nid node_num %d <= 0, invalid\n", node_num);
        return NULL;
    }

    for (i = 0; i < node_num; i++) {
        page = alloc_pages_node(nids[i], flags | __GFP_THISNODE | __GFP_ZERO | GFP_HIGHUSER_MOVABLE, order);
        if (!page) {
            continue;
        }

        dma_addr = dma_map_page(dev, page, 0, PAGE_SIZE << order, DMA_BIDIRECTIONAL);
        if (dma_mapping_error(dev, dma_addr)) {
            pr_err("hns3: hns3_numa_dma_alloc_coherent dma_mapping_error dma_addr is 0x%llx\n",
                   dma_addr);
            __free_pages(page, order);
            continue;
        }

        *dma_handle = dma_addr;
        add_page_cnt(order);
        return page_address(page);
    }

    pr_err("hns3: hns3_numa_dma_alloc_coherent find all the node, no memory\n");
    return NULL;
}
EXPORT_SYMBOL(hns3_numa_dma_alloc_coherent);

void hns3_numa_dma_free_coherent(struct device *dev, size_t size, void *mem, dma_addr_t dma_addr)
{
    if (!size) {
        pr_err("hns3: hns3_numa_dma_free_coherent size is 0\n");
        return;
    }
    dma_unmap_page(dev, dma_addr, size, DMA_BIDIRECTIONAL);
    free_pages((unsigned long)mem, get_order(size));
    del_page_cnt(get_order(size));
}
EXPORT_SYMBOL(hns3_numa_dma_free_coherent);
#endif