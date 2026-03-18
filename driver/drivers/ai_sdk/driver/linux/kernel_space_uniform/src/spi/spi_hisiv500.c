/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
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
 * Description: SPI file function description.
 * Author: huawei
 * Create: 2021-02-07
 */

#include "hispi.h"

#define SPI_APB_IF_V500_BASE                       (0)

#define SPI_CS_CTRL_REG         (SPI_APB_IF_V500_BASE + 0x000) /* CS control register */
#define SPI_COMMON_CTRL_REG     (SPI_APB_IF_V500_BASE + 0x004) /* General control register */
#define SPI_EN_REG              (SPI_APB_IF_V500_BASE + 0x008) /* SPI enable register */
#define SPI_FIFO_LEVEL_CTRL_REG (SPI_APB_IF_V500_BASE + 0x00C) /* FIFO threshold control register */
#define INTR_SPI_MASK_REG       (SPI_APB_IF_V500_BASE + 0x010) /* Interrupt mask register. 1:Mask interrupt; 0: Not */
#define SPI_DIN_REG             (SPI_APB_IF_V500_BASE + 0x014) /* Input data register */
#define SPI_DOUT_REG            (SPI_APB_IF_V500_BASE + 0x018) /* Output data register */
#define SPI_STATE_REG           (SPI_APB_IF_V500_BASE + 0x01C) /* Status register */
#define INTR_SPI_RAW_REG        (SPI_APB_IF_V500_BASE + 0x020) /* Raw interrupt status register. 1:have; 0:no int */
#define INTR_SPI_REG            (SPI_APB_IF_V500_BASE + 0x024) /* Interrupt status register. 1:have; 0:no int  */
#define INTR_SPI_CLR_REG        (SPI_APB_IF_V500_BASE + 0x028) /* Interrupt clear register */

/* Bit fields in HISI_SPI_IMR, 4 bits */
#define IMR_RXOF        BIT(0)      /* Receive Overflow */
#define IMR_RXTO        BIT(1)      /* Receive Timeout */
#define IMR_RX          BIT(2)      /* Receive */
#define IMR_TX          BIT(3)      /* Transmit */
#define IMR_MASK        (IMR_RXOF | IMR_RXTO | IMR_RX | IMR_TX)

#define DIV_RATIO_PRE_LARGE 200 /* Large frequency divider */
#define DIV_RATIO_PRE_SMALL_8 8 /* Small frequency divider, applicable to total clock: 200 MHz */
#define DIV_RATIO_PRE_SMALL_6 6 /* Small frequency divider, applicable to total clock: 150 MHz */
#define SPEED_HZ_THRESHOLD 100000 /* Frequency division system adjustment threshold */
#define DIV_RATIO_MAX 65024 /* Max frequency divider, (DIV_RATIO_POST_MAX + 1) * DIV_RATIO_PRE_MAX */
#define DIV_RATIO_POST_MAX 0xFF
#define DIV_RATIO_PRE_MAX 0xFE
#define DIV_RATIO_PRE 2 /* div_ratio_pre must be even */
#define TIMEOUT_LIMIT (1<<13)
#define FIFO_DEPTH 256
#define CS_MAX_NUM 1

/* General control register. */
union spi_common_ctrl {
    struct {
        unsigned int rsv_2 : 1; /* [0] */
        unsigned int loop_back : 1; /* [1] */
        unsigned int clk_polarity : 1; /* [2] */
        unsigned int clk_phase : 1; /* [3] */
        unsigned int div_ratio_pre : 8; /* [11:4] */
        unsigned int div_ratio_post : 8; /* [19:12] */
        unsigned int frame_size : 5; /* [24:20] */
        unsigned int hi_spd : 1; /* [25] */
        unsigned int dma_en_tx : 1; /* [26] */
        unsigned int dma_en_rx : 1; /* [27] */
        unsigned int timeout_limit_cnt_sel : 2; /* [29:28] */
        unsigned int rsv : 2; /* [31:30] */
    } bits;
    unsigned int u32;
};

