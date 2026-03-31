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

#include <linux/dma-direction.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/version.h>

#include "virtmngagent_msg_common.h"
#include "virtmng_extension.h"
#include "virtmng_interface.h"
#include "virtmng_public_def.h"


#ifndef DRV_UT
void *devdrv_dma_alloc_coherent(struct device *dev, size_t size, dma_addr_t *dma_addr, gfp_t gfp)
{
    return dma_alloc_coherent(dev, size, dma_addr, gfp);
}
EXPORT_SYMBOL(devdrv_dma_alloc_coherent);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
static inline void *dma_zalloc_coherent(struct device *dev, size_t size, dma_addr_t *dma_handle, gfp_t flag)
{

    void *ret = dma_alloc_coherent(dev, size, dma_handle, flag | __GFP_ZERO);
    return ret;
}
#endif

void *devdrv_dma_zalloc_coherent(struct device *dev, size_t size, dma_addr_t *dma_addr, gfp_t gfp)
{
    return dma_zalloc_coherent(dev, size, dma_addr, gfp);
}
EXPORT_SYMBOL(devdrv_dma_zalloc_coherent);

void devdrv_dma_free_coherent(struct device *dev, size_t size, void *addr, dma_addr_t dma_addr)
{
    dma_free_coherent(dev, size, addr, dma_addr);
}
EXPORT_SYMBOL(devdrv_dma_free_coherent);

dma_addr_t devdrv_dma_map_single(struct device *dev, void *ptr, size_t size, enum dma_data_direction dir)
{
    return dma_map_single(dev, ptr, size, dir);
}
EXPORT_SYMBOL(devdrv_dma_map_single);

void devdrv_dma_unmap_single(struct device *dev, dma_addr_t addr, size_t size, enum dma_data_direction dir)
{
    dma_unmap_single(dev, addr, size, dir);
}
EXPORT_SYMBOL(devdrv_dma_unmap_single);

dma_addr_t devdrv_dma_map_page(struct device *dev, struct page *page,
    size_t offset, size_t size, enum dma_data_direction dir)
{
    return dma_map_page(dev, page, offset, size, dir);
}
EXPORT_SYMBOL(devdrv_dma_map_page);

void devdrv_dma_unmap_page(struct device *dev, dma_addr_t addr, size_t size, enum dma_data_direction dir)
{
    dma_unmap_page(dev, addr, size, dir);
}
EXPORT_SYMBOL(devdrv_dma_unmap_page);
#endif