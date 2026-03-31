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
 * Description: hisi Huawei-developed i2Cv400 logical layer source file
 * Author: huawei
 * Create: 2022-04-14
 */
#include <linux/io.h>
#include <linux/bitfield.h>
#include "hisi_i2c.h"
#include "kdrv_i2c_api.h"

/******************************************************************************/
/*                      xxx i2c Registers' Definitions                            */
/******************************************************************************/
#define FRAME_CTRL_REG          (0x0000)     /* I2C frame control register */
#define  HISI_I2C_FRAME_CTRL_SPEED_MODE GENMASK(1, 0)
#define  HISI_I2C_FRAME_CTRL_ADDR_TEN   BIT(2)
#define SLV_ADDR_REG            (0x0004)     /* Address register of the target device accessed by the I2C */
#define  HISI_I2C_SLV_ADDR_VAL  GENMASK(9, 0)
#define CMD_TXDATA_REG          (0x0008)     /* I2C device command and TX data control register */
#define  HISI_I2C_CMD_TXDATA_DATA   GENMASK(7, 0)
#define  HISI_I2C_CMD_TXDATA_RW     BIT(8)
#define  HISI_I2C_CMD_TXDATA_P_EN   BIT(9)
#define  HISI_I2C_CMD_TXDATA_SR_EN  BIT(10)
#define RXDATA_REG              (0x000C)     /* I2C device RX data interface */
#define SS_SCL_HCNT_REG         (0x0010)     /* I2C device frequency divider value register. SS mode */
#define SS_SCL_LCNT_REG         (0x0014)     /* I2C device frequency divider value register. SS mode */
#define FS_SCL_HCNT_REG         (0x0018)     /* I2C device frequency divider value register. FS mode */
#define FS_SCL_LCNT_REG         (0x001C)     /* I2C device frequency divider value register. FS mode */
#define HS_SCL_HCNT_REG         (0x0020)     /* I2C device frequency divider value register. HS mode */
#define HS_SCL_LCNT_REG         (0x0024)     /* I2C device frequency divider value register. HS mode */
#define FIFO_CTRL_REG           (0x0028)     /* FIFO control register */
#define  HISI_I2C_FIFO_RX_AF_THRESH GENMASK(7, 2)
#define  HISI_I2C_FIFO_TX_AE_THRESH GENMASK(13, 8)
#define  HISI_I2C_FIFO_RX_CLR       BIT(0)
#define  HISI_I2C_FIFO_TX_CLR       BIT(1)
#define FIFO_STATE_REG          (0x002C)     /* FIFO status register */
#define  HISI_I2C_FIFO_STATE_RX_RERR    BIT(0)
#define  HISI_I2C_FIFO_STATE_RX_WERR    BIT(1)
#define  HISI_I2C_FIFO_STATE_RX_EMPTY   BIT(3)
#define  HISI_I2C_FIFO_STATE_TX_RERR    BIT(6)
#define  HISI_I2C_FIFO_STATE_TX_WERR    BIT(7)
#define  HISI_I2C_FIFO_STATE_TX_FULL    BIT(11)
#define SDA_HOLD_REG            (0x0030)     /* I2C SDA HOLD time control register */
#define  HISI_I2C_SDA_HOLD_TX   GENMASK(15, 0)
#define DMA_CTRL_REG            (0x0034)     /* DMA mode control register */
#define FS_SPK_LEN_REG          (0x0038)     /* Deglitch length register in fast mode */
#define HS_SPK_LEN_REG          (0x003C)     /* Deglitch length register in high-speed mode */
#define INT_CLR_REG0            (0x0040)     /* Interrupt clear register */
#define  HISI_I2C_INT_CLR_TX_AEMPTY    BIT(0)
#define INT_MSTAT_REG           (0x0044)     /* Interrupt mask status register */
#define  HISI_I2C_INT_MSTAT_ALL         GENMASK(4, 0)
#define  HISI_I2C_INT_MSTAT_TRANS_CPLT  BIT(0)
#define  HISI_I2C_INT_MSTAT_TRANS_ERR   BIT(1)
#define  HISI_I2C_INT_MSTAT_FIFO_ERR    BIT(2)
#define  HISI_I2C_INT_MSTAT_RX_FULL     BIT(3)
#define  HISI_I2C_INT_MSTAT_TX_EMPTY    BIT(4)
#define  HISI_I2C_INT_ERR_MASK         (HISI_I2C_INT_MSTAT_TRANS_ERR)
#define INT_CLR_REG             (0x0048)     /* Interrupt clear register */
#define  HISI_I2C_INT_CLR_TRANS_CPLT    BIT(0)
#define  HISI_I2C_INT_CLR_TRANS_ERR     BIT(1)
#define  HISI_I2C_INT_CLR_FIFO_ERR      BIT(2)
#define  HISI_I2C_INT_CLR_RX_AFULL      BIT(3)
#define INT_MASK_REG            (0x004C)     /* Interrupt mask register */
#define TRANS_STATE_REG         (0x0050)     /* Transfer status register */
#define TRANS_ERR_REG           (0x0054)     /* Transfer error register */
#define  HISI_I2C_TRANS_ERR_TXDATA_NOACK         BIT(0)
#define  HISI_I2C_TRANS_ERR_10B_RD_NORSTRT       BIT(1)
#define  HISI_I2C_TRANS_ERR_10ADDR2_NOACK        BIT(2)
#define  HISI_I2C_TRANS_ERR_10ADDR1_NOACK        BIT(3)
#define  HISI_I2C_TRANS_ERR_7B_ADDR_NOACK        BIT(4)
#define  HISI_I2C_TRANS_ERR_HS_ACKDET            BIT(5)
#define  HISI_I2C_TRANS_ERR_HS_NORSTRT           BIT(6)
#define  HISI_I2C_TRANS_ERR_SBYTE_NORSTRT        BIT(7)
#define  HISI_I2C_TRANS_ERR_SBYTE_ACKDET         BIT(8)
#define  HISI_I2C_TRANS_ERR_GCALL_READ           BIT(9)
#define  HISI_I2C_TRANS_ERR_GCALL_NOACK          BIT(10)

