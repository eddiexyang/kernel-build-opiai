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
 * Description: i2c misc source file
 * Author: huawei
 * Create: 2022-12-6
 */

#include <linux/module.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/securec.h>
#include <asm/delay.h>
#include <linux/version.h>
#include <linux/device.h>
#include "hisi_i2c.h"

#ifndef ASCEND_MDC_LITE
#include "i2c_misc_310.h"
#else
#include "i2c_misc_610.h"
#endif

static void __iomem *g_subctrl_vbase[I2C_MAX_NUM] = {0};
static spinlock_t g_subctrl_lock[SUB_CTRL_MAX] = {0};
static int g_lock_init_flag[SUB_CTRL_MAX] = {0};

int hisi_subctrl_i2c_reset_assert(struct device *i2c_dev, u32 bus_id)
{
    unsigned long flags;
    int subctrl_id;
    u32 reg_offset;
    u32 mask;
    u32 reg_val;

    if (bus_id >= I2C_MAX_NUM) {
        return -EINVAL;
    }
    subctrl_id = bus_id_to_subctrl_id(bus_id);
    if (subctrl_id < 0) {
        pr_err("bus id %u is invaild\n", bus_id);
        return -EINVAL;
    }

    reg_offset = g_subctrl_resets[bus_id].reset_req_offset;
    mask = g_subctrl_resets[bus_id].reset_mask;

    spin_lock_irqsave(&g_subctrl_lock[subctrl_id], flags);
    reg_val = readl_relaxed(g_subctrl_vbase[bus_id] + reg_offset);
    rmb();
    writel_relaxed((reg_val | mask), (g_subctrl_vbase[bus_id] + reg_offset));
    rmb();
    spin_unlock_irqrestore(&g_subctrl_lock[subctrl_id], flags);
    return 0;
}

int hisi_subctrl_i2c_reset_deassert(struct device *i2c_dev, u32 bus_id)
{
    u32 reg_offset;
    u32 reg_val;
    u32 mask;
    int subctrl_id;
    unsigned long flags;

    if (bus_id >= I2C_MAX_NUM) {
        return -EINVAL;
    }
    subctrl_id = bus_id_to_subctrl_id(bus_id);
    if (subctrl_id < 0) {
        pr_err("bus id %u is invaild\n", bus_id);
        return -EINVAL;
    }

    reg_offset = g_subctrl_resets[bus_id].reset_dreq_offset;
    mask = g_subctrl_resets[bus_id].reset_mask;
    spin_lock_irqsave(&g_subctrl_lock[subctrl_id], flags);
    reg_val = readl_relaxed(g_subctrl_vbase[bus_id] + reg_offset);
    rmb();
    writel_relaxed((reg_val | mask), (g_subctrl_vbase[bus_id] + reg_offset));
    rmb();
    spin_unlock_irqrestore(&g_subctrl_lock[subctrl_id], flags);
    return 0;
}

int hisi_subctrl_i2c_get_reset_status(struct device *i2c_dev, u32 bus_id)
{
    unsigned long flags;
    u32 reg_val;
    u32 reg_offset;
    int subctrl_id;
    u32 mask;

    if (bus_id >= I2C_MAX_NUM) {
        return -EINVAL;
    }
    subctrl_id = bus_id_to_subctrl_id(bus_id);
    if (subctrl_id < 0) {
        pr_err("bus id %u is invaild\n", bus_id);
        return -EINVAL;
    }

    reg_offset = g_subctrl_resets[bus_id].reset_st_offset;
    mask = g_subctrl_resets[bus_id].reset_mask;
    spin_lock_irqsave(&g_subctrl_lock[subctrl_id], flags);
    reg_val = readl_relaxed(g_subctrl_vbase[bus_id] + reg_offset);
    rmb();
    spin_unlock_irqrestore(&g_subctrl_lock[subctrl_id], flags);

    return ((mask & reg_val) ? 1 : 0);
}

