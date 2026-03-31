/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: aiao_reg.h
 * Author: Hisilicon multimedia software group
 * Create: 2012/10/14
 * History:
 *   1.Date        : 2012/10/14
 *     Modification: Created file
 */

#include "aio_pub.h"
#ifndef AIAO_REG_H
#define AIAO_REG_H

#define BUFFER_ADDR_ALIGN     128 /* 缓存基址128字节对齐 */
#define BUFFER_ADDR_SIZE_BASE 128 /* 缓存大小基数128字节 */

/******************************************************************************/
/*                                   AIO 寄存器定义                           */
/******************************************************************************/
/* AIO 模块寄存器基地址 */
#define AIAO_INT_ENA_REG    0x0U /* AIO模块中断使能寄存器。 */
#define AIAO_INT_STATUS_REG 0x4U /* AIO模块中断状态寄存器。 */
#define AIAO_INT_RAW_REG    0x8U /* AIO模块原始中断寄存器。 */

#define AIP_SWITCH_RX_BCLK    0x0028U /* AIAO I2S RX BCLK SWITCH 配置寄存器,内部bclk选择 */
#define AOP_SWITCH_TX_BCLK    0x002cU /* AIAO I2S TX BCLK SWITCH 配置寄存器,内部bclk选择 */
#define AIO_SOFT_RESET_STATUS 0x0030U /* AIO 软复位完成状态 */

#define AIAO_OUTSTANDING_REG 0x34U /* AIAO总线Outstanding寄存器。 */
#define AIAO_I2S_MUX_SEL_REG 0x60U /* I2S通路选择控制寄存器。 */

/* 对应 aip0/aop0,aip1/aop1,aop2 */
#define AIAO_I2S08_REG_CFG0  0x0140U /* I2S08 CRG 配置寄存器0 */
#define AIAO_I2S08_REG_CFG1  0x0144U /* I2S08 CRG 配置寄存器1 */
#define AIAO_I2S09_REG_CFG0  0x0148U /* I2S09 CRG 配置寄存器0 */
#define AIAO_I2S09_REG_CFG1  0x014cU /* I2S09 CRG 配置寄存器1 */
#define AIAO_I2S010_REG_CFG0 0x0148U /* I2S010 CRG 配置寄存器0 */
#define AIAO_I2S010_REG_CFG1 0x014cU /* I2S010 CRG 配置寄存器1 */

#define aip_i2s_reg_cfg0(n) ((td_u32)(0x0100 + 8 * (n)))
#define aip_i2s_reg_cfg1(n) ((td_u32)(0x0104 + 8 * (n)))
#define aop_i2s_reg_cfg0(n) ((td_u32)(0x0140 + 8 * (n)))
#define aop_i2s_reg_cfg1(n) ((td_u32)(0x0144 + 8 * (n)))

/* aip */
#define aip_inf_attri_reg(n)      ((td_u32)(0x1000 + 0x100 * (n))) /* 接收通道的接口属性设置寄存器。 */
#define aip_ctrl_reg(n)           ((td_u32)(0x1004 + 0x100 * (n))) /* 接收通道的控制寄存器。 */

/* #define aip_buff_saddr_reg_h(n) ((td_u32)(0x107c + 0x100 * (n)) 接收通道的DDR缓存起始地址寄存器(高32位)。各芯片有差异 */
#define aip_buff_saddr_reg(n)     ((td_u32)(0x1080 + 0x100 * (n))) /* 接收通道的DDR缓存起始地址寄存器(低32位)。 */
#define aip_buff_size_reg(n)      ((td_u32)(0x1084 + 0x100 * (n))) /* 接收通道的DDR缓存大小寄存器。 */
#define aip_buff_wptr_reg(n)      ((td_u32)(0x1088 + 0x100 * (n))) /* 接收通道的DDR缓存写地址寄存器。 */
#define aip_buff_rptr_reg(n)      ((td_u32)(0x108c + 0x100 * (n))) /* 接收通道的DDR缓存读地址寄存器。 */
#define aip_buff_alfull_th_reg(n) ((td_u32)(0x1090 + 0x100 * (n))) /* 接收通道的DDR缓存几乎满水线寄存器。 */
#define aip_trans_size_reg(n)     ((td_u32)(0x1094 + 0x100 * (n))) /* 接收通道的数据传输长度寄存器。 */
#define aip_wptr_tmp_reg(n)       ((td_u32)(0x1098 + 0x100 * (n))) /* 上报传输完成中断时，保存接收通道的写地址寄存器。 */

#define aip_int_ena_reg(n)    ((td_u32)(0x10A0 + 0x100 * (n))) /* 接收通道的中断使能寄存器。 */
#define aip_int_raw_reg(n)    ((td_u32)(0x10A4 + 0x100 * (n))) /* 接收通道的原始中断寄存器。 */
#define aip_int_status_reg(n) ((td_u32)(0x10A8 + 0x100 * (n))) /* 接收通道的中断状态寄存器。 */
#define aip_int_clr_reg(n)    ((td_u32)(0x10AC + 0x100 * (n))) /* 接收通道的中断清除寄存器。 */

