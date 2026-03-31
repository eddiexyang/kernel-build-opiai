/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description:audio codec driver
 * Author: Hisilicon multimedia software group
 * Create: 2022-09-10
 */

#include "acodec.h"
#include "ot_inner_acodec.h"
#include "securec.h"
#include "aiao_ext.h"
#include "sys_ext.h"
#include "dev_ext.h"
#include "mkp_aio.h"
#include "acodec_def.h"
#ifdef AUDIO_UT_VCAST
#include <linux/device.h>
#endif

#define UMAP_ACODEC_MINOR_BASE      32 // rky add
static osal_dev_t *g_acodec = TD_NULL;

#define acodec_delay_ms(time) osal_udelay((time) * 1000) /* 1000:ms */

static td_void *g_acodec_base_inner = TD_NULL;
static td_void *g_medai_subctrl_base = TD_NULL;

td_u32 acodec_hal_read_reg(td_u32 offset)
{
    return (*(volatile td_u32*)((td_u8 *)g_acodec_base_inner + offset));
}

td_void acodec_hal_write_reg(td_u32 offset, td_u32 value)
{
    *(volatile td_u32*)((td_u8 *)g_acodec_base_inner + offset) = value;
}

static td_void acodec_hal_hal_media_ctrl_write_reg(td_u32 offset, td_u32 value)
{
    *(volatile td_u32*)((td_u8 *)g_medai_subctrl_base + offset) = value;
}

static td_void audio_inner_crg_config(void)
{
    // step 1 关闭 mclk
    acodec_hal_hal_media_ctrl_write_reg(0x45c, 0x1); // offset: 0x45c, value 0x1
    // step 2 解复位ANA_RST、DIG_RST
    acodec_hal_hal_media_ctrl_write_reg(0xb04, 0x7); // offset: 0xb04, value 0x7
    // step 3 MCLK 使能
    acodec_hal_hal_media_ctrl_write_reg(0x458, 0x1); // offset: 0x458, value 0x1
}

