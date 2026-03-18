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
 * Description: GPIO driver logic layer source file.
 * Author: huawei
 * Create: 2022-04-25
 */
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include "hisi_gpio.h"
#include "gpio_misc.h"

#define HISI_GPIO_NAME "hgpio"
#define to_hisi_gpio(x) container_of(x, struct hisi_gpio, chip)

static void hisi_gpio_irq_mask(struct irq_data *d)
{
    struct hisi_gpio *hgpio = (struct hisi_gpio *)irq_data_get_irq_chip_data(d);
    unsigned long flags;

    spin_lock_irqsave(&hgpio->lock, flags);
    hgpio->gpio_ops->mask_int(&hgpio->reg_region, d->hwirq);
    spin_unlock_irqrestore(&hgpio->lock, flags);
}

static void hisi_gpio_irq_unmask(struct irq_data *d)
{
    struct hisi_gpio *hgpio = (struct hisi_gpio *)irq_data_get_irq_chip_data(d);
    unsigned long flags;

    spin_lock_irqsave(&hgpio->lock, flags);
    hgpio->gpio_ops->unmask_int(&hgpio->reg_region, d->hwirq);
    spin_unlock_irqrestore(&hgpio->lock, flags);
}

static void hisi_gpio_irq_enable(struct irq_data *d)
{
    struct hisi_gpio *hgpio = (struct hisi_gpio *)irq_data_get_irq_chip_data(d);
    unsigned long flags;

    spin_lock_irqsave(&hgpio->lock, flags);
    hgpio->gpio_ops->enable_int(&hgpio->reg_region, d->hwirq, 1);
    spin_unlock_irqrestore(&hgpio->lock, flags);
}

static void hisi_gpio_irq_disable(struct irq_data *d)
{
    struct hisi_gpio *hgpio = (struct hisi_gpio *)irq_data_get_irq_chip_data(d);
    unsigned long flags;

    spin_lock_irqsave(&hgpio->lock, flags);
    hgpio->gpio_ops->enable_int(&hgpio->reg_region, d->hwirq, 0);
    spin_unlock_irqrestore(&hgpio->lock, flags);
}

static int hisi_gpio_irq_set_type(struct irq_data *d, unsigned int type)
{
    struct hisi_gpio *hgpio = (struct hisi_gpio *)irq_data_get_irq_chip_data(d);
    unsigned long flags;
    int ret;

    pr_debug("hisi_gpio_irq_set_type: hwirq = %lu, type = %u\n", d->hwirq, type);
    spin_lock_irqsave(&hgpio->lock, flags);
    ret = hgpio->gpio_ops->set_int_type(&hgpio->reg_region, d->hwirq, type);
    spin_unlock_irqrestore(&hgpio->lock, flags);

    if (ret != 0) {
        pr_err("irq: unsupported type %d\n", type);
    }
    return 0;
}

static int hisi_gpio_irq_set_wake(struct irq_data *d, unsigned int enable)
{
    struct hisi_gpio *hgpio = irq_data_get_irq_chip_data(d);
    struct context_save_regs *ctx = &hgpio->ctx;
    irq_hw_number_t bit = irqd_to_hwirq(d);

    if (enable != 0) {
        ctx->wake_en |= BIT(bit);
    } else {
        ctx->wake_en &= ~BIT(bit);
    }
    return 0;
}

static int hisi_gpio_to_irq(struct gpio_chip *chip, unsigned int offset)
{
    struct hisi_gpio *hgpio = to_hisi_gpio(chip);
    int irq;

    irq = irq_create_mapping(hgpio->domain, offset);
    return irq;
}

static irqreturn_t hisi_gpio_irq_handler(int irq, void *arg)
{
    struct hisi_gpio *hgpio = (struct hisi_gpio *)arg;
    int i = 0;
    unsigned long irq_status;
    unsigned long flags;

    irq_status = hgpio->gpio_ops->get_int_status(&hgpio->reg_region);
    pr_debug("hisi_gpio_irq_handler: irq_status [%lx]\n", irq_status);
    if (irq_status == 0) {
        return IRQ_NONE;
    }

    for_each_set_bit(i, &irq_status, hgpio->chip.ngpio)
    {
        pr_debug("hisi_gpio_irq_handler: i = %d\n", i);
        spin_lock_irqsave(&hgpio->lock, flags);
        hgpio->gpio_ops->clear_int(&hgpio->reg_region, i);
        spin_unlock_irqrestore(&hgpio->lock, flags);
        /* desc->handle_irq, i.e. hooked handle_simple_irq --->action->handler
         --->Finally, the processing function mounted to each GPIO and request_irq is called. */
        (void)generic_handle_irq(irq_find_mapping(hgpio->domain, i));
    }
    return IRQ_HANDLED;
}

