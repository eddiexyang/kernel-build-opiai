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
* Create: 2023-1-10
*/

#include "kernel_cgroup_mem_adapt.h"

#include "devdrv_interface.h"
#include "soc_adapt.h"
#include "trs_chan_mem.h"
#include "trs_pm_adapt.h"
#include "trs_rsv_mem.h"
#include "trs_chan_near_ops_mem.h"

static void *trs_chan_mem_alloc_dma(struct trs_id_inst *inst, struct device *dev,
    size_t size, dma_addr_t *dma_addr, enum dma_data_direction dir)
{
    void *vaddr = NULL;

    if (dev != NULL) {
        vaddr = ka_alloc_pages_exact(size, GFP_KERNEL | __GFP_ZERO  | __GFP_ACCOUNT | __GFP_RETRY_MAYFAIL);
        if (vaddr == NULL) {
            return NULL;
        }

        *dma_addr = devdrv_dma_map_single(dev, vaddr, size, DMA_BIDIRECTIONAL);
        if (dma_mapping_error(dev, *dma_addr)) {
            free_pages_exact(vaddr, size);
            return NULL;
        }
    }

    return vaddr;
}

static void *trs_chan_mem_alloc_ddr_dma(struct trs_id_inst *inst, size_t size, phys_addr_t *paddr)
{
    return trs_chan_mem_alloc_dma(inst, devdrv_get_pci_dev_by_devid(inst->devid),
        size, (dma_addr_t *)paddr, DMA_BIDIRECTIONAL);
}

static void *trs_chan_mem_alloc_ddr_phy(size_t size, phys_addr_t *paddr)
{
    void *vaddr = NULL;

    vaddr = ka_alloc_pages_exact(size, GFP_KERNEL | __GFP_ZERO  | __GFP_ACCOUNT | __GFP_RETRY_MAYFAIL);
    if (vaddr == NULL) {
        return NULL;
    }
    *paddr = virt_to_phys(vaddr);

    return vaddr;
}

static inline void trs_chan_mem_free_ddr_phy(void *vaddr, size_t size)
{
    free_pages_exact(vaddr, size);
}

void *trs_chan_ops_sq_mem_alloc(struct trs_id_inst *inst, struct trs_chan_type *types,
    struct trs_chan_sq_para *sq_para, u64 *phy_addr, u32 *attr)
{
    size_t size = sq_para->sqe_size * sq_para->sq_depth;
    void *vaddr = NULL;
    u32 sq_mem_side;

    *attr = 0;
    sq_mem_side = trs_soc_get_sq_mem_side(inst);
    if (sq_mem_side == TRS_CHAN_DEV_MEM) {
        vaddr = trs_chan_mem_alloc_rsv(inst, RSV_MEM_HW_SQCQ, size, (phys_addr_t *)phy_addr, 0);
    } else if (sq_mem_side == TRS_CHAN_HOST_MEM) {
        vaddr = trs_chan_mem_alloc_ddr_dma(inst, size, phy_addr);
        *attr |= (0x1 << CHAN_MEM_FLAG_LOCAL);
    } else if (sq_mem_side == TRS_CHAN_HOST_PHY_MEM) {
        vaddr = trs_chan_mem_alloc_ddr_phy(size, phy_addr);
        *attr |= (0x1 << CHAN_MEM_FLAG_LOCAL);
    }

    return vaddr;
}

static void trs_chan_mem_free_dma(struct trs_id_inst *inst, struct device *dev, void *vaddr,
    dma_addr_t dma_addr, size_t size)
{
    if (dev != NULL) {
        devdrv_dma_unmap_single(dev, dma_addr, size, DMA_BIDIRECTIONAL);
    }

    free_pages_exact(vaddr, size);
}

static void trs_chan_mem_free_ddr_dma(struct trs_id_inst *inst, void *vaddr, size_t size, phys_addr_t paddr)
{
    trs_chan_mem_free_dma(inst, devdrv_get_pci_dev_by_devid(inst->devid), vaddr, (dma_addr_t)paddr, size);
}

void trs_chan_ops_sq_mem_free(struct trs_id_inst *inst, struct trs_chan_type *types,
    struct trs_chan_sq_para *sq_para, void *sq_addr, u64 phy_addr)
{
    size_t size = sq_para->sqe_size * sq_para->sq_depth;
    u32 sq_mem_side;

    sq_mem_side = trs_soc_get_sq_mem_side(inst);
    if (sq_mem_side == TRS_CHAN_DEV_MEM) {
        trs_chan_mem_free_rsv(inst, RSV_MEM_HW_SQCQ, sq_addr, size);
    } else if (sq_mem_side == TRS_CHAN_HOST_MEM) {
        trs_chan_mem_free_ddr_dma(inst, sq_addr, size, phy_addr);
    } else if (sq_mem_side == TRS_CHAN_HOST_PHY_MEM) {
        trs_chan_mem_free_ddr_phy(sq_addr, size);
    }
}

