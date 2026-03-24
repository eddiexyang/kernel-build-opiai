/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2023. All rights reserved.
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
 * Description: SPI common header file of the kernel driver and user-mode driver.
 * Author: huawei
 * Create: 2020-07-27
 */
#ifndef HISPI_H
#define HISPI_H

#include <linux/spi/spi.h>
#include "kdrv_spi_api.h"
#ifdef __cplusplus
extern "C" {
#endif

#define WIDTH_32_BITS 32U
#define WIDTH_16_BITS 16U
#define WIDTH_8_BITS 8U
#define CLK_FRE_DEFAULT 200000000
#define SPEED_HZ_MAX 25000000
#define SPEED_HZ_MIN 4000

enum hispi_rx_threshold {
    HISPI_RX_1,
    HISPI_RX_2,
    HISPI_RX_3,
    HISPI_RX_4,
    HISPI_RX_5,
    HISPI_RX_9,
    HISPI_RX_7,
    HISPI_RX_8
};

enum hispi_tx_threshold {
    HISPI_TX_1,
    HISPI_TX_4,
    HISPI_TX_8,
    HISPI_TX_16,
    HISPI_TX_32,
    HISPI_TX_64,
    HISPI_TX_128
};

struct hisi_trans_info {
    const void *tx_buf;
    void *rx_buf;
    u32 tx_cnt;
    u32 rx_cnt;
    u32 max_speed_hz;
    u32 mode;
    u8 is_lsb_first;
    u8 bits_per_word;
    u8 chip_select;
};

struct region {
    void __iomem *io_base;
    u32 io_size;
};

struct hisi_spi {
    struct spi_controller *master;
    struct device *dev;
    struct region reg;
    const struct hisi_spi_ops *soc_ops;
    struct reset_control *rst;
    struct clk *clk;
    struct hisi_trans_info info;
    int irq;
    int num_chipselect;
    u32	bus_num;
    u32 fifo_depth;
    u32 clk_freq;
    u32 rx_fifo_level;
};

struct master_find_data {
    uint32_t spi_no;
    struct hisi_spi *hispi;
};

struct hisi_spi_ops {
    int (*setup)(struct region *reg, struct hisi_trans_info *info, u32 clk_freq, u32 rx_fifo_level);
    int (*cleanup)(struct region *reg);
    void (*enable)(struct region *reg);
    void (*disable)(struct region *reg);
    void (*enable_int)(struct region *reg);
    void (*disable_int)(struct region *reg);
    void (*write_tx)(struct region *reg, u32 data);
    int (*read_rx)(struct region *reg);
    void (*get_current_config)(struct region *reg, u32 clk_freq, struct kdrv_spi_master_config *config);
    void (*get_trans_status)(struct region *reg, struct kdrv_spi_trans_status *status);
    void (*get_trans_irq)(struct region *reg, struct kdrv_spi_trans_irq *status);
    void (*get_err_irq)(struct region *reg, struct kdrv_spi_err_irq *status);
    void (*get_fifo_info)(struct region *reg, struct kdrv_spi_fifo *fifo);
    bool (*rx_not_empty)(struct region *reg);
    bool (*tx_not_full)(struct region *reg);
    void (*flush_rx_fifo)(struct region *reg);
    void (*flush_tx_fifo)(struct region *reg);
    void (*clear_rxoi)(struct region *reg);
    unsigned int (*get_fifo_depth)(void);
};

int hisi_spi_reset(struct hisi_spi *hispi);
const struct hisi_spi_ops *spi_get_ops(void);
struct hisi_spi *hisi_spi_get_by_spi_no(uint32_t spi_no);

#ifdef CONFIG_SPI_UDRV
int spi_crg_init(u32 bus_num);
#endif

#ifdef __cplusplus
}
#endif
#endif
