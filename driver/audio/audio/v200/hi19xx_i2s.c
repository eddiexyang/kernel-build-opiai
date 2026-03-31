/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>

#include "hi19xx_i2s.h"
#include "soc_asp_codec_interface.h"
#include "hi_osal.h"
#include "mkp_aio.h"
#include "aiao_hal_comm.h"
#include <securec.h>

#define PCM_MIC_MODE_4MIC      4
#define PLATFORM_NAME_MAX_SIZE 30
#define I2S_ENABLE_STATE       1
static int g_i2s_gain_level = 0;

static void i2s3_init(void *i2s_reg_base);
static void i2s2_init(void *i2s_reg_base);
static void i2s1_init(void *i2s_reg_base);

static void i2s_reg_write(void *i2s_reg_base, unsigned int offset, unsigned int value)
{
    if (i2s_reg_base == NULL) {
        aio_err_trace("i2s_reg_write(),i2s base reg is null\n");
        return;
    }

    *(volatile td_u32 *)((td_u8 *)i2s_reg_base + offset) = value;
}

static void i2s_reg_read(const void *i2s_reg_base, unsigned int offset, unsigned int *value)
{
    if (i2s_reg_base == NULL) {
        aio_err_trace("i2s_reg_read(),i2s base reg is null\n");
        return;
    }

    *value = (*(const volatile td_u32 *)((td_u8 *)i2s_reg_base + offset));
}

/**
* @set i2s bit width
* @i2s_reg_base: i2s reg base addr
* @audio_device_id: device id
* @attr: save audio attr
*/

static void asp_i2s1_start_playback(void *i2s_reg_base)
{
    unsigned int val = 0;
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_I2S1_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S1_CTRL, i2s1_if_tx_ena, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_I2S1_CTRL_ADDR(0), val);
}

static void asp_i2s1_stop_playback(void * i2s_reg_base)
{
    unsigned int val = 0;
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_I2S1_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S1_CTRL, i2s1_if_tx_ena, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_I2S1_CTRL_ADDR(0), val);
}

static void asp_i2s1_start_capture(void * i2s_reg_base)
{
    unsigned int val = 0;
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_I2S1_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S1_CTRL, i2s1_if_rx_ena, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_I2S1_CTRL_ADDR(0), val);
}

static void asp_i2s1_stop_capture(void * i2s_reg_base)
{
    unsigned int val = 0;
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_I2S1_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S1_CTRL, i2s1_if_rx_ena, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_I2S1_CTRL_ADDR(0), val);
}

static void asp_i2s2_start_playback(void * i2s_reg_base)
{
    unsigned int val = 0;
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_I2S2_PCM_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S2_PCM_CTRL, i2s2_if_tx_ena, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_I2S2_PCM_CTRL_ADDR(0), val);
}

static void asp_i2s2_stop_playback(void * i2s_reg_base)
{
    unsigned int val = 0;
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_I2S2_PCM_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S2_PCM_CTRL, i2s2_if_tx_ena, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_I2S2_PCM_CTRL_ADDR(0), val);
}

static void asp_i2s2_start_capture(void * i2s_reg_base)
{
    unsigned int val = 0;
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_I2S2_PCM_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S2_PCM_CTRL, i2s2_if_rx_ena, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_I2S2_PCM_CTRL_ADDR(0), val);
}

static void asp_i2s2_stop_capture(void * i2s_reg_base)
{
    unsigned int val = 0;
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_I2S2_PCM_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S2_PCM_CTRL, i2s2_if_rx_ena, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_I2S2_PCM_CTRL_ADDR(0), val);
}

static void asp_i2s3_start_playback(void * i2s_reg_base)
{
    unsigned int val = 0;
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_I2S3_PCM_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S3_PCM_CTRL, i2s3_if_tx_ena, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_I2S3_PCM_CTRL_ADDR(0), val);
}

static void asp_i2s3_stop_playback(void * i2s_reg_base)
{
    unsigned int val = 0;
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_I2S3_PCM_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S3_PCM_CTRL, i2s3_if_tx_ena, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_I2S3_PCM_CTRL_ADDR(0), val);
}

static void asp_i2s3_start_capture(void * i2s_reg_base)
{
    unsigned int val = 0;
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_I2S3_PCM_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S3_PCM_CTRL, i2s3_if_rx_ena, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_I2S3_PCM_CTRL_ADDR(0), val);
}

static void asp_i2s3_stop_capture(void * i2s_reg_base)
{
    unsigned int val = 0;
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_I2S3_PCM_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S3_PCM_CTRL, i2s3_if_rx_ena, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_I2S3_PCM_CTRL_ADDR(0), val);
}