/* FIFO threshold control register. */
union spi_fifo_level_ctrl {
    struct {
        unsigned int dma_busrt_lvl_tx : 3; /* [2:0] */
        unsigned int intr_lvl_tx : 3; /* [5:3] */
        unsigned int dma_busrt_lvl_rx : 3; /* [8:6] */
        unsigned int intr_lvl_rx : 3; /* [11:9] */
        unsigned int rsv : 20; /* [31:12] */
    } bits;
    unsigned int u32;
};

/* Status register. */
union spi_state {
    struct {
        unsigned int tx_fifo_empty : 1; /* [0] */
        unsigned int tx_fifo_not_full : 1; /* [1] */
        unsigned int rx_fifo_not_empty : 1; /* [2] */
        unsigned int rx_fifo_full : 1; /* [3] */
        unsigned int busy : 1; /* [4] */
        unsigned int rsv : 27; /* [31:5] */
    } bits;
    unsigned int u32;
};

/* Interrupt status register. 1:have; 0:no int */
union intr_spi {
    struct {
        unsigned int intr_rx_overflow : 1; /* [0] */
        unsigned int intr_rx_timeout : 1; /* [1] */
        unsigned int intr_rx : 1; /* [2] */
        unsigned int intr_tx : 1; /* [3] */
        unsigned int tf_ecc_multi_err : 1; /* [4] */
        unsigned int tf_ecc_1bit_err : 1; /* [5] */
        unsigned int rf_ecc_multi_err : 1; /* [6] */
        unsigned int rf_ecc_1bit_err : 1; /* [7] */
        unsigned int rsv : 24; /* [31:8] */
    } bits;
    unsigned int u32;
};

// base on formula: ratio = (1 + post) * pre, the ratio is known
static int hispi_calculate_ratio(u32 clk_freq, u32 max_speed_hz, u32 *div_ratio_pre, u32 *div_ratio_post)
{
    u32 ratio;
    u32 pre = 0;
    u32 post = DIV_RATIO_POST_MAX + 1;
    u32 tmp = 0;

    ratio = clk_freq / max_speed_hz;
    tmp = clk_freq % max_speed_hz;
    if ((tmp != 0) || ((ratio & 1) != 0) || (ratio > DIV_RATIO_MAX)) {
        return -EINVAL;
    }

    while (post > DIV_RATIO_POST_MAX) {
        pre += DIV_RATIO_PRE;
        if (pre > DIV_RATIO_PRE_MAX) {
            return -EINVAL;
        }
        tmp = ratio % pre;
        if (tmp != 0) {
            continue;
        }
        post = ratio / pre - 1;
    }
    *div_ratio_pre = pre;
    *div_ratio_post = post;
    return 0;
}

static int hispi_setctrl(struct region *reg, struct hisi_trans_info *info, u32 clk_freq)
{
    int ret;
    union spi_common_ctrl cfg;
    u32 div_ratio_pre;
    u32 div_ratio_post;

    cfg.u32 = 0;
    cfg.bits.frame_size = info->bits_per_word - 1;
    cfg.bits.loop_back = (info->mode & SPI_LOOP) ? 1 : 0;
    cfg.bits.clk_phase = (info->mode & SPI_CPHA) ? 1 : 0;
    cfg.bits.clk_polarity = (info->mode & SPI_CPOL) ? 1 : 0;

    ret = hispi_calculate_ratio(clk_freq, info->max_speed_hz, &div_ratio_pre, &div_ratio_post);
    if (ret) {
        pr_err("max_speed_hz:%u is invalid\n", info->max_speed_hz);
        return ret;
    }

    cfg.bits.div_ratio_pre = div_ratio_pre;
    cfg.bits.div_ratio_post = div_ratio_post;
    iowrite32(cfg.u32, reg->io_base + SPI_COMMON_CTRL_REG);
    return ret;
}

static void hispi_set_fifo_level(struct region *reg, u32 rx_fifo_level)
{
    union spi_fifo_level_ctrl levelc;
    levelc.u32 = ioread32(reg->io_base + SPI_FIFO_LEVEL_CTRL_REG);
    levelc.bits.intr_lvl_rx = rx_fifo_level;
    levelc.bits.intr_lvl_tx = HISPI_TX_64;
    iowrite32(levelc.u32, reg->io_base + SPI_FIFO_LEVEL_CTRL_REG);
}