#define TRANS_ERR_TXDATA_NOACK         0
#define TRANS_ERR_10B_RD_NORSTRT       1
#define TRANS_ERR_10ADDR2_NOACK        2
#define TRANS_ERR_10ADDR1_NOACK        3
#define TRANS_ERR_7B_ADDR_NOACK        4
#define TRANS_ERR_HS_ACKDET            5
#define TRANS_ERR_HS_NORSTRT           6
#define TRANS_ERR_SBYTE_NORSTRT        7
#define TRANS_ERR_SBYTE_ACKDET         8
#define TRANS_ERR_GCALL_READ           9
#define TRANS_ERR_GCALL_NOACK          10

#define HISI_I2C_STD_SPEED_MODE  0
#define HISI_I2C_FAST_SPEED_MODE 1
#define HISI_I2C_HIGH_SPEED_MODE 2
#define HISI_I2C_TX_F_AE_THRESH  1
#define HISI_I2C_RX_F_AF_THRESH  60
#define HISI_I2C_TX_FIFO_DEPTH   64
#define HISI_I2C_RX_FIFO_DEPTH   64

static void hisi_i2c_enable_tx_aempty_int_v400(struct io_region *reg, u32 enable)
{
    u32 int_mask;

    int_mask = readl(reg->io_base + INT_MASK_REG);
    if (enable == 1) {
        int_mask |= HISI_I2C_INT_MSTAT_TX_EMPTY;
    } else {
        int_mask &= ~HISI_I2C_INT_MSTAT_TX_EMPTY;
    }
    writel_relaxed(int_mask, reg->io_base + INT_MASK_REG);
}