void audio_ao_i2s_trigger(ot_audio_dev audio_device_id, void *i2s_reg_base)
{
    if (audio_device_id == 0) {
        asp_i2s1_start_playback(i2s_reg_base);
    } else if (audio_device_id == 1) {
        asp_i2s3_start_playback(i2s_reg_base);
    } else {
        asp_i2s3_start_playback(i2s_reg_base);
    }
}

void audio_ao_i2s_stop(ot_audio_dev audio_device_id, void *i2s_reg_base)
{
    if (audio_device_id == 0) {
        asp_i2s1_stop_playback(i2s_reg_base);
    } else if (audio_device_id == 1) {
        asp_i2s3_stop_playback(i2s_reg_base);
    } else {
        asp_i2s3_stop_playback(i2s_reg_base);
    }
}

void audio_ai_i2s_trigger(ot_audio_dev audio_device_id, void *i2s_reg_base)
{
    if (audio_device_id == 0) {
        asp_i2s1_start_capture(i2s_reg_base);
    } else if (audio_device_id == 1) {
        asp_i2s2_start_capture(i2s_reg_base);
    } else {
        asp_i2s3_start_capture(i2s_reg_base);
    }
}

void audio_ai_i2s_stop(ot_audio_dev audio_device_id, void *i2s_reg_base)
{
    if (audio_device_id == 0) {
        asp_i2s1_stop_capture(i2s_reg_base);
    } else if (audio_device_id == 1) {
        asp_i2s2_stop_capture(i2s_reg_base);
    } else {
        asp_i2s3_stop_capture(i2s_reg_base);
    }
}

void i2s1_ctl_cfg(void *i2s_reg_base)
{
    unsigned int val = 0;
    /* STEP1: 配置i2s1为I2S模式 */
    /* CONFIG: REG_i2s1_PCM_CTRL */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_I2S1_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S1_CTRL, i2s1_rx_clk_sel, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S1_CTRL, i2s1_tx_clk_sel, 0x1);
    /*  32bit */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S1_CTRL, i2s1_codec_io_wordlength, 0x3);
    /*  二进制补码 */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S1_CTRL, i2s1_codec_data_format, 0x0);
    /*  左声道在前 */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S1_CTRL, i2s1_chnnl_mode, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S1_CTRL, i2s1_lrclk_mode, 0x0);
    /*  I2S模式64BIT frame */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S1_CTRL, i2s1_frame_mode, 0x0);
    /*  I2S */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S1_CTRL, i2s1_func_mode, 0x0);
    /*  正常工作模式 */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S1_CTRL, i2s1_direct_loop, 0x0);
    /*  master */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S1_CTRL, i2s1_mst_slv, 0x0);
    /*  RX enable */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S1_CTRL, i2s1_if_rx_ena, 0x1);
    /*  tx enable */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S1_CTRL, i2s1_if_tx_ena, 0x1);
    /*  fs:48k  */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S1_CTRL, fs_i2s1, 0x3);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_I2S1_CTRL_ADDR(0), val);

    /* TDM Mode disable */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_I2S1_TDM_CTRL0_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S1_TDM_CTRL0, s1_tdm_if_en, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_I2S1_TDM_CTRL0_ADDR(0), val);

    /* select:I2S Mode */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_I2S1_TDM_CTRL1_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S1_TDM_CTRL1, s1_i2s_tdm_mode, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_I2S1_TDM_CTRL1_ADDR(0), val);

    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_I2S1_TDM_CTRL2_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S1_TDM_CTRL2, s1_tdm_256_if_en, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S1_TDM_CTRL2, i2s1_tdm256_sel, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_I2S1_TDM_CTRL2_ADDR(0), val);
}

static void i2s1_mix_cfg(void *i2s_reg_base)
{
    unsigned int val = 0;
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_CODEC_DIN_MUX0_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_DIN_MUX0, i2s1_tx_r_sel, 0x2);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_DIN_MUX0, i2s1_tx_l_sel, 0x2);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_CODEC_DIN_MUX0_ADDR(0), val);
}

