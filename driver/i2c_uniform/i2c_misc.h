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
 * Description: i2c_misc header file
 * Author: huawei
 * Create: 2022-12-06
 */
#ifndef __I2C_MISC_H_
#define __I2C_MISC_H_

/**
 * This file is used to declare the top abstract interface on which IP integration depends.
 */
#include <linux/device.h>

int hisi_i2c_get_clk_rst_info(struct hisi_i2c_controller *hi2c);
int hisi_subctrl_i2c_reset_assert(struct device *i2c_dev, u32 bus_id);
int hisi_subctrl_i2c_reset_deassert(struct device *i2c_dev, u32 bus_id);
int hisi_subctrl_i2c_get_reset_status(struct device *i2c_dev, u32 bus_id);
int hisi_subctrl_i2c_clkgate_enable(struct device *i2c_dev, u32 bus_id);
int hisi_subctrl_i2c_clkgate_disable(struct device *i2c_dev, u32 bus_id);
bool hisi_subctrl_i2c_clkgate_is_enabled(struct device *i2c_dev, u32 bus_id);
int hisi_subctrl_i2c_recovery(struct device *i2c_dev, u32 bus_id);
#ifdef CONFIG_DEBUG_UDRV
int hisi_i2c_subctrl_init(u32 bus_id, struct device *dev);
#endif
#endif