static td_void audio_codec_dig_config(void)
{
    acodec_dig_ctrl1 acodec_digctrl1;
    acodec_dig_ctrl2 acodec_digctrl2;
    acodec_dig_ctrl3 acodec_digctrl3;
    acodec_dig_ctrl4 acodec_digctrl4;
    acodec_dig_ctrl5 acodec_digctrl5;
    // step 0
    acodec_digctrl1.ul32 = acodec_hal_read_reg(ACODEC_DIG_CTRL1_ADDR);
    acodec_digctrl1.bits.dacl_rst_n = 0x1;
    acodec_digctrl1.bits.dacr_rst_n = 0x1;
    acodec_digctrl1.bits.adcl_rst_n = 0x1;
    acodec_digctrl1.bits.adcr_rst_n = 0x1;
    acodec_digctrl1.bits.dacl_en = 0x0;
    acodec_digctrl1.bits.dacr_en = 0x0;
    acodec_digctrl1.bits.adcl_en = 0x0;
    acodec_digctrl1.bits.adcr_en = 0x0;
    acodec_hal_write_reg(ACODEC_DIG_CTRL1_ADDR, acodec_digctrl1.ul32); // offset 0xCC配置值

    acodec_digctrl1.bits.i2s1_data_bits = 0x3;
    acodec_digctrl1.bits.i2s2_data_bits = 0x3;
    acodec_digctrl1.bits.i2s1_fs_sel = 0x1a;
    acodec_digctrl1.bits.i2s2_fs_sel = 0x1a;
    acodec_hal_write_reg(ACODEC_DIG_CTRL1_ADDR, acodec_digctrl1.ul32); // offset 0xCC配置值

    // step 2 TX左右声道配置
    acodec_digctrl2.ul32 = acodec_hal_read_reg(ACODEC_DIG_CTRL2_ADDR);
    acodec_digctrl2.bits.smutel = 0x0;
    acodec_digctrl2.bits.smuter = 0x0;
    acodec_digctrl2.bits.sunmutel = 0x0;
    acodec_digctrl2.bits.sunmuter = 0x0;
    acodec_digctrl2.bits.dacvu = 0x1;
    acodec_digctrl2.bits.mutel_rate = 0x0;
    acodec_digctrl2.bits.muter_rate = 0x0;
    acodec_digctrl2.bits.dacl_deemph = 0x0;
    acodec_digctrl2.bits.reserved_0 = 0x0;
    acodec_digctrl2.bits.dal_i2ssel = 0x0;
    acodec_digctrl2.bits.dacl_lrsel = 0x0;
    acodec_digctrl2.bits.dacr_i2ssel = 0x0;
    acodec_digctrl2.bits.dacr_lrsel = 0x1;
    acodec_hal_write_reg(ACODEC_DIG_CTRL2_ADDR, acodec_digctrl2.ul32); // offset 0xD0配置值

    acodec_digctrl3.ul32 = acodec_hal_read_reg(ACODEC_DIG_CTRL3_ADDR);
    acodec_digctrl3.bits.dacl_mute = 0x0;
    acodec_digctrl3.bits.dacl_vol = 0x06;
    acodec_digctrl3.bits.dacr_mute = 0x0;
    acodec_digctrl3.bits.dacr_vol = 0x06;
    acodec_digctrl3.bits.dacr2dacl_en = 0x0;
    acodec_digctrl3.bits.dacr2dacl_vol = 0x24;
    acodec_digctrl3.bits.dacl2dacr_en = 0;
    acodec_digctrl3.bits.dacl2dacr_vol = 0x24;
    acodec_hal_write_reg(ACODEC_DIG_CTRL3_ADDR, acodec_digctrl3.ul32); // offset 0xD4配置值

    // step 1 RX左右声道配置
    acodec_digctrl4.ul32 = acodec_hal_read_reg(ACODEC_DIG_CTRL4_ADDR);
    acodec_digctrl4.bits.adcl_mute = 0;
    acodec_digctrl4.bits.adcl_vol = 0x1E;
    acodec_digctrl4.bits.adcr_mute = 0x0;
    acodec_digctrl4.bits.adcr_vol = 0x1E;
    acodec_digctrl4.bits.adcl_hpf_en = 0x1;
    acodec_digctrl4.bits.adcr_hpf_en = 0x1;
    acodec_digctrl4.bits.reserved_0 = 0x0;
    acodec_digctrl4.bits.adcl_i2ssel = 0x0;
    acodec_digctrl4.bits.adcl_lrsel = 0x0;
    acodec_digctrl4.bits.adcr_i2ssel = 0x0;
    acodec_digctrl4.bits.adcr_lrsel = 0x1;
    acodec_hal_write_reg(ACODEC_DIG_CTRL4_ADDR, acodec_digctrl4.ul32); // offset 0xD8配置值

    acodec_digctrl5.ul32 = acodec_hal_read_reg(ACODEC_DIG_CTRL5_ADDR);
    acodec_digctrl5.bits.adcl2dacl_en = 0x0;
    acodec_digctrl5.bits.adcl2dacl_vol = 0x24;
    acodec_digctrl5.bits.adcr2dacl_en = 0x0;
    acodec_digctrl5.bits.adcr2dacl_vol = 0x24;
    acodec_digctrl5.bits.adcl2dacr_en = 0x0;
    acodec_digctrl5.bits.adcl2dacr_vol = 0x24;
    acodec_digctrl5.bits.adcr2dacr_en = 0x0;
    acodec_digctrl5.bits.adcr2dacr_vol = 0x24;
    acodec_hal_write_reg(ACODEC_DIG_CTRL5_ADDR, acodec_digctrl5.ul32); // offset 0xDC配置值

    // step 3 使能信号
    acodec_digctrl1.ul32 = acodec_hal_read_reg(ACODEC_DIG_CTRL1_ADDR);
    acodec_digctrl1.bits.dacl_en = 0x1;
    acodec_digctrl1.bits.dacr_en = 0x1;
    acodec_digctrl1.bits.adcl_en = 0x1;
    acodec_digctrl1.bits.adcr_en = 0x1;
    acodec_hal_write_reg(ACODEC_DIG_CTRL1_ADDR, acodec_digctrl1.ul32); // offset 0xCC配置值
}

