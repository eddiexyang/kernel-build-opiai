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
 * Create: 2022-12-07
 */
#ifndef __GPIO_MISC_H__
#define __GPIO_MISC_H__
/**
 * This file is used to declare the top abstract interface on which IP integration depends.
 */

#include <linux/device.h>

int hisi_subctrl_gpio_reset_assert(struct device *dev, u32 host_id);
int hisi_subctrl_gpio_reset_deassert(struct device *dev, u32 host_id);
int hisi_subctrl_gpio_get_reset_status(struct device *dev, u32 host_id);
int hisi_subctrl_gpio_clkgate_enable(struct device *dev, u32 host_id);
int hisi_subctrl_gpio_clkgate_disable(struct device *dev, u32 host_id);
bool hisi_subctrl_gpio_clkgate_is_enabled(struct device *dev, u32 host_id);
int hisi_gpio_get_clk_rst_info(struct hisi_gpio *hgpio);
#ifdef CONFIG_GPIO_UDRV
int hisi_gpio_crg_init(u32 host_id, struct device *dev);
#endif
#endif