static void hispi_keep_cs_low(struct region *reg)
{
    iowrite32(0xFFFFFFFF, reg->io_base + SPI_CS_CTRL_REG);
}

static void hispi_change_cs_high(struct region *reg)
{
    iowrite32(0, reg->io_base + SPI_CS_CTRL_REG);
}

static void hispi_enable_int(struct region *reg)
{
    iowrite32(~(u32)IMR_MASK, reg->io_base + INTR_SPI_MASK_REG);
}

static void hispi_enable(struct region *reg)
{
    hispi_keep_cs_low(reg);
    iowrite32(1, reg->io_base + SPI_EN_REG);
}

static void hispi_disable_int(struct region *reg)
{
    iowrite32(0xFF, reg->io_base + INTR_SPI_MASK_REG);
}

static void hispi_disable(struct region *reg)
{
    hispi_change_cs_high(reg);
    iowrite32(0, reg->io_base + SPI_EN_REG);
}

static bool hispi_tx_not_full(struct region *reg)
{
    union spi_state status;
    status.u32 = ioread32(reg->io_base + SPI_STATE_REG);
    return (status.bits.tx_fifo_not_full == 1);
}

static void hispi_write_tx(struct region *reg, u32 data)
{
    iowrite32(data, reg->io_base + SPI_DIN_REG);
}

static int hispi_read_rx(struct region *reg)
{
    int data;
    data = ioread32(reg->io_base + SPI_DOUT_REG);
    return data;
}

static bool hisi_rx_not_empty(struct region *reg)
{
    union spi_state status;
    status.u32 = ioread32(reg->io_base + SPI_STATE_REG);
    return (status.bits.rx_fifo_not_empty == 1);
}

static void hispi_get_current_config(struct region *reg, u32 clk_freq, struct kdrv_spi_master_config *config)
{
    union spi_common_ctrl cfg;
    u32 mode = 0;
    u32 clk;
    u32 ratio_pre;

    cfg.u32 = ioread32(reg->io_base + SPI_COMMON_CTRL_REG);
    config->cfg.bits_per_word = cfg.bits.frame_size + 1;
    config->cfg.is_loop = (cfg.bits.loop_back) ? 1 : 0;
    mode = (cfg.bits.clk_phase) ? (mode | SPI_CPHA) : mode;
    mode = (cfg.bits.clk_polarity) ? (mode | SPI_CPOL) : mode;
    config->cfg.mode = mode;
    if (cfg.bits.div_ratio_pre == 0) {
        ratio_pre = DIV_RATIO_PRE_SMALL_8;
        pr_err("ratio pre is 0, invalid.\n");
    } else {
        ratio_pre = cfg.bits.div_ratio_pre;
    }
    clk = cfg.bits.div_ratio_post + 1;
    config->cfg.speed = clk_freq / (clk * ratio_pre);

    /* v500 ip has only one CS */
    config->cs = 0;
}

static void hispi_get_trans_status(struct region *reg, struct kdrv_spi_trans_status *status)
{
    union spi_state temp;
    temp.u32 = ioread32(reg->io_base + SPI_STATE_REG);
    status->busy = temp.bits.busy;
    status->rx_full = temp.bits.rx_fifo_full;
    status->rx_not_empty = temp.bits.rx_fifo_not_empty;
    status->tx_not_full = temp.bits.tx_fifo_not_full;
    status->tx_empty = temp.bits.tx_fifo_empty;
}

static void hispi_get_trans_irq(struct region *reg, struct kdrv_spi_trans_irq *status)
{
    union intr_spi temp;
    temp.u32 = ioread32(reg->io_base + INTR_SPI_REG);
    status->tx_irq = temp.bits.intr_tx;
    status->rx_irq = (temp.bits.intr_rx == 1 || temp.bits.intr_rx_timeout == 1);
    status->rx_overflow = temp.bits.intr_rx_overflow;
    status->rx_timeout = temp.bits.intr_rx_timeout;
}

