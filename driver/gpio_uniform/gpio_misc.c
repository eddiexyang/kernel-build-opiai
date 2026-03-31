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
 * Create: 2022-9-24
 */

#include <linux/module.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/clk-provider.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/securec.h>
#include <asm/delay.h>
#include <linux/version.h>
#include <linux/device.h>
#include <linux/reset-controller.h>
#include <linux/reset.h>
#include "hisi_gpio.h"

enum {
    AO_GPIO0,
    AO_GPIO1,
    PERI_GPIO2,
    PERI_GPIO3,
    IO_GPIO4,
    ISP_GPIO5,
    IO_GPIO6,
    MEDIA_GPIO7,
    GPIO_MAX_NUM,
};

enum {
    PERI_SUB_CTRL,
    IO_SUB_CTRL,
    ISP_SUB_CTRL,
    AO_SUB_CTRL,
    MEDIA_SUB_CTRL,
    SUB_CTRL_MAX,
};

#define PERI_GPIO_RESET_REQ_REG               (0xb00)  /* Soft reset request control register */
#define PERI_GPIO_RESET_DREQ_REG              (0xb04)  /* Soft reset derequest control register */
#define PERI_GPIO_RESET_ST_REG                (0x5b00)  /* Soft reset status register */

#define MEDIA_GPIO_RESET_REQ_REG               (0xc10)  /* Soft reset request control register */
#define MEDIA_GPIO_RESET_DREQ_REG              (0xc14)  /* Soft reset derequest control register */
#define MEDIA_GPIO_RESET_ST_REG                (0x5c10)  /* Soft reset status register */

#define IO_GPIO_RESET_REQ_REG               (0x400)  /* Soft reset request control register */
#define IO_GPIO_RESET_DREQ_REG              (0x404)  /* Soft reset derequest control register */
#define IO_GPIO_RESET_ST_REG                (0x5400)  /* Soft reset status register */

#define ISP_GPIO_RESET_REQ_REG               (0xa08)  /* Soft reset request control register */
#define ISP_GPIO_RESET_DREQ_REG              (0xa0C)  /* Soft reset derequest control register */
#define ISP_GPIO_RESET_ST_REG                (0x5a08)  /* Soft reset status register */

#define AO_GPIO_RESET_REQ_REG               (0xa30)  /* Soft reset request control register */
#define AO_GPIO_RESET_DREQ_REG              (0xa34)  /* Soft reset derequest control register */
#define AO_GPIO_RESET_ST_REG                (0x5a30)  /* Soft reset status register */

#define PERI_GPIO_ICG_EN_REG               (0x400)  /* Clock enable control register */
#define PERI_GPIO_ICG_DIS_REG              (0x404)  /* Clock disable control register */
#define PERI_GPIO_ICG_ST_REG               (0x5400)  /* Clock enable status register */

#define MEDIA_GPIO_ICG_EN_REG               (0x4f0)  /* Clock enable control register */
#define MEDIA_GPIO_ICG_DIS_REG              (0x4f4)  /* Clock disable control register */
#define MEDIA_GPIO_ICG_ST_REG               (0x54f0)  /* Clock enable status register */

#define IO_GPIO_ICG_EN_REG               (0x308)
#define IO_GPIO_ICG_DIS_REG              (0x30C)
#define IO_GPIO_ICG_ST_REG               (0x5308)

#define ISP_GPIO_ICG_EN_REG               (0x310)
#define ISP_GPIO_ICG_DIS_REG              (0x314)
#define ISP_GPIO_ICG_ST_REG               (0x5310)

#define AO_GPIO_ICG_EN_REG               (0x338)
#define AO_GPIO_ICG_DIS_REG              (0x33c)
#define AO_GPIO_ICG_ST_REG               (0x5338)

static const u64 g_gpio_subctrl_pbase[SUB_CTRL_MAX] = {
    [PERI_SUB_CTRL] = 0x80130000,
    [IO_SUB_CTRL] = 0xA0120000,
    [ISP_SUB_CTRL] = 0x300130000,
    [AO_SUB_CTRL] = 0xc0120000,
    [MEDIA_SUB_CTRL] = 0x400130000,
};