static void i2s1_clk_cfg(void *i2s_reg_base)
{
    unsigned int val = 0;

    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_CODEC_SW_RST_N_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_SW_RST_N, codec_sw_rst_n, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_CODEC_SW_RST_N_ADDR(0), val);

    /*  REG_CODEC_CLK_EN2:打开i2s1时钟 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_CODEC_CLK_EN2_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN2, i2s1_tdm_clken, 0x1);
    /*  REG_CODEC_CLK_EN2:打开mixer4时钟 */
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN2, dacr_mixer4_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN2, dacl_mixer4_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN2, i2s2_tx_l_srcdn_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN2, i2s2_tx_r_srcdn_clken, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_CODEC_CLK_EN2_ADDR(0), val);

    /*  REG_CODEC_CLK_EN0 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_CODEC_CLK_EN0_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN0, audio_r_dn_afifo_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN0, audio_l_dn_afifo_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN0, audio_r_up_afifo_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN0, audio_l_up_afifo_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN0, audio_up_l_pga_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN0, audio_up_r_pga_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN0, audio_dn_l_pga_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN0, audio_dn_r_pga_clken, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_CODEC_CLK_EN0_ADDR(0), val);

    /*  REG_CODEC_CLK_EN1 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_CODEC_CLK_EN1_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, audio_dn_l_srcup_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, audio_dn_r_srcup_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, audio_up_l_srcdn_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, audio_up_r_srcdn_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, i2s2_rx_r_srcdn_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, i2s2_rx_l_srcdn_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, i2s2_rx_r_srcup_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, i2s2_rx_l_srcup_clken, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_CODEC_CLK_EN1_ADDR(0), val);

    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_CODEC_CLK_EN2_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN2, spa_1_up_afifo_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN2, spa_3_up_afifo_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN2, spa_2_up_afifo_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN2, spa_4_up_afifo_clken, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_CODEC_CLK_EN2_ADDR(0), val);
}

static void i2s2_mux_cfg(void *i2s_reg_base)
{
    unsigned int val = 0;

    /* mixer4 configure */
    /* L:mute select */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_DACL_MIXER4_CTRL0_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACL_MIXER4_CTRL0, dacl_mixer4_in1_mute, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACL_MIXER4_CTRL0, dacl_mixer4_in2_mute, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACL_MIXER4_CTRL0, dacl_mixer4_in3_mute, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACL_MIXER4_CTRL0, dacl_mixer4_in4_mute, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACL_MIXER4_CTRL0, dacl_mixer4_gain1, 0x3);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACL_MIXER4_CTRL0, dacl_mixer4_gain2, 0x3);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACL_MIXER4_CTRL0, dacl_mixer4_gain3, 0x3);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACL_MIXER4_CTRL0, dacl_mixer4_gain4, 0x3);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACL_MIXER4_CTRL0, dacl_mixer4_in1_id, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACL_MIXER4_CTRL0, dacl_mixer4_in2_id, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACL_MIXER4_CTRL0, dacl_mixer4_in3_id, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACL_MIXER4_CTRL0, dacl_mixer4_in4_id, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_DACL_MIXER4_CTRL0_ADDR(0), val);

    /* L:mixer fade in/out */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_DACL_MIXER4_CTRL1_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACL_MIXER4_CTRL1, dacl_mixer4_fade_en, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_DACL_MIXER4_CTRL1_ADDR(0), val);

    /* R:mute select */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_DACR_MIXER4_CTRL0_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACR_MIXER4_CTRL0, dacr_mixer4_in1_mute, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACR_MIXER4_CTRL0, dacr_mixer4_in2_mute, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACR_MIXER4_CTRL0, dacr_mixer4_in3_mute, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACR_MIXER4_CTRL0, dacr_mixer4_in4_mute, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACR_MIXER4_CTRL0, dacr_mixer4_gain1, 0x3);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACR_MIXER4_CTRL0, dacr_mixer4_gain2, 0x3);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACR_MIXER4_CTRL0, dacr_mixer4_gain3, 0x3);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACR_MIXER4_CTRL0, dacr_mixer4_gain4, 0x3);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACR_MIXER4_CTRL0, dacr_mixer4_in1_id, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACR_MIXER4_CTRL0, dacr_mixer4_in2_id, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACR_MIXER4_CTRL0, dacr_mixer4_in3_id, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACR_MIXER4_CTRL0, dacr_mixer4_in4_id, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_DACR_MIXER4_CTRL0_ADDR(0), val);

    /* R:mixer fade in/out */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_DACR_MIXER4_CTRL1_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, DACR_MIXER4_CTRL1, dacr_mixer4_fade_en, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_DACR_MIXER4_CTRL1_ADDR(0), val);

    /* mute mixer2 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S2_TX_MIXER2_CTRL, i2s2_tx_mixer2_in1_mute, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S2_TX_MIXER2_CTRL, i2s2_tx_mixer2_in2_mute, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_I2S2_TX_MIXER2_CTRL_ADDR(0), val);

    /*  REG_CODEC_DIN_MUX0 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_CODEC_DIN_MUX0_ADDR(0), &val);
    /*  配置上行通路，au_up的数据源选择:i2s2_rx */
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_DIN_MUX0, au_up_l_din_sel, 0x2);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_DIN_MUX0, au_up_r_din_sel, 0x2);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_DIN_MUX0, i2s2_tx_r_sel, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_CODEC_DIN_MUX0_ADDR(0), val);

    /*  REG_CODEC_DIN_MUX1 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_CODEC_DIN_MUX1_ADDR(0), &val);

    /*  配置上行通路，srcdn的数据源选择:mixer4_l */
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_DIN_MUX1, bt_tx_srcdn_din_sel, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_CODEC_DIN_MUX1_ADDR(0), val);
}