static int hisi_gpio_irq_domain_map(struct irq_domain *domain, unsigned int irq, irq_hw_number_t hwirq)
{
    struct hisi_gpio *hgpio = domain->host_data;

    pr_debug("hisi_gpio_irq_domain_map: hw irq = %d, irq = %d\n", (int)hwirq, irq);
    irq_set_chip_and_handler(irq, &hgpio->irq_chip, handle_simple_irq); // set desc->handle_irq function
    // set irq_data.chip_data = data, use irq_data_get_irq_chip_data to get chip_data
    irq_set_chip_data(irq, domain->host_data);

    irq_clear_status_flags(irq, IRQ_NOREQUEST);
    return 0;
}

static const struct irq_domain_ops hisi_gpio_irq_simple_ops = {
    .map = hisi_gpio_irq_domain_map,
};

static int hisi_gpio_config_irq(struct hisi_gpio *hgpio)
{
    int ret;

    hgpio->domain = irq_domain_add_simple(hgpio->dev->of_node, hgpio->chip.ngpio, 0, &hisi_gpio_irq_simple_ops, hgpio);
    if (!hgpio->domain) {
        pr_err("failed to request irq domain\n");
        return -ENODEV;
    }
    ret = devm_request_irq(hgpio->dev, hgpio->irq, hisi_gpio_irq_handler, IRQF_SHARED, "gpio", hgpio);
    if (ret != 0) {
        pr_err("failed to request gpio irq\n");
        return ret;
    }
    if (hgpio->gpio_ops->enable_comb_int != NULL) {
        hgpio->gpio_ops->enable_comb_int(&hgpio->reg_region, 1);
    }
    return 0;
}

static int hisi_gpio_request(struct gpio_chip *chip, unsigned int offset)
{
    if (offset >= chip->ngpio) {
        return -EINVAL;
    }
    return 0;
}

/* 1:input */
static int hisi_gpio_direction_in(struct gpio_chip *chip, unsigned int offset)
{
    struct hisi_gpio *hgpio = to_hisi_gpio(chip);
    unsigned long flags;

    spin_lock_irqsave(&hgpio->lock, flags);
    hgpio->gpio_ops->set_direction_in(&hgpio->reg_region, offset);
    spin_unlock_irqrestore(&hgpio->lock, flags);
    return 0;
}

static int hisi_gpio_direction_out(struct gpio_chip *chip, unsigned int offset, int value)
{
    struct hisi_gpio *hgpio = to_hisi_gpio(chip);
    unsigned long flags;

    spin_lock_irqsave(&hgpio->lock, flags);
    hgpio->gpio_ops->set_val(&hgpio->reg_region, offset, value);
    hgpio->gpio_ops->set_direction_out(&hgpio->reg_region, offset);
    spin_unlock_irqrestore(&hgpio->lock, flags);
    return 0;
}
/* return 0:output  1:input */
static int hisi_gpio_get_direction(struct gpio_chip *chip, unsigned int offset)
{
    struct hisi_gpio *hgpio = to_hisi_gpio(chip);
    unsigned long flags;
    enum gpio_dir_type dir;

    spin_lock_irqsave(&hgpio->lock, flags);
    hgpio->gpio_ops->get_direction(&hgpio->reg_region, offset, &dir);
    spin_unlock_irqrestore(&hgpio->lock, flags);
    return dir;
}

static int hisi_gpio_get(struct gpio_chip *chip, unsigned int offset)
{
    struct hisi_gpio *hgpio = to_hisi_gpio(chip);
    unsigned long flags;
    int ret;

    spin_lock_irqsave(&hgpio->lock, flags);
    ret = hgpio->gpio_ops->get_val(&hgpio->reg_region, offset);
    spin_unlock_irqrestore(&hgpio->lock, flags);
    return ret;
}

