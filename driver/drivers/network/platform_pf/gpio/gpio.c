/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: gpio
 * Author: huawei
 * Create: 2022-08-15
 */

#include <linux/io.h>
#include "gpio.h"

#ifndef DEFINE_HNS_LLT
#define STATIC static
#else
#define STATIC
#endif

STATIC void gpio_reg_write(void __iomem *gpio_base, unsigned int reg, unsigned int index, bool val)
{
    unsigned int gpio_val;
    void __iomem *reg_addr;

    reg_addr = gpio_base + reg;
    gpio_val = readl(reg_addr);
    gpio_val &= ~(1u << index);
    gpio_val |= ((unsigned int)val << index);
    writel(gpio_val, reg_addr);
}

unsigned int gpio_get_present(void __iomem *gpio_base, unsigned int index)
{
    unsigned int present_val;
    unsigned int gpio_val;
    void __iomem *reg_addr;

    /* read gpio data */
    reg_addr = gpio_base + GPIO_SWPORT_DR;
    gpio_val = readl(reg_addr);
    present_val = (gpio_val >> index) & 0x1;
    present_val = ((present_val == 0) ? 1 : 0);

    return present_val;
}

void gpio_present_init(void __iomem *gpio_base, unsigned int index)
{
    /* Configuring the GPIO Mode : Input Mode */
    gpio_reg_write(gpio_base, GPIO_SWPORT_DDR, index, 0);
}

void gpio_set(void __iomem *gpio_base, unsigned int index, bool val)
{
    gpio_reg_write(gpio_base, GPIO_SWPORT_DR, index, val);
}

void gpio_phy_init(void __iomem *gpio_base, unsigned int index)
{
    /* Configuring the GPIO Mode : Output Mode */
    gpio_reg_write(gpio_base, GPIO_SWPORT_DDR, index, 1);
}