static void i2s1_init(void * i2s_reg_base)
{
    i2s1_ctl_cfg(i2s_reg_base);
    i2s1_mix_cfg(i2s_reg_base);
    i2s2_mux_cfg(i2s_reg_base);
    i2s1_clk_cfg(i2s_reg_base);
}

static void i2s2_ctl_cfg(void *i2s_reg_base)
{
    unsigned int val = 0;
    /* STEP1: 配置I2S2为I2S模式 */
    /* CONFIG: REG_i2s2_PCM_CTRL */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_I2S2_PCM_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S2_PCM_CTRL, i2s2_rx_clk_sel, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S2_PCM_CTRL, i2s2_tx_clk_sel, 0x1);
    /*  32bit */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S2_PCM_CTRL, i2s2_codec_io_wordlength, 0x3);
    /*  二进制补码 */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S2_PCM_CTRL, i2s2_codec_data_format, 0x0);
    /*  左声道在前 */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S2_PCM_CTRL, i2s2_chnnl_mode, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S2_PCM_CTRL, i2s2_lrclk_mode, 0x0);
    /*  I2S模式64BIT frame */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S2_PCM_CTRL, i2s2_frame_mode, 0x0);
    /*  I2S */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S2_PCM_CTRL, i2s2_func_mode, 0x0);
    /*  正常工作模式 */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S2_PCM_CTRL, i2s2_direct_loop, 0x0);
    /*  master */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S2_PCM_CTRL, i2s2_mst_slv, 0x0);
    /*  RX enable */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S2_PCM_CTRL, i2s2_if_rx_ena, 0x1);
    /*  tx enable */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S2_PCM_CTRL, i2s2_if_tx_ena, 0x1);
    /*  fs:48k  */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S2_PCM_CTRL, fs_i2s2, 0x3);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_I2S2_PCM_CTRL_ADDR(0), val);
}

static void i2s2_pga_bypass(void *i2s_reg_base)
{
    unsigned int val = 0;

    /* bypass:REG_AUDIO_L_DN_PGA_CTRL */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, AUDIO_L_DN_PGA_CTRL, audio_l_dn_pga_bypass, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_ADDR(0), val);

    /*  bypass:REG_AUDIO_R_DN_PGA_CTRL */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, AUDIO_R_DN_PGA_CTRL, audio_r_dn_pga_bypass, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_ADDR(0), val);

    /*  bypass:REG_AUDIO_L_UP_PGA_CTRL */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, AUDIO_L_UP_PGA_CTRL, audio_l_up_pga_bypass, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_ADDR(0), val);

    /*  bypass:REG_AUDIO_R_UP_PGA_CTRL */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, AUDIO_R_UP_PGA_CTRL, audio_r_up_pga_bypass, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_ADDR(0), val);

    /*  bypass:REG_I2S2_L_RX_PGA_CTRL */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S2_L_RX_PGA_CTRL, i2s2_l_rx_pga_bypass, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_ADDR(0), val);

    /*  bypass:REG_I2S2_R_RX_PGA_CTRL */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S2_R_RX_PGA_CTRL, i2s2_r_rx_pga_bypass, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_ADDR(0), val);
}

static void i2s2_pga_ctl(struct asp_i2s_priv *i2s, unsigned int val)
{
    i2s_reg_write(i2s, SOC_ASP_CODEC_AUDIO_L_DN_PGA_CTRL_ADDR(0), val);
    i2s_reg_write(i2s, SOC_ASP_CODEC_AUDIO_R_DN_PGA_CTRL_ADDR(0), val);
    i2s_reg_write(i2s, SOC_ASP_CODEC_AUDIO_L_UP_PGA_CTRL_ADDR(0), val);
    i2s_reg_write(i2s, SOC_ASP_CODEC_AUDIO_R_UP_PGA_CTRL_ADDR(0), val);
    i2s_reg_write(i2s, SOC_ASP_CODEC_I2S2_L_RX_PGA_CTRL_ADDR(0), val);
    i2s_reg_write(i2s, SOC_ASP_CODEC_I2S2_R_RX_PGA_CTRL_ADDR(0), val);
}