void *trs_chan_sq_mem_alloc(struct trs_id_inst *inst, u32 sqe_size, u32 sq_depth, u64 *phy_addr, u32 *attr)
{
    struct trs_chan_sq_para sq_para;
    struct trs_chan_type types;

    sq_para.sqe_size = sqe_size;
    sq_para.sq_depth = sq_depth;

    return trs_chan_ops_sq_mem_alloc(inst, &types, &sq_para, phy_addr, attr);
}
EXPORT_SYMBOL(trs_chan_sq_mem_alloc);

void trs_chan_sq_mem_free(struct trs_id_inst *inst, u32 sqe_size, u32 sq_depth, void *sq_addr, u64 phy_addr)
{
    struct trs_chan_sq_para sq_para;
    struct trs_chan_type types;

    sq_para.sqe_size = sqe_size;
    sq_para.sq_depth = sq_depth;

    return trs_chan_ops_sq_mem_free(inst, &types, &sq_para, sq_addr, phy_addr);
}
EXPORT_SYMBOL(trs_chan_sq_mem_free);

void *trs_chan_ops_cq_mem_alloc(struct trs_id_inst *inst, struct trs_chan_type *types,
    struct trs_chan_cq_para *cq_para, u64 *phy_addr, u32 *attr)
{
    size_t size = cq_para->cqe_size * cq_para->cq_depth;
    void *vaddr = NULL;
    u32 cq_mem_side;

    *attr = 0;
    cq_mem_side = trs_soc_get_cq_mem_side(inst);
    if (cq_mem_side == TRS_CHAN_DEV_MEM) {
        /* To reduce time consumption, drv not clear rsv type cq mem. Tsfw clear when sqcq creat mbox operation */
        vaddr = trs_chan_mem_alloc_rsv(inst, RSV_MEM_HW_SQCQ, size, (phys_addr_t *)phy_addr, 0);
    } else if (cq_mem_side == TRS_CHAN_HOST_MEM) {
        vaddr = trs_chan_mem_alloc_ddr_dma(inst, size, phy_addr);
        *attr |= (0x1 << CHAN_MEM_FLAG_LOCAL);
    } else if (cq_mem_side == TRS_CHAN_HOST_PHY_MEM) {
        vaddr = trs_chan_mem_alloc_ddr_phy(size, phy_addr);
        *attr |= (0x1 << CHAN_MEM_FLAG_LOCAL);
    }
    return vaddr;
}

void trs_chan_ops_cq_mem_free(struct trs_id_inst *inst, struct trs_chan_type *types,
    struct trs_chan_cq_para *cq_para, void *cq_addr, u64 phy_addr)
{
    size_t size = cq_para->cqe_size * cq_para->cq_depth;
    u32 cq_mem_side;

    cq_mem_side = trs_soc_get_cq_mem_side(inst);
    if (cq_mem_side == TRS_CHAN_DEV_MEM) {
        trs_chan_mem_free_rsv(inst, RSV_MEM_HW_SQCQ, cq_addr, size);
    } else if (cq_mem_side == TRS_CHAN_HOST_MEM) {
        trs_chan_mem_free_ddr_dma(inst, cq_addr, size, phy_addr);
    } else if (cq_mem_side == TRS_CHAN_HOST_PHY_MEM) {
        trs_chan_mem_free_ddr_phy(cq_addr, size);
    }
}

void trs_chan_ops_flush_sqe_cache(struct trs_id_inst *inst,
    struct trs_chan_type *types, void *addr, u64 pa, u32 len)
{
    if (trs_soc_get_sq_mem_side(inst) != TRS_CHAN_DEV_MEM) {
        dma_addr_t dma_addr = (dma_addr_t)pa;
        dma_sync_single_for_device(devdrv_get_pci_dev_by_devid(inst->devid), dma_addr, len, DMA_TO_DEVICE);
    }
}

void trs_chan_flush_sqe_cache(struct trs_id_inst *inst, u64 pa, u32 len)
{
    struct trs_chan_type types;

    trs_chan_ops_flush_sqe_cache(inst, &types, NULL, pa, len);
}
EXPORT_SYMBOL(trs_chan_flush_sqe_cache);

void trs_chan_ops_invalid_cqe_cache(struct trs_id_inst *inst,
    struct trs_chan_type *types, void *addr, u64 pa, u32 len)
{
    if (trs_soc_get_cq_mem_side(inst) != TRS_CHAN_DEV_MEM) {
        dma_addr_t dma_addr = (dma_addr_t)pa;
        dma_sync_single_for_cpu(devdrv_get_pci_dev_by_devid(inst->devid), dma_addr, len, DMA_FROM_DEVICE);
    }
}

