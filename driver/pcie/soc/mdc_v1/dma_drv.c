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
#include "resource_drv.h"
#include "dma_drv.h"

void devdrv_set_va_enable(void __iomem *io_base)
{
    return;
}

void devdrv_dma_config_axim_aruser_mode(void __iomem *io_base)
{
    u32 regval = 0;

    /* SDI_AXIM_AWUSER_MODE_CTRL.awuser_stash_mode (11 bit) set 0x1 */
    devdrv_dma_reg_rd(io_base, DEVDRV_SDI_AXIM_AWUSER_MODE_CTRL, &regval);
    regval = regval | (0x1 << 11);
    devdrv_dma_reg_wr(io_base, DEVDRV_SDI_AXIM_AWUSER_MODE_CTRL, regval);

    /* SDI_AXIM_AWUSER_SET_1.awuser_stash_set (1~10 bit) set 0x0 */
    devdrv_dma_reg_rd(io_base, DEVDRV_SDI_AXIM_AWUSER_SET_1, &regval);
    regval = regval & (~0x7FF);
    devdrv_dma_reg_wr(io_base, DEVDRV_SDI_AXIM_AWUSER_SET_1, regval);
}

void devdrv_dma_check_sram_init_status(const void __iomem *io_base, unsigned long timeout)
{
    u32 regval = 0;

    devdrv_dma_reg_rd(io_base, DEVDRV_DMA_COMMON_AND_CH_ERR_STS, &regval);
    while ((regval & 0x10000) != 0x10000) {
        if (timeout-- == 0) {
            devdrv_err("Calling devdrv_dma_check_sram_init_status check failed.\n");
            break;
        }

        devdrv_dma_reg_rd(io_base, DEVDRV_DMA_COMMON_AND_CH_ERR_STS, &regval);
        udelay(1);
    }
}

void devdrv_dma_err_interrupt_unmask(void __iomem *io_base)
{
    devdrv_dma_reg_wr(io_base, DEVDRV_DMA_ERR_MASK, 0x0);
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

void devdrv_dma_chan_err_interrupt_mask(void __iomem *io_base, u32 val)
{
    return;
}
