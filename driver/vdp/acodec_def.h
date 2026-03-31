/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: RemixV100 audio codec register define
 * Author: Hisilicon multimedia software group
 * Create: 2022-09-10
 */

#ifndef ACODEC_DEF_H
#define ACODEC_DEF_H

#define MEDIA_SUBCTRL_REGS_BASE   0x0400130000
#define ACODEC_REGS_BASE 0x0401050000

#define ACODEC_MAX_REG_SIZE 0x1000

/* acodec analog control register */
#define ACODEC_ANA_CTRL0_ADDR 0x00
#define ACODEC_ANA_CTRL1_ADDR 0x04
#define ACODEC_ANA_CTRL2_ADDR 0x08
#define ACODEC_ANA_CTRL3_ADDR 0x0C
#define ACODEC_ANA_CTRL4_ADDR 0x10
#define ACODEC_ANA_CTRL5_ADDR 0x14

/* acodec dig control register */
#define ACODEC_DIG_CTRL1_ADDR 0xCC
#define ACODEC_DIG_CTRL2_ADDR 0xD0
#define ACODEC_DIG_CTRL3_ADDR 0xD4
#define ACODEC_DIG_CTRL4_ADDR 0xD8
#define ACODEC_DIG_CTRL5_ADDR 0xDC

/* acodec depop control register */
#define ACODEC_POP_CFG0_ADDR  0xEC
#define ACODEC_POP_CFG1_ADDR  0xF0
#define ACODEC_POP_CFG2_ADDR  0xF4

/* default value of acodec analog register */
#define ACODEC_ANA_CTRL0_DEFAULT 0xC080DEFF
#define ACODEC_ANA_CTRL1_DEFAULT 0x6ECE2900 /* 相比IP的默认值，增加使能了RCTUNE */
#define ACODEC_ANA_CTRL2_DEFAULT 0x40550076
#define ACODEC_ANA_CTRL3_DEFAULT 0x3584B555
#define ACODEC_ANA_CTRL4_DEFAULT 0x8AFF0000
#define ACODEC_ANA_CTRL5_DEFAULT 0x00000000

/* normal working value of acodec analog register */
#define ACODEC_ANA_CTRL0_NORMAL 0xC080DEFF
#define ACODEC_ANA_CTRL1_NORMAL 0x0ECE2900
#define ACODEC_ANA_CTRL2_NORMAL 0x40550076
#define ACODEC_ANA_CTRL3_NORMAL 0x3584B555
#define ACODEC_ANA_CTRL4_NORMAL 0x8AFF0000
#define ACODEC_ANA_CTRL5_NORMAL 0x00000000

/* 输入音量 */
#define ACODEC_IN_ANA_VOL_BOOST     20
#define ACODEC_IN_ANA_VOL_GAIN      36
#define ACODEC_IN_ANA_VOL_GAIN_STEP 3
#define ACODEC_IN_ANA_VOL_MAX       (ACODEC_IN_ANA_VOL_BOOST + ACODEC_IN_ANA_VOL_GAIN)
#define ACODEC_IN_ANA_VOL_MIN       0
#define ACODEC_IN_DIG_VOL_MAX       30
#define ACODEC_IN_TOTAL_VOL_MAX     (ACODEC_IN_ANA_VOL_MAX + ACODEC_IN_DIG_VOL_MAX)
#define ACODEC_IN_TOTAL_VOL_MIN     (-78)

/* 输出音量 */
#define ACODEC_OUT_DIG_VOL_MAX      6
#define ACODEC_OUT_DIG_VOL_MIN      (-121)
#define ACODEC_OUT_TOTAL_VOL_MAX    ACODEC_OUT_DIG_VOL_MAX
#define ACODEC_OUT_TOTAL_VOL_MIN    ACODEC_OUT_DIG_VOL_MIN

