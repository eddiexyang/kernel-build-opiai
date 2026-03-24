/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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
#include <linux/types.h>
#include <linux/wait.h>

#include "devdrv_interface.h"
#include "devdrv_dma_interface.h"
#include "devdrv_util.h"
#include "devdrv_atu.h"
#include "devdrv_dma.h"

int devdrv_dma_sync_copy_plus(u32 dev_id, enum devdrv_dma_data_type type, int instance, u64 src, u64 dst, u32 size,
                              enum devdrv_dma_direction direction)
{
    int ret;
    struct devdrv_dma_node dma_node = {0};
    struct devdrv_dma_dev *dma_dev = NULL;

    dma_node.src_addr = src;
    dma_node.dst_addr = dst;
    dma_node.size = size;
    dma_node.direction = direction;

    dma_dev = devdrv_get_dma_dev(dev_id);
    if (dma_dev == NULL) {
        devdrv_err("Call devdrv_get_dma_dev failed, dma_dev is NULL. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    ret = devdrv_dma_para_check(dev_id, type, DEVDRV_DMA_SYNC, NULL);
    if (ret) {
        devdrv_err("Dma parameter check failed. (dev_id=%u)\n", dev_id);
        return ret;
    }

    ret = devdrv_dma_node_check(dev_id, &dma_node, 1, dma_dev);
    if (ret) {
        devdrv_err("Dma node check failed. (dev_id=%u)\n", dev_id);
        return ret;
    }

    ret = devdrv_dma_copy(dma_dev, type, instance, &dma_node, 1, DEVDRV_DMA_WAIT_INTR, DEVDRV_DMA_SYNC, NULL);

    return ret;
}
EXPORT_SYMBOL(devdrv_dma_sync_copy_plus);

int devdrv_dma_sync_copy(u32 dev_id, enum devdrv_dma_data_type type, u64 src, u64 dst, u32 size,
                         enum devdrv_dma_direction direction)
{
    return devdrv_dma_sync_copy_plus(dev_id, type, DEVDRV_INVALID_INSTANCE, src, dst, size, direction);
}
EXPORT_SYMBOL(devdrv_dma_sync_copy);

int devdrv_dma_async_copy_plus(u32 dev_id, enum devdrv_dma_data_type type, int instance, u64 src, u64 dst, u32 size,
                               enum devdrv_dma_direction direction, struct devdrv_asyn_dma_para_info *para_info)
{
    int ret;
    struct devdrv_dma_node dma_node = {0};
    struct devdrv_dma_dev *dma_dev = NULL;

    dma_node.src_addr = src;
    dma_node.dst_addr = dst;
    dma_node.size = size;
    dma_node.direction = direction;

    dma_dev = devdrv_get_dma_dev(dev_id);
    if (dma_dev == NULL) {
        devdrv_err_spinlock("Call devdrv_get_dma_dev failed, dma_dev is NULL. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    ret = devdrv_dma_para_check(dev_id, type, DEVDRV_DMA_ASYNC, para_info);
    if (ret) {
        devdrv_err_spinlock("Dma parameter check failed. (dev_id=%u)\n", dev_id);
        return ret;
    }

    ret = devdrv_dma_node_check(dev_id, &dma_node, 1, dma_dev);
    if (ret) {
        devdrv_err_spinlock("Dma node check failed. (dev_id=%u)\n", dev_id);
        return ret;
    }

    ret = devdrv_dma_copy(dma_dev, type, instance, &dma_node, 1, DEVDRV_DMA_WAIT_INTR, DEVDRV_DMA_ASYNC, para_info);

    return ret;
}
EXPORT_SYMBOL(devdrv_dma_async_copy_plus);

int devdrv_dma_async_copy(u32 dev_id, enum devdrv_dma_data_type type, u64 src, u64 dst, u32 size,
                          enum devdrv_dma_direction direction, struct devdrv_asyn_dma_para_info *para_info)
{
    return devdrv_dma_async_copy_plus(dev_id, type, DEVDRV_INVALID_INSTANCE, src, dst, size, direction, para_info);
}
EXPORT_SYMBOL(devdrv_dma_async_copy);

int devdrv_dma_sync_link_copy_plus(u32 dev_id, enum devdrv_dma_data_type type, int wait_type, int instance,
                                   struct devdrv_dma_node *dma_node, u32 node_cnt)
{
    int ret;
    struct devdrv_dma_dev *dma_dev = NULL;

    dma_dev = devdrv_get_dma_dev(dev_id);
    if (dma_dev == NULL) {
        devdrv_err("Call devdrv_get_dma_dev failed, dma_dev is NULL. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    ret = devdrv_dma_para_check(dev_id, type, DEVDRV_DMA_SYNC, NULL);
    if (ret) {
        devdrv_err("Dma parameter check failed. (dev_id=%u)\n", dev_id);
        return ret;
    }

    ret = devdrv_dma_node_check(dev_id, dma_node, node_cnt, dma_dev);
    if (ret) {
        devdrv_err("Dma node check failed. (dev_id=%u)\n", dev_id);
        return ret;
    }

    ret = devdrv_dma_copy(dma_dev, type, instance, dma_node, node_cnt, wait_type, DEVDRV_DMA_SYNC, NULL);

    return ret;
}
EXPORT_SYMBOL(devdrv_dma_sync_link_copy_plus);

int devdrv_dma_sync_link_copy(u32 dev_id, enum devdrv_dma_data_type type, int wait_type,
                              struct devdrv_dma_node *dma_node, u32 node_cnt)
{
    return devdrv_dma_sync_link_copy_plus(dev_id, type, wait_type, DEVDRV_INVALID_INSTANCE, dma_node, node_cnt);
}
EXPORT_SYMBOL(devdrv_dma_sync_link_copy);

int devdrv_dma_async_link_copy_plus(u32 dev_id, enum devdrv_dma_data_type type, int instance,
                                    struct devdrv_dma_node *dma_node, u32 node_cnt,
                                    struct devdrv_asyn_dma_para_info *para_info)
{
    int ret;
    struct devdrv_dma_dev *dma_dev = NULL;

    dma_dev = devdrv_get_dma_dev(dev_id);
    if (dma_dev == NULL) {
        devdrv_err("Call devdrv_get_dma_dev failed, dma_dev is NULL. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    ret = devdrv_dma_para_check(dev_id, type, DEVDRV_DMA_ASYNC, para_info);
    if (ret) {
        devdrv_err("Dma parameter check failed. (dev_id=%u)\n", dev_id);
        return ret;
    }

    ret = devdrv_dma_node_check(dev_id, dma_node, node_cnt, dma_dev);
    if (ret) {
        devdrv_err("Dma node check failed. (dev_id=%u)\n", dev_id);
        return ret;
    }

    ret = devdrv_dma_copy(dma_dev, type, instance, dma_node, node_cnt,
        DEVDRV_DMA_WAIT_INTR, DEVDRV_DMA_ASYNC, para_info);

    return ret;
}
EXPORT_SYMBOL(devdrv_dma_async_link_copy_plus);

int devdrv_dma_async_link_copy(u32 dev_id, enum devdrv_dma_data_type type, struct devdrv_dma_node *dma_node,
                               u32 node_cnt, struct devdrv_asyn_dma_para_info *para_info)
{
    return devdrv_dma_async_link_copy_plus(dev_id, type, DEVDRV_INVALID_INSTANCE, dma_node, node_cnt, para_info);
}
EXPORT_SYMBOL(devdrv_dma_async_link_copy);