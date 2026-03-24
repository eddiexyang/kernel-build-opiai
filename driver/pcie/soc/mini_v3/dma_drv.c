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

#include <linux/delay.h>
#include <asm/io.h>
#include "dma_drv.h"
#include "resource_drv.h"

void devdrv_set_va_enable(void __iomem *io_base)
{
    u32 regval;

    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_QUEUE_CTRL1, &regval);
    regval |= DMA_CTRL1_VA_CQ_SQ_ENABLE_MASK;
    devdrv_dma_reg_wr(io_base, DEVDRV_DMA_QUEUE_CTRL1, regval);
}

void devdrv_dma_err_interrupt_unmask(void __iomem *io_base)
{
    devdrv_dma_reg_wr(io_base, DEVDRV_DMA_ERR_MASK, 0x0);
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

void devdrv_set_dma_arb_weight(void __iomem *io_base, u32 val)
{
    return;
}

void devdrv_dma_chan_err_interrupt_mask(void __iomem *io_base, u32 val)
{
    return;
}
