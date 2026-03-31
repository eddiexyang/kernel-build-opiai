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
 * Description: spi_misc_610 header file
 * Author: huawei
 * Create: 2023-10-12
 */
#ifndef __SPI_MISC_610_H_
#define __SPI_MISC_610_H_

enum {
    MEDIA_SPI0,
    PERI_SPI1,
    MEDIA_SPI2,
    PERI_SPI3,
    SPI_MAX_NUM
};

enum {
    PERI_SUB_CTRL,
    AO_HISS_SUB_CTRL,
    ISP_SUB_CTRL,
    AO_SUB_CTRL,
    MEDIA_SUB_CTRL,
    SUB_CTRL_MAX
};

#define MEDIA_SPI0_RESET_REQ_REG            (0xeb8)  /* Soft reset request control register */
#define MEDIA_SPI0_RESET_DREQ_REG           (0xebc)  /* Soft reset derequest control register */
#define MEDIA_SPI0_RESET_ST_REG             (0x5eb8)  /* Soft reset status register */
#define MEDIA_SPI0_ICG_EN_REG               (0x3e0)  /* Clock enable control register */
#define MEDIA_SPI0_ICG_DIS_REG              (0x3e4) /* Clock disable control register */
#define MEDIA_SPI0_ICG_ST_REG               (0x53e0)  /* Clock enable status register */

#define PERI_SPI1_RESET_REQ_REG             (0xa68)  /* Soft reset request control register */
#define PERI_SPI1_RESET_DREQ_REG            (0xa6c)  /* Soft reset derequest control register */
#define PERI_SPI1_RESET_ST_REG              (0x5a68)  /* Soft reset status register */
#define PERI_SPI1_ICG_EN_REG                (0x3b8)  /* Clock enable control register */
#define PERI_SPI1_ICG_DIS_REG               (0x3bc)  /* Clock disable control register */
#define PERI_SPI1_ICG_ST_REG                (0x53b8)  /* Clock enable status register */

#define MEDIA_SPI2_RESET_REQ_REG            (0xeb0)  /* Soft reset request control register */
#define MEDIA_SPI2_RESET_DREQ_REG           (0xeb4)  /* Soft reset derequest control register */
#define MEDIA_SPI2_RESET_ST_REG             (0x5eb0)  /* Soft reset status register */
#define MEDIA_SPI2_ICG_EN_REG               (0x3d8)  /* Clock enable control register */
#define MEDIA_SPI2_ICG_DIS_REG              (0x3dc)  /* Clock disable control register */
#define MEDIA_SPI2_ICG_ST_REG               (0x53d8)  /* Clock enable status register */

#define PERI_SPI3_RESET_REQ_REG             (0xa60)  /* Soft reset request control register */
#define PERI_SPI3_RESET_DREQ_REG            (0xa64)  /* Soft reset derequest control register */
#define PERI_SPI3_RESET_ST_REG              (0x5a60)  /* Soft reset status register */
#define PERI_SPI3_ICG_EN_REG                (0x3b0)  /* Clock enable control register */
#define PERI_SPI3_ICG_DIS_REG               (0x3b4)  /* Clock disable control register */
#define PERI_SPI3_ICG_ST_REG                (0x53b0)  /* Clock enable status register */

struct spi_crg_reset_cell {
    unsigned int reset_req_offset;
    unsigned int reset_dreq_offset;
    unsigned int reset_st_offset;
    unsigned int reset_mask;
};

struct spi_crg_clkgate {
    unsigned int en_offset;
    unsigned int dis_offset;
    unsigned int st_offset;
    unsigned int mask;
};

struct spi_num_subctrl_cell {
    int spi_num;
    int subctrl_id;
};

extern const u64 g_spi_subctrl_pbase[SUB_CTRL_MAX];
extern struct spi_crg_reset_cell g_spi_subctrl_resets[SPI_MAX_NUM];
extern struct spi_crg_clkgate g_spi_subctrl_clkgates[SPI_MAX_NUM];
int bus_num_to_subctrl_index(u32 bus_num);

#endif