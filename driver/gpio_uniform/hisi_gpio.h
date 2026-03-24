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
 * Description: GPIO header file of the driver logic layer
 * Author: huawei
 * Create: 2022-04-25
 */
#ifndef HISI_GPIO_H
#define HISI_GPIO_H
#include <linux/gpio.h>
#include <linux/reset-controller.h>
#include <linux/reset.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>

enum gpio_dir_type {
    GPIO_DIR_OUT = 0,
    GPIO_DIR_IN
};

struct io_region {
    void __iomem *io_base;
    u32 io_size;
};

struct context_save_regs {
    u32 data;
    u32 dir;
    u32 ext;
    u32 int_en;
    u32 int_mask;
    u32 int_type;
    u32 int_pol;
    u32 int_deb;
    u32 wake_en;
};

struct hisi_gpio_ops {
    void (*set_direction_in)(struct io_region *reg_region, u32 offset);
    void (*set_direction_out)(struct io_region *reg_region, u32 offset);
    void (*get_direction)(struct io_region *reg_region, u32 offset, enum gpio_dir_type *dir);
    int (*get_val)(struct io_region *reg_region, u32 offset);
    void (*set_val)(struct io_region *reg_region, u32 offset, u32 val);
    void (*set_debounce)(struct io_region *reg_region, u32 offset, u32 debounce);
    void (*mask_int)(struct io_region *reg_region, u32 hwirq);
    void (*unmask_int)(struct io_region *reg_region, u32 hwirq);
    void (*enable_int)(struct io_region *reg_region, u32 hwirq, u32 enable);
    int (*set_int_type)(struct io_region *reg_region, u32 type, u32 hwirq);
    int (*get_int_status)(struct io_region *reg_region);
    void (*clear_int)(struct io_region *reg_region, u32 offset);
    void (*save_regs)(struct io_region *reg_region, struct context_save_regs *ctx);
    void (*restore_regs)(struct io_region *reg_region, struct context_save_regs *ctx);
    void (*enable_comb_int)(struct io_region *reg_region, u32 enable);
};

struct hisi_gpio {
    struct device *dev;
    struct io_region reg_region;
    struct gpio_chip chip;
    struct irq_chip irq_chip;
    struct irq_domain *domain;
    int irq;
    u32 host_id;
    spinlock_t lock;
    struct reset_control *rst;
    struct clk *clk;
    const struct hisi_gpio_ops *gpio_ops;
    struct context_save_regs ctx;
};
const struct hisi_gpio_ops *hisi_gpio_get_ops(void);
#endif
