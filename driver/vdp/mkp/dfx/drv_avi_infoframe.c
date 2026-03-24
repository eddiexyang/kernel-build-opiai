/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi debug.
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#include "drv_avi_infoframe.h"
#include <linux/file.h>
#include <linux/major.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/hwspinlock.h>
#include <linux/pm_wakeup.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <securec.h>
#include "drv_hdmi_common.h"
#include "drv_hdmi_intf.h"
#include "drv_hdmi_event.h"
#include "drv_hdmi_edid.h"
#include "hdmi_osal.h"
#include "drv_hdmi_compatibility.h"
#include "drv_hdmi_debug.h"
#include "drv_hdmi_detection.h"
#include "drv_hdmi_timing.h"

struct avi_info_reg {
    void *avi_pkt_header;
    void *avi_sub_pkt0_l;
    void *avi_sub_pkt0_h;
    void *avi_sub_pkt1_l;
    void *avi_sub_pkt1_h;
    void *avi_sub_pkt2_l;
    void *avi_sub_pkt2_h;
    void *avi_sub_pkt3_l;
    void *avi_sub_pkt3_h;
    void *cea_avi_cfg;
    void *av_mixer_cfg;
};

struct audio_info_reg {
    void *hdmi_base_reg;
    void *aif_pkt_header;
    void *aif_sub_pkt0_l;
    void *aif_sub_pkt0_h;
    void *aif_sub_pkt1_l;
    void *aif_sub_pkt1_h;
    void *aif_sub_pkt2_l;
    void *aif_sub_pkt2_h;
    void *aif_sub_pkt3_l;
    void *aif_sub_pkt3_h;
    void *cea_aud_cfg;
    void *aud_chst_cfg0;
    void *aud_chst_cfg1;
    void *tx_audio_ctrl;
    void *aud_i2s_ctrl;
    void *aud_acr_ctrl;
    void *acr_n_val_sw;
};

td_s32 set_avi_infoframe_RGB(struct avi_info_reg avi_reg)
{
    writel(0x085810BF, avi_reg.avi_sub_pkt0_l);
    return TD_SUCCESS;
}

td_s32 set_avi_infoframe_yuv444(struct avi_info_reg avi_reg)
{
    writel(0x585087, avi_reg.avi_sub_pkt0_l);
    return TD_SUCCESS;
}

td_s32 set_avi_infoframe_yuv422(struct avi_info_reg avi_reg)
{
    writel(0x5830A7, avi_reg.avi_sub_pkt0_l);
    return TD_SUCCESS;
}

td_s32 set_avi_infoframe_yuv420(struct avi_info_reg avi_reg)
{
    writel(0x587067, avi_reg.avi_sub_pkt0_l);
    return TD_SUCCESS;
}

void set_audio_infoframe_48k_2ch(void)
{
    void *hdmi_base_reg = NULL;
    td_u64 base = HDMI_TX_REG_BASE_ADDRESS;
    hdmi_base_reg = osal_ioremap(base, 0x2804);
    if (hdmi_base_reg == NULL) {
        hdmi_dfx_err("hdmi_base_reg remap failed!\n");
        return;
    }

    struct audio_info_reg audio_reg = {
        .aif_pkt_header = hdmi_base_reg + AIF_PKT_HEADER_ADDRESS,
        .aif_sub_pkt0_l = hdmi_base_reg + AIF_SUB_PKT0_L_ADDRESS,
        .aif_sub_pkt0_h = hdmi_base_reg + AIF_SUB_PKT0_H_ADDRESS,
        .aif_sub_pkt1_l = hdmi_base_reg + AIF_SUB_PKT1_L_ADDRESS,
        .aif_sub_pkt1_h = hdmi_base_reg + AIF_SUB_PKT1_H_ADDRESS,
        .aif_sub_pkt2_l = hdmi_base_reg + AIF_SUB_PKT2_L_ADDRESS,
        .aif_sub_pkt2_h = hdmi_base_reg + AIF_SUB_PKT2_H_ADDRESS,
        .aif_sub_pkt3_l = hdmi_base_reg + AIF_SUB_PKT3_L_ADDRESS,
        .aif_sub_pkt3_h = hdmi_base_reg + AIF_SUB_PKT3_H_ADDRESS,
        .cea_aud_cfg = hdmi_base_reg + CEA_AUD_CFG_ADDRESS,
        .aud_chst_cfg0 = hdmi_base_reg + AUD_CHST_CFG0_ADDRESS,
        .aud_chst_cfg1 = hdmi_base_reg + AUD_CHST_CFG1_ADDRESS,
        .tx_audio_ctrl = hdmi_base_reg + TX_AUDIO_CTRL_ADDRESS,
        .aud_i2s_ctrl = hdmi_base_reg + AUD_I2S_CTRL_ADDRESS,
        .aud_acr_ctrl = hdmi_base_reg + AUD_ACR_CTRL_ADDRESS,
        .acr_n_val_sw = hdmi_base_reg + ACR_N_VAL_SW_ADDRESS
    };

    writel(0x000a0184, audio_reg.aif_pkt_header);
    writel(0x00000170, audio_reg.aif_sub_pkt0_l);
    writel(0x00000000, audio_reg.aif_sub_pkt0_h);
    writel(0x00000000, audio_reg.aif_sub_pkt1_l);
    writel(0x00000000, audio_reg.aif_sub_pkt1_h);
    writel(0x00000000, audio_reg.aif_sub_pkt2_l);
    writel(0x00000000, audio_reg.aif_sub_pkt2_h);
    writel(0x00000000, audio_reg.aif_sub_pkt3_l);
    writel(0x00000000, audio_reg.aif_sub_pkt3_h);
    writel(0x00000003, audio_reg.cea_aud_cfg);

    writel(0x02000000, audio_reg.aud_chst_cfg0);
    writel(0x000000db, audio_reg.aud_chst_cfg1);
    writel(0x000e40f1, audio_reg.tx_audio_ctrl);
    writel(0x00000b00, audio_reg.aud_i2s_ctrl);
    writel(0x000000db, audio_reg.aud_chst_cfg1);
    writel(0x4001a1040, audio_reg.aud_acr_ctrl);
    writel(0x4001a1048, audio_reg.acr_n_val_sw);

    iounmap(hdmi_base_reg);
}