typedef union {
    struct {
        unsigned int    pd_vref_s             : 1; /* [0]  */
        unsigned int    pd_rctune             : 1; /* [1]  */
        unsigned int    pd_adcl               : 1; /* [2]  */
        unsigned int    pd_adcr               : 1; /* [3]  */
        unsigned int    pd_linein_l           : 1; /* [4]  */
        unsigned int    pd_linein_r           : 1; /* [5]  */
        unsigned int    pd_micbias1           : 1; /* [6]  */
        unsigned int    pd_micbias2           : 1; /* [7]  */
        unsigned int    byp_chop_ctcm_rx      : 1; /* [8]  */
        unsigned int    ana_loop              : 1; /* [9]  */
        unsigned int    pd_dacl               : 1; /* [10]  */
        unsigned int    pd_dacr               : 1; /* [11]  */
        unsigned int    pd_dac_vref_s         : 1; /* [12]  */
        unsigned int    pdb_ctcm_ibias        : 1; /* [13]  */
        unsigned int    pd_ctcm               : 1; /* [14]  */
        unsigned int    pd_bias_s             : 1; /* [15]  */
        unsigned int    linein_l_gain         : 5; /* [20..16]  */
        unsigned int    adcl_gain_boost       : 1; /* [21]  */
        unsigned int    adc_dwa_byps          : 1; /* [22]  */
        unsigned int    mute_linein_l         : 1; /* [23]  */
        unsigned int    linein_r_gain         : 5; /* [28..24]  */
        unsigned int    adcr_gain_boost       : 1; /* [29]  */
        unsigned int    pd_ctcm_rx            : 1; /* [30]  */
        unsigned int    mute_linein_r         : 1; /* [31]  */
    } bits;
    unsigned int    ul32;
} acodec_ana_reg0;

typedef union {
    struct {
        unsigned int    linein_l_sel          : 4; /* [3..0]  */
        unsigned int    linein_r_sel          : 4; /* [7..4]  */
        unsigned int    adc_flstn             : 2; /* [9..8]  */
        unsigned int    adc_adatn             : 2; /* [11..10]  */
        unsigned int    adc_clktiming_sel     : 1; /* [12]  */
        unsigned int    adc_clk_edge_inv_sel  : 1; /* [13]  */
        unsigned int    sel_clk_chop_mic      : 2; /* [15..14]  */
        unsigned int    adc_tune_int1_code    : 5; /* [20..16]  */
        unsigned int    adc_rctune_en         : 1; /* [21]  */
        unsigned int    mute_dacl             : 1; /* [22]  */
        unsigned int    mute_dacr             : 1; /* [23]  */
        unsigned int    adc_tune_int2_code    : 5; /* [28..24]  */
        unsigned int    rctune_rstn           : 1; /* [29]  */
        unsigned int    adc_tune_sel          : 1; /* [30]  */
        unsigned int    pu_pop_pullb_reg      : 1; /* [31]  */
    } bits;
    unsigned int    ul32;
} acodec_ana_reg1;

typedef union {
    struct {
        unsigned int    adc_clk_dly_sel       : 1; /* [0]  */
        unsigned int    pd_ctcm_tx_s          : 1; /* [1]  */
        unsigned int    pop_lineout_pull_en_s : 1; /* [2]  */
        unsigned int    adc_tune_int3_code    : 5; /* [7..3]  */
        unsigned int    sel_clk_chop_ctcm     : 2; /* [9..8]  */
        unsigned int    sel_clk_chop_dac_vref : 2; /* [11..10]  */
        unsigned int    sel_clk_chop_adc_ph   : 1; /* [12]  */
        unsigned int    ctrl_mclk_ph          : 1; /* [13]  */
        unsigned int    ctrl_clk_dac_ph       : 1; /* [14]  */
        unsigned int    ctrl_clk_adc_ph       : 1; /* [15]  */
        unsigned int    sel_clk_chop_linein   : 2; /* [17..16]  */
        unsigned int    sel_clk_chop_vb       : 2; /* [19..18]  */
        unsigned int    sel_clk_chop_adc_st1  : 2; /* [21..20]  */
        unsigned int    sel_clk_chop_adc_st2  : 2; /* [23..22]  */
        unsigned int    byp_chop_ctcm         : 1; /* [24]  */
        unsigned int    byp_chop_dac_vref     : 1; /* [25]  */
        unsigned int    byp_chop_linein       : 1; /* [26]  */
        unsigned int    byp_chop_adc_vb       : 1; /* [27]  */
        unsigned int    byp_chop_adc_st2      : 1; /* [28]  */
        unsigned int    byp_chop_adc_st1      : 1; /* [29]  */
        unsigned int    pop_spd_cfg           : 2; /* [31..30]  */
    } bits;
    unsigned int    ul32;
} acodec_ana_reg2;