struct gpio_crg_reset_cell {
    u32 id;
    u32 reset_req_offset;
    u32 reset_dreq_offset;
    u32 reset_st_offset;
    u32 reset_mask;
};

struct gpio_crg_clkgate {
    u32 id;
    const char *name;
    u32 en_offset;
    u32 dis_offset;
    u32 st_offset;
    u32 mask;
};

static const struct gpio_crg_reset_cell g_gpio_subctrl_resets[GPIO_MAX_NUM] = {
    { AO_GPIO0, AO_GPIO_RESET_REQ_REG, AO_GPIO_RESET_DREQ_REG, AO_GPIO_RESET_ST_REG, 0x1},
    { AO_GPIO1, AO_GPIO_RESET_REQ_REG, AO_GPIO_RESET_DREQ_REG, AO_GPIO_RESET_ST_REG, 0x2},
    { PERI_GPIO2, PERI_GPIO_RESET_REQ_REG, PERI_GPIO_RESET_DREQ_REG, PERI_GPIO_RESET_ST_REG, 0x1},
    { PERI_GPIO3, PERI_GPIO_RESET_REQ_REG, PERI_GPIO_RESET_DREQ_REG, PERI_GPIO_RESET_ST_REG, 0x2},
    { IO_GPIO4, IO_GPIO_RESET_REQ_REG, IO_GPIO_RESET_DREQ_REG, IO_GPIO_RESET_ST_REG, 0x1},
    { ISP_GPIO5, ISP_GPIO_RESET_REQ_REG, ISP_GPIO_RESET_DREQ_REG, ISP_GPIO_RESET_ST_REG, 0x1},
    { IO_GPIO6, IO_GPIO_RESET_REQ_REG, IO_GPIO_RESET_DREQ_REG, IO_GPIO_RESET_ST_REG, 0x2},
    { MEDIA_GPIO7, MEDIA_GPIO_RESET_REQ_REG, MEDIA_GPIO_RESET_DREQ_REG, MEDIA_GPIO_RESET_ST_REG, 0x1},
};

static const struct gpio_crg_clkgate g_gpio_subctrl_clkgates[GPIO_MAX_NUM] = {
    { AO_GPIO0, "icg_en_gpio0", AO_GPIO_ICG_EN_REG, AO_GPIO_ICG_DIS_REG, AO_GPIO_ICG_ST_REG, 0x1},
    { AO_GPIO1, "icg_en_gpio1", AO_GPIO_ICG_EN_REG, AO_GPIO_ICG_DIS_REG, AO_GPIO_ICG_ST_REG, 0x2},
    { PERI_GPIO2, "icg_en_gpio2", PERI_GPIO_ICG_EN_REG, PERI_GPIO_ICG_DIS_REG, PERI_GPIO_ICG_ST_REG, 0x1},
    { PERI_GPIO3, "icg_en_gpio3", PERI_GPIO_ICG_EN_REG, PERI_GPIO_ICG_DIS_REG, PERI_GPIO_ICG_ST_REG, 0x2},
    { IO_GPIO4, "icg_en_gpio4", IO_GPIO_ICG_EN_REG, IO_GPIO_ICG_DIS_REG, IO_GPIO_ICG_ST_REG, 0x1},
    { ISP_GPIO5, "icg_en_gpio5", ISP_GPIO_ICG_EN_REG, ISP_GPIO_ICG_DIS_REG, ISP_GPIO_ICG_ST_REG, 0x1},
    { IO_GPIO6, "icg_en_gpio6", IO_GPIO_ICG_EN_REG, IO_GPIO_ICG_DIS_REG, IO_GPIO_ICG_ST_REG, 0x2},
    { MEDIA_GPIO7, "icg_en_gpio7", MEDIA_GPIO_ICG_EN_REG, MEDIA_GPIO_ICG_DIS_REG, MEDIA_GPIO_ICG_ST_REG, 0x1},
};

struct gpio_subctrl_recover_cell {
    u32 cfg_offset;
    u32 clk_mux_sel_mask;
    u32 clk_oe_cfg_mask;
    u32 dat_mux_sel_mask;
    u32 scl_cfg_mask;
    u32 dat_oe_cfg_mask;
};