static void i2s2_pga_cfg(struct asp_i2s_priv *i2s)
{
    switch (g_i2s_gain_level) {
        case 0:
            i2s2_pga_bypass(i2s);
            break;
        case 1:
            i2s2_pga_ctl(i2s, I2S_GAIN_LEVEL1);
            break;
        default:
            break;
    }
}

static void i2s2_src_cfg(void *i2s_reg_base)
{
    unsigned int val = 0;

    /* src configure */
    /* SRCUP升采样配置 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_SRCUP_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCUP_CTRL, audio_l_dn_srcup_src_mode, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCUP_CTRL, audio_r_dn_srcup_src_mode, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCUP_CTRL, audio_l_dn_srcup_fifo_clr, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCUP_CTRL, audio_r_dn_srcup_fifo_clr, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCUP_CTRL, i2s2_rx_l_srcup_src_mode, 0x4);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCUP_CTRL, i2s2_rx_r_srcup_src_mode, 0x4);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCUP_CTRL, i2s2_rx_l_up_srcup_fifo_clr, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCUP_CTRL, i2s2_rx_r_up_srcup_fifo_clr, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCUP_CTRL, dacl_mixer4_srcup_src_mode, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCUP_CTRL, dacr_mixer4_srcup_src_mode, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCUP_CTRL, dacl_mixer4_srcup_fifo_clr, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCUP_CTRL, dacr_mixer4_srcup_fifo_clr, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_SRCUP_CTRL_ADDR(0), val);

    /*  SRCDN降采样配置0,SRCDN时钟 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_SRCDN_CTRL0_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCDN_CTRL0, audio_l_up_srcdn_src_mode, 0x3);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCDN_CTRL0, audio_r_up_srcdn_src_mode, 0x3);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCDN_CTRL0, audio_l_up_srcdn_fifo_clr, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCDN_CTRL0, audio_r_up_srcdn_fifo_clr, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_SRCDN_CTRL0_ADDR(0), val);

    /*  SRCDN降采样配置1,SRCDN时钟 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_SRCDN_CTRL1_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCDN_CTRL1, i2s2_rx_l_srcdn_src_mode, 0x7);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCDN_CTRL1, i2s2_rx_r_srcdn_src_mode, 0x7);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCDN_CTRL1, i2s2_rx_l_srcdn_fifo_clr, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCDN_CTRL1, i2s2_rx_r_srcdn_fifo_clr, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCDN_CTRL1, i2s2_tx_l_srcdn_src_mode, 0x7);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCDN_CTRL1, i2s2_tx_r_srcdn_src_mode, 0x7);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCDN_CTRL1, i2s2_tx_l_srcdn_fifo_clr, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCDN_CTRL1, i2s2_tx_r_srcdn_fifo_clr, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_SRCDN_CTRL1_ADDR(0), val);
}

static void i2s2_fs_cfg(void *i2s_reg_base)
{
    unsigned int val = 0;

    /* sample rate configure */
    /*  audio下行采样率:48K */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_FS_CTRL0_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL0, fs_audio_r_dn_afifo, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL0, fs_audio_l_dn_afifo, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_FS_CTRL0_ADDR(0), val);

    /*  REG_FS_CTRL1:配置audio上行通路为48K */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_FS_CTRL1_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL1, fs_audio_l_uplink, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL1, fs_audio_r_uplink, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_FS_CTRL1_ADDR(0), val);

    /*  REG_FS_CTRL2:配置PGA为48K,PGA BYPASS */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_FS_CTRL2_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL2, fs_audio_l_up_pga, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL2, fs_audio_r_up_pga, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL2, fs_audio_l_dn_pga, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL2, fs_audio_r_dn_pga, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_FS_CTRL2_ADDR(0), val);

    /*  REG_FS_CTRL3 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_FS_CTRL3_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL3, fs_i2s2_rx_l_srcup_din, 0x3);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL3, fs_i2s2_rx_l_srcup_dout, 0x4);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL3, fs_audio_l_dn_srcup_din, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL3, fs_audio_r_dn_srcup_din, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL3, fs_audio_l_dn_srcup_dout, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL3, fs_audio_r_dn_srcup_dout, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_FS_CTRL3_ADDR(0), val);

    /*  REG_FS_CTRL4 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_FS_CTRL4_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL4, fs_i2s2_rx_r_srcup_din, 0x3);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL4, fs_i2s2_rx_r_srcup_dout, 0x4);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL4, fs_audio_up_l_srcdn_din, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL4, fs_audio_up_l_srcdn_dout, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL4, fs_audio_up_r_srcdn_din, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL4, fs_audio_up_r_srcdn_dout, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_FS_CTRL4_ADDR(0), val);

    /*  REG_FS_CTRL5 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_FS_CTRL5_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL5, fs_i2s2_rx_l_srcdn_din, 0x4);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL5, fs_i2s2_rx_l_srcdn_dout, 0x3);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_FS_CTRL5_ADDR(0), val);

    /*  REG_FS_CTRL6 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_FS_CTRL6_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL6, fs_i2s2_rx_r_srcdn_din, 0x4);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL6, fs_i2s2_rx_r_srcdn_dout, 0x3);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL6, fs_dacl_mixer4, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL6, fs_dacr_mixer4, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL6, fs_i2s2_tx_r_srcdn_din, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL6, fs_i2s2_tx_r_srcdn_dout, 0x3);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL6, fs_i2s2_tx_l_srcdn_din, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL6, fs_i2s2_tx_l_srcdn_dout, 0x3);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_FS_CTRL6_ADDR(0), val);
}

static void i2s2_clk_cfg(void *i2s_reg_base)
{
    unsigned int val = 0;

    /* reset & clock configure */
    /*  CODEC_SW_RST:reset codec */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_CODEC_SW_RST_N_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_SW_RST_N, codec_sw_rst_n, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_CODEC_SW_RST_N_ADDR(0), val);

    /*  REG_CODEC_CLK_EN2:打开i2s2时钟 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_CODEC_CLK_EN2_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN2, i2s2_pcm_clken, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_CODEC_CLK_EN2_ADDR(0), val);

    /*  REG_CODEC_CLK_EN2:打开mixer4时钟 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_CODEC_CLK_EN2_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN2, dacr_mixer4_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN2, dacl_mixer4_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN2, i2s2_tx_l_srcdn_clken, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN2, i2s2_tx_r_srcdn_clken, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_CODEC_CLK_EN2_ADDR(0), val);

    /*  REG_CODEC_CLK_EN0 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_CODEC_CLK_EN0_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN0, audio_r_dn_afifo_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN0, audio_l_dn_afifo_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN0, audio_r_up_afifo_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN0, audio_l_up_afifo_clken, 0x1);

    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN0, audio_up_l_pga_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN0, audio_up_r_pga_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN0, audio_dn_l_pga_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN0, audio_dn_r_pga_clken, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_CODEC_CLK_EN0_ADDR(0), val);

    /*  REG_CODEC_CLK_EN1 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_CODEC_CLK_EN1_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, i2s2_rx_r_pga_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, i2s2_rx_l_pga_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, audio_dn_l_srcup_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, audio_dn_r_srcup_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, audio_up_l_srcdn_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, audio_up_r_srcdn_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, i2s2_rx_r_srcdn_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, i2s2_rx_l_srcdn_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, i2s2_rx_r_srcup_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, i2s2_rx_l_srcup_clken, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_CODEC_CLK_EN1_ADDR(0), val);
}

static void i2s2_init(void *i2s_reg_base)
{
    /* i2s2_pcm interface config */
    i2s2_ctl_cfg(i2s_reg_base);

    /* i2s2 route pga config */
    i2s2_pga_cfg(i2s_reg_base);

    /* i2s2 route srcup/srcdn config */
    i2s2_src_cfg(i2s_reg_base);

    /* i2s2 sample rate config */
    i2s2_fs_cfg(i2s_reg_base);

    /* i2s2 mux config */
    i2s2_mux_cfg(i2s_reg_base);

    /* i2s2 clk config */
    i2s2_clk_cfg(i2s_reg_base);
}

