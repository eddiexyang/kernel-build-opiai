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
 * Description: usb pcs
 * Author:huawei
 * Create: 2022-10-09
 */
#include <linux/delay.h>
#include <linux/io.h>
#include "usb_hisi.h"

#define PCS_REG_CSR1 0x4
#define PCS_REG_CSR3 0xC
#define PCS_REG_CSR5 0x14
#define PCS_REG_CSR12 0x30
#define PCS_REG_CSR34 0x88
#define PCS_REG_CSR35 0x8c
#define PCS_REG_CSR36 0x90

#define USB_PCS_TX_FFE_0DB_DEFAULT 0x700
#define USB_PCS_TX_FFE_3P5DB_DEFAULT 0x55c0
#define USB_PCS_TX_FFE_6DB_DEFAULT 0x7540

enum usb_pcs_adap_rate {
    PCS_RATE_GEN1_5G,
    PCS_RATE_GEN2_10G,
};

enum usb_pcs_adap_pclkrate {
    PCS_PCLK_RATE_125M,
    PCS_PCLK_RATE_250M,
    PCS_PCLK_RATE_312P5M,
    PCS_PCLK_RATE_500M,
    PCS_PCLK_RATE_625M,
    PCS_PCLK_RATE_1250M,
};

union usb_pcs_csr1 {
    struct {
        unsigned int pcs_tx_soft_rstn : 1; /* [0] */
        unsigned int bist_soft_rstn : 1; /* [1] */
        unsigned int pcs_rx_soft_rstn : 1; /* [2] */
        unsigned int pcs_sync_tx_soft_restn : 1; /* [3] */
        unsigned int pcs_sync_rx_soft_restn : 1; /* [4] */
        unsigned int rsv : 1; /* [5] */
        unsigned int rate_online_clk_en : 1; /* [6] */
        unsigned int pcs_tx_clk_en : 1; /* [7] */
        unsigned int bist_clk_en : 1; /* [8] */
        unsigned int pcs_rx_clk_en : 1; /* [9] */
        unsigned int pclk_en : 1; /* [10] */
        unsigned int usb_clk_en : 1; /* [11] */
        unsigned int rsv2 : 2; /* [13:12] */
        unsigned int trace_clk_sel : 1; /* [14] */
        unsigned int adap_sdpi_clk_en : 1; /* [15] */
        unsigned int tx_lfps_clk_en_dg : 1; /* [16] */
        unsigned int tx_lfps_clk_en_gt : 1; /* [17] */
        unsigned int tx_lfps_soft_rstn : 1; /* [18] */
        unsigned int rsv3 : 13; /* [31:19] */
    } bits;
    unsigned int u32;
};

union usb_pcs_csr3 {
    struct {
        unsigned int adap_txelecidle : 1; /* [0] */
        unsigned int adap_txdetectrx_lpbk : 1; /* [1] */
        unsigned int adap_elasticity_buffer_mode : 1; /* [2] */
        unsigned int adap_pclkrate : 3; /* [5:3] */
        unsigned int adap_width : 2; /* [7:6] */
        unsigned int adap_rate : 3; /* [10:8] */
        unsigned int adap_powerdown : 4; /* [14:11] */
        unsigned int adap_reset : 1; /* [15] */
        unsigned int adap_phymode : 4; /* [19:16] */
        unsigned int rsv : 12; /* [31:20] */
    } bits;
    unsigned int u32;
};

union usb_pcs_csr5 {
    struct {
        unsigned int adap_txelecidle_pin_en : 1; /* [0] */
        unsigned int adap_txdetectrx_lpbk_pin_en : 1; /* [1] */
        unsigned int adap_elasticity_buffer_mode_pin_en : 1; /* [2] */
        unsigned int adap_pclkrate_pin_en : 1; /* [3] */
        unsigned int adap_width_pin_en : 1; /* [4] */
        unsigned int adap_rate_pin_en : 1; /* [5] */
        unsigned int adap_powerdown_pin_en : 1; /* [6] */
        unsigned int adap_reset_pin_en : 1; /* [7] */
        unsigned int adap_phymode_pin_en : 1; /* [8] */
        unsigned int adap_rxstandby_pin_en : 1; /* [9] */
        unsigned int adap_blockaligncontrol_pin_en : 1; /* [10] */
        unsigned int adap_rxeqtraining_pin_en : 1; /* [11] */
        unsigned int adap_rxpolarity_pin_en : 1; /* [12] */
        unsigned int adap_txonezeros_pin_en : 1; /* [13] */
        unsigned int adap_txdeemph_pin_en : 1; /* [14] */
        unsigned int adap_rxtermination_pin_en : 1; /* [15] */
        unsigned int rsv : 16; /* [31:16] */
    } bits;
    unsigned int u32;
};

union usb_pcs_csr12 {
    struct {
        unsigned int adap_p3_phystatus_wait_time : 8; /* [7:0] */
        unsigned int adap_rate_phystatus_wait_time : 8; /* [15:8] */
        unsigned int adap_rxtx_recover_mode : 1; /* [16] */
        unsigned int adap_curr_rate : 3; /* [19:17] */
        unsigned int adap_rate_reset_value : 3; /* [22:20] */
        unsigned int adap_pcs_hold_reset : 1; /* [23] */
        unsigned int adap_rx_blkal_syal_unlock_reset_ctrl : 1; /* [24] */
        unsigned int rsv : 7; /* [31:25] */
    } bits;
    unsigned int u32;
};