static void hisi_gpio_set(struct gpio_chip *chip, unsigned int offset, int value)
{
    struct hisi_gpio *hgpio = to_hisi_gpio(chip);
    unsigned long flags;

    spin_lock_irqsave(&hgpio->lock, flags);
    hgpio->gpio_ops->set_val(&hgpio->reg_region, offset, value);
    spin_unlock_irqrestore(&hgpio->lock, flags);
}

static int hisi_gpio_set_debounce(struct gpio_chip *chip, unsigned int offset, unsigned int debounce)
{
    struct hisi_gpio *hgpio = to_hisi_gpio(chip);
    unsigned long flags;

    spin_lock_irqsave(&hgpio->lock, flags);
    hgpio->gpio_ops->set_debounce(&hgpio->reg_region, offset, debounce);
    spin_unlock_irqrestore(&hgpio->lock, flags);
    return 0;
}

static int hisi_gpio_set_config(struct gpio_chip *chip, unsigned int offset, unsigned long config)
{
    switch (pinconf_to_config_param(config)) {
        case PIN_CONFIG_INPUT_DEBOUNCE:
            return hisi_gpio_set_debounce(chip, offset, pinconf_to_config_argument(config));
        default:
            break;
    }

    return -ENOTSUPP;
}

static int hisi_gpio_parse_resource(struct platform_device *pdev, struct hisi_gpio *hgpio)
{
    struct resource *res;
    u32 ngpios;
    u32 base;
    int ret;

    /* Obtains the register address space. */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    hgpio->reg_region.io_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(hgpio->reg_region.io_base)) {
        dev_err(&pdev->dev, "ioremap(0x%llx, 0x%llx) fail\n", res->start, res->end);
        return PTR_ERR(hgpio->reg_region.io_base);
    }
    hgpio->reg_region.io_size = res->end - res->start + 1;

    ret = device_property_read_u32(&pdev->dev, "base", &base);
    if (ret != 0) {
        dev_err(&pdev->dev, "get gpio base error\n");
        return -EINVAL;
    }
    hgpio->chip.base = base;
    // ngpios:number of GPIOs per controller
    ret = device_property_read_u32(&pdev->dev, "ngpios", &ngpios);
    if (ret != 0) {
        dev_err(&pdev->dev, "failed to get number of gpios\n");
        return -EINVAL;
    }
    ret = device_property_read_u32(&pdev->dev, "host_id", &hgpio->host_id);
    if (ret != 0) {
        dev_err(&pdev->dev, "failed to get gpio host_id, ret = %d\n", ret);
        return ret;
    }
    hgpio->chip.ngpio = ngpios;
    /* Obtaining the Interrupt ID of a Device */
    hgpio->irq = platform_get_irq(pdev, 0);
    if (hgpio->irq < 0) {
        dev_info(&pdev->dev, "failed to get gpio irq\n");
        return -EINVAL;
    }
    return 0;
}

static void hisi_gpio_fill_gpio_chip(struct gpio_chip *gpio_chip, struct hisi_gpio *hgpio)
{
    gpio_chip->label = HISI_GPIO_NAME;
    gpio_chip->owner = THIS_MODULE;
    gpio_chip->request = hisi_gpio_request;
    gpio_chip->direction_input = hisi_gpio_direction_in;
    gpio_chip->direction_output = hisi_gpio_direction_out;
    gpio_chip->get_direction = hisi_gpio_get_direction;
    gpio_chip->get = hisi_gpio_get;
    gpio_chip->set = hisi_gpio_set;
    gpio_chip->to_irq = hisi_gpio_to_irq;
    gpio_chip->set_config = hisi_gpio_set_config;
    gpio_chip->parent = hgpio->dev;
}

