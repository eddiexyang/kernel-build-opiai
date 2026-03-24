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
 * Create: 2022-12-05
 */
#ifndef __SPI_MISC_H__
#define __SPI_MISC_H__

#include <linux/device.h>
#include "hispi.h"

int hisi_spi_get_clk_rst_info(struct hisi_spi *hispi, struct device *dev);
int hisi_subctrl_spi_reset_assert(struct device *spi_dev, u32 bus);
int hisi_subctrl_spi_reset_deassert(struct device *spi_dev, u32 bus);
int hisi_subctrl_spi_get_reset_status(struct device *spi_dev, u32 bus);
int hisi_subctrl_spi_clkgate_enable(struct device *spi_dev, u32 bus);
int hisi_subctrl_spi_clkgate_disable(struct device *spi_dev, u32 bus);

#endif
