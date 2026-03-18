/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Create: 2023-2-7
 */
#ifndef HCLGE_SERDES_COMMON_H
#define HCLGE_SERDES_COMMON_H

#include <linux/types.h>
#include "hclge_main.h"

#ifndef DEFINE_HNS_LLT
#define STATIC static
#else
#define STATIC
#endif

/*
 * Hilink PAM mode
 */
typedef enum {
    HILINK_PAM_MODE_NRZ = 0,
    HILINK_PAM_MODE_PAM2 = HILINK_PAM_MODE_NRZ,
    HILINK_PAM_MODE_PAM4 = 1,
    HILINK_PAM_MODE_PAM8,
    HILINK_PAM_MODE_END
} HILINK_PAM_MODE_E;

/*
 * Hilink init type
 */
typedef enum {
    HILINK_INIT_SS_EN = 0x1,       /* init ss only */
    HILINK_INIT_CS_DS_EN = 0x2,    /* init cs && ds only, M1 use */
    HILINK_INIT_POWER_EN = 0x4,    /* powerup only */
    HILINK_POWER_DOWN_BEFORE_INIT = 0x8,    /* power down  macro before init */

    HILINK_INIT_WITHOUT_POWER =
        HILINK_INIT_SS_EN | HILINK_INIT_CS_DS_EN, /* init macro without powerup, M0 use */
    HILINK_INIT_NORMAL =
        HILINK_INIT_SS_EN | HILINK_INIT_CS_DS_EN | HILINK_INIT_POWER_EN | HILINK_POWER_DOWN_BEFORE_INIT,
} HILINK_INIT_TYPE_E;

/**
 * Chip used mode
 */
typedef enum {
    HILINK_SERDES_NORMAL_MODE = 0x01,
    HILINK_SERDES_REPEATER_MODE = 0x02,
    HILINK_SERDES_RETIMER_MODE = 0x04,

    HILINK_SERDES_2PLL_MODE = 0x08,
    HILINK_SERDES_FASTPI_MODE = 0x10,
    HILINK_SERDES_COMPATIBLT_MODE = 0x20,

    HILINK_SERDES_REPEATER_WITH_2PLL_MODE = HILINK_SERDES_REPEATER_MODE | HILINK_SERDES_2PLL_MODE,
    HILINK_SERDES_REPEATER_WITH_FASTPI_MODE = HILINK_SERDES_REPEATER_MODE | HILINK_SERDES_FASTPI_MODE,
    HILINK_SERDES_RETIMER_WITH_2PLL_MODE = HILINK_SERDES_RETIMER_MODE | HILINK_SERDES_2PLL_MODE,
    HILINK_SERDES_RETIMER_WITH_FASTPI_MODE = HILINK_SERDES_RETIMER_MODE | HILINK_SERDES_FASTPI_MODE,
    HILINK_SERDES_RETIMER_WITH_COMPATIBLT_MODE = HILINK_SERDES_RETIMER_MODE | HILINK_SERDES_COMPATIBLT_MODE
} HILINK_SERDES_APPLICATION_E;

/*
 * Hilink used mode
 */
typedef enum {
    HILINK_USE_MODE_NORMAL = 0,
    HILINK_USE_MODE_PCIE,
    HILINK_USE_MODE_SATA,
    HILINK_USE_MODE_SAS,
    HILINK_USE_MODE_HCCS,
    HILINK_USE_MODE_ETH,
    HILINK_USE_MODE_FC,
    HILINK_USE_MODE_CXL,
    HILINK_USE_MODE_ROH,
    HILINK_USE_MODE_ETH_ROH, /* lane0~3:roh, lane4~7:eth */
    HILINK_USE_MODE_ROH_ETH, /* lane0~3:eth, lane4~7:roh */
    HILINK_USE_MODE_END
} HILINK_USE_MODE_E;

/*
 * Hilink SSC Type
 */
