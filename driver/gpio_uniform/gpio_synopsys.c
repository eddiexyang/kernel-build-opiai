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
 * Description: GPIO synopsys physical layer source file.
 * Author: huawei
 * Create: 2022-04-25
 */
#include <linux/bitops.h>
#include <linux/irq.h>
#include <linux/err.h>
#include "hisi_gpio.h"

/* Address offset definition of the standard synopsys GPIO register */
#define GPIO_SWPORT_DR_REG (0x00)                  /* data register */
#define GPIO_SWPORT_DDR_REG (0x04)                 /* direction register */
#define GPIO_INTEN_REG (0x30)                      /* Interrupt enable register */
#define GPIO_INTMASK_REG (0x34)                    /* Interrupt mask register */
#define GPIO_INTTYPE_LEVEL_REG (0x38)              /* Interrupt type register */
#define GPIO_INT_POLARITY_REG (0x3C)               /* Interrupt trigger polarity control register */
#define GPIO_INTSTATUS_REG (0x40)                  /* Masked interrupt status register */
#define GPIO_RAW_INTSTATUS_REG (0x44)              /* Raw interrupt status register */
#define GPIO_DEBOUNCE_REG (0x48)                   /* Deglitch control register */
#define GPIO_PORT_EOI_REG (0x4c)                   /* Interrupt clear register */
#define GPIO_EXT_PORT_REG (0x50)                   /* External port data register */
#define GPIO_INT_DEDGE_REG (0x54)                  /* Dual-edge interrupt enable register */
#define GPIO_LS_SYNC_REG (0x60)                    /* Interrupt synchronization enable register */
#define GPIO_INTCOMB_MASK_REG (0x0FFC)             /* Combined interrupt mask register */

static void hisi_gpio_reg_write_bits(struct io_region *reg_region, u32 reg, u32 bit, bool set)
{
    u32 value;

    value = readl(reg_region->io_base + reg);
    if (set) {
        value |= BIT(bit);
    } else {
        value &= ~BIT(bit);
    }
    writel(value, reg_region->io_base + reg);
}

static void hisi_gpio_int_mask_syns(struct io_region *reg_region, u32 hwirq)
{
    hisi_gpio_reg_write_bits(reg_region, GPIO_INTMASK_REG, hwirq, 1);
}

static void hisi_gpio_int_unmask_syns(struct io_region *reg_region, u32 hwirq)
{
    hisi_gpio_reg_write_bits(reg_region, GPIO_INTMASK_REG, hwirq, 0);
}

static void hisi_gpio_enable_int_syns(struct io_region *reg_region, u32 hwirq, u32 enable)
{
    hisi_gpio_reg_write_bits(reg_region, GPIO_INTEN_REG, hwirq, enable);
}

static void hisi_gpio_enable_comb_int_syns(struct io_region *reg_region, u32 enable)
{
    hisi_gpio_reg_write_bits(reg_region, GPIO_INTCOMB_MASK_REG, 0, enable);
}