static void __iomem *g_gpio_subctrl_vbase[GPIO_MAX_NUM] = {0};
static spinlock_t g_subctrl_lock[SUB_CTRL_MAX] = {0};
static int g_lock_init_flag[SUB_CTRL_MAX] = {0};
static int host_id_to_subctrl_id(u32 host_id)
{
    int index;
    switch (host_id) {
        case PERI_GPIO2:
        case PERI_GPIO3:
            index = PERI_SUB_CTRL;
            break;
        case ISP_GPIO5:
            index = ISP_SUB_CTRL;
            break;
        case IO_GPIO4:
        case IO_GPIO6:
            index = IO_SUB_CTRL;
            break;
        case MEDIA_GPIO7:
            index = MEDIA_SUB_CTRL;
            break;
        case AO_GPIO0:
        case AO_GPIO1:
            index = AO_SUB_CTRL;
            break;
        default:
            index = -1;
    }
    return index;
}

int hisi_subctrl_gpio_reset_assert(struct device *dev, u32 host_id)
{
    u32 reg_offset;
    u32 mask;
    u32 reg_val;
    int subctrl_id;
    unsigned long flags;

    if (host_id >= GPIO_MAX_NUM) {
        return -EINVAL;
    }
    subctrl_id = host_id_to_subctrl_id(host_id);
    if (subctrl_id < 0) {
        pr_err("host_id %u is invaild\n", host_id);
        return -EINVAL;
    }

    reg_offset = g_gpio_subctrl_resets[host_id].reset_req_offset;
    mask = g_gpio_subctrl_resets[host_id].reset_mask;

    spin_lock_irqsave(&g_subctrl_lock[subctrl_id], flags);
    reg_val = readl_relaxed(g_gpio_subctrl_vbase[host_id] + reg_offset);
    rmb();
    writel_relaxed((reg_val | mask), (g_gpio_subctrl_vbase[host_id] + reg_offset));
    rmb();
    spin_unlock_irqrestore(&g_subctrl_lock[subctrl_id], flags);
    return 0;
}

int hisi_subctrl_gpio_reset_deassert(struct device *dev, u32 host_id)
{
    int subctrl_id;
    unsigned long flags;
    u32 reg_val;
    u32 mask;
    u32 reg_offset;

    if (host_id >= GPIO_MAX_NUM) {
        return -EINVAL;
    }
    subctrl_id = host_id_to_subctrl_id(host_id);
    if (subctrl_id < 0) {
        pr_err("host_id %u is invaild\n", host_id);
        return -EINVAL;
    }

    reg_offset = g_gpio_subctrl_resets[host_id].reset_dreq_offset;
    mask = g_gpio_subctrl_resets[host_id].reset_mask;
    spin_lock_irqsave(&g_subctrl_lock[subctrl_id], flags);
    reg_val = readl_relaxed(g_gpio_subctrl_vbase[host_id] + reg_offset);
    rmb();
    writel_relaxed((reg_val | mask), (g_gpio_subctrl_vbase[host_id] + reg_offset));
    rmb();
    spin_unlock_irqrestore(&g_subctrl_lock[subctrl_id], flags);
    return 0;
}

int hisi_subctrl_gpio_get_reset_status(struct device *dev, u32 host_id)
{
    unsigned long flags;
    u32 reg_offset;
    u32 reg_val;
    u32 mask;
    int subctrl_id;

    if (host_id >= GPIO_MAX_NUM) {
        return -EINVAL;
    }
    subctrl_id = host_id_to_subctrl_id(host_id);
    if (subctrl_id < 0) {
        pr_err("host_id %u is invaild\n", host_id);
        return -EINVAL;
    }

    reg_offset = g_gpio_subctrl_resets[host_id].reset_st_offset;
    mask = g_gpio_subctrl_resets[host_id].reset_mask;
    spin_lock_irqsave(&g_subctrl_lock[subctrl_id], flags);
    reg_val = readl_relaxed(g_gpio_subctrl_vbase[host_id] + reg_offset);
    rmb();
    spin_unlock_irqrestore(&g_subctrl_lock[subctrl_id], flags);

    return ((mask & reg_val) ? 1 : 0);
}