typedef enum {
    HILINK_NO_SSC = 0,
    HILINK_SINGLE_SSC,
    HILINK_MULTI_SSC_FROM_INSIDE,
    HILINK_MULTI_SSC_FROM_SOUTH,
    HILINK_MULTI_SSC_FROM_NORTH,
    HILINK_MULTI_SSC_FROM_WEST,
    HILINK_MULTI_SSC_FROM_EAST,
    HILINK_SSC_TYPE_END
} HILINK_SSC_TYPE_E;

/*
 * Serdes error code
 */
#define SERDES_OK                                   0
#define SERDES_ERROR                                1
#define SERDES_ERROR_PARA                           2
#define SERDES_NOT_SUPPORT                          3
#define SERDES_TIMEOUT                              5
#define SERDES_NULL                                 14

typedef enum {
    SERDES_TX = 0,
    SERDES_RX,
    SERDES_DS,
    SERDES_MODULE_END
} SERDES_MODULE_E;

/*
 * Serdes power type define
 */
typedef enum {
    SERDES_POWER_DOWN = 0,           /* power down */
    SERDES_POWER_UP,                 /* power up */
    SERDES_POWER_DOWN_WITH_DSCLK,    /* power down wiclk */
    SERDES_POWER_UP_WITH_DSCLK,      /* power up wiclk */
    SERDES_POWER_DOWN_WITHOUT_DSCLK, /* power down woclk */
    SERDES_POWER_UP_WITHOUT_DSCLK,   /* power up woclk */
    SERDES_POWER_END
} SERDES_POWER_TYPE_E;

/* CS cfg info */
typedef struct {
    u8 init_type;            /* init_type: value is specified by HILINK_INIT_TYPE_E */
    u8 serdes_rate;          /* rate */
    u8 ref_clock_sel;        /* refclk_id used by cs: 0-refclk0 or 1-refclk1 */
    HILINK_SSC_TYPE_E ssc_type;
    HILINK_USE_MODE_E use_mode;
} HILINK_CS_USE_S;

/* @brief: Maximum number of cs or pll used in a macro */
#define HILINK_SERDES_MAX_CS_NUM       2
#define HILINK_SERDES_PLL_ID_0 0
#define HILINK_SERDES_PLL_ID_1 1
/* @brief: Maximum number of ds used in a macro */
#define HILINK_SERDES_MAX_DS_NUM       8

/*
 * TX parameters
 * If the HiLink does not support a parameter, set this parameter to 0.
 */
typedef struct {
    signed char fir_pre3;      /* -16~15 */
    signed char fir_pre2;      /* -16~15 */
    signed char fir_pre1;      /* -30~30 */
    unsigned char fir_main;     /*   0~63 */
    signed char fir_post1;     /* -30~30 */
    signed char fir_post2;     /* -16~15 */
    unsigned char swing;        /* H60: hswing_en; Other: swing */
    signed char rsv;
} HILINK_TX_PARA_S;

/*
 * RX CTLE parameters
 * The parameter type and length can be flexibly specified for different hilinks.
 */
#define HILINK_SERDES_RX_PARA_COUNT 18
typedef struct {
    unsigned char data[HILINK_SERDES_RX_PARA_COUNT];
} HILINK_RX_PARA_S;

/* DS cfg info */
#define HILINK_DS_RSV_BYTE_NUM 2
typedef struct {
    u8 tx_rate;            /* rate */
    u8 rx_rate;            /* rate */
    u8 master_en;          /* master/slave mode select: 0-slave or 1-master */
    u8 tx_cs_sel;          /* pll_id used by tx */

    u8 rx_cs_sel;          /* pll_id used by tx */
    u8 tx_pn;
    u8 rx_pn;
    u8 tx_bitorder;

    u8 rx_bitorder;
    u8 rxtermfloating;
    u8 rsv[HILINK_DS_RSV_BYTE_NUM];
    HILINK_TX_PARA_S tx_cfg;
    HILINK_RX_PARA_S rx_cfg;
    HILINK_PAM_MODE_E pam_mode;
    HILINK_USE_MODE_E use_mode;
} HILINK_DS_USE_S;