static void i2s3_ctl_cfg(void *i2s_reg_base)
{
    unsigned int val = 0;
    /* CONFIG: REG_I2S3_PCM_CTRL */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_I2S3_PCM_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S3_PCM_CTRL, i2s3_rx_clk_sel, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S3_PCM_CTRL, i2s3_tx_clk_sel, 0x1);
    /*  32bit */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S3_PCM_CTRL, i2s3_codec_io_wordlength, 0x3);
    /*  二进制补码 */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S3_PCM_CTRL, i2s3_codec_data_format, 0x0);
    /*  左声道在前 */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S3_PCM_CTRL, i2s3_chnnl_mode, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S3_PCM_CTRL, i2s3_lrclk_mode, 0x0);
    /*  I2S模式64BIT frame */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S3_PCM_CTRL, i2s3_frame_mode, 0x0);
    /*  I2S */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S3_PCM_CTRL, i2s3_func_mode, 0x0);
    /*  正常工作模式 */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S3_PCM_CTRL, i2s3_direct_loop, 0x0);
    /*  master */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S3_PCM_CTRL, i2s3_mst_slv, 0x0);
    /*  rx enable */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S3_PCM_CTRL, i2s3_if_rx_ena, 0x1);
    /*  tx enable */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S3_PCM_CTRL, i2s3_if_tx_ena, 0x1);
    /*  fs:48k  */
    SET_MEMBER_CACHE(ASP_CODEC, val, I2S3_PCM_CTRL, fs_i2s3, 0x4);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_I2S3_PCM_CTRL_ADDR(0), val);
}