static void hisi_i2c_enable_rx_afull_int_v400(struct io_region *reg, u32 enable)
{
    u32 int_mask;

    int_mask = readl(reg->io_base + INT_MASK_REG);
    if (enable == 1) {
        int_mask |= HISI_I2C_INT_MSTAT_RX_FULL;
    } else {
        int_mask &= ~HISI_I2C_INT_MSTAT_RX_FULL;
    }
    writel_relaxed(int_mask, reg->io_base + INT_MASK_REG);
}

static void hisi_i2c_enable_err_int_v400(struct io_region *reg, u32 enable)
{
    u32 int_mask;

    int_mask = readl(reg->io_base + INT_MASK_REG);
    if (enable == 1) {
        int_mask |= HISI_I2C_INT_ERR_MASK;
    } else {
        int_mask &= ~HISI_I2C_INT_ERR_MASK;
    }
    writel_relaxed(int_mask, reg->io_base + INT_MASK_REG);
}

static void hisi_i2c_enable_trans_cplt_int_v400(struct io_region *reg, u32 enable)
{
    u32 int_mask;

    int_mask = readl(reg->io_base + INT_MASK_REG);
    if (enable == 1) {
        int_mask |= HISI_I2C_INT_MSTAT_TRANS_CPLT;
    } else {
        int_mask &= ~HISI_I2C_INT_MSTAT_TRANS_CPLT;
    }
    writel_relaxed(int_mask, reg->io_base + INT_MASK_REG);
}

static void hisi_i2c_disable_all_int_v400(struct io_region *reg)
{
    writel_relaxed(0, reg->io_base + INT_MASK_REG);
}

static void hisi_i2c_clear_all_int_v400(struct io_region *reg)
{
    writel_relaxed(HISI_I2C_INT_MSTAT_ALL, reg->io_base + INT_CLR_REG);
    writel_relaxed((HISI_I2C_INT_CLR_TX_AEMPTY), reg->io_base + INT_CLR_REG0);
}

static char *abort_sources_str[] = {
    [TRANS_ERR_7B_ADDR_NOACK] =
    "slave address not acknowledged (7bit mode)",
    [TRANS_ERR_10ADDR1_NOACK] =
    "first address byte not acknowledged (10bit mode)",
    [TRANS_ERR_10ADDR2_NOACK] =
    "second address byte not acknowledged (10bit mode)",
    [TRANS_ERR_TXDATA_NOACK] =
    "data not acknowledged",
    [TRANS_ERR_GCALL_NOACK] =
    "no acknowledgement for a general call",
    [TRANS_ERR_GCALL_READ] =
    "read after general call",
    [TRANS_ERR_HS_ACKDET] =
    "hs mode acknowledged",
    [TRANS_ERR_SBYTE_ACKDET] =
    "start byte acknowledged",
    [TRANS_ERR_HS_NORSTRT] =
    "trying to HS MODE send byte when restart is disabled",
    [TRANS_ERR_SBYTE_NORSTRT] =
    "trying to send start byte when restart is disabled",
    [TRANS_ERR_10B_RD_NORSTRT] =
    "trying to read when restart is disabled (10bit mode)",
};

static void hisi_i2c_handle_errors_v400(struct device *dev, struct io_region *reg, u32 xfer_err, u32 err_source)
{
    u32 fifo_state;
    u32 err_state;
    unsigned long abort_source;
    int i;

    fifo_state = readl(reg->io_base + FIFO_STATE_REG);
    if (fifo_state & HISI_I2C_FIFO_STATE_RX_RERR) {
        dev_err(dev, "rx fifo error read\n");
    }
    if (fifo_state & HISI_I2C_FIFO_STATE_RX_WERR) {
        dev_err(dev, "rx fifo error write\n");
    }
    if (fifo_state & HISI_I2C_FIFO_STATE_TX_RERR) {
        dev_err(dev, "tx fifo error read\n");
    }
    if (fifo_state & HISI_I2C_FIFO_STATE_TX_WERR) {
        dev_err(dev, "tx fifo error write\n");
    }

    if (xfer_err & HISI_I2C_INT_MSTAT_TRANS_ERR) {
        err_state = readl(reg->io_base + TRANS_ERR_REG);
        abort_source = (unsigned long)err_state;
        for_each_set_bit(i, &abort_source, ARRAY_SIZE(abort_sources_str)) {
            dev_err(dev, "hisi_i2c_handle_errors_v400: %s\n", abort_sources_str[i]);
        }
    }
}

