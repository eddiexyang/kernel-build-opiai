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
 * Description: spi misc 310 source file
 * Author: huawei
 * Create: 2023-10-12
 */

#include <linux/io.h>
#include "hispi.h"
#include "spi_misc_310.h"

const u64 g_spi_subctrl_pbase[SUB_CTRL_MAX] = {
    [PERI_SUB_CTRL] = 0x80130000,
    [AO_HISS_SUB_CTRL] = 0x110100000,
    [ISP_SUB_CTRL] = 0x300130000,
    [AO_SUB_CTRL] = 0xc0120000,
    [MEDIA_SUB_CTRL] = 0x400130000,
};

struct spi_crg_reset_cell g_spi_subctrl_resets[SPI_MAX_NUM] = {
    { PERI_SPI_RESET_REQ_REG, PERI_SPI_RESET_DREQ_REG, PERI_SPI_RESET_ST_REG, 0x1},
    { PERI_SPI_RESET_REQ_REG, PERI_SPI_RESET_DREQ_REG, PERI_SPI_RESET_ST_REG, 0x2},
    { AO_HISS_SPI_RESET_REQ_REG, AO_HISS_SPI_RESET_DREQ_REG, AO_HISS_SPI_RESET_ST_REG, 0x1},
    { ISP_SPI_RESET_REQ_REG, ISP_SPI_RESET_DREQ_REG, ISP_SPI_RESET_ST_REG, 0x1},
    { ISP_SPI_RESET_REQ_REG, ISP_SPI_RESET_DREQ_REG, ISP_SPI_RESET_ST_REG, 0x2},
    { AO_SPI_RESET_REQ_REG, AO_SPI_RESET_DREQ_REG, AO_SPI_RESET_ST_REG, 0x1},
    { PERI_SPI_RESET_REQ_REG, PERI_SPI_RESET_DREQ_REG, PERI_SPI_RESET_ST_REG, 0x4},
    { MEDIA_SPI_RESET_REQ_REG, MEDIA_SPI_RESET_DREQ_REG, MEDIA_SPI_RESET_ST_REG, 0x1},
    { MEDIA_SPI_RESET_REQ_REG, MEDIA_SPI_RESET_DREQ_REG, MEDIA_SPI_RESET_ST_REG, 0x2},
};

struct spi_crg_clkgate g_spi_subctrl_clkgates[SPI_MAX_NUM] = {
    { PERI_SPI_ICG_EN_REG, PERI_SPI_ICG_DIS_REG, PERI_SPI_ICG_ST_REG, 0x1},
    { PERI_SPI_ICG_EN_REG, PERI_SPI_ICG_DIS_REG, PERI_SPI_ICG_ST_REG, 0x2},
    { AO_HISS_SPI_ICG_EN_REG, AO_HISS_SPI_ICG_DIS_REG, AO_HISS_SPI_ICG_ST_REG, 0x1},
    { ISP_SPI_ICG_EN_REG, ISP_SPI_ICG_DIS_REG, ISP_SPI_ICG_ST_REG, 0x1},
    { ISP_SPI_ICG_EN_REG, ISP_SPI_ICG_DIS_REG, ISP_SPI_ICG_ST_REG, 0x2},
    { AO_SPI_ICG_EN_REG, AO_SPI_ICG_DIS_REG, AO_SPI_ICG_ST_REG, 0x1},
    { PERI_SPI_ICG_EN_REG, PERI_SPI_ICG_DIS_REG, PERI_SPI_ICG_ST_REG, 0x4},
    { MEDIA_SPI_ICG_EN_REG, MEDIA_SPI_ICG_DIS_REG, MEDIA_SPI_ICG_ST_REG, 0x1},
    { MEDIA_SPI_ICG_EN_REG, MEDIA_SPI_ICG_DIS_REG, MEDIA_SPI_ICG_ST_REG, 0x2},
};

int bus_num_to_subctrl_index(u32 bus_num)
{
    int index;
    switch (bus_num) {
        case PERI_SPI0:
        case PERI_SPI1:
        case PERI_SPI6:
            index = PERI_SUB_CTRL;
            break;
        case AO_HISS_SPI2:
            index = AO_HISS_SUB_CTRL;
            break;
        case ISP_SPI3:
        case ISP_SPI4:
            index = ISP_SUB_CTRL;
            break;
        case AO_SPI5:
            index = AO_SUB_CTRL;
            break;
        case MEDIA_SPI7:
        case MEDIA_SPI8:
            index = MEDIA_SUB_CTRL;
            break;
        default:
            index = -1;
    }
    return index;
}