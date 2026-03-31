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

#include <asm/io.h>
#include "agentdrv_interface.h"
#include "agentdrv_unit.h"
#include "devdrv_util.h"

void devdrv_set_va_enable(void __iomem *io_base)
{
    return;
}

void devdrv_dma_config_axim_aruser_mode(void __iomem *io_base)
{
    return;
}

void devdrv_dma_check_sram_init_status(const void __iomem *io_base, unsigned long timeout)
{
    return;
}

void devdrv_dma_err_interrupt_unmask(void __iomem *io_base)
{
    return;
}

void devdrv_set_dma_arb_weight(void __iomem *io_base, u32 val)
{
    u32 regval;

    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_CTRL0, &regval);
    regval &= ~DMA_CTRL0_ARB_WAIGHT_MASK;
    regval |= (val << DMA_CTRL0_ARB_WAIGHT_OFFSET) & DMA_CTRL0_ARB_WAIGHT_MASK;

    devdrv_dma_reg_wr(io_base, DEVDRV_DMA_QUEUE_CTRL0, regval);
}

int devdrv_dma_map_for_ts(u32 dev_id, bool is_sq,
    u64 *phy_addr, u32 *len, u64 *dma_addr)
{
    static u64 phy_base = MAX_SQ_CQ_BUF_PHY_BASE;
    struct devdrv_dma_dev *dma_dev = NULL;
    dma_addr_t dma_addr_d;
    u32 val = 0;
    int ret;

    if ((dma_addr == NULL) || (phy_addr == NULL) || (len == NULL)) {
        devdrv_err("Input parameter is error. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }

    if (is_sq) {
        *len = DEVDRV_DMA_SQ_DESC_SIZE * DEVDRV_MAX_TS_DMA_CH_SQ_DEPTH;
    } else {
        *len = DEVDRV_DMA_CQ_DESC_SIZE * DEVDRV_MAX_TS_DMA_CH_CQ_DEPTH;
    }
    if ((*len >= MAX_SQ_CQ_BUF_LEN) ||
        ((phy_base + *len) > ((u64)MAX_SQ_CQ_BUF_PHY_BASE + MAX_SQ_CQ_BUF_LEN))) {
        devdrv_err("Lenght out of range. (dev_id=%d; len=%u)\n", dev_id, *len);
        return -EINVAL;
    }

    *phy_addr = phy_base;
    dma_dev = devdrv_get_dma_dev(dev_id);
    if (dma_dev == NULL) {
        ret = agentdrv_get_rc_ep_mode(&val);
        if (ret) {
            devdrv_err("Got rc or ep mode failed. (ret=%d)\n", ret);
            return -EINVAL;
        }

        if (val == DEVDRV_PCIE_EP_MODE) {
            devdrv_err("Function devdrv_get_dma_dev failed, dma_dev is NULL. (dev_id=%d)\n", dev_id);
            return -EINVAL;
        } else {
            *dma_addr = (u64)phy_base;
            phy_base += *len;
            return 0;
        }
    }

    dma_addr_d = dma_map_resource(dma_dev->dev, *phy_addr, *len, DMA_BIDIRECTIONAL, 0);
    if (dma_mapping_error(dma_dev->dev, dma_addr_d)) {
        devdrv_warn("DMA map error. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }
    *dma_addr = (u64)dma_addr_d;
    phy_base += *len;
    return 0;
}
EXPORT_SYMBOL(devdrv_dma_map_for_ts);

void devdrv_dma_chan_err_interrupt_mask(void __iomem *io_base, u32 val)
{
    return;
}