static td_s32 set_avi_infoframe_selection(struct avi_info_reg avi_reg, td_u32 color_mode)
{
    td_s32 ret = 0;

    if (color_mode == HDMI_AVI_INFOFRAME_RGB) {
        ret = set_avi_infoframe_RGB(avi_reg);
    } else if (color_mode == HDMI_AVI_INFOFRAME_YUV444) {
        ret = set_avi_infoframe_yuv444(avi_reg);
    } else if (color_mode == HDMI_AVI_INFOFRAME_YUV422) {
        ret = set_avi_infoframe_yuv422(avi_reg);
    } else if (color_mode == HDMI_AVI_INFOFRAME_YUV420) {
        ret = set_avi_infoframe_yuv420(avi_reg);
    }

    return ret;
}

void hdmi_stop_pattern(td_u64 base)
{
    void *timing_ctrl = NULL;
    void *hdmi_pattern = NULL;

    void *hdmi_reg_base = NULL;
    hdmi_reg_base = ioremap(base, 0x2804);
    if (hdmi_reg_base == NULL) {
        hdmi_dfx_err("hdmi_reg_base remap failed!\n");
        return;
    }

    timing_ctrl = hdmi_reg_base + TIMING_CTRL_OFFSET;
    hdmi_pattern = hdmi_reg_base + HDMI_PATTERN_OFFSET;

    writel(0x2, timing_ctrl);
    writel(0x0, hdmi_pattern);
    mdelay(10U);
    iounmap(hdmi_reg_base);
}


td_s32 set_avi_infoframe_hdmi_mode(drv_hdmi_infoframe_colorspace *hdmi_infoframe_colorspace)
{
    set_audio_infoframe_48k_2ch();
    td_u32 color_mode = hdmi_infoframe_colorspace->colorspace;
    if (color_mode > HDMI_AVI_INFOFRAME_YUV420) {
        hdmi_dfx_err("hdmi color_mode index is out of range");
        return TD_FAILURE;
    }

    void *hdmi_reg_base = NULL;
    td_u64 base = HDMI_TX_REG_BASE_ADDRESS;
    hdmi_reg_base = ioremap(base, 0x2804);
    if (hdmi_reg_base == NULL) {
        hdmi_dfx_err("hdmi_reg_base remap failed!\n");
        return TD_FAILURE;
    }

    struct avi_info_reg avi_reg = {
        .avi_pkt_header = hdmi_reg_base + AVI_PKT_HEADER_ADDRESS,
        .avi_sub_pkt0_l = hdmi_reg_base + AVI_SUB_PKT0_L_ADDRESS,
        .avi_sub_pkt0_h = hdmi_reg_base + AVI_SUB_PKT0_H_ADDRESS,
        .avi_sub_pkt1_l = hdmi_reg_base + AVI_SUB_PKT1_L_ADDRESS,
        .avi_sub_pkt1_h = hdmi_reg_base + AVI_SUB_PKT1_H_ADDRESS,
        .avi_sub_pkt2_l = hdmi_reg_base + AVI_SUB_PKT2_L_ADDRESS,
        .avi_sub_pkt2_h = hdmi_reg_base + AVI_SUB_PKT2_H_ADDRESS,
        .avi_sub_pkt3_l = hdmi_reg_base + AVI_SUB_PKT3_L_ADDRESS,
        .avi_sub_pkt3_h = hdmi_reg_base + AVI_SUB_PKT3_H_ADDRESS,
        .cea_avi_cfg = hdmi_reg_base + CEA_AVI_CFG_ADDRESS,
        .av_mixer_cfg = hdmi_reg_base + AV_MIXER_CFG_ADDRESS
    };

    int ret = TD_SUCCESS;
    hdmi_stop_pattern(base);

    writel(0x0, avi_reg.cea_avi_cfg);

    writel(0xD0282, avi_reg.avi_pkt_header);
    ret = set_avi_infoframe_selection(avi_reg, color_mode);
    writel(0x4000, avi_reg.avi_sub_pkt0_h);
    writel(0x0, avi_reg.avi_sub_pkt1_l);
    writel(0x0, avi_reg.avi_sub_pkt1_h);
    writel(0x0, avi_reg.avi_sub_pkt2_l);
    writel(0x0, avi_reg.avi_sub_pkt2_h);
    writel(0x0, avi_reg.avi_sub_pkt3_l);
    writel(0x0, avi_reg.avi_sub_pkt3_h);
    writel(0x3, avi_reg.cea_avi_cfg);
    writel(0x00000701, avi_reg.av_mixer_cfg);

    iounmap(hdmi_reg_base);
    return ret;
}