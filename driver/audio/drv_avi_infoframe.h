/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi debug.
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#ifndef DRV_AVI_INFOFRAME_H
#define DRV_AVI_INFOFRAME_H

#include "ot_type.h"
#include "drv_hdmi_ioctl.h"
#include "drv_hdmi_timing.h"

#define AVI_PKT_HEADER_ADDRESS 0x1818
#define AVI_SUB_PKT0_L_ADDRESS 0x181c
#define AVI_SUB_PKT0_H_ADDRESS 0x1820
#define AVI_SUB_PKT1_L_ADDRESS 0x1824
#define AVI_SUB_PKT1_H_ADDRESS 0x1828
#define AVI_SUB_PKT2_L_ADDRESS 0x182c
#define AVI_SUB_PKT2_H_ADDRESS 0x1830
#define AVI_SUB_PKT3_L_ADDRESS 0x1834
#define AVI_SUB_PKT3_H_ADDRESS 0x1838
#define CEA_AVI_CFG_ADDRESS 0x19a4
#define AV_MIXER_CFG_ADDRESS 0x1a08

#define AIF_PKT_HEADER_ADDRESS 0x183c
#define AIF_SUB_PKT0_L_ADDRESS 0x1840
#define AIF_SUB_PKT0_H_ADDRESS 0x1844
#define AIF_SUB_PKT1_L_ADDRESS 0x1848
#define AIF_SUB_PKT1_H_ADDRESS 0x184c
#define AIF_SUB_PKT2_L_ADDRESS 0x1850
#define AIF_SUB_PKT2_H_ADDRESS 0x1854
#define AIF_SUB_PKT3_L_ADDRESS 0x1858
#define AIF_SUB_PKT3_H_ADDRESS 0x185c
#define CEA_AUD_CFG_ADDRESS 0x19ac

#define AUD_CHST_CFG0_ADDRESS 0x100c
#define AUD_CHST_CFG1_ADDRESS 0x1010
#define TX_AUDIO_CTRL_ADDRESS 0x1000
#define AUD_I2S_CTRL_ADDRESS 0x1004
#define AUD_ACR_CTRL_ADDRESS 0x1040
#define ACR_N_VAL_SW_ADDRESS 0x1048

typedef enum {
    HDMI_AVI_INFOFRAME_RGB,
    HDMI_AVI_INFOFRAME_YUV444,
    HDMI_AVI_INFOFRAME_YUV422,
    HDMI_AVI_INFOFRAME_YUV420
} hdmi_avi_infoframe_colorspace;

typedef enum {
    HDMI_INFOFRAME_AUIDO,
    HDMI_INFOFRAME_AVI
} hdmi_infoframe_mode;

td_s32 set_avi_infoframe_hdmi_mode(drv_hdmi_infoframe_colorspace *hdmi_infoframe_colorspace);

#endif /* DRV_AVI_INFOFRAME_H */