static void set_slave_addr(struct io_region *reg, bool is_10bit_addr, u16 addr)
{
    u32 val;

    val = readl(reg->io_base + FRAME_CTRL_REG);
    val &= ~HISI_I2C_FRAME_CTRL_ADDR_TEN;
    if (is_10bit_addr) {
        val |= HISI_I2C_FRAME_CTRL_ADDR_TEN; /* 1：10bit address */
    }
    writel(val, reg->io_base + FRAME_CTRL_REG);

    val = readl(reg->io_base + SLV_ADDR_REG);
    val &= ~HISI_I2C_SLV_ADDR_VAL;
    val |= FIELD_PREP(HISI_I2C_SLV_ADDR_VAL, addr);
    writel(val, reg->io_base + SLV_ADDR_REG);
}

static void clear_tx_rx_fifo(struct io_region *reg)
{
    u32 val;
    /* FIFO clearing. Set this parameter to 1 and then to 0. */
    val = readl(reg->io_base + FIFO_CTRL_REG);
    val |= HISI_I2C_FIFO_RX_CLR | HISI_I2C_FIFO_TX_CLR;
    writel(val, reg->io_base + FIFO_CTRL_REG);
    val &= ~(HISI_I2C_FIFO_RX_CLR | HISI_I2C_FIFO_TX_CLR);
    writel(val, reg->io_base + FIFO_CTRL_REG);
}

static void hisi_i2c_xfer_init_v400(struct io_region *reg, u16 addr, bool is_10bit_addr)
{
    /* Write the slave address and address width accessed by the master. */
    set_slave_addr(reg, is_10bit_addr, addr);
    /* clear rx/tx fifo */
    clear_tx_rx_fifo(reg);
}

static u8 hisi_i2c_read_rx_data_v400(struct io_region *reg)
{
    u8 data;
    data = (u8)readl(reg->io_base + RXDATA_REG);
    return data;
}

static void hisi_i2c_write_tx_data_v400(struct io_region *reg, u8 data, u16 wr, bool need_stop, bool need_restart)
{
    u32 val = 0;

    if (need_restart) {
        val |= HISI_I2C_CMD_TXDATA_SR_EN;
    }

    if (need_stop) {
        val |= HISI_I2C_CMD_TXDATA_P_EN;
    }

    if (wr) { // read
        val |= HISI_I2C_CMD_TXDATA_RW;
    } else {
        val &= (~HISI_I2C_CMD_TXDATA_RW);
        val |= FIELD_PREP(HISI_I2C_CMD_TXDATA_DATA, data);
    }
    writel(val, reg->io_base + CMD_TXDATA_REG);
}

static void hisi_i2c_set_speed_mode(struct io_region *reg, u32 speed_mode)
{
    u32 val;

    val = readl(reg->io_base + FRAME_CTRL_REG);
    val &= ~HISI_I2C_FRAME_CTRL_SPEED_MODE;
    val |= FIELD_PREP(HISI_I2C_FRAME_CTRL_SPEED_MODE, speed_mode);
    writel(val, reg->io_base + FRAME_CTRL_REG);
}

static void hisi_i2c_set_tx_sda_hold(struct io_region *reg, unsigned int sda_hold_cnt)
{
    u32 val;

    val = readl(reg->io_base + SDA_HOLD_REG);
    val &= ~HISI_I2C_SDA_HOLD_TX;
    val |= FIELD_PREP(HISI_I2C_SDA_HOLD_TX, sda_hold_cnt);
    writel(val, reg->io_base + SDA_HOLD_REG);
}