static void hisi_gpio_fill_irq_chip(struct irq_chip *irq_chip, struct hisi_gpio *hgpio)
{
    irq_chip->name = HISI_GPIO_NAME;
    irq_chip->irq_mask = hisi_gpio_irq_mask;
    irq_chip->irq_unmask = hisi_gpio_irq_unmask;
    irq_chip->irq_set_type = hisi_gpio_irq_set_type;
    irq_chip->irq_enable = hisi_gpio_irq_enable;
    irq_chip->irq_disable = hisi_gpio_irq_disable;
    irq_chip->irq_set_wake = hisi_gpio_irq_set_wake;
}

int hisi_gpio_reset(struct device *dev, u32 host_id)
{
    int ret;
    bool enabled;

    /* 1.Turn off the clock */
    ret = hisi_subctrl_gpio_clkgate_disable(dev, host_id);
    if (ret != 0) {
        dev_err(dev, "hisi_subctrl_gpio_clkgate_disable failed, ret=%d\n", ret);
        return ret;
    }

    /* 2.Reset */
    ret = hisi_subctrl_gpio_reset_assert(dev, host_id);
    if (ret != 0) {
        dev_err(dev, "gpio reset failed, ret=%d\n", ret);
        return ret;
    }
    /* 3.Turn on the clock */
    ret = hisi_subctrl_gpio_clkgate_enable(dev, host_id);
    if (ret != 0) {
        dev_err(dev, "gpio icg enable failed, ret=%d\n", ret);
        return ret;
    }
    /* 4.Wait 100us */
    udelay(100);
    /* 5.Turn off the clock */
    ret = hisi_subctrl_gpio_clkgate_disable(dev, host_id);
    if (ret != 0) {
        dev_err(dev, "hisi_subctrl_gpio_clkgate_disable failed, ret=%d\n", ret);
        return ret;
    }
    /* 6.Deassert reset */
    ret = hisi_subctrl_gpio_reset_deassert(dev, host_id);
    if (ret != 0) {
        dev_err(dev, "gpio dereset failed, ret=%d\n", ret);
        return ret;
    }
    /* 7.Obtains the reset deassertion status. */
    ret = hisi_subctrl_gpio_get_reset_status(dev, host_id);
    if (ret != 0) {
        dev_err(dev, "dev is not deassert status\n");
        return ret;
    }
    /* 8.Turn on the clock */
    ret = hisi_subctrl_gpio_clkgate_enable(dev, host_id);
    if (ret != 0) {
        dev_err(dev, "gpio icg enable failed, ret=%d\n", ret);
        return ret;
    }
    enabled = hisi_subctrl_gpio_clkgate_is_enabled(dev, host_id);
    if (!enabled) {
        dev_err(dev, "hisi_subctrl_gpio_clkgate_is_enabled fail\n");
        return -1;
    }
    return 0;
}

static int hisi_gpio_probe(struct platform_device *pdev)
{
    int ret;
    struct hisi_gpio *hgpio;
    struct gpio_chip *gpio_chip;
    struct irq_chip *irq_chip;

    dev_info(&pdev->dev, "hisi_gpio_probe: start\n");
    hgpio = (struct hisi_gpio *)devm_kzalloc(&pdev->dev, sizeof(*hgpio), GFP_KERNEL);
    if (!hgpio) {
        dev_err(&pdev->dev, "kzalloc error\n");
        return -ENOMEM;
    }
    hgpio->gpio_ops = hisi_gpio_get_ops();
    hgpio->dev = &pdev->dev;
    /* Save Device Driver Handle */
    platform_set_drvdata(pdev, hgpio);
    spin_lock_init(&hgpio->lock);

    ret = hisi_gpio_parse_resource(pdev, hgpio);
    if (ret != 0) {
        return ret;
    }
#ifdef CONFIG_GPIO_UDRV
    ret = hisi_gpio_crg_init(hgpio->host_id, hgpio->dev);
    if (ret != 0) {
        return ret;
    }
#endif
    ret = hisi_gpio_get_clk_rst_info(hgpio);
    if (ret != 0) {
        return ret;
    }
    ret = hisi_gpio_reset(hgpio->dev, hgpio->host_id);
    if (ret != 0) {
        return ret;
    }

    irq_chip = &hgpio->irq_chip;
    hisi_gpio_fill_irq_chip(irq_chip, hgpio);
    /* Config Interrupt */
    ret = hisi_gpio_config_irq(hgpio);
    if (ret < 0) {
        return ret;
    }
    gpio_chip = &hgpio->chip;
    hisi_gpio_fill_gpio_chip(gpio_chip, hgpio);
    ret = gpiochip_add(gpio_chip);
    if (ret < 0) {
        dev_err(&pdev->dev, "add gpiochip fail! ret %d\n", ret);
        return ret;
    }
    dev_info(&pdev->dev, "hisi_gpio_probe: end\n");
    return 0;
}