static void i2s3_pga_bypass(void *i2s_reg_base)
{
    unsigned int val = 0;

    /* bypass:REG_CODEC3_L_DN_PGA */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC3_L_DN_PGA_CTRL, codec3_l_dn_pga_bypass, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_ADDR(0), val);

    /*  bypass:REG_CODEC3_R_DN_PGA */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC3_R_DN_PGA_CTRL, codec3_r_dn_pga_bypass, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_ADDR(0), val);

    /*  bypass:REG_MDM_5G_L_UP_PGA */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, MDM_5G_L_UP_PGA_CTRL, mdm_5g_l_up_pga_bypass, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_ADDR(0), val);

    /*  bypass:REG_MDM_5G_R_UP_PGA */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, MDM_5G_R_UP_PGA_CTRL, mdm_5g_r_up_pga_bypass, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_ADDR(0), val);
}

static void i2s3_pga_ctl(void *i2s_reg_base, unsigned int val)
{
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_CODEC3_L_DN_PGA_CTRL_ADDR(0), val);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_CODEC3_R_DN_PGA_CTRL_ADDR(0), val);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_MDM_5G_L_UP_PGA_CTRL_ADDR(0), val);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_MDM_5G_R_UP_PGA_CTRL_ADDR(0), val);
}

static void i2s3_pga_cfg(void *i2s_reg_base)
{
    switch (g_i2s_gain_level) {
        case 0:
            i2s3_pga_bypass(i2s_reg_base);
            break;
        case 1:
            i2s3_pga_ctl(i2s_reg_base, I2S_GAIN_LEVEL1);
            break;
        default:
            break;
    }
}

static void i2s3_src_cfg(void *i2s_reg_base)
{
    unsigned int val = 0;
    /*  bypass:codec3下行通路SRCUP升采样配置 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_SRCUP_CTRL_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCUP_CTRL, codec3_l_dn_srcup_src_mode, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, SRCUP_CTRL, codec3_r_dn_srcup_src_mode, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_SRCUP_CTRL_ADDR(0), val);

    /*  bypass:SRCDN降采样配置,后面会关闭SRCDN时钟 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_SRCDN_CTRL0_ADDR(0), &val);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_SRCDN_CTRL0_ADDR(0), val);
}

static void i2s3_fs_cfg(void *i2s_reg_base)
{
    unsigned int val = 0;
    /*  CODEC3下行采样率:48K */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_FS_CTRL0_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL0, fs_codec3_l_dn_afifo, 0x3);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL0, fs_codec3_r_dn_afifo, 0x3);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_FS_CTRL0_ADDR(0), val);

    /*  REG_FS_CTRL1:配置mdm_5g上行通路为48K */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_FS_CTRL1_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL1, fs_mdm_5g_l_up_afifo, 0x3);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL1, fs_mdm_5g_r_up_afifo, 0x3);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_FS_CTRL1_ADDR(0), val);

    /*  REG_FS_CTRL2:配置PGA为48K */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_FS_CTRL2_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL2, fs_mdm_5g_l_up_pga, 0x3);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL2, fs_mdm_5g_r_up_pga, 0x3);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL2, fs_codec3_l_dn_pga, 0x3);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL2, fs_codec3_r_dn_pga, 0x3);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_FS_CTRL2_ADDR(0), val);

    /*  REG_FS_CTRL3 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_FS_CTRL3_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL3, fs_codec3_l_dn_srcup_din, 0x3);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL3, fs_codec3_r_dn_srcup_din, 0x3);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL3, fs_codec3_l_dn_srcup_dout, 0x3);
    SET_MEMBER_CACHE(ASP_CODEC, val, FS_CTRL3, fs_codec3_r_dn_srcup_dout, 0x3);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_FS_CTRL3_ADDR(0), val);
}

static void i2s3_mux_cfg(void *i2s_reg_base)
{
    unsigned int val = 0;
    /*  REG_CODEC_DIN_MUX1 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_CODEC_DIN_MUX1_ADDR(0), &val);
    /*  配置上行通路，MDM_5G_UP的数据源选择:i2s3_rx */
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_DIN_MUX1, mdm_5g_up_l_din_sel, 0x4);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_DIN_MUX1, mdm_5g_up_r_din_sel, 0x4);
    /*  mdm_5g_r_afifo_din的数据源选择：bm_mdm_5g */
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_DIN_MUX1, mdm_5g_l_afifo_din_sel, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_DIN_MUX1, mdm_5g_r_afifo_din_sel, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_CODEC_DIN_MUX1_ADDR(0), val);
}

