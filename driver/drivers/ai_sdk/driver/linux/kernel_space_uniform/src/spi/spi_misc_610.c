/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * Description: spi misc 610 source file
 * Author: huawei
 * Create: 2023-10-12
 */

#include <linux/io.h>
#include "hispi.h"
#include "spi_misc_610.h"

const u64 g_spi_subctrl_pbase[SUB_CTRL_MAX] = {
    [PERI_SUB_CTRL] = 0x0a40c0000,
    [AO_HISS_SUB_CTRL] = 0x110100000,
    [ISP_SUB_CTRL] = 0xa80c0000,
    [AO_SUB_CTRL] = 0xc0120000,
    [MEDIA_SUB_CTRL] = 0xfc0c0000,
};

struct spi_crg_reset_cell g_spi_subctrl_resets[SPI_MAX_NUM] = {
    { MEDIA_SPI0_RESET_REQ_REG, MEDIA_SPI0_RESET_DREQ_REG, MEDIA_SPI0_RESET_ST_REG, 0x1},
    { PERI_SPI1_RESET_REQ_REG, PERI_SPI1_RESET_DREQ_REG, PERI_SPI1_RESET_ST_REG, 0x3},
    { MEDIA_SPI2_RESET_REQ_REG, MEDIA_SPI2_RESET_DREQ_REG, MEDIA_SPI2_RESET_ST_REG, 0x1},
    { PERI_SPI3_RESET_REQ_REG, PERI_SPI3_RESET_DREQ_REG, PERI_SPI3_RESET_ST_REG, 0x3},
};

struct spi_crg_clkgate g_spi_subctrl_clkgates[SPI_MAX_NUM] = {
    { MEDIA_SPI0_ICG_EN_REG, MEDIA_SPI0_ICG_DIS_REG, MEDIA_SPI0_ICG_ST_REG, 0x1},
    { PERI_SPI1_ICG_EN_REG, PERI_SPI1_ICG_DIS_REG, PERI_SPI1_ICG_ST_REG, 0x2},
    { MEDIA_SPI2_ICG_EN_REG, MEDIA_SPI2_ICG_DIS_REG, MEDIA_SPI2_ICG_ST_REG, 0x1},
    { PERI_SPI3_ICG_EN_REG, PERI_SPI3_ICG_DIS_REG, PERI_SPI3_ICG_ST_REG, 0x1},
};

struct spi_num_subctrl_cell g_spi_num_subctrl[SPI_MAX_NUM] = {
    { MEDIA_SPI0,  MEDIA_SUB_CTRL },
    { PERI_SPI1,   PERI_SUB_CTRL  },
    { MEDIA_SPI2,  MEDIA_SUB_CTRL },
    { PERI_SPI3,   PERI_SUB_CTRL  },
};

int bus_num_to_subctrl_index(u32 bus_num)
{
    if (bus_num >= SPI_MAX_NUM) {
        return -1;
    }

    return g_spi_num_subctrl[bus_num].subctrl_id;
}