static void hisi_i2c_set_fifo_threshold(struct io_region *reg)
{
    u32 val;

    val = readl(reg->io_base + FIFO_CTRL_REG);
    val &= ~HISI_I2C_FIFO_RX_AF_THRESH;
    val &= ~HISI_I2C_FIFO_TX_AE_THRESH;
    val |= FIELD_PREP(HISI_I2C_FIFO_RX_AF_THRESH, HISI_I2C_RX_F_AF_THRESH);
    val |= FIELD_PREP(HISI_I2C_FIFO_TX_AE_THRESH, HISI_I2C_TX_F_AE_THRESH);
    writel(val, reg->io_base + FIFO_CTRL_REG);
}

static void hisi_i2c_configure_bus_v400(struct io_region *reg, u32 bus_freq_hz, struct i2c_timings_cfg *cfg)
{
    u32 speed_mode;

    /* Sets the frequency division value of the corresponding speed. */
    switch (bus_freq_hz) {
        case I2C_MAX_FAST_MODE_FREQ:
            speed_mode = HISI_I2C_FAST_SPEED_MODE;
            writel(cfg->scl_hcnt, reg->io_base + FS_SCL_HCNT_REG);
            writel(cfg->scl_lcnt, reg->io_base + FS_SCL_LCNT_REG);
            break;
        case I2C_MAX_HIGH_SPEED_MODE_FREQ:
            speed_mode = HISI_I2C_HIGH_SPEED_MODE;
            writel(cfg->scl_hcnt, reg->io_base + HS_SCL_HCNT_REG);
            writel(cfg->scl_lcnt, reg->io_base + HS_SCL_LCNT_REG);
            break;
        case I2C_MAX_STD_MODE_FREQ:
        default:
            speed_mode = HISI_I2C_STD_SPEED_MODE;
            writel(cfg->scl_hcnt, reg->io_base + SS_SCL_HCNT_REG);
            writel(cfg->scl_lcnt, reg->io_base + SS_SCL_LCNT_REG);
            break;
    }
    /*  Configuring the Operating Speed of an I2C Device */
    hisi_i2c_set_speed_mode(reg, speed_mode);

    /* Configure the SDA hold time required when sending */
    hisi_i2c_set_tx_sda_hold(reg, cfg->sda_hold_cnt);
    /* In fast mode, deglitch length */
    writel(cfg->spk_len, reg->io_base + FS_SPK_LEN_REG);
    /* Configuring the rx_fifo almost full threshold and tx_fifo almost empty threshold */
    hisi_i2c_set_fifo_threshold(reg);
}

static void hisi_i2c_clear_err_int_v400(struct io_region *reg)
{
    writel_relaxed(HISI_I2C_INT_CLR_TRANS_ERR, reg->io_base + INT_CLR_REG);
}

static void hisi_i2c_clear_tx_aempty_int_v400(struct io_region *reg)
{
    writel_relaxed((HISI_I2C_INT_CLR_TX_AEMPTY), reg->io_base + INT_CLR_REG0);
}

static void hisi_i2c_clear_rx_afull_int_v400(struct io_region *reg)
{
    writel_relaxed((HISI_I2C_INT_CLR_RX_AFULL), reg->io_base + INT_CLR_REG);
}

static void hisi_i2c_clear_trans_cplt_int_v400(struct io_region *reg)
{
    writel_relaxed((HISI_I2C_INT_CLR_TRANS_CPLT), reg->io_base + INT_CLR_REG);
}

static bool hisi_i2c_tx_fifo_full_v400(struct io_region *reg)
{
    u32 fifo_state;

    fifo_state = readl(reg->io_base + FIFO_STATE_REG);
    if (fifo_state & HISI_I2C_FIFO_STATE_TX_FULL) {
        return true;
    }
    return false;
}

static bool hisi_i2c_rx_fifo_empty_v400(struct io_region *reg)
{
    u32 fifo_state;
    fifo_state = readl(reg->io_base + FIFO_STATE_REG);
    if (fifo_state & HISI_I2C_FIFO_STATE_RX_EMPTY) {
        return true;
    }
    return false;
}