int hisi_subctrl_i2c_clkgate_enable(struct device *i2c_dev, u32 bus_id)
{
    unsigned long flags = 0;
    int subctrl_id;
    u32 reg_offset;
    u32 reg_val;
    u32 mask;

    if (bus_id >= I2C_MAX_NUM) {
        return -EINVAL;
    }
    subctrl_id = bus_id_to_subctrl_id(bus_id);
    if (subctrl_id < 0) {
        pr_err("bus id %u is invaild\n", bus_id);
        return -EINVAL;
    }
    spin_lock_irqsave(&g_subctrl_lock[subctrl_id], flags);

    reg_offset = g_subctrl_clkgates[bus_id].en_offset;
    mask = g_subctrl_clkgates[bus_id].mask;

    reg_val = readl_relaxed(g_subctrl_vbase[bus_id] + reg_offset);
    rmb();
    reg_val = reg_val | mask;
    writel_relaxed(reg_val, (g_subctrl_vbase[bus_id] + reg_offset));
    rmb();

    spin_unlock_irqrestore(&g_subctrl_lock[subctrl_id], flags);
    return 0;
}

int hisi_subctrl_i2c_clkgate_disable(struct device *i2c_dev, u32 bus_id)
{
    u32 reg_offset;
    u32 reg_val;
    u32 mask;
    unsigned long flags = 0;
    int subctrl_id;

    if (bus_id >= I2C_MAX_NUM) {
        return -EINVAL;
    }
    subctrl_id = bus_id_to_subctrl_id(bus_id);
    if (subctrl_id < 0) {
        pr_err("bus id %u is invaild\n", bus_id);
        return -EINVAL;
    }
    spin_lock_irqsave(&g_subctrl_lock[subctrl_id], flags);

    reg_offset = g_subctrl_clkgates[bus_id].dis_offset;
    mask = g_subctrl_clkgates[bus_id].mask;

    reg_val = readl_relaxed(g_subctrl_vbase[bus_id] + reg_offset);
    rmb();
    reg_val = reg_val | mask;
    writel_relaxed(reg_val, (g_subctrl_vbase[bus_id] + reg_offset));
    rmb();
    spin_unlock_irqrestore(&g_subctrl_lock[subctrl_id], flags);
    return 0;
}

bool hisi_subctrl_i2c_clkgate_is_enabled(struct device *i2c_dev, u32 bus_id)
{
    u32 reg_offset;
    u32 reg_val;
    u32 mask;
    unsigned long flags = 0;
    int subctrl_id;

    if (bus_id >= I2C_MAX_NUM) {
        return false;
    }
    subctrl_id = bus_id_to_subctrl_id(bus_id);
    if (subctrl_id < 0) {
        pr_err("bus id %u is invaild\n", bus_id);
        return false;
    }
    spin_lock_irqsave(&g_subctrl_lock[subctrl_id], flags);
    reg_offset = g_subctrl_clkgates[bus_id].st_offset;
    mask = g_subctrl_clkgates[bus_id].mask;
    reg_val = readl_relaxed(g_subctrl_vbase[bus_id] + reg_offset);
    rmb();
    spin_unlock_irqrestore(&g_subctrl_lock[subctrl_id], flags);
    return ((reg_val & mask) ? true : false);
}