static void i2s3_clk_cfg(void *i2s_reg_base)
{
    unsigned int val = 0;
    /*  CODEC_SW_RST:reset codec */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_CODEC_SW_RST_N_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_SW_RST_N, codec_sw_rst_n, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_CODEC_SW_RST_N_ADDR(0), val);

    /*  REG_CODEC_CLK_EN2:打开i2s3时钟 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_CODEC_CLK_EN2_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN2, i2s3_pcm_clken, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_CODEC_CLK_EN2_ADDR(0), val);

    /*  REG_CODEC_CLK_EN0 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_CODEC_CLK_EN0_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN0, codec3_l_dn_afifo_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN0, codec3_r_dn_afifo_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN0, mdm_5g_l_up_afifo_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN0, mdm_5g_r_up_afifo_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN0, codec3_dn_l_pga_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN0, codec3_dn_r_pga_clken, 0x1);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_CODEC_CLK_EN0_ADDR(0), val);

    /*  REG_CODEC_CLK_EN1 */
    i2s_reg_read(i2s_reg_base, SOC_ASP_CODEC_CODEC_CLK_EN1_ADDR(0), &val);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, mdm_5g_l_up_pga_clken, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, mdm_5g_r_up_pga_clken, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, codec3_dn_l_srcup_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, codec3_dn_r_srcup_clken, 0x1);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, mdm_5g_up_l_srcdn_clken, 0x0);
    SET_MEMBER_CACHE(ASP_CODEC, val, CODEC_CLK_EN1, mdm_5g_up_r_srcdn_clken, 0x0);
    i2s_reg_write(i2s_reg_base, SOC_ASP_CODEC_CODEC_CLK_EN1_ADDR(0), val);
}

static void i2s3_init(void *i2s_reg_base)
{
    /* i2s3_pcm interface config */
    i2s3_ctl_cfg(i2s_reg_base);

    /* i2s3 route pga config */
    i2s3_pga_cfg(i2s_reg_base);

    /* i2s3 route srcup/srcdn config */
    i2s3_src_cfg(i2s_reg_base);

    /* i2s3 sample rate config */
    i2s3_fs_cfg(i2s_reg_base);

    /* i2s3 mux config */
    i2s3_mux_cfg(i2s_reg_base);

    /* i2s3 clk config */
    i2s3_clk_cfg(i2s_reg_base);
}

static void anai2s_init(void *reg_base)
{
    i2s_reg_write(reg_base, 0x0, 0x7f); // 配置i2s fifo
    i2s_reg_write(reg_base, 0x4, 0xe06b3); // 配置时钟
    i2s_reg_write(reg_base, 0x7c, 0x6b3); // 配置采样率
    i2s_reg_write(reg_base, 0x84, 0x6000); // 使能I2S
}

static void hdmi_i2s0_init(void *i2s_reg_base)
{
    i2s_reg_write(i2s_reg_base, 0x0, 0x7f); // 配置i2s fifo
    i2s_reg_write(i2s_reg_base, 0xc, 0x6000); // 配置时钟
    i2s_reg_write(i2s_reg_base, 0x4, 0xe06b3); // 配置采样率
}

static void hdmi_i2s1_init(void *i2s_reg_base)
{
    i2s_reg_write(i2s_reg_base, 0x0, 0x7f); // 配置i2s fifo
    i2s_reg_write(i2s_reg_base, 0x4, 0xe06b3); // 配置时钟
    i2s_reg_write(i2s_reg_base, 0x48, 0x6030); // 配置采样率
    i2s_reg_write(i2s_reg_base, 0x40, 0x6b3);
}

void audio_i2s_init(ot_audio_dev audio_device_id, void *i2s_reg_base)
{
    if (audio_device_id == I2S_DIGIT_0) {
        i2s1_init(i2s_reg_base);      // 0
    } else if (audio_device_id == I2S_DIGIT_1) {
        i2s3_init(i2s_reg_base);      // 1
    } else if (audio_device_id == I2S_ANA) {
        anai2s_init(i2s_reg_base);    // 2
    } else if (audio_device_id == I2S_HDMI_0) {
        hdmi_i2s0_init(i2s_reg_base); // 3
    } else if (audio_device_id == I2S_HDMI_1) {
        hdmi_i2s1_init(i2s_reg_base);
    }
}