typedef union {
    struct {
        unsigned int    ibadj_micbias         : 2; /* [1..0]  */
        unsigned int    ibadj_ctcm            : 2; /* [3..2]  */
        unsigned int    ibadj_dac             : 2; /* [5..4]  */
        unsigned int    ibadj_adc             : 2; /* [7..6]  */
        unsigned int    ibadj_linein          : 2; /* [9..8]  */
        unsigned int    ibadj_dac_vref        : 2; /* [11..10]  */
        unsigned int    micbias_adj           : 2; /* [13..12]  */
        unsigned int    byp_chop_mic_ro7_sel  : 1; /* [14]  */
        unsigned int    rst                   : 1; /* [15]  */
        unsigned int    vref_sel              : 5; /* [20..16]  */
        unsigned int    vref_fs_rst           : 1; /* [21]  */
        unsigned int    vref_exmode           : 1; /* [22]  */
        unsigned int    pop_r_track_l_en      : 1; /* [23]  */
        unsigned int    pop_pdm_dly_cfg_08    : 2; /* [25..24]  */
        unsigned int    lineoutr_pd_org       : 1; /* [26]  */
        unsigned int    pop_dis_08            : 1; /* [27]  */
        unsigned int    lineoutl_pd_org_08    : 1; /* [28]  */
        unsigned int    rst_pop_08            : 1; /* [29]  */
        unsigned int    linein_pull0          : 1; /* [30]  */
        unsigned int    linein_pull1          : 1; /* [31]  */
    } bits;
    unsigned int    ul32;
} acodec_ana_reg3;

typedef union {
    struct {
        unsigned int    adc_mis_seln            : 4; /* [3..0]  */
        unsigned int    adc_mis_selp            : 4; /* [7..4]  */
        unsigned int    vref_pulldown_res_sel0_7 : 8; /* [15..8]  */
        unsigned int    pop_rmpn_tn             : 4; /* [19..16]  */
        unsigned int    pop_rmpp_tn             : 4; /* [23..20]  */
        unsigned int    rmp_bias_sel            : 2; /* [25..24]  */
        unsigned int    rmp_pulse_sel           : 2; /* [27..26]  */
        unsigned int    pdb_dac_clk             : 1; /* [28]  */
        unsigned int    rstb_dac                : 1; /* [29]  */
        unsigned int    vref_pu_pdb             : 1; /* [30]  */
        unsigned int    pop_sel_0h1s            : 1; /* [31]  */
    } bits;
    unsigned int    ul32;
} acodec_ana_reg4;

typedef union {
    struct {
        unsigned int    pd_res_sel_lineout      : 3; /* [2..0]  */
        unsigned int    pullout_weak            : 1; /* [3]  */
        unsigned int    vref_pulldown_res_sel8_11 : 4; /* [7..4]  */
        unsigned int    reserved_0              : 24; /* [31..8]  */
    } bits;
    unsigned int    ul32;
} acodec_ana_reg5;

typedef union {
    struct {
        unsigned int    reserved_0            : 8; /* [7..0] */
        unsigned int    i2s2_fs_sel           : 5; /* [12..8] */
        unsigned int    i2s1_fs_sel           : 5; /* [17..13] */
        unsigned int    dig_loop              : 1; /* [18] */
        unsigned int    dig_bypass            : 1; /* [19] */
        unsigned int    i2s2_data_bits        : 2; /* [21..20] */
        unsigned int    i2s1_data_bits        : 2; /* [23..22] */
        unsigned int    adcr_en               : 1; /* [24] */
        unsigned int    adcl_en               : 1; /* [25] */
        unsigned int    dacr_en               : 1; /* [26] */
        unsigned int    dacl_en               : 1; /* [27] */
        unsigned int    adcr_rst_n            : 1; /* [28] */
        unsigned int    adcl_rst_n            : 1; /* [29] */
        unsigned int    dacr_rst_n            : 1; /* [30] */
        unsigned int    dacl_rst_n            : 1; /* [31] */
    } bits;
    unsigned int ul32;
} acodec_dig_ctrl1;