#define HILINK_MACRO_INIT_RSV_BYTE_NUM 3
/* Init data for internal use */
typedef struct {
    u32 system_clk_khz;
    HILINK_CS_USE_S cs_info[HILINK_SERDES_MAX_CS_NUM];
    HILINK_DS_USE_S ds_info[HILINK_SERDES_MAX_DS_NUM];
    u8 app_mode;
    u8 rsv[HILINK_MACRO_INIT_RSV_BYTE_NUM]; /* reserved parameter: flexibly specified based on
                                            the application scenario of the chip. */
} HILINK_MACRO_INIT_S;

typedef struct tagH60_FREQ_CS_CFG {
    u32 ref_clk_khz      : 20;
    u32 ref_clk_sel      : 12;

    u32 hs_clk_mhz       : 20;
    u32 m_div            : 2;
    u32 fpll_mode        : 2;
    u32 ka_div           : 4;
    u32 kb_div           : 4;

    u32 vco_freq_khz     : 16;
    u32 na_div           : 2;
    u32 nb_div_p         : 6;
    u32 nb_div_s         : 4;
    u32 sdmdiv4divider   : 4;

    u32 fcw_hex;

    u32 coreclkdiv_sel     : 12;
    u32 coreclkdiv_double  : 4;
    u32 coreclk2digdiv     : 4;
    u32 jcom               : 4;
    u32 j40b               : 4;
    u32 j32b               : 4;
} H60_FREQ_CS_CFG;

typedef struct tagH60_FREQ_CFG {
    u32 data_rate;
    /* cs_cfg */
    H60_FREQ_CS_CFG cs_cfg;
    u32 pam4mode        : 8;
    u32 rate_mode_div   : 8;
    u32 rx_bus_sel      : 8;
    u32 tx_bus_sel      : 8;
} H60_FREQ_CFG;

typedef struct tagCHIP_INFO {
    u8 chip_id;
    u8 die_id;
    u8 macro_id;
    u8 csds_id;             /* cs_id or ds_id, not used at the same time */
    u8 ds_num;
    u8 ds_mask;
    u8 connect_type;        /* 0: pcie connect, 1: hccs connect */
} CHIP_INFO_S;

/* system clk */
#define CHIP_SYSTEM_CLK_KHZ                          500000U

#define H60_MACRO_OFFSET_0 0
#define H60_MACRO_OFFSET_1 1
#define H60_DEFAULT_SERDES_LANE_NUM 4
#define H60_DS_0         0
#define H60_DS_1         1
#define H60_DS_2         2
#define H60_DS_3         3
#define H60_DS_INIT_MASK 0xf

/* data mode */
#define H60_PAM4_MODE_EN   1
#define H60_NRZ_MODE_EN    0

H60_FREQ_CFG *hisds_get_freq_data_h60(u32 data_rate);
void h60_read_reg_field(struct hclge_dev *hdev, u32 reg, u32 mask, u32 offset, u32 *val);
void h60_write_reg_field(struct hclge_dev *hdev, u32 reg, u32 mask, u32 offset, u32 val);
u32 h60_wait_reg_value(struct hclge_dev *hdev, const CHIP_INFO_S *chip_info, u32 reg, u32 mask, u32 offset,
                       u32 expect_value, u32 max_wait_times, u32 delay_per_loop_us);
u32 h60_init_reg_write_ensure(struct hclge_dev *hdev, const CHIP_INFO_S *chip_info, u32 reg, u32 mask, u32 offset,
                              u32 expect_value);
void hisds_get_hsclk_h60(struct hclge_dev *hdev, const CHIP_INFO_S *chip_info, u32 pll_id, u32 *hsclk);
void hisds_get_data_rate_h60(struct hclge_dev *hdev, const CHIP_INFO_S *chip_info, SERDES_MODULE_E module,
    u32 *data_rate_mhz);
bool macro0_in_use(struct hclge_dev *hdev);
bool macro1_in_use(struct hclge_dev *hdev);
void hclge_init_chip_info(struct hclge_dev *hdev, CHIP_INFO_S *chip_info, u8 macro_id);
#endif