static td_void audio_codec_power_down(void)
{
    // step 1
    acodec_hal_write_reg(ACODEC_ANA_CTRL0_ADDR, 0xC080DEFF); // offset 0x0 寄存器
    acodec_hal_write_reg(ACODEC_ANA_CTRL1_ADDR, 0x0ECE2900); // offset 0x4 寄存器
    acodec_hal_write_reg(ACODEC_ANA_CTRL2_ADDR, 0x40550076); // offset 0x8 寄存器
    acodec_hal_write_reg(ACODEC_ANA_CTRL3_ADDR, 0x3584B555); // offset 0xc 寄存器
    acodec_hal_write_reg(ACODEC_ANA_CTRL4_ADDR, 0x8AFF0000); // offset 0x10 寄存器
    acodec_hal_write_reg(ACODEC_ANA_CTRL5_ADDR, 0x00000000); // offset 0x14 寄存器

    // step 2 解复位ANA_RST、DIG_RST
    acodec_hal_hal_media_ctrl_write_reg(0xb00, 0x7); // offset: 0xb00, value 0x7
    // step 3 MCLK 使能
    acodec_hal_hal_media_ctrl_write_reg(0x45c, 0x1); // offset: 0x45c, value 0x1
}

static td_void audio_codec_ana_config(void)
{
    td_u32 audio_ana_ro;
    // ana reg 0
    acodec_hal_write_reg(ACODEC_ANA_CTRL0_ADDR, 0x2000); // offset 0x0配置值
    // ana reg 1
    acodec_hal_write_reg(ACODEC_ANA_CTRL1_ADDR, 0xEE0E2944); // offset 0x4配置值
    // ana reg 2
    acodec_hal_write_reg(ACODEC_ANA_CTRL2_ADDR, 0x40550070); // offset 0x8配置值
    // ana reg 3
    acodec_hal_write_reg(ACODEC_ANA_CTRL3_ADDR, 0x09a4B555); // offset 0xC配置值
    // ana reg 4
    acodec_hal_write_reg(ACODEC_ANA_CTRL4_ADDR, 0xDAFF0000); // offset 0x10配置值
    // ana reg 5
    acodec_hal_write_reg(ACODEC_ANA_CTRL5_ADDR, 0x00000000); // offset 0x10配置值

    acodec_delay_ms(20U); // delay 20ms

    // ana reg 3  系统解复位
    acodec_hal_write_reg(ACODEC_ANA_CTRL3_ADDR, 0x09843555); // offset 0xC配置值
    // ana reg 4
    acodec_hal_write_reg(ACODEC_ANA_CTRL4_ADDR, 0xFAFF0000); // offset 0x10配置值
    // ana reg 1
    acodec_hal_write_reg(ACODEC_ANA_CTRL1_ADDR, 0xAE2E2911); // offset 0x4配置值

    audio_ana_ro = acodec_hal_read_reg(0x24);
    aio_info_trace("audio ana read = 0x%x\n", audio_ana_ro);
}

static td_s32 acodec_set_gain_micl(const unsigned int *arg)
{
    acodec_ana_reg0 ana_reg0;
    td_u32 reg_val;

    if (arg == TD_NULL) {
        return TD_FAILURE;
    }

    reg_val = *arg;
    /* RemixV100范围是 0x0-0xF  0-36dB */
    if (reg_val > 0xC) {
        aio_err_trace("illegal gain = %u, please use 0x0~0xC value.\n", reg_val);
        return TD_FAILURE;
    }

    ana_reg0.ul32 = acodec_hal_read_reg(ACODEC_ANA_CTRL0_ADDR);
    ana_reg0.bits.linein_l_gain = reg_val;
    acodec_hal_write_reg(ACODEC_ANA_CTRL0_ADDR, ana_reg0.ul32);

    return TD_SUCCESS;
}

static td_s32 acodec_set_gain_micr(const unsigned int *arg)
{
    acodec_ana_reg0 ana_reg0;
    td_u32 reg_val;

    if (arg == TD_NULL) {
        return TD_FAILURE;
    }

    reg_val = *arg;
    /* RemixV100范围是 0x0-0xC  0-36db */
    if (reg_val > 0xC) {
        aio_err_trace("illegal gain = %u, please use 0x0~0xC value.\n", reg_val);
        return TD_FAILURE;
    }

    ana_reg0.ul32 = acodec_hal_read_reg(ACODEC_ANA_CTRL0_ADDR);
    ana_reg0.bits.linein_r_gain = reg_val;
    acodec_hal_write_reg(ACODEC_ANA_CTRL0_ADDR, ana_reg0.ul32);

    return TD_SUCCESS;
}