int hisi_subctrl_gpio_clkgate_enable(struct device *dev, u32 host_id)
{
    unsigned long flags = 0;
    int subctrl_id;
    u32 reg_offset;
    u32 reg_val;
    u32 mask;

    if (host_id >= GPIO_MAX_NUM) {
        return -EINVAL;
    }
    subctrl_id = host_id_to_subctrl_id(host_id);
    if (subctrl_id < 0) {
        pr_err("bus id %u is invaild\n", host_id);
        return -EINVAL;
    }
    spin_lock_irqsave(&g_subctrl_lock[subctrl_id], flags);

    reg_offset = g_gpio_subctrl_clkgates[host_id].en_offset;
    mask = g_gpio_subctrl_clkgates[host_id].mask;

    reg_val = readl_relaxed(g_gpio_subctrl_vbase[host_id] + reg_offset);
    rmb();
    reg_val = reg_val | mask;
    writel_relaxed(reg_val, (g_gpio_subctrl_vbase[host_id] + reg_offset));
    rmb();

    spin_unlock_irqrestore(&g_subctrl_lock[subctrl_id], flags);
    return 0;
}

int hisi_subctrl_gpio_clkgate_disable(struct device *dev, u32 host_id)
{
    u32 reg_offset;
    u32 reg_val;
    u32 mask;
    int subctrl_id;
    unsigned long flags = 0;

    if (host_id >= GPIO_MAX_NUM) {
        return -EINVAL;
    }
    subctrl_id = host_id_to_subctrl_id(host_id);
    if (subctrl_id < 0) {
        pr_err("bus id %u is invaild\n", host_id);
        return -EINVAL;
    }
    spin_lock_irqsave(&g_subctrl_lock[subctrl_id], flags);

    reg_offset = g_gpio_subctrl_clkgates[host_id].dis_offset;
    mask = g_gpio_subctrl_clkgates[host_id].mask;

    reg_val = readl_relaxed(g_gpio_subctrl_vbase[host_id] + reg_offset);
    rmb();
    reg_val = reg_val | mask;
    writel_relaxed(reg_val, (g_gpio_subctrl_vbase[host_id] + reg_offset));
    rmb();
    spin_unlock_irqrestore(&g_subctrl_lock[subctrl_id], flags);
    return 0;
}

bool hisi_subctrl_gpio_clkgate_is_enabled(struct device *dev, u32 host_id)
{
    u32 reg_offset;
    u32 reg_val;
    u32 mask;
    unsigned long flags = 0;
    int subctrl_id;

    if (host_id >= GPIO_MAX_NUM) {
        return false;
    }
    subctrl_id = host_id_to_subctrl_id(host_id);
    if (subctrl_id < 0) {
        pr_err("bus id %u is invaild\n", host_id);
        return false;
    }
    spin_lock_irqsave(&g_subctrl_lock[subctrl_id], flags);
    reg_offset = g_gpio_subctrl_clkgates[host_id].st_offset;
    mask = g_gpio_subctrl_clkgates[host_id].mask;
    reg_val = readl_relaxed(g_gpio_subctrl_vbase[host_id] + reg_offset);
    rmb();
    spin_unlock_irqrestore(&g_subctrl_lock[subctrl_id], flags);
    return ((reg_val & mask) ? true : false);
}

int hisi_gpio_get_clk_rst_info(struct hisi_gpio *hgpio)
{
    return 0;
}

int hisi_gpio_crg_init(u32 host_id, struct device *dev)
{
    int subctrl_id;

    if (host_id >= GPIO_MAX_NUM) {
        return -EINVAL;
    }
    subctrl_id = host_id_to_subctrl_id(host_id);
    if (subctrl_id < 0) {
        dev_err(dev, "host_id %u is invaild\n", host_id);
        return -EINVAL;
    }

    g_gpio_subctrl_vbase[host_id] = devm_ioremap(dev, g_gpio_subctrl_pbase[subctrl_id], 0x10000);
    if (!g_gpio_subctrl_vbase[host_id]) {
        dev_err(dev, "subctrl_id = %d devm_ioremap failed\n", subctrl_id);
        g_gpio_subctrl_vbase[host_id] = NULL;
        return -ENOMEM;
    }
    if (g_lock_init_flag[subctrl_id] == 0) {
        spin_lock_init(&g_subctrl_lock[subctrl_id]);
        g_lock_init_flag[subctrl_id] = 1;
    }

    return 0;
}