static u32 hisi_i2c_get_tx_fifo_limit_v400(struct io_region *reg)
{
    u32 val;
    u32 tx_thresh;

    val = readl(reg->io_base + FIFO_CTRL_REG);
    tx_thresh = FIELD_GET(HISI_I2C_FIFO_TX_AE_THRESH, val);
    return (HISI_I2C_TX_FIFO_DEPTH - tx_thresh);
}

static void hisi_i2c_get_int_status_v400(struct io_region *reg, struct i2c_int_status_info *status)
{
    u32 int_stat;

    int_stat = readl(reg->io_base + INT_MSTAT_REG);
    pr_debug("[%s]int_stat 0x%x\n", __FUNCTION__, int_stat);
    if (int_stat & HISI_I2C_INT_MSTAT_TX_EMPTY) {
        status->is_tx_aempty_irq = 1;
    }
    if (int_stat & HISI_I2C_INT_MSTAT_TRANS_ERR) {
        status->xfer_err = int_stat;
        status->is_err_irq = 1;
    }
    if (int_stat & HISI_I2C_INT_MSTAT_RX_FULL) {
        status->is_rx_afull_irq = 1;
    }
    if (int_stat & HISI_I2C_INT_MSTAT_TRANS_CPLT) {
        status->is_trans_cplt_irq = 1;
    }
}

static void hisi_i2c_get_err_status(struct io_region *reg, struct kdrv_i2c_err_status *status)
{
    u32 val;

    val = readl(reg->io_base + SLV_ADDR_REG);
    status->slv_addr = FIELD_GET(HISI_I2C_SLV_ADDR_VAL, val);

    val = readl(reg->io_base + FRAME_CTRL_REG);
    status->addr_width = (val & HISI_I2C_FRAME_CTRL_ADDR_TEN) ? 10 : 7; // 10bit and 7bit addr

    val = readl(reg->io_base + FIFO_STATE_REG);
    status->fifo_err.bits.rx_fifo_rerror = (val & HISI_I2C_FIFO_STATE_RX_RERR) ? 1 : 0;
    status->fifo_err.bits.tx_fifo_rerror = (val & HISI_I2C_FIFO_STATE_TX_RERR) ? 1 : 0;
    status->fifo_err.bits.rx_fifo_werror = (val & HISI_I2C_FIFO_STATE_RX_WERR) ? 1 : 0;
    status->fifo_err.bits.tx_fifo_werror = (val & HISI_I2C_FIFO_STATE_TX_WERR) ? 1 : 0;

    val = readl(reg->io_base + TRANS_ERR_REG);
    status->trans_err.bits.rd_10b_no_restart = (val & HISI_I2C_TRANS_ERR_10B_RD_NORSTRT) ? 1 : 0;
    status->trans_err.bits.sbyte_no_restart = (val & HISI_I2C_TRANS_ERR_SBYTE_NORSTRT) ? 1 : 0;
    status->trans_err.bits.hs_no_restart = (val & HISI_I2C_TRANS_ERR_HS_NORSTRT) ? 1 : 0;
    status->trans_err.bits.sbyte_ackdet = (val & HISI_I2C_TRANS_ERR_SBYTE_ACKDET) ? 1 : 0;
    status->trans_err.bits.hs_ackdet = (val & HISI_I2C_TRANS_ERR_HS_ACKDET) ? 1 : 0;
    status->trans_err.bits.gcall_read = (val & HISI_I2C_TRANS_ERR_GCALL_READ) ? 1 : 0;
    status->trans_err.bits.gcall_noack = (val & HISI_I2C_TRANS_ERR_GCALL_NOACK) ? 1 : 0;
    status->trans_err.bits.txdata_noack = (val & HISI_I2C_TRANS_ERR_TXDATA_NOACK) ? 1 : 0;
    status->trans_err.bits.addr2_10b_noack = (val & HISI_I2C_TRANS_ERR_10ADDR2_NOACK) ? 1 : 0;
    status->trans_err.bits.addr1_10b_noack = (val & HISI_I2C_TRANS_ERR_10ADDR1_NOACK) ? 1 : 0;
    status->trans_err.bits.addr_7b_noack = (val & HISI_I2C_TRANS_ERR_7B_ADDR_NOACK) ? 1 : 0;
}