/* aop */
#define aop_inf_attri_reg(m) ((td_u32)(0x2000 + 0x100 * (m))) /* 发送通道的接口属性设置寄存器。 */
#define aop_ctrl_reg(m)      ((td_u32)(0x2004 + 0x100 * (m))) /* 发送通道控制寄存器。 */

/* #define aop_buff_saddr_reg_h(m) ((td_u32)(0x207c + 0x100 * (m))) 发送通道的DDR缓存起始地址寄存器(高32位)。各芯片有差异 */
#define aop_buff_saddr_reg(m)      ((td_u32)(0x2080 + 0x100 * (m))) /* 发送通道的DDR缓存起始地址寄存器(低32位)。 */
#define aop_buff_size_reg(m)       ((td_u32)(0x2084 + 0x100 * (m))) /* 发送通道的DDR缓存大小寄存器。 */
#define aop_buff_wptr_reg(m)       ((td_u32)(0x2088 + 0x100 * (m))) /* 发送通道的DDR缓存写地址寄存器。 */
#define aop_buff_rptr_reg(m)       ((td_u32)(0x208C + 0x100 * (m))) /* 发送通道的DDR缓存读地址寄存器。 */
#define aop_buff_alempty_th_reg(m) ((td_u32)(0x2090 + 0x100 * (m))) /* 发送通道的DDR缓存几乎空水线寄存器。 */
#define aop_trans_size_reg(m)      ((td_u32)(0x2094 + 0x100 * (m))) /* 发送通道的数据传输长度寄存器。 */
#define aop_rptr_tmp_reg(m)        ((td_u32)(0x2098 + 0x100 * (m))) /* 上报传输完成中断时，保存发送通道的读地址寄存器。 */

#define aop_int_ena_reg(m)    ((td_u32)(0x20A0 + 0x100 * (m))) /* 发送通道的中断使能寄存器。 */
#define aop_int_raw_reg(m)    ((td_u32)(0x20A4 + 0x100 * (m))) /* 发送通道的原始中断寄存器。 */
#define aop_int_status_reg(m) ((td_u32)(0x20A8 + 0x100 * (m))) /* 发送通道的中断状态寄存器。 */
#define aop_int_clr_reg(m)    ((td_u32)(0x20AC + 0x100 * (m))) /* 发送通道的中断清除寄存器。 */

/******************************************************************************/
/*                 AIO register bit field define                              */
/******************************************************************************/
/************************************************************************/
#define AIAO_SPDIF_SUPPORT
#define AIAO_TX_DSP_SUPPORT

