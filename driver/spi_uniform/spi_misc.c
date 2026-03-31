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
 * Create: 2022-4-30
 */

#include <linux/io.h>
#include "hispi.h"
#ifndef ASCEND_MDC_LITE
#include "spi_misc_310.h"
#else
#include "spi_misc_610.h"
#endif

static void __iomem *g_spi_subctrl_vbase[SUB_CTRL_MAX] = {0};
static spinlock_t g_spi_subctrl_lock[SUB_CTRL_MAX] = {0};
static unsigned int g_spi_ref_count[SUB_CTRL_MAX] = {0};

int hisi_subctrl_spi_reset_assert(struct device *spi_dev, u32 bus_num)
{
    u32 reg_offset, reg_val, mask;
    int subctrl_id;
    unsigned long flags;

    if (bus_num >= SPI_MAX_NUM) {
        return -EINVAL;
    }
    subctrl_id = bus_num_to_subctrl_index(bus_num);

    reg_offset = g_spi_subctrl_resets[bus_num].reset_req_offset;
    mask = g_spi_subctrl_resets[bus_num].reset_mask;

    spin_lock_irqsave(&g_spi_subctrl_lock[subctrl_id], flags);
    reg_val = readl_relaxed(g_spi_subctrl_vbase[subctrl_id] + reg_offset);
    rmb();
    writel_relaxed((reg_val | mask), (g_spi_subctrl_vbase[subctrl_id] + reg_offset));
    rmb();
    spin_unlock_irqrestore(&g_spi_subctrl_lock[subctrl_id], flags);
    return 0;
}

int hisi_subctrl_spi_reset_deassert(struct device *spi_dev, u32 bus_num)
{
    unsigned long flags;
    u32 reg_val, mask, reg_offset;
    int subctrl_id;

    if (bus_num >= SPI_MAX_NUM) {
        return -EINVAL;
    }
    subctrl_id = bus_num_to_subctrl_index(bus_num);

    reg_offset = g_spi_subctrl_resets[bus_num].reset_dreq_offset;
    mask = g_spi_subctrl_resets[bus_num].reset_mask;
    spin_lock_irqsave(&g_spi_subctrl_lock[subctrl_id], flags);
    reg_val = readl_relaxed(g_spi_subctrl_vbase[subctrl_id] + reg_offset);
    rmb();
    writel_relaxed((reg_val | mask), (g_spi_subctrl_vbase[subctrl_id] + reg_offset));
    rmb();
    spin_unlock_irqrestore(&g_spi_subctrl_lock[subctrl_id], flags);
    return 0;
}

int hisi_subctrl_spi_get_reset_status(struct device *spi_dev, u32 bus_num)
{
    u32 mask, reg_offset, reg_val;
    unsigned long flags;
    int subctrl_id;

    if (bus_num >= SPI_MAX_NUM) {
        return -EINVAL;
    }
    subctrl_id = bus_num_to_subctrl_index(bus_num);

    reg_offset = g_spi_subctrl_resets[bus_num].reset_st_offset;
    mask = g_spi_subctrl_resets[bus_num].reset_mask;
    spin_lock_irqsave(&g_spi_subctrl_lock[subctrl_id], flags);
    reg_val = readl_relaxed(g_spi_subctrl_vbase[subctrl_id] + reg_offset);
    rmb();
    spin_unlock_irqrestore(&g_spi_subctrl_lock[subctrl_id], flags);

    return ((mask & reg_val) ? 1 : 0);
}

int hisi_subctrl_spi_clkgate_enable(struct device *spi_dev, u32 bus_num)
{
    u32 reg_offset;
    u32 reg_val;
    u32 mask;
    unsigned long flags = 0;
    int subctrl_id;

    if (bus_num >= SPI_MAX_NUM) {
        return -EINVAL;
    }
    subctrl_id = bus_num_to_subctrl_index(bus_num);
    spin_lock_irqsave(&g_spi_subctrl_lock[subctrl_id], flags);

    reg_offset = g_spi_subctrl_clkgates[bus_num].en_offset;
    mask = g_spi_subctrl_clkgates[bus_num].mask;

    reg_val = readl_relaxed(g_spi_subctrl_vbase[subctrl_id] + reg_offset);
    rmb();
    reg_val = reg_val | mask;
    writel_relaxed(reg_val, (g_spi_subctrl_vbase[subctrl_id] + reg_offset));
    rmb();

    spin_unlock_irqrestore(&g_spi_subctrl_lock[subctrl_id], flags);
    return 0;
}

int hisi_subctrl_spi_clkgate_disable(struct device *spi_dev, u32 bus_num)
{
    u32 reg_offset;
    u32 reg_val;
    u32 mask;
    unsigned long flags = 0;
    int subctrl_id;

    if (bus_num >= SPI_MAX_NUM) {
        return -EINVAL;
    }
    subctrl_id = bus_num_to_subctrl_index(bus_num);
    spin_lock_irqsave(&g_spi_subctrl_lock[subctrl_id], flags);

    reg_offset = g_spi_subctrl_clkgates[bus_num].dis_offset;
    mask = g_spi_subctrl_clkgates[bus_num].mask;

    reg_val = readl_relaxed(g_spi_subctrl_vbase[subctrl_id] + reg_offset);
    rmb();
    reg_val = reg_val | mask;
    writel_relaxed(reg_val, (g_spi_subctrl_vbase[subctrl_id] + reg_offset));
    rmb();
    spin_unlock_irqrestore(&g_spi_subctrl_lock[subctrl_id], flags);
    return 0;
}

int spi_crg_init(u32 bus_num)
{
    int subctrl_id;

    if (bus_num >= SPI_MAX_NUM) {
        return -EINVAL;
    }
    subctrl_id = bus_num_to_subctrl_index(bus_num);
    if (subctrl_id < 0) {
        printk("bus num %u is invaild\n", bus_num);
        return -EINVAL;
    }
    if (g_spi_ref_count[subctrl_id] > 0) {
        pr_info("subctrl %d already init\n", subctrl_id);
        g_spi_ref_count[subctrl_id]++;
        return 0;
    }
    g_spi_ref_count[subctrl_id] = 1;
    spin_lock_init(&g_spi_subctrl_lock[subctrl_id]);
    g_spi_subctrl_vbase[subctrl_id] = ioremap(g_spi_subctrl_pbase[subctrl_id], 0x10000);
    if (!g_spi_subctrl_vbase[subctrl_id]) {
        pr_err("subctrl_id = %d ioremap failed\n", subctrl_id);
        g_spi_subctrl_vbase[subctrl_id] = NULL;
        return -ENOMEM;
    }

    return 0;
}

int hisi_spi_get_clk_rst_info(struct hisi_spi *hispi, struct device *dev)
{
    return 0;
}