typedef union {
    struct {
        unsigned int    dacr_lrsel            : 1; /* [0] */
        unsigned int    dacr_i2ssel           : 1; /* [1] */
        unsigned int    dacl_lrsel            : 1; /* [2] */
        unsigned int    dal_i2ssel            : 1; /* [3] */
        unsigned int    reserved_0            : 15; /* [18..4] */
        unsigned int    dacr_deemph           : 2; /* [20..19] */
        unsigned int    dacl_deemph           : 2; /* [22..21] */
        unsigned int    muter_rate            : 2; /* [24..23] */
        unsigned int    mutel_rate            : 2; /* [26..25] */
        unsigned int    dacvu                 : 1; /* [27] */
        unsigned int    sunmuter              : 1; /* [28] */
        unsigned int    sunmutel              : 1; /* [29] */
        unsigned int    smuter                : 1; /* [30] */
        unsigned int    smutel                : 1; /* [31] */
    } bits;
    unsigned int ul32;
} acodec_dig_ctrl2;

typedef union {
    struct {
        unsigned int    dacl2dacr_vol         : 7; /* [6..0] */
        unsigned int    dacl2dacr_en          : 1; /* [7] */
        unsigned int    dacr2dacl_vol         : 7; /* [14..8] */
        unsigned int    dacr2dacl_en          : 1; /* [15] */
        unsigned int    dacr_vol              : 7; /* [22..16] */
        unsigned int    dacr_mute             : 1; /* [23] */
        unsigned int    dacl_vol              : 7; /* [30..24] */
        unsigned int    dacl_mute             : 1; /* [31] */
    } bits;
    unsigned int ul32;
} acodec_dig_ctrl3;

typedef union {
    struct {
        unsigned int    adcr_lrsel            : 1; /* [0] */
        unsigned int    adcr_i2ssel           : 1; /* [1] */
        unsigned int    adcl_lrsel            : 1; /* [2] */
        unsigned int    adcl_i2ssel           : 1; /* [3] */
        unsigned int    reserved_0            : 10; /* [13..4] */
        unsigned int    adcr_hpf_en           : 1; /* [14] */
        unsigned int    adcl_hpf_en           : 1; /* [15] */
        unsigned int    adcr_vol              : 7; /* [22..16] */
        unsigned int    adcr_mute             : 1; /* [23] */
        unsigned int    adcl_vol              : 7; /* [30..24] */
        unsigned int    adcl_mute             : 1; /* [31] */
    } bits;
    unsigned int ul32;
} acodec_dig_ctrl4;

typedef union {
    struct {
        unsigned int    adcr2dacr_vol         : 7; /* [6..0] */
        unsigned int    adcr2dacr_en          : 1; /* [7] */
        unsigned int    adcl2dacr_vol         : 7; /* [14..8] */
        unsigned int    adcl2dacr_en          : 1; /* [15] */
        unsigned int    adcr2dacl_vol         : 7; /* [22..16] */
        unsigned int    adcr2dacl_en          : 1; /* [23] */
        unsigned int    adcl2dacl_vol         : 7; /* [30..24] */
        unsigned int    adcl2dacl_en          : 1; /* [31] */
    } bits;
    unsigned int ul32;
} acodec_dig_ctrl5;

typedef union {
    struct {
        unsigned int    pop_dly_step0         : 3; /* [2..0] */
        unsigned int    pop_sel_step0         : 3; /* [5..3] */
        unsigned int    pop_dly_step1         : 3; /* [8..6] */
        unsigned int    pop_sel_step1         : 3; /* [11..9] */
        unsigned int    pop_dly_step2         : 3; /* [14..12] */
        unsigned int    pop_sel_step2         : 3; /* [17..15] */
        unsigned int    pop_dly_step3         : 3; /* [20..18] */
        unsigned int    pop_sel_step3         : 3; /* [23..21] */
        unsigned int    pop_sel               : 1; /* [24] */
        unsigned int    reserved_0            : 7; /* [31..25] */
    } bits;
    unsigned int ul32;
} acodec_pop_cfg0;

#endif /* End of #ifndef ACODEC_DEF_H */