static int hisi_gpio_remove(struct platform_device *pdev)
{
    struct hisi_gpio *hgpio = (struct hisi_gpio *)platform_get_drvdata(pdev);

    if (hgpio->domain != NULL) {
        irq_domain_remove(hgpio->domain);
    }

    gpiochip_remove(&hgpio->chip);
    return 0;
}

static const struct of_device_id hisi_gpio_of_match[] = {
    {
        .compatible = "hisilicon,udrv_gpio",
    },
    {},
};

static int hisi_gpio_suspend(struct device *dev)
{
#ifdef USING_HISI_GPIO_PM
    int ret;
    unsigned long flags;
    struct hisi_gpio *hgpio = dev_get_drvdata(dev);

    dev_info(hgpio->dev, "hisi_gpio_suspend,enter\n");
    spin_lock_irqsave(&hgpio->lock, flags);
    hgpio->gpio_ops->save_regs(&hgpio->reg_region, &hgpio->ctx);
    spin_unlock_irqrestore(&hgpio->lock, flags);
    /* Turn off the clock */
    ret = hisi_subctrl_gpio_clkgate_disable(dev, hgpio->host_id);
    if (ret != 0) {
        dev_err(dev, "hisi_subctrl_gpio_clkgate_disable failed, ret=%d\n", ret);
        return ret;
    }
    /* Reset */
    ret = hisi_subctrl_gpio_reset_assert(dev, hgpio->host_id);
    if (ret != 0) {
        dev_err(dev, "gpio reset failed, ret=%d\n", ret);
        return ret;
    }
#endif
    return 0;
}

static int hisi_gpio_resume(struct device *dev)
{
#ifdef USING_HISI_GPIO_PM
    int ret;
    unsigned long flags;
    struct hisi_gpio *hgpio = dev_get_drvdata(dev);

    dev_info(dev, "enter hisi_gpio_resume\n");

    /* Turn off the clock */
    ret = hisi_subctrl_gpio_clkgate_disable(dev, hgpio->host_id);
    if (ret != 0) {
        dev_err(dev, "hisi_subctrl_gpio_clkgate_disable failed, ret=%d\n", ret);
        return ret;
    }
    /* Deassert reset */
    ret = hisi_subctrl_gpio_reset_deassert(dev, hgpio->host_id);
    if (ret != 0) {
        dev_err(dev, "gpio dereset failed, ret=%d\n", ret);
        return ret;
    }
    /* Turn on the clock */
    ret = hisi_subctrl_gpio_clkgate_enable(dev, hgpio->host_id);
    if (ret != 0) {
        dev_err(dev, "gpio icg enable failed, ret=%d\n", ret);
        return ret;
    }
    spin_lock_irqsave(&hgpio->lock, flags);
    hgpio->gpio_ops->restore_regs(&hgpio->reg_region, &hgpio->ctx);
    if (hgpio->gpio_ops->enable_comb_int != NULL) {
        hgpio->gpio_ops->enable_comb_int(&hgpio->reg_region, 1);
    }
    spin_unlock_irqrestore(&hgpio->lock, flags);
#endif
    return 0;
}
static const struct dev_pm_ops hisi_gpio_pm = {
    .suspend        = hisi_gpio_suspend,
    .resume         = hisi_gpio_resume,
};

static struct platform_driver hisi_gpio_driver = {
    .probe = hisi_gpio_probe,
    .remove = hisi_gpio_remove,
    .driver = {
        .name = HISI_GPIO_NAME,
        .pm = &hisi_gpio_pm,
        .owner = THIS_MODULE,
        .of_match_table = hisi_gpio_of_match,
    },
};

module_platform_driver(hisi_gpio_driver);
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("HiSilicon Gpio Controller Driver");
MODULE_LICENSE("GPL");