static td_s32 acodec_get_gain_micl(td_u32 *reg_val)
{
    acodec_ana_reg0 ana_reg0;

    if (reg_val == TD_NULL) {
        return TD_FAILURE;
    }

    ana_reg0.ul32 = acodec_hal_read_reg(ACODEC_ANA_CTRL0_ADDR);

    *reg_val = (td_u32)ana_reg0.bits.linein_l_gain;

    return TD_SUCCESS;
}

static td_s32 acodec_get_gain_micr(td_u32 *reg_val)
{
    acodec_ana_reg0 ana_reg0;

    if (reg_val == TD_NULL) {
        return TD_FAILURE;
    }

    ana_reg0.ul32 = acodec_hal_read_reg(ACODEC_ANA_CTRL0_ADDR);

    *reg_val = (td_u32)ana_reg0.bits.linein_r_gain;

    return TD_SUCCESS;
}

static td_s32 acodec_set_dacl_vol(const unsigned int *arg)
{
    acodec_dig_ctrl3 dig_ctrl3;
    ot_acodec_volume_ctrl vol_ctrl = {0};
    td_s32 ret;

    if (arg == TD_NULL) {
        return TD_FAILURE;
    }

    ret = memcpy_s(&vol_ctrl, sizeof(vol_ctrl), arg, sizeof(ot_acodec_volume_ctrl));
    if (ret != EOK) {
        aio_err_trace("memcpy_s fail, ret = 0x%x.\n", (td_u32)ret);
        return TD_FAILURE;
    }

    if ((vol_ctrl.volume_ctrl > 0x7F) || (vol_ctrl.volume_ctrl_mute > 1)) {
        aio_err_trace("illegal volume value, volume_ctrl = %u, volume_ctrl_mute = %u!\n",
            vol_ctrl.volume_ctrl, vol_ctrl.volume_ctrl_mute);
        return TD_FAILURE;
    }

    dig_ctrl3.ul32 = acodec_hal_read_reg(ACODEC_DIG_CTRL3_ADDR);
    dig_ctrl3.bits.dacl_mute = vol_ctrl.volume_ctrl_mute;
    dig_ctrl3.bits.dacl_vol = vol_ctrl.volume_ctrl;
    acodec_hal_write_reg(ACODEC_DIG_CTRL3_ADDR, dig_ctrl3.ul32);

    return TD_SUCCESS;
}

static td_s32 acodec_set_dacr_vol(const unsigned int *arg)
{
    acodec_dig_ctrl3 dig_ctrl3;
    ot_acodec_volume_ctrl vol_ctrl = {0};
    td_s32 ret;

    if (arg == TD_NULL) {
        aio_err_trace("acodec_set_dacr_vol arg == NULL \n");
        return TD_FAILURE;
    }

    ret = memcpy_s(&vol_ctrl, sizeof(vol_ctrl), arg, sizeof(ot_acodec_volume_ctrl));
    if (ret != EOK) {
        aio_err_trace("acodec_set_dacr_vol memcpy_s fail, ret = 0x%x!\n", (td_u32)ret);
        return TD_FAILURE;
    }

    if ((vol_ctrl.volume_ctrl > 0x7F) || (vol_ctrl.volume_ctrl_mute > 1)) {
        aio_err_trace("illegal volume value, volume_ctrl = %u, volume_ctrl_mute = %u\n",
            vol_ctrl.volume_ctrl, vol_ctrl.volume_ctrl_mute);
        return TD_FAILURE;
    }

    dig_ctrl3.ul32 = acodec_hal_read_reg(ACODEC_DIG_CTRL3_ADDR);
    dig_ctrl3.bits.dacr_mute = vol_ctrl.volume_ctrl_mute;
    dig_ctrl3.bits.dacr_vol = vol_ctrl.volume_ctrl;
    acodec_hal_write_reg(ACODEC_DIG_CTRL3_ADDR, dig_ctrl3.ul32);

    return TD_SUCCESS;
}