#define I2C_RECOVERY_CLK_CNT   9
int hisi_subctrl_i2c_recovery(struct device *i2c_dev, u32 bus_id)
{
    u32 val;
    int i;
    int subctrl_id;
    void __iomem *subctrl_vbase;
    unsigned long flags = 0;

    if (bus_id >= I2C_MAX_NUM) {
        return -EINVAL;
    }
    pr_info("[%s][%d]\n", __FUNCTION__, __LINE__);
    subctrl_id = bus_id_to_subctrl_id(bus_id);
    if (subctrl_id < 0) {
        pr_err("bus_id %u is invaild\n", bus_id);
        return -EINVAL;
    }
    spin_lock_irqsave(&g_subctrl_lock[subctrl_id], flags);
    subctrl_vbase = g_subctrl_vbase[bus_id];
    /* Step 1 Set i2c*_clk_mux_sel and i2c*_dat_mux_sel in SUBCTRL to high to completely control
    the pin behavior of the I2C SCL and SDA interfaces through software. */
    val = readl(subctrl_vbase + g_i2c_recover_cfg[bus_id].cfg_offset);
    val |= g_i2c_recover_cfg[bus_id].clk_mux_sel_mask;
    val |= g_i2c_recover_cfg[bus_id].dat_mux_sel_mask;
    writel_relaxed(val, subctrl_vbase + g_i2c_recover_cfg[bus_id].cfg_offset);

    /* Step 2 Set i2c*_clk_oe_cfg in SUBCTRL to high and configure the SCL pin as output.
    Set i2c*_dat_oe_cfg in IO_WRAP_UP SUBCTRL to low and configure the SDA pin as an input pin. */
    val = readl(subctrl_vbase + g_i2c_recover_cfg[bus_id].cfg_offset);
    val |= g_i2c_recover_cfg[bus_id].clk_oe_cfg_mask;
    val &= ~(g_i2c_recover_cfg[bus_id].dat_oe_cfg_mask);
    writel_relaxed(val, subctrl_vbase + g_i2c_recover_cfg[bus_id].cfg_offset);

    /* Step 3 Set i2c*_scl_cfg in SUBCTRL to high to control the I2C SCL interface pin to output high level. */
    /* Step 4 Set i2c*_scl_cfg in SUBCTRLL to low to control the I2C SCL interface pin to output low level. */
    /* Step 5 Repeat steps 3 and 4 to make the level of the I2C pin continuously flip. At the same time, the
    program inserts a waiting time so that the flip frequency is 10 kHz and transmits at least nine SCL clock pulses. */
    for (i = 0; i < I2C_RECOVERY_CLK_CNT; i++) {
        val = readl(subctrl_vbase + g_i2c_recover_cfg[bus_id].cfg_offset);
        val |= g_i2c_recover_cfg[bus_id].scl_cfg_mask;
        writel_relaxed(val, subctrl_vbase + g_i2c_recover_cfg[bus_id].cfg_offset);
        udelay(50);
        val = readl(subctrl_vbase + g_i2c_recover_cfg[bus_id].cfg_offset);
        val &= ~(g_i2c_recover_cfg[bus_id].scl_cfg_mask);
        writel_relaxed(val, subctrl_vbase + g_i2c_recover_cfg[bus_id].cfg_offset);
        udelay(50);
    }

    /* Step 6 Set i2c*_clk_mux_sel and i2c*_dat_mux_sel in SUBCTRL to low,
    and set the level of the I2C pin to be controlled by the I2C controller. */
    val = readl(subctrl_vbase + g_i2c_recover_cfg[bus_id].cfg_offset);
    val &= ~(g_i2c_recover_cfg[bus_id].clk_mux_sel_mask);
    val &= ~(g_i2c_recover_cfg[bus_id].dat_mux_sel_mask);
    writel_relaxed(val, subctrl_vbase + g_i2c_recover_cfg[bus_id].cfg_offset);
    spin_unlock_irqrestore(&g_subctrl_lock[subctrl_id], flags);
    return 0;
}

int hisi_i2c_get_clk_rst_info(struct hisi_i2c_controller *hi2c)
{
    return 0;
}

int hisi_i2c_subctrl_init(u32 bus_id, struct device *dev)
{
    int subctrl_id;

    if (bus_id >= I2C_MAX_NUM) {
        return -EINVAL;
    }
    subctrl_id = bus_id_to_subctrl_id(bus_id);
    if (subctrl_id < 0) {
        dev_err(dev, "bus id %u is invaild\n", bus_id);
        return -EINVAL;
    }

    g_subctrl_vbase[bus_id] = devm_ioremap(dev, g_subctrl_pbase[subctrl_id], 0x10000);
    if (!g_subctrl_vbase[bus_id]) {
        dev_err(dev, "subctrl_id = %d devm_ioremap failed\n", subctrl_id);
        g_subctrl_vbase[bus_id] = NULL;
        return -ENOMEM;
    }
    if (g_lock_init_flag[subctrl_id] == 0) {
        spin_lock_init(&g_subctrl_lock[subctrl_id]);
        g_lock_init_flag[subctrl_id] = 1;
    }

    return 0;
}