union usb_pcs_csr34 {
    struct {
        unsigned int tx_ffe_0db : 18; /* [17:0] */
        unsigned int txmargin : 4; /* [21:18] */
        unsigned int rsv : 10; /* [31:22] */
    } bits;
    unsigned int u32;
};

union usb_pcs_csr35 {
    struct {
        unsigned int tx_ffe_3p5db : 18; /* [17:0] */
        unsigned int rsv : 14; /* [31:18] */
    } bits;
    unsigned int u32;
};

union usb_pcs_csr36 {
    struct {
        unsigned int tx_ffe_6db : 18; /* [17:0] */
        unsigned int rsv : 14; /* [31:18] */
    } bits;
    unsigned int u32;
};

static void usb_set_pcs_adap_rate(void __iomem *base)
{
    union usb_pcs_csr3 value;
    value.u32 = readl(base + PCS_REG_CSR3);
    value.bits.adap_rate = PCS_RATE_GEN1_5G;
    writel(value.u32, base + PCS_REG_CSR3);
}

static void usb_set_pcs_adap_cur_rate(void __iomem *base)
{
    union usb_pcs_csr12 value;
    value.u32 = readl(base + PCS_REG_CSR12);
    value.bits.adap_rate_reset_value = PCS_RATE_GEN1_5G;
    value.bits.adap_curr_rate = PCS_RATE_GEN1_5G;
    writel(value.u32, base + PCS_REG_CSR12);
}

static void usb_set_pcs_adap_pclk_rate(void __iomem *base)
{
    union usb_pcs_csr3 value;
    value.u32 = readl(base + PCS_REG_CSR3);
    value.bits.adap_pclkrate = PCS_PCLK_RATE_125M;
    writel(value.u32, base + PCS_REG_CSR3);
}

static void usb_set_pcs_tx_ffe_0db(void __iomem *base)
{
    union usb_pcs_csr34 value;
    value.u32 = readl(base + PCS_REG_CSR34);
    value.bits.tx_ffe_0db = USB_PCS_TX_FFE_0DB_DEFAULT;
    writel(value.u32, base + PCS_REG_CSR34);
}

static void usb_set_pcs_tx_ffe_3p5db(void __iomem *base)
{
    union usb_pcs_csr35 value;
    value.u32 = readl(base + PCS_REG_CSR35);
    value.bits.tx_ffe_3p5db = USB_PCS_TX_FFE_3P5DB_DEFAULT;
    writel(value.u32, base + PCS_REG_CSR35);
}

static void usb_set_pcs_tx_ffe_6db(void __iomem *base)
{
    union usb_pcs_csr36 value;
    value.u32 = readl(base + PCS_REG_CSR36);
    value.bits.tx_ffe_6db = USB_PCS_TX_FFE_6DB_DEFAULT;
    writel(value.u32, base + PCS_REG_CSR36);
}

static void usb_set_pcs_clk_en(void __iomem *base)
{
    union usb_pcs_csr1 value;
    value.u32 = readl(base + PCS_REG_CSR1);
    value.bits.adap_sdpi_clk_en = 1;
    value.bits.usb_clk_en = 1;
    value.bits.pcs_rx_clk_en = 1;
    value.bits.pcs_tx_clk_en = 1;
    writel(value.u32, base + PCS_REG_CSR1);
}

static void usb_set_pcs_hold_reset(void __iomem *base)
{
    union usb_pcs_csr12 value;
    value.u32 = readl(base + PCS_REG_CSR12);
    value.bits.adap_pcs_hold_reset = 0;
    writel(value.u32, base + PCS_REG_CSR12);
}

static void usb_set_pcs_pin_en(void __iomem *base)
{
    union usb_pcs_csr5 value;
    value.u32 = readl(base + PCS_REG_CSR5);
    value.bits.adap_rxtermination_pin_en = 1;
    value.bits.adap_txdeemph_pin_en = 1;
    value.bits.adap_txonezeros_pin_en = 1;
    value.bits.adap_rxpolarity_pin_en = 1;
    value.bits.adap_rxeqtraining_pin_en = 1;
    value.bits.adap_blockaligncontrol_pin_en = 1;
    value.bits.adap_rxstandby_pin_en = 1;
    value.bits.adap_phymode_pin_en = 1;
    value.bits.adap_reset_pin_en = 1;
    value.bits.adap_powerdown_pin_en = 1;
    value.bits.adap_rate_pin_en = 1;
    value.bits.adap_width_pin_en = 1;
    value.bits.adap_pclkrate_pin_en = 1;
    value.bits.adap_elasticity_buffer_mode_pin_en = 1;
    value.bits.adap_txdetectrx_lpbk_pin_en = 1;
    value.bits.adap_txelecidle_pin_en = 1;
    writel(value.u32, base + PCS_REG_CSR5);
}

void usbdrv_pcs_init(void __iomem *base)
{
    usb_set_pcs_adap_rate(base);
    usb_set_pcs_adap_cur_rate(base);
    usb_set_pcs_adap_pclk_rate(base);
    usb_set_pcs_tx_ffe_0db(base);
    usb_set_pcs_tx_ffe_3p5db(base);
    usb_set_pcs_tx_ffe_6db(base);
    usb_set_pcs_clk_en(base);
    usb_set_pcs_hold_reset(base);
    udelay(200);
    usb_set_pcs_pin_en(base);
}