static td_s32 acodec_set_adcl_vol(const unsigned int *arg)
{
    acodec_dig_ctrl4 dig_ctrl4;
    ot_acodec_volume_ctrl vol_ctrl = {0};
    td_s32 ret;

    if (arg == TD_NULL) {
        return TD_FAILURE;
    }

    ret = memcpy_s(&vol_ctrl, sizeof(vol_ctrl), arg, sizeof(ot_acodec_volume_ctrl));
    if (ret != EOK) {
        aio_err_trace("memcpy_s fail, ret = 0x%x.\n", (td_u32)ret);
        return TD_FAILURE;
    }

    if ((vol_ctrl.volume_ctrl > 0x7F) || (vol_ctrl.volume_ctrl_mute > 1)) {
        aio_err_trace("illegal volume value, volume_ctrl = %u, volume_ctrl_mute = %u\n",
            vol_ctrl.volume_ctrl, vol_ctrl.volume_ctrl_mute);
        return TD_FAILURE;
    }

    dig_ctrl4.ul32 = acodec_hal_read_reg(ACODEC_DIG_CTRL4_ADDR);
    dig_ctrl4.bits.adcl_mute = vol_ctrl.volume_ctrl_mute;
    dig_ctrl4.bits.adcl_vol = vol_ctrl.volume_ctrl;
    acodec_hal_write_reg(ACODEC_DIG_CTRL4_ADDR, dig_ctrl4.ul32);

    return TD_SUCCESS;
}

static td_s32 acodec_set_adcr_vol(const unsigned int *arg)
{
    acodec_dig_ctrl4 dig_ctrl4;
    ot_acodec_volume_ctrl vol_ctrl = {0};
    td_s32 ret;

    if (arg == TD_NULL) {
        return TD_FAILURE;
    }

    ret = memcpy_s(&vol_ctrl, sizeof(vol_ctrl), arg, sizeof(ot_acodec_volume_ctrl));
    if (ret != EOK) {
        aio_err_trace("memcpy_s fail, ret = 0x%x!\n", (td_u32)ret);
        return TD_FAILURE;
    }

    if ((vol_ctrl.volume_ctrl > 0x7F) || (vol_ctrl.volume_ctrl_mute > 1)) {
        aio_err_trace("illegal volume value, volume_ctrl = %u, volume_ctrl_mute = %u.\n",
            vol_ctrl.volume_ctrl, vol_ctrl.volume_ctrl_mute);
        return TD_FAILURE;
    }

    dig_ctrl4.ul32 = acodec_hal_read_reg(ACODEC_DIG_CTRL4_ADDR);
    dig_ctrl4.bits.adcr_mute = vol_ctrl.volume_ctrl_mute;
    dig_ctrl4.bits.adcr_vol = vol_ctrl.volume_ctrl;
    acodec_hal_write_reg(ACODEC_DIG_CTRL4_ADDR, dig_ctrl4.ul32);

    return TD_SUCCESS;
}

static td_s32 acodec_get_dacl_vol(unsigned int *arg)
{
    acodec_dig_ctrl3 dig_ctrl3;
    ot_acodec_volume_ctrl *vol_ctrl = TD_NULL;

    if (arg == TD_NULL) {
        return TD_FAILURE;
    }

    vol_ctrl = (ot_acodec_volume_ctrl *)arg;

    dig_ctrl3.ul32 = acodec_hal_read_reg(ACODEC_DIG_CTRL3_ADDR);
    vol_ctrl->volume_ctrl_mute = dig_ctrl3.bits.dacl_mute;
    vol_ctrl->volume_ctrl = dig_ctrl3.bits.dacl_vol;

    return TD_SUCCESS;
}

static td_s32 acodec_get_dacr_vol(unsigned int *arg)
{
    acodec_dig_ctrl3 dig_ctrl3;
    ot_acodec_volume_ctrl *vol_ctrl = TD_NULL;

    if (arg == TD_NULL) {
        return TD_FAILURE;
    }

    vol_ctrl = (ot_acodec_volume_ctrl *)arg;

    dig_ctrl3.ul32 = acodec_hal_read_reg(ACODEC_DIG_CTRL3_ADDR);
    vol_ctrl->volume_ctrl_mute = dig_ctrl3.bits.dacr_mute;
    vol_ctrl->volume_ctrl = dig_ctrl3.bits.dacr_vol;

    return TD_SUCCESS;
}

static td_s32 acodec_get_adcl_vol(unsigned int *arg)
{
    acodec_dig_ctrl4 dig_ctrl4;
    ot_acodec_volume_ctrl *vol_ctrl = TD_NULL;

    if (arg == TD_NULL) {
        return TD_FAILURE;
    }

    vol_ctrl = (ot_acodec_volume_ctrl *)arg;

    dig_ctrl4.ul32 = acodec_hal_read_reg(ACODEC_DIG_CTRL4_ADDR);
    vol_ctrl->volume_ctrl_mute = dig_ctrl4.bits.adcl_mute;
    vol_ctrl->volume_ctrl = dig_ctrl4.bits.adcl_vol;

    return TD_SUCCESS;
}