/***************************** macro definition ******************************/
// define the union u_aiao_sys_crg
typedef union {
    // define the struct bits
    struct {
        unsigned int aiao_cken : 1;        // [0]
        unsigned int reserved_0 : 3;       // [3..1]
        unsigned int aiao_srst_req : 1;    // [4]
        unsigned int reserved_1 : 3;       // [7..5]
        unsigned int aiao_clk_sel : 1;     // [8]
        unsigned int reserved_2 : 3;       // [11..9]
        unsigned int aiaoclk_skipcfg : 5;  // [16..12]
        unsigned int aiaoclk_loaden : 1;   // [17]
        unsigned int reserved_3 : 14;      // [31..18]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_s40_aiao_sys_crg;

// define the union u_aiao_int_ena
typedef union {
    // define the struct bits
    struct {
        unsigned int rx_ch0_int_ena : 1;       // [0]
        unsigned int rx_ch1_int_ena : 1;       // [1]
        unsigned int rx_ch2_int_ena : 1;       // [2]
        unsigned int rx_ch3_int_ena : 1;       // [3]
        unsigned int rx_ch4_int_ena : 1;       // [4]
        unsigned int rx_ch5_int_ena : 1;       // [5]
        unsigned int rx_ch6_int_ena : 1;       // [6]
        unsigned int rx_ch7_int_ena : 1;       // [7]
        unsigned int reserved_1 : 8;           // [15..8]
        unsigned int tx_ch0_int_ena : 1;       // [16]
        unsigned int tx_ch1_int_ena : 1;       // [17]
        unsigned int tx_ch2_int_ena : 1;       // [18]
        unsigned int tx_ch3_int_ena : 1;       // [19]
        unsigned int tx_ch4_int_ena : 1;       // [20]
        unsigned int tx_ch5_int_ena : 1;       // [21]
        unsigned int tx_ch6_int_ena : 1;       // [22]
        unsigned int tx_ch7_int_ena : 1;       // [23]
        unsigned int spdiftx_ch0_int_ena : 1;  // [24]
        unsigned int spdiftx_ch1_int_ena : 1;  // [25]
        unsigned int spdiftx_ch2_int_ena : 1;  // [26]
        unsigned int spdiftx_ch3_int_ena : 1;  // [27]
        unsigned int reserved_0 : 4;           // [31..28]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_aiao_int_ena;

// define the union u_aiao_int_status
typedef union {
    // define the struct bits
    struct {
        unsigned int rx_ch0_int_status : 1;       // [0]
        unsigned int rx_ch1_int_status : 1;       // [1]
        unsigned int rx_ch2_int_status : 1;       // [2]
        unsigned int rx_ch3_int_status : 1;       // [3]
        unsigned int rx_ch4_int_status : 1;       // [4]
        unsigned int rx_ch5_int_status : 1;       // [5]
        unsigned int rx_ch6_int_status : 1;       // [6]
        unsigned int rx_ch7_int_status : 1;       // [7]
        unsigned int reserved_1 : 8;              // [15..8]
        unsigned int tx_ch0_int_status : 1;       // [16]
        unsigned int tx_ch1_int_status : 1;       // [17]
        unsigned int tx_ch2_int_status : 1;       // [18]
        unsigned int tx_ch3_int_status : 1;       // [19]
        unsigned int tx_ch4_int_status : 1;       // [20]
        unsigned int tx_ch5_int_status : 1;       // [21]
        unsigned int tx_ch6_int_status : 1;       // [22]
        unsigned int tx_ch7_int_status : 1;       // [23]
        unsigned int spdiftx_ch0_int_status : 1;  // [24]
        unsigned int spdiftx_ch1_int_status : 1;  // [25]
        unsigned int spdiftx_ch2_int_status : 1;  // [26]
        unsigned int spdiftx_ch3_int_status : 1;  // [27]
        unsigned int reserved_0 : 4;              // [31..28]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_aiao_int_status;

// define the union u_aiao_int_raw
typedef union {
    // define the struct bits
    struct {
        unsigned int rx_ch0_int_raw : 1;       // [0]
        unsigned int rx_ch1_int_raw : 1;       // [1]
        unsigned int rx_ch2_int_raw : 1;       // [2]
        unsigned int rx_ch3_int_raw : 1;       // [3]
        unsigned int rx_ch4_int_raw : 1;       // [4]
        unsigned int rx_ch5_int_raw : 1;       // [5]
        unsigned int rx_ch6_int_raw : 1;       // [6]
        unsigned int rx_ch7_int_raw : 1;       // [7]
        unsigned int reserved_1 : 8;           // [15..8]
        unsigned int tx_ch0_int_raw : 1;       // [16]
        unsigned int tx_ch1_int_raw : 1;       // [17]
        unsigned int tx_ch2_int_raw : 1;       // [18]
        unsigned int tx_ch3_int_raw : 1;       // [19]
        unsigned int tx_ch4_int_raw : 1;       // [20]
        unsigned int tx_ch5_int_raw : 1;       // [21]
        unsigned int tx_ch6_int_raw : 1;       // [22]
        unsigned int tx_ch7_int_raw : 1;       // [23]
        unsigned int spdiftx_ch0_int_raw : 1;  // [24]
        unsigned int spdiftx_ch1_int_raw : 1;  // [25]
        unsigned int spdiftx_ch2_int_raw : 1;  // [26]
        unsigned int spdiftx_ch3_int_raw : 1;  // [27]
        unsigned int reserved_0 : 4;           // [31..28]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_aiao_int_raw;

// define the union u_hw_capability
typedef union {
    // define the struct bits
    struct {
        unsigned int rx_ch0_cap : 1;       // [0]
        unsigned int rx_ch1_cap : 1;       // [1]
        unsigned int rx_ch2_cap : 1;       // [2]
        unsigned int rx_ch3_cap : 1;       // [3]
        unsigned int rx_ch4_cap : 1;       // [4]
        unsigned int rx_ch5_cap : 1;       // [5]
        unsigned int rx_ch6_cap : 1;       // [6]
        unsigned int rx_ch7_cap : 1;       // [7]
        unsigned int reserved_1 : 8;       // [15..8]
        unsigned int tx_ch0_cap : 1;       // [16]
        unsigned int tx_ch1_cap : 1;       // [17]
        unsigned int tx_ch2_cap : 1;       // [18]
        unsigned int tx_ch3_cap : 1;       // [19]
        unsigned int tx_ch4_cap : 1;       // [20]
        unsigned int tx_ch5_cap : 1;       // [21]
        unsigned int tx_ch6_cap : 1;       // [22]
        unsigned int tx_ch7_cap : 1;       // [23]
        unsigned int spdiftx_ch0_cap : 1;  // [24]
        unsigned int spdiftx_ch1_cap : 1;  // [25]
        unsigned int spdiftx_ch2_cap : 1;  // [26]
        unsigned int spdiftx_ch3_cap : 1;  // [27]
        unsigned int reserved_0 : 4;       // [31..28]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_hw_capability;

// define the union u_vhb_outstanding
typedef union {
    // define the struct bits
    struct {
        unsigned int vhb_outst_num : 3;  // [0..2]
        unsigned int reserved_0 : 29;    // [31..3]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_vhb_outstanding;

// define the union u_iis_mux_sel
typedef union {
    // define the struct bits
    struct {
        unsigned int pad_tx_en : 1;             // [0]
        unsigned int audio_rx_bclk_sel : 1;     // [1]
        unsigned int rx_sd_sel : 1;             // [2]
        unsigned int audio_mclk_sel : 1;        // [3]
        unsigned int reserved_0 : 28;           // [31..4]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_iis_mux_sel;

// define the union u_spdif_tx_mux
typedef union {
    // define the struct bits
    struct {
        unsigned int spdif_tx_0_port_sel : 3;  // [2..0]
        unsigned int spdif_tx_0_port_en : 1;   // [3]
        unsigned int reserved_3 : 4;           // [7..4]
        unsigned int spdif_tx_1_port_sel : 3;  // [10..8]
        unsigned int spdif_tx_1_port_en : 1;   // [11]
        unsigned int reserved_2 : 4;           // [15..12]
        unsigned int spdif_tx_2_port_sel : 3;  // [18..16]
        unsigned int spdif_tx_2_port_en : 1;   // [19]
        unsigned int reserved_1 : 4;           // [23..20]
        unsigned int spdif_tx_3_port_sel : 3;  // [26..24]
        unsigned int spdif_tx_3_port_en : 1;   // [27]
        unsigned int reserved_0 : 4;           // [31..28]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdif_tx_mux;

// define the union u_i2s_crg_cfg0
typedef union {
    // define the struct bits
    struct {
        unsigned int aiao_mclk_div : 27;  // [26..0]
        unsigned int reserved_0 : 5;      // [31..27]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_i2s_crg_cfg0;

// define the union u_i2s_crg_cfg1
typedef union {
    // define the struct bits
    struct {
        unsigned int aiao_bclk_div : 4;       // [3..0]
        unsigned int aiao_fsclk_div : 3;      // [6..4]
        unsigned int reserved_1 : 1;          // [7]
        unsigned int aiao_cken : 1;           // [8]
        unsigned int aiao_srst_req : 1;       // [9]
        unsigned int aiao_bclk_oen : 1;       // [10]
        unsigned int aiao_bclk_sel : 1;       // [11]
        unsigned int aiao_bclkin_pctrl : 1;   // [12]
        unsigned int aiao_bclkout_pctrl : 1;  // [13]
        unsigned int aiao_bclk_en : 1;        // [14]
        unsigned int aiao_ws_en : 1;          // [15]
        unsigned int reserved_0 : 16;         // [31..16]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_i2s_crg_cfg1;

// define the union u_spdif_crg_cfg0
typedef union {
    // define the struct bits
    struct {
        unsigned int aiao_mclk_div_spdif : 27;  // [26..0]
        unsigned int reserved_0 : 5;            // [31..27]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdif_crg_cfg0;

// define the union u_spdif_crg_cfg1_03
typedef union {
    // define the struct bits
    struct {
        unsigned int aiao_bclk_div_spdif : 4;   // [3..0]
        unsigned int aiao_fsclk_div_spdif : 3;  // [6..4]
        unsigned int reserved_1 : 1;            // [7]
        unsigned int aiao_cken_spdif : 1;       // [8]
        unsigned int aiao_srst_req_spdif : 1;   // [9]
        unsigned int reserved_0 : 22;           // [31..10]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdif_crg_cfg1;

// define the union u_aiao_rxswitch_cfg
typedef union {
    // define the struct bits
    struct {
        unsigned int inner_bclk_ws_sel_rx_00 : 4;  // [3..0]
        unsigned int inner_bclk_ws_sel_rx_01 : 4;  // [7..4]
        unsigned int inner_bclk_ws_sel_rx_02 : 4;  // [11..8]
        unsigned int inner_bclk_ws_sel_rx_03 : 4;  // [15..12]
        unsigned int inner_bclk_ws_sel_rx_04 : 4;  // [19..16]
        unsigned int inner_bclk_ws_sel_rx_05 : 4;  // [23..20]
        unsigned int inner_bclk_ws_sel_rx_06 : 4;  // [27..24]
        unsigned int inner_bclk_ws_sel_rx_07 : 4;  // [31..28]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_aiao_switch_rx_bclk;

// define the union u_aiao_txswitch_cfg
typedef union {
    // define the struct bits
    struct {
        unsigned int inner_bclk_ws_sel_tx_00 : 4;  // [3..0]
        unsigned int inner_bclk_ws_sel_tx_01 : 4;  // [7..4]
        unsigned int inner_bclk_ws_sel_tx_02 : 4;  // [11..8]
        unsigned int inner_bclk_ws_sel_tx_03 : 4;  // [15..12]
        unsigned int inner_bclk_ws_sel_tx_04 : 4;  // [19..16]
        unsigned int inner_bclk_ws_sel_tx_05 : 4;  // [23..20]
        unsigned int inner_bclk_ws_sel_tx_06 : 4;  // [27..24]
        unsigned int inner_bclk_ws_sel_tx_07 : 4;  // [31..28]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_aiao_switch_tx_bclk;

// define the union u_rx_if_attri
typedef union {
    // define the struct bits
    struct {
        unsigned int rx_mode : 2;           // [1..0]
        unsigned int rx_i2s_precision : 2;  // [3..2]
        unsigned int rx_ch_num : 3;         // [6..4]
        unsigned int rx_multislot_en : 1;   // [7]
        unsigned int rx_sd_offset : 8;      // [15..8]
        unsigned int rx_trackmode : 3;      // [18..16]
        unsigned int reserved_0 : 1;        // [19]
        unsigned int rx_sd_source_sel : 4;  // [23..20]
        unsigned int rx_sd0_sel : 2;        // [25..24]
        unsigned int rx_sd1_sel : 2;        // [27..26]
        unsigned int rx_sd2_sel : 2;        // [29..28]
        unsigned int rx_sd3_sel : 2;        // [31..30]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_rx_if_attri;

// define the union u_rx_dsp_ctrl
typedef union {
    // define the struct bits
    struct {
        unsigned int mute_en : 1;          // [0]
        unsigned int mute_fade_en : 1;     // [1]
        unsigned int pause_en : 1;         // [2]
        unsigned int pause_fade_en : 1;    // [3]
        unsigned int reserved_3 : 4;       // [7..4]
        unsigned int volume : 7;           // [14..8]
        unsigned int reserved_2 : 1;       // [15]
        unsigned int fade_in_rate : 4;     // [19..16]
        unsigned int fade_out_rate : 4;    // [23..20]
        unsigned int reserved_1 : 3;       // [26..24]
        unsigned int bypass_en : 1;        // [27]
        unsigned int rx_enable : 1;        // [28]
        unsigned int rx_disable_done : 1;  // [29]
        unsigned int reserved_0 : 2;       // [31..30]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_rx_dsp_ctrl;

// define the union u_rx_ws_cnt
typedef union {
    // define the struct bits
    struct {
        unsigned int ws_count : 24;   // [23..0]
        unsigned int reserved_0 : 8;  // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_rx_ws_cnt;

// define the union u_rx_bclk_cnt
typedef union {
    // define the struct bits
    struct {
        unsigned int bclk_count : 24;  // [23..0]
        unsigned int reserved_0 : 8;   // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_rx_bclk_cnt;

// define the union u_rx_buff_size
typedef union {
    // define the struct bits
    struct {
        unsigned int rx_buff_size : 24;  // [23..0]
        unsigned int reserved_0 : 8;     // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_rx_buff_size;

// define the union u_rx_buff_wptr
typedef union {
    // define the struct bits
    struct {
        unsigned int rx_buff_wptr : 24;  // [23..0]
        unsigned int reserved_0 : 8;     // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_rx_buff_wptr;

// define the union u_rx_buff_rptr
typedef union {
    // define the struct bits
    struct {
        unsigned int rx_buff_rptr : 24;  // [23..0]
        unsigned int reserved_0 : 8;     // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_rx_buff_rptr;

// define the union u_rx_buff_alfull_th
typedef union {
    // define the struct bits
    struct {
        unsigned int rx_buff_alfull_th : 24;  // [23..0]
        unsigned int reserved_0 : 8;          // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_rx_buff_alfull_th;

// define the union u_rx_trans_size
typedef union {
    // define the struct bits
    struct {
        unsigned int rx_trans_size : 24;  // [23..0]
        unsigned int reserved_0 : 8;      // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_rx_trans_size;

// define the union u_rx_wptr_tmp
typedef union {
    // define the struct bits
    struct {
        unsigned int rx_wptr_tmp : 24;  // [23..0]
        unsigned int reserved_0 : 8;    // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_rx_wptr_tmp;

// define the union u_rx_int_ena
typedef union {
    // define the struct bits
    struct {
        unsigned int rx_trans_int_ena : 1;       // [0]
        unsigned int rx_full_int_ena : 1;        // [1]
        unsigned int rx_alfull_int_ena : 1;      // [2]
        unsigned int rx_bfifo_full_int_ena : 1;  // [3]
        unsigned int rx_ififo_full_int_ena : 1;  // [4]
        unsigned int rx_stop_int_ena : 1;        // [5]
        unsigned int reserved_1 : 1;             // [6]
        unsigned int rx_fifo_lost_int_ena : 1;   // [7]
        unsigned int reserved_0 : 24;            // [31..8]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_rx_int_ena;

// define the union u_rx_int_raw
typedef union {
    // define the struct bits
    struct {
        unsigned int rx_trans_int_raw : 1;       // [0]
        unsigned int rx_full_int_raw : 1;        // [1]
        unsigned int rx_alfull_int_raw : 1;      // [2]
        unsigned int rx_bfifo_full_int_raw : 1;  // [3]
        unsigned int rx_ififo_full_int_raw : 1;  // [4]
        unsigned int rx_stop_int_raw : 1;        // [5]
        unsigned int reserved_1 : 1;             // [6]
        unsigned int rx_fifo_lost_int_raw : 1;   // [7]
        unsigned int reserved_0 : 24;            // [31..8]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_rx_int_raw;

// define the union u_rx_int_status
typedef union {
    // define the struct bits
    struct {
        unsigned int rx_trans_int_status : 1;       // [0]
        unsigned int rx_full_int_status : 1;        // [1]
        unsigned int rx_alfull_int_status : 1;      // [2]
        unsigned int rx_bfifo_full_int_status : 1;  // [3]
        unsigned int rx_ififo_full_int_status : 1;  // [4]
        unsigned int rx_stop_int_status : 1;        // [5]
        unsigned int reserved_1 : 1;                // [6]
        unsigned int rx_fifo_lost_int_status : 1;   // [7]
        unsigned int reserved_0 : 24;               // [31..8]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_rx_int_status;

// define the union u_rx_int_clr
typedef union {
    // define the struct bits
    struct {
        unsigned int rx_trans_int_clear : 1;       // [0]
        unsigned int rx_full_int_clear : 1;        // [1]
        unsigned int rx_alfull_int_clear : 1;      // [2]
        unsigned int rx_bfifo_full_int_clear : 1;  // [3]
        unsigned int rx_ififo_full_int_clear : 1;  // [4]
        unsigned int rx_stop_int_clear : 1;        // [5]
        unsigned int reserved_1 : 1;               // [6]
        unsigned int rx_fifo_lost_int_clear : 1;   // [7]
        unsigned int reserved_0 : 24;              // [31..8]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_rx_int_clr;

// define the union u_tx_if_attri
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_mode : 2;            // [1..0]
        unsigned int tx_i2s_precision : 2;   // [3..2]
        unsigned int tx_ch_num : 2;          // [5..4]
        unsigned int tx_underflow_ctrl : 1;  // [6]
        unsigned int tx_multislot_en : 1;    // [7]
        unsigned int tx_sd_offset : 8;       // [15..8]
        unsigned int tx_trackmode : 3;       // [18..16]
        unsigned int reserved_0 : 1;         // [19]
        unsigned int tx_sd_source_sel : 4;   // [23..20]
        unsigned int tx_sd0_sel : 2;         // [25..24]
        unsigned int tx_sd1_sel : 2;         // [27..26]
        unsigned int tx_sd2_sel : 2;         // [29..28]
        unsigned int tx_sd3_sel : 2;         // [31..30]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_if_attri;

// define the union u_tx_dsp_ctrl
typedef union {
    // define the struct bits
    struct {
        unsigned int mute_en : 1;          // [0]
        unsigned int mute_fade_en : 1;     // [1]
        unsigned int reserved_3 : 6;       // [7..2]
        unsigned int volume : 7;           // [14..8]
        unsigned int reserved_2 : 1;       // [15]
        unsigned int fade_in_rate : 4;     // [19..16]
        unsigned int fade_out_rate : 4;    // [23..20]
        unsigned int reserved_1 : 3;       // [26..24]
        unsigned int bypass_en : 1;        // [27]
        unsigned int tx_enable : 1;        // [28]
        unsigned int tx_disable_done : 1;  // [29]
        unsigned int reserved_0 : 2;       // [31..30]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_dsp_ctrl;

// define the union u_tx_dsp_status
typedef union {
    // define the struct bits
    struct {
        unsigned int mute_fade_vol : 7;  // [6..0]
        unsigned int reserved_0 : 25;    // [31..7]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_dsp_status;

// define the union u_tx_ws_cnt
typedef union {
    // define the struct bits
    struct {
        unsigned int ws_count : 24;   // [23..0]
        unsigned int reserved_0 : 8;  // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_ws_cnt;

// define the union u_tx_bclk_cnt
typedef union {
    // define the struct bits
    struct {
        unsigned int bclk_count : 24;  // [23..0]
        unsigned int reserved_0 : 8;   // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_bclk_cnt;

// define the union u_tx_buff_size
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_buff_size : 24;  // [23..0]
        unsigned int reserved_0 : 8;     // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_buff_size;

// define the union u_tx_buff_wptr
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_buff_wptr : 24;  // [23..0]
        unsigned int reserved_0 : 8;     // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_buff_wptr;

// define the union u_tx_buff_rptr
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_buff_rptr : 24;  // [23..0]
        unsigned int reserved_0 : 8;     // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_buff_rptr;

// define the union u_tx_buff_alempty_th
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_buff_alempty_th : 24;  // [23..0]
        unsigned int reserved_0 : 8;           // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_buff_alempty_th;

// define the union u_tx_trans_size
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_trans_size : 24;  // [23..0]
        unsigned int reserved_0 : 8;      // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_trans_size;

// define the union u_tx_rptr_tmp
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_rptr_tmp : 24;  // [23..0]
        unsigned int reserved_0 : 8;    // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_rptr_tmp;

// define the union u_tx_int_ena
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_trans_int_ena : 1;        // [0]
        unsigned int tx_empty_int_ena : 1;        // [1]
        unsigned int tx_alempty_int_ena : 1;      // [2]
        unsigned int tx_bfifo_empty_int_ena : 1;  // [3]
        unsigned int tx_ififo_empty_int_ena : 1;  // [4]
        unsigned int tx_stop_int_ena : 1;         // [5]
        unsigned int tx_mfade_int_ena : 1;        // [6]
        unsigned int tx_dat_break_int_ena : 1;    // [7]
        unsigned int reserved_0 : 24;             // [31..8]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_int_ena;

// define the union u_tx_int_raw
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_trans_int_raw : 1;        // [0]
        unsigned int tx_empty_int_raw : 1;        // [1]
        unsigned int tx_alempty_int_raw : 1;      // [2]
        unsigned int tx_bfifo_empty_int_raw : 1;  // [3]
        unsigned int tx_ififo_empty_int_raw : 1;  // [4]
        unsigned int tx_stop_int_raw : 1;         // [5]
        unsigned int tx_mfade_int_raw : 1;        // [6]
        unsigned int tx_dat_break_int_raw : 1;    // [7]
        unsigned int reserved_0 : 24;             // [31..8]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_int_raw;

// define the union u_tx_int_status
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_trans_int_status : 1;        // [0]
        unsigned int tx_empty_int_status : 1;        // [1]
        unsigned int tx_alempty_int_status : 1;      // [2]
        unsigned int tx_bfifo_empty_int_status : 1;  // [3]
        unsigned int tx_ififo_empty_int_status : 1;  // [4]
        unsigned int tx_stop_int_status : 1;         // [5]
        unsigned int tx_mfade_int_status : 1;        // [6]
        unsigned int tx_dat_break_int_status : 1;    // [7]
        unsigned int reserved_0 : 24;                // [31..8]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_int_status;

// define the union u_tx_int_clr
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_trans_int_clear : 1;        // [0]
        unsigned int tx_empty_int_clear : 1;        // [1]
        unsigned int tx_alempty_int_clear : 1;      // [2]
        unsigned int tx_bfifo_empty_int_clear : 1;  // [3]
        unsigned int tx_ififo_empty_int_clear : 1;  // [4]
        unsigned int tx_stop_int_clear : 1;         // [5]
        unsigned int tx_mfade_int_clear : 1;        // [6]
        unsigned int tx_dat_break_int_clear : 1;    // [7]
        unsigned int reserved_0 : 24;               // [31..8]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_int_clr;

// define the union u_spdiftx_if_attri
typedef union {
    // define the struct bits
    struct {
        unsigned int reserved_2 : 2;        // [1..0]
        unsigned int tx_i2s_precision : 2;  // [3..2]
        unsigned int tx_ch_num : 2;         // [5..4]
        unsigned int reserved_1 : 10;       // [15..6]
        unsigned int tx_trackmode : 3;      // [18..16]
        unsigned int reserved_0 : 13;       // [31..19]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdiftx_if_attri;

// define the union u_spdiftx_dsp_status
typedef union {
    // define the struct bits
    struct {
        unsigned int mute_fade_vol : 7;  // [6..0]
        unsigned int reserved_0 : 25;    // [31..7]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdiftx_dsp_status;

// define the union u_spdiftx_ws_cnt
typedef union {
    // define the struct bits
    struct {
        unsigned int ws_count : 24;   // [23..0]
        unsigned int reserved_0 : 8;  // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdiftx_ws_cnt;

// define the union u_spdiftx_bclk_cnt
typedef union {
    // define the struct bits
    struct {
        unsigned int bclk_count : 24;  // [23..0]
        unsigned int reserved_0 : 8;   // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdiftx_bclk_cnt;

// define the union u_spdiftx_buff_size
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_buff_size : 24;  // [23..0]
        unsigned int reserved_0 : 8;     // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdiftx_buff_size;

// define the union u_spdiftx_buff_wptr
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_buff_wptr : 24;  // [23..0]
        unsigned int reserved_0 : 8;     // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdiftx_buff_wptr;

// define the union u_spdiftx_buff_rptr
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_buff_rptr : 24;  // [23..0]
        unsigned int reserved_0 : 8;     // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdiftx_buff_rptr;

// define the union u_spdiftx_buff_alempty_th
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_buff_alempty_th : 24;  // [23..0]
        unsigned int reserved_0 : 8;           // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdiftx_buff_alempty_th;

// define the union u_spdiftx_trans_size
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_trans_size : 24;  // [23..0]
        unsigned int reserved_0 : 8;      // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdiftx_trans_size;

// define the union u_spdiftx_rptr_tmp
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_rptr_tmp : 24;  // [23..0]
        unsigned int reserved_0 : 8;    // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdiftx_rptr_tmp;

// define the union u_spdiftx_int_ena
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_trans_int_ena : 1;        // [0]
        unsigned int tx_empty_int_ena : 1;        // [1]
        unsigned int tx_alempty_int_ena : 1;      // [2]
        unsigned int tx_bfifo_empty_int_ena : 1;  // [3]
        unsigned int tx_ififo_empty_int_ena : 1;  // [4]
        unsigned int tx_stop_int_ena : 1;         // [5]
        unsigned int tx_mfade_int_ena : 1;        // [6]
        unsigned int tx_dat_break_int_ena : 1;    // [7]
        unsigned int reserved_0 : 24;             // [31..8]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdiftx_int_ena;

// define the union u_spdiftx_int_raw
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_trans_int_raw : 1;        // [0]
        unsigned int tx_empty_int_raw : 1;        // [1]
        unsigned int tx_alempty_int_raw : 1;      // [2]
        unsigned int tx_bfifo_empty_int_raw : 1;  // [3]
        unsigned int tx_ififo_empty_int_raw : 1;  // [4]
        unsigned int tx_stop_int_raw : 1;         // [5]
        unsigned int tx_mfade_int_raw : 1;        // [6]
        unsigned int tx_dat_break_int_raw : 1;    // [7]
        unsigned int reserved_0 : 24;             // [31..8]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdiftx_int_raw;

// define the union u_spdiftx_int_status
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_trans_int_status : 1;        // [0]
        unsigned int tx_empty_int_status : 1;        // [1]
        unsigned int tx_alempty_int_status : 1;      // [2]
        unsigned int tx_bfifo_empty_int_status : 1;  // [3]
        unsigned int tx_ififo_empty_int_status : 1;  // [4]
        unsigned int tx_stop_int_status : 1;         // [5]
        unsigned int tx_mfade_int_status : 1;        // [6]
        unsigned int tx_dat_break_int_status : 1;    // [7]
        unsigned int reserved_0 : 24;                // [31..8]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdiftx_int_status;

// define the union u_spdiftx_int_clr
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_trans_int_clear : 1;        // [0]
        unsigned int tx_empty_int_clear : 1;        // [1]
        unsigned int tx_alempty_int_clear : 1;      // [2]
        unsigned int tx_bfifo_empty_int_clear : 1;  // [3]
        unsigned int tx_ififo_empty_int_clear : 1;  // [4]
        unsigned int tx_stop_int_clear : 1;         // [5]
        unsigned int tx_mfade_int_clear : 1;        // [6]
        unsigned int tx_dat_break_int_clear : 1;    // [7]
        unsigned int reserved_0 : 24;               // [31..8]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdiftx_int_clr;

// define the union u_spdif_ctrl
typedef union {
    // define the struct bits
    struct {
        unsigned int spdif_en : 1;     // [0]
        unsigned int reserved_0 : 31;  // [31..1]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdif_ctrl;

// define the union u_spdif_config
typedef union {
    // define the struct bits
    struct {
        unsigned int txdata_en : 1;    // [0]
        unsigned int mode : 2;         // [2..1]
        unsigned int reserved_0 : 29;  // [31..3]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdif_config;

// define the union u_spdif_intrupt
typedef union {
    // define the struct bits
    struct {
        unsigned int lfifo_empty_intr : 1;   // [0]
        unsigned int rfifo_empty_intr : 1;   // [1]
        unsigned int lfifo_single_intr : 1;  // [2]
        unsigned int rfifo_single_intr : 1;  // [3]
        unsigned int reserved_0 : 28;        // [31..4]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdif_intrupt;

// define the union u_spdif_intr_mask
typedef union {
    // define the struct bits
    struct {
        unsigned int lfifo_empty_mask : 1;    // [0]
        unsigned int rfifo_empty__mask : 1;   // [1]
        unsigned int lfifo_single__mask : 1;  // [2]
        unsigned int rfifo_single__mask : 1;  // [3]
        unsigned int reserved_0 : 28;         // [31..4]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdif_intr_mask;

// define the union u_spdif_intr_state
typedef union {
    // define the struct bits
    struct {
        unsigned int lfifo_empty_state : 1;   // [0]
        unsigned int rfifo_empty_state : 1;   // [1]
        unsigned int lfifo_single_state : 1;  // [2]
        unsigned int rfifo_single_state : 1;  // [3]
        unsigned int reserved_0 : 28;         // [31..4]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdif_intr_state;

// define the union u_spdif_ch_status1
typedef union {
    // define the struct bits
    struct {
        unsigned int l_use_a : 1;      // [0]
        unsigned int l_use_b : 1;      // [1]
        unsigned int l_use_c : 1;      // [2]
        unsigned int l_use_d : 3;      // [5..3]
        unsigned int l_use_mode : 2;   // [7..6]
        unsigned int r_use_a : 1;      // [8]
        unsigned int r_use_b : 1;      // [9]
        unsigned int r_use_c : 1;      // [10]
        unsigned int r_use_d : 3;      // [13..11]
        unsigned int r_use_mode : 2;   // [15..14]
        unsigned int reserved_0 : 16;  // [31..16]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdif_ch_status1;

// define the union u_spdif_ch_status2
typedef union {
    // define the struct bits
    struct {
        unsigned int l_category_code : 8;  // [7..0]
        unsigned int r_category_code : 8;  // [15..8]
        unsigned int reserved_0 : 16;      // [31..16]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdif_ch_status2;

// define the union u_spdif_ch_status3
typedef union {
    // define the struct bits
    struct {
        unsigned int l_src_nb : 4;     // [3..0]
        unsigned int l_ch_nb : 4;      // [7..4]
        unsigned int r_src_nb : 4;     // [11..8]
        unsigned int r_ch_nb : 4;      // [15..12]
        unsigned int reserved_0 : 16;  // [31..16]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdif_ch_status3;

// define the union u_spdif_ch_status4
typedef union {
    // define the struct bits
    struct {
        unsigned int l_freq : 4;          // [3..0]
        unsigned int l_clk_accuracy : 2;  // [5..4]
        unsigned int reserved_1 : 2;      // [7..6]
        unsigned int r_freq : 4;          // [11..8]
        unsigned int r_clk_accuracy : 2;  // [13..12]
        unsigned int reserved_0 : 18;     // [31..14]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdif_ch_status4;

// define the union u_spdif_ch_status5
typedef union {
    // define the struct bits
    struct {
        unsigned int l_bit_max : 1;    // [0]
        unsigned int l_bit_width : 3;  // [3..1]
        unsigned int l_org_freq : 4;   // [7..4]
        unsigned int r_bit_max : 1;    // [8]
        unsigned int r_bit_width : 3;  // [11..9]
        unsigned int r_org_freq : 4;   // [15..12]
        unsigned int reserved_0 : 16;  // [31..16]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_spdif_ch_status5;

// define the union u_aio_crg35
typedef union {
    // define the struct bits
    struct {
        unsigned int aio_hrst_req : 1;  // [0]
        unsigned int aio_hcken : 1;     // [1]
        unsigned int aio_cksel : 2;     // [3..2]
        unsigned int reserved : 28;     // [31..4]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_aio_crg35;

#endif /* AIAO_REG_H */