static void hispi_get_err_irq(struct region *reg, struct kdrv_spi_err_irq *status)
{
    union intr_spi temp;
    temp.u32 = ioread32(reg->io_base + INTR_SPI_REG);
    status->tf_ecc_multi_err = temp.bits.tf_ecc_multi_err;
    status->tf_ecc_1bit_err = temp.bits.tf_ecc_1bit_err;
    status->rf_ecc_multi_err = temp.bits.rf_ecc_multi_err;
    status->rf_ecc_1bit_err = temp.bits.rf_ecc_1bit_err;
}

static void hispi_get_fifo_info(struct region *reg, struct kdrv_spi_fifo *fifo)
{
    union spi_fifo_level_ctrl levelc;
    levelc.u32 = ioread32(reg->io_base + SPI_FIFO_LEVEL_CTRL_REG);
    fifo->intr_lvl_tx = levelc.bits.intr_lvl_tx;
    fifo->intr_lvl_rx = levelc.bits.intr_lvl_rx;
}

static bool hisi_spi_busy(struct region *reg)
{
    union spi_state status;
    status.u32 = ioread32(reg->io_base + SPI_STATE_REG);
    return (status.bits.busy == 1);
}

static void hispi_flush_rx_fifo(struct region *reg)
{
    unsigned long limit = TIMEOUT_LIMIT;

    do {
        while (hisi_rx_not_empty(reg)) {
            hispi_read_rx(reg);
        }
    } while (hisi_spi_busy(reg) && limit--);
}

static void hispi_clear_rx_overflow_int(struct region *reg)
{
    iowrite32(0x1, reg->io_base + INTR_SPI_CLR_REG);
}

static int hisi_setup(struct region *reg, struct hisi_trans_info *info, u32 clk_freq, u32 rx_fifo_level)
{
    int ret;

    if ((info->max_speed_hz < SPEED_HZ_MIN) || (info->max_speed_hz > SPEED_HZ_MAX)) {
        pr_err("max_speed_hz:%u is invalid\n", info->max_speed_hz);
        return -EINVAL;
    }
    if ((info->bits_per_word < WIDTH_8_BITS) || (info->bits_per_word > WIDTH_32_BITS)) {
        pr_err("bits_per_word:%u is invalid\n", info->bits_per_word);
        return -EINVAL;
    }
    if (info->chip_select >= CS_MAX_NUM) {
        pr_err("chip_select:%u is invalid\n", info->chip_select);
        return -EINVAL;
    }
    /* Step1 Write SPI_EN【0】:0，Disabling SPI */
    hispi_disable(reg);

    /* Step2 Write SPI_COMMON_CTRL，Configure high-speed/common mode, data bit width, frequency divider, clock phase,
    and clock polarity based on the scenario. Configure SPI_CS_CTRL to control the CS function */
    ret = hispi_setctrl(reg, info, clk_freq);
    if (ret) {
        pr_err("hispi_setctrl failed\n");
        return ret;
    }

    /* Step3 Write SPI_FIFO_LEVEL_CTRL，Set FIFO level */
    hispi_set_fifo_level(reg, rx_fifo_level);
    return 0;
}

static unsigned int hispi_get_fifo_depth(void)
{
    return FIFO_DEPTH;
}

const struct hisi_spi_ops hisiv500_spi_ops = {
    .setup = hisi_setup,
    .enable = hispi_enable,
    .disable = hispi_disable,
    .enable_int = hispi_enable_int,
    .disable_int = hispi_disable_int,
    .write_tx = hispi_write_tx,
    .read_rx = hispi_read_rx,
    .get_current_config = hispi_get_current_config,
    .get_trans_status = hispi_get_trans_status,
    .get_trans_irq = hispi_get_trans_irq,
    .get_err_irq = hispi_get_err_irq,
    .get_fifo_info = hispi_get_fifo_info,
    .tx_not_full = hispi_tx_not_full,
    .rx_not_empty = hisi_rx_not_empty,
    .flush_rx_fifo = hispi_flush_rx_fifo,
    .clear_rxoi = hispi_clear_rx_overflow_int,
    .get_fifo_depth = hispi_get_fifo_depth,
};

const struct hisi_spi_ops *spi_get_ops(void)
{
    return &hisiv500_spi_ops;
}