static td_s32 acodec_get_adcr_vol(unsigned int *arg)
{
    acodec_dig_ctrl4 dig_ctrl4;
    ot_acodec_volume_ctrl *vol_ctrl = TD_NULL;

    if (arg == TD_NULL) {
        return TD_FAILURE;
    }

    vol_ctrl = (ot_acodec_volume_ctrl *)arg;

    dig_ctrl4.ul32 = acodec_hal_read_reg(ACODEC_DIG_CTRL4_ADDR);
    vol_ctrl->volume_ctrl_mute = dig_ctrl4.bits.adcr_mute;
    vol_ctrl->volume_ctrl = dig_ctrl4.bits.adcr_vol;

    return TD_SUCCESS;
}

static long ioctl_gain_mic(unsigned int cmd, unsigned int *arg)
{
    switch (cmd) {
        /* set/get gain mic */
        case OT_ACODEC_SET_GAIN_MICL:
            return acodec_set_gain_micl(arg);

        case OT_ACODEC_SET_GAIN_MICR:
            return acodec_set_gain_micr(arg);

        case OT_ACODEC_GET_GAIN_MICL:
            return acodec_get_gain_micl(arg);

        case OT_ACODEC_GET_GAIN_MICR:
            return acodec_get_gain_micr(arg);

        default:
            aio_err_trace("ioctl_gain_mic error!\n");
            return TD_FAILURE;
    }
}

static long ioctl_set_vol(unsigned int cmd, unsigned int *arg)
{
    switch (cmd) {
        /* set ADC/DAC volume */
        case OT_ACODEC_SET_DACL_VOLUME:
            return acodec_set_dacl_vol(arg);

        case OT_ACODEC_SET_DACR_VOLUME:
            return acodec_set_dacr_vol(arg);

        case OT_ACODEC_SET_ADCL_VOLUME:
            return acodec_set_adcl_vol(arg);

        case OT_ACODEC_SET_ADCR_VOLUME:
            return acodec_set_adcr_vol(arg);

        default:
            aio_err_trace("ioctl_set_vol error!\n");
            return TD_FAILURE;
    }
}

static long ioctl_get_vol(unsigned int cmd, unsigned int *arg)
{
    switch (cmd) {
        /* get ADC/DAC volume */
        case OT_ACODEC_GET_DACL_VOLUME:
            return acodec_get_dacl_vol(arg);

        case OT_ACODEC_GET_DACR_VOLUME:
            return acodec_get_dacr_vol(arg);

        case OT_ACODEC_GET_ADCL_VOLUME:
            return acodec_get_adcl_vol(arg);

        case OT_ACODEC_GET_ADCR_VOLUME:
            return acodec_get_adcr_vol(arg);

        default:
            aio_err_trace("ioctl_get_vol error!\n");
            return TD_FAILURE;
    }
}

static long acodec_ioctl(unsigned int cmd, unsigned long arg_org, void *private_data)
{
    ot_unused(private_data);
    unsigned int *arg = (unsigned int *)(td_uintptr_t)arg_org;
    if (g_acodec_base_inner == TD_NULL) {
        aio_err_trace("haven't ioremap acodec regs.\n");
        return TD_FAILURE;
    }

    switch (cmd) {
        /* set ADC/DAC volume */
        case OT_ACODEC_SET_DACL_VOLUME:
        case OT_ACODEC_SET_DACR_VOLUME:
        case OT_ACODEC_SET_ADCL_VOLUME:
        case OT_ACODEC_SET_ADCR_VOLUME:
            return ioctl_set_vol(cmd, arg);

        /* get ADC/DAC volume */
        case OT_ACODEC_GET_DACL_VOLUME:
        case OT_ACODEC_GET_DACR_VOLUME:
        case OT_ACODEC_GET_ADCL_VOLUME:
        case OT_ACODEC_GET_ADCR_VOLUME:
            return ioctl_get_vol(cmd, arg);

        default:
            aio_err_trace("acodec_ioctl error!\n");
            return TD_FAILURE;
    }
}

static td_s32 acodec_open(void *private_data)
{
    ot_unused(private_data);
    return 0;
}