static int hisi_gpio_set_int_type_syns(struct io_region *reg_region, u32 hwirq, u32 type)
{
    hisi_gpio_reg_write_bits(reg_region, GPIO_INT_DEDGE_REG, hwirq, 0);
    switch (type) {
        case IRQ_TYPE_LEVEL_HIGH:
            /* GPIO_INTTYPE_LEVEL_REG: 0-level, 1-edge; GPIO_INT_POLARITY_REG: 0-low, 1-high */
            hisi_gpio_reg_write_bits(reg_region, GPIO_INTTYPE_LEVEL_REG, hwirq, 0);
            hisi_gpio_reg_write_bits(reg_region, GPIO_INT_POLARITY_REG, hwirq, 1);
            break;
        case IRQ_TYPE_LEVEL_LOW:
            hisi_gpio_reg_write_bits(reg_region, GPIO_INTTYPE_LEVEL_REG, hwirq, 0);
            hisi_gpio_reg_write_bits(reg_region, GPIO_INT_POLARITY_REG, hwirq, 0);
            break;
        case IRQ_TYPE_EDGE_FALLING:
            hisi_gpio_reg_write_bits(reg_region, GPIO_INTTYPE_LEVEL_REG, hwirq, 1);
            hisi_gpio_reg_write_bits(reg_region, GPIO_INT_POLARITY_REG, hwirq, 0);
            break;
        case IRQ_TYPE_EDGE_RISING:
            hisi_gpio_reg_write_bits(reg_region, GPIO_INTTYPE_LEVEL_REG, hwirq, 1);
            hisi_gpio_reg_write_bits(reg_region, GPIO_INT_POLARITY_REG, hwirq, 1);
            break;
        case IRQ_TYPE_EDGE_BOTH:
            hisi_gpio_reg_write_bits(reg_region, GPIO_INTTYPE_LEVEL_REG, hwirq, 1);
            hisi_gpio_reg_write_bits(reg_region, GPIO_INT_DEDGE_REG, hwirq, 1);
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

static void hisi_gpio_set_direction_in_syns(struct io_region *reg_region, u32 offset)
{
    hisi_gpio_reg_write_bits(reg_region, GPIO_SWPORT_DDR_REG, offset, 0); /* bit 设置0:input */
}

static void hisi_gpio_set_direction_out_syns(struct io_region *reg_region, u32 offset)
{
    hisi_gpio_reg_write_bits(reg_region, GPIO_SWPORT_DDR_REG, offset, 1); /* bit 设置1:output */
}

static void hisi_gpio_get_direction_syns(struct io_region *reg_region, u32 offset, enum gpio_dir_type *dir)
{
    u32 value;

    value = readl(reg_region->io_base + GPIO_SWPORT_DDR_REG);
    if (value & BIT(offset)) {
        *dir = GPIO_DIR_OUT;
    } else {
        *dir = GPIO_DIR_IN;
    }
}

static int hisi_gpio_get_val_syns(struct io_region *reg_region, u32 offset)
{
    int value;

    value = readl(reg_region->io_base + GPIO_EXT_PORT_REG);
    return !!(value & BIT(offset));
}

static void hisi_gpio_set_val_syns(struct io_region *reg_region, u32 offset, u32 value)
{
    hisi_gpio_reg_write_bits(reg_region, GPIO_SWPORT_DR_REG, offset, !!value);
}

static void hisi_gpio_set_debounce_syns(struct io_region *reg_region, u32 offset, u32 debounce)
{
    hisi_gpio_reg_write_bits(reg_region, GPIO_DEBOUNCE_REG, offset, !!debounce);
}

static int hisi_gpio_get_int_status_syns(struct io_region *reg_region)
{
    return readl(reg_region->io_base + GPIO_INTSTATUS_REG);
}

static void hisi_gpio_clear_int_syns(struct io_region *reg_region, u32 offset)
{
    u32 val = 0;
    val |= BIT(offset);
    writel(val, reg_region->io_base + GPIO_PORT_EOI_REG);
}

static void hisi_gpio_save_regs_syns(struct io_region *reg_region, struct context_save_regs *ctx)
{
    ctx->dir = readl(reg_region->io_base + GPIO_SWPORT_DDR_REG);
    ctx->data = readl(reg_region->io_base + GPIO_SWPORT_DR_REG);
    ctx->ext = readl(reg_region->io_base + GPIO_EXT_PORT_REG);
    ctx->int_mask = readl(reg_region->io_base + GPIO_INTMASK_REG);
    ctx->int_en = readl(reg_region->io_base + GPIO_INTEN_REG);
    ctx->int_pol = readl(reg_region->io_base + GPIO_INT_POLARITY_REG);
    ctx->int_type = readl(reg_region->io_base + GPIO_INTTYPE_LEVEL_REG);
    ctx->int_deb = readl(reg_region->io_base + GPIO_DEBOUNCE_REG);
    /* Mask out interrupts */
    writel(~ctx->wake_en, reg_region->io_base + GPIO_INTMASK_REG);
}

static void hisi_gpio_restore_regs_syns(struct io_region *reg_region, struct context_save_regs *ctx)
{
    writel(ctx->dir, reg_region->io_base + GPIO_SWPORT_DDR_REG);
    writel(ctx->data, reg_region->io_base + GPIO_SWPORT_DR_REG);
    writel(ctx->ext, reg_region->io_base + GPIO_EXT_PORT_REG);
    writel(ctx->int_mask, reg_region->io_base + GPIO_INTMASK_REG);
    writel(ctx->int_en, reg_region->io_base + GPIO_INTEN_REG);
    writel(ctx->int_pol, reg_region->io_base + GPIO_INT_POLARITY_REG);
    writel(ctx->int_type, reg_region->io_base + GPIO_INTTYPE_LEVEL_REG);
    writel(ctx->int_deb, reg_region->io_base + GPIO_DEBOUNCE_REG);
    /* Clear out spurious interrupts */
    writel(0xffffffff, reg_region->io_base + GPIO_PORT_EOI_REG);
}

static const struct hisi_gpio_ops syn_gpio_ops = {
    .set_direction_in = hisi_gpio_set_direction_in_syns,
    .set_direction_out = hisi_gpio_set_direction_out_syns,
    .get_direction = hisi_gpio_get_direction_syns,
    .get_val = hisi_gpio_get_val_syns,
    .set_val = hisi_gpio_set_val_syns,
    .set_debounce = hisi_gpio_set_debounce_syns,
    .mask_int = hisi_gpio_int_mask_syns,
    .unmask_int = hisi_gpio_int_unmask_syns,
    .enable_int = hisi_gpio_enable_int_syns,
    .set_int_type = hisi_gpio_set_int_type_syns,
    .get_int_status = hisi_gpio_get_int_status_syns,
    .clear_int = hisi_gpio_clear_int_syns,
    .save_regs = hisi_gpio_save_regs_syns,
    .restore_regs = hisi_gpio_restore_regs_syns,
    .enable_comb_int = hisi_gpio_enable_comb_int_syns,
};
const struct hisi_gpio_ops *hisi_gpio_get_ops(void)
{
    return &syn_gpio_ops;
}