void hisi_i2c_get_timing_cfg(struct io_region *reg, struct kdrv_i2c_timing_cfg *cfg)
{
    u32 val;
    u32 speed_mode;

    val = readl(reg->io_base + FRAME_CTRL_REG);
    speed_mode = FIELD_GET(HISI_I2C_FRAME_CTRL_SPEED_MODE, val);
    /* Obtains the frequency division value of the corresponding speed. */
    switch (speed_mode) {
        case HISI_I2C_FAST_SPEED_MODE:
            cfg->speed_mode = KDRV_I2C_FAST_SPEED_MODE;
            cfg->scl_hcnt = readl(reg->io_base + FS_SCL_HCNT_REG);
            cfg->scl_lcnt = readl(reg->io_base + FS_SCL_LCNT_REG);

            break;
        case HISI_I2C_HIGH_SPEED_MODE:
            cfg->speed_mode = KDRV_I2C_HIGH_SPEED_MODE;
            cfg->scl_hcnt = readl(reg->io_base + HS_SCL_HCNT_REG);
            cfg->scl_lcnt = readl(reg->io_base + HS_SCL_LCNT_REG);
            break;
        case I2C_MAX_STD_MODE_FREQ:
        default:
            cfg->speed_mode = KDRV_I2C_STD_SPEED_MODE;
            cfg->scl_hcnt = readl(reg->io_base + SS_SCL_HCNT_REG);
            cfg->scl_lcnt = readl(reg->io_base + SS_SCL_LCNT_REG);
            break;
    }

    val = readl(reg->io_base + SDA_HOLD_REG);
    cfg->sda_hold_cnt = FIELD_GET(HISI_I2C_SDA_HOLD_TX, val);
    cfg->spk_len = readl(reg->io_base + FS_SPK_LEN_REG);
}

const struct hisi_i2c_ops hisi_i2c_v400_ops = {
    .config_bus = hisi_i2c_configure_bus_v400,
    .disable_all_int = hisi_i2c_disable_all_int_v400,
    .enable_tx_aempty_int = hisi_i2c_enable_tx_aempty_int_v400,
    .enable_rx_afull_int = hisi_i2c_enable_rx_afull_int_v400,
    .enable_trans_cplt_int = hisi_i2c_enable_trans_cplt_int_v400,
    .enable_err_int = hisi_i2c_enable_err_int_v400,
    .clear_all_int = hisi_i2c_clear_all_int_v400,
    .clear_tx_aempty_int = hisi_i2c_clear_tx_aempty_int_v400,
    .clear_rx_afull_int = hisi_i2c_clear_rx_afull_int_v400,
    .clear_err_int = hisi_i2c_clear_err_int_v400,
    .clear_trans_cplt_int = hisi_i2c_clear_trans_cplt_int_v400,
    .write_tx_data = hisi_i2c_write_tx_data_v400,
    .read_rx_data = hisi_i2c_read_rx_data_v400,
    .handle_errors = hisi_i2c_handle_errors_v400,
    .get_int_status_info = hisi_i2c_get_int_status_v400,
    .tx_fifo_full = hisi_i2c_tx_fifo_full_v400,
    .rx_fifo_empty = hisi_i2c_rx_fifo_empty_v400,
    .xfer_init = hisi_i2c_xfer_init_v400,
    .get_tx_fifo_limit = hisi_i2c_get_tx_fifo_limit_v400,
    .get_err_status = hisi_i2c_get_err_status,
    .get_timing_cfg = hisi_i2c_get_timing_cfg,
};

const struct hisi_i2c_ops *hisi_i2c_get_ops(void)
{
    return &hisi_i2c_v400_ops;
}