static td_s32 acodec_close(void *private_data)
{
    ot_unused(private_data);
    return 0;
}

static struct osal_fileops g_acodec_fops = {
    .module = THIS_MODULE,
    .unlocked_ioctl = acodec_ioctl,
    .open = acodec_open,
    .release = acodec_close
};
int acodec_resume(osal_dev_t *dev)
{
    // 先配置数字 media subctrl
    audio_inner_crg_config();
    // dig
    audio_codec_dig_config();
    // ana
    audio_codec_ana_config();
    aio_info_trace("audio acodec resume success!\n");
    return 0;
}

int acodec_suspend(osal_dev_t *dev)
{
    audio_codec_power_down();
    aio_info_trace("audio acodec suspend success\n");
    return 0;
}

struct osal_pmops g_acodec_pm = {
    .pm_resume = acodec_resume,
    .pm_suspend = acodec_suspend,
};

static td_s32 acodec_device_init(void)
{
    td_s32 ret;
    acodec_pop_cfg0 pop_cfg0;
    td_void *acodec_base_tmp = TD_NULL;

    g_acodec_base_inner = osal_ioremap(ACODEC_REGS_BASE, ACODEC_MAX_REG_SIZE);
    if (g_acodec_base_inner == TD_NULL) {
        aio_err_trace("could not ioremap acodec regs\n");
        return -1;
    }

    g_medai_subctrl_base = osal_ioremap(MEDIA_SUBCTRL_REGS_BASE, ACODEC_MAX_REG_SIZE);
    if (g_medai_subctrl_base == TD_NULL) {
        aio_err_trace("could not ioremap acodec regs\n");
        osal_iounmap(g_acodec_base_inner);
        return -1;
    }

    // 先配置数字 media subctrl
    audio_inner_crg_config();
    // dig
    audio_codec_dig_config();
    // ana
    audio_codec_ana_config();

    aio_info_trace("acodec init");
    return 0;
}

static td_s32 acodec_device_exit(void)
{
    td_s32 ret;

    if (g_acodec_base_inner != TD_NULL) {
        osal_iounmap(g_acodec_base_inner);
        g_acodec_base_inner = TD_NULL;
    }
    if (g_medai_subctrl_base != TD_NULL) {
        osal_iounmap(g_medai_subctrl_base);
        g_medai_subctrl_base = TD_NULL;
    }
    return 0;
}

static td_s32 acodec_register_dev(td_void)
{
    td_s32 ret;

    g_acodec = osal_createdev(UMAP_DEVNAME_ACODEC_BASE);
    if (g_acodec == TD_NULL) {
        aio_err_trace("could not create device\n");
        return TD_FAILURE;
    }

    g_acodec->minor = UMAP_ACODEC_MINOR_BASE;
    g_acodec->fops = &g_acodec_fops;
    g_acodec->osal_pmops = &g_acodec_pm;

    ret = osal_registerdevice(g_acodec);
    if (ret != TD_SUCCESS) {
        osal_destroydev(g_acodec);
        g_acodec = TD_NULL;

        aio_err_trace("could not register device!!!\n");
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_void acodec_unregister_dev(td_void)
{
    if (g_acodec != TD_NULL) {
        osal_deregisterdevice(g_acodec);
        osal_destroydev(g_acodec);
        g_acodec = TD_NULL;
    }
}

td_s32 acodec_init(void)
{
    ot_mpp_chn mpp_chn = {0};
    td_bool reset_flag = TD_FALSE;

    if (acodec_register_dev() != TD_SUCCESS) {
        return TD_FAILURE;
    }

    if ((!ckfn_sys_entry()) || (!ckfn_sys_drv_ioctrl())) {
        acodec_unregister_dev();
        aio_err_trace("sys module may not insert\n");
        return TD_FAILURE;
    }

    if (acodec_device_init() < 0) {
        acodec_unregister_dev();
        aio_err_trace("acodec device init fail, deregister it!\n");
        return TD_FAILURE;
    }

    aio_info_trace("load acodec.ko ....OK!\n");
    return TD_SUCCESS;
}

void acodec_exit(void)
{
    ot_mpp_chn mpp_chn = {0};
    td_bool reset = TD_TRUE;
    if (acodec_device_exit() < 0) {
        aio_err_trace("acodec driver exit fail\n");
    }

    acodec_unregister_dev();
    aio_info_trace("unload acodec.ko OK!\n");
}
