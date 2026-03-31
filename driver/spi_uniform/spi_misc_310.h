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
 * Description: spi_misc_310 header file
 * Author: huawei
 * Create: 2023-10-12
 */
#ifndef __SPI_MISC_310_H_
#define __SPI_MISC_310_H_

enum {
    PERI_SPI0,
    PERI_SPI1,
    AO_HISS_SPI2,
    ISP_SPI3,
    ISP_SPI4,
    AO_SPI5,
    PERI_SPI6,
    MEDIA_SPI7,
    MEDIA_SPI8,
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

#define PERI_SPI_RESET_REQ_REG               (0xa10)  /* Soft reset request control register */
#define PERI_SPI_RESET_DREQ_REG              (0xa14)  /* Soft reset derequest control register */
#define PERI_SPI_RESET_ST_REG                (0x5a10)  /* Soft reset status register */

#define AO_HISS_SPI_RESET_REQ_REG               (0xa60)  /* Soft reset request control register */
#define AO_HISS_SPI_RESET_DREQ_REG              (0xa64)  /* Soft reset derequest control register */
#define AO_HISS_SPI_RESET_ST_REG                (0x5a60)  /* Soft reset status register */

#define ISP_SPI_RESET_REQ_REG               (0xa18)  /* Soft reset request control register */
#define ISP_SPI_RESET_DREQ_REG              (0xa1c)  /* Soft reset derequest control register */
#define ISP_SPI_RESET_ST_REG                (0x5a18)  /* Soft reset status register */

#define AO_SPI_RESET_REQ_REG               (0xa20)  /* Soft reset request control register */
#define AO_SPI_RESET_DREQ_REG              (0xa24)  /* Soft reset derequest control register */
#define AO_SPI_RESET_ST_REG                (0x5a20)  /* Soft reset status register */

#define MEDIA_SPI_RESET_REQ_REG               (0xc20)  /* Soft reset request control register */
#define MEDIA_SPI_RESET_DREQ_REG              (0xc24)  /* Soft reset derequest control register */
#define MEDIA_SPI_RESET_ST_REG                (0x5c20)  /* Soft reset status register */

#define PERI_SPI_ICG_EN_REG               (0x318)  /* Clock enable control register */
#define PERI_SPI_ICG_DIS_REG              (0x31C)  /* Clock disable control register */
#define PERI_SPI_ICG_ST_REG               (0x5318)  /* Clock enable status register */

#define AO_HISS_SPI_ICG_EN_REG               (0x3B0)
#define AO_HISS_SPI_ICG_DIS_REG              (0x3B4)
#define AO_HISS_SPI_ICG_ST_REG               (0x53B0)

#define ISP_SPI_ICG_EN_REG               (0x320)
#define ISP_SPI_ICG_DIS_REG              (0x324)
#define ISP_SPI_ICG_ST_REG               (0x5320)

#define AO_SPI_ICG_EN_REG               (0x328)
#define AO_SPI_ICG_DIS_REG              (0x32c)
#define AO_SPI_ICG_ST_REG               (0x5328)

#define MEDIA_SPI_ICG_EN_REG               (0x918)
#define MEDIA_SPI_ICG_DIS_REG              (0x91c)
#define MEDIA_SPI_ICG_ST_REG               (0x5918)

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

extern const u64 g_spi_subctrl_pbase[SUB_CTRL_MAX];
extern struct spi_crg_reset_cell g_spi_subctrl_resets[SPI_MAX_NUM];
extern struct spi_crg_clkgate g_spi_subctrl_clkgates[SPI_MAX_NUM];
int bus_num_to_subctrl_index(u32 bus_num);

#endif