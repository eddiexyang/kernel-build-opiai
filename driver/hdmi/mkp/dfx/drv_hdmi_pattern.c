/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi debug.
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#include "drv_hdmi_pattern.h"
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

void hdmi_set_timing_default(td_u64 base)
{
    void *timing_ctrl = NULL;
    void *hsync_timing_cconfig0 = NULL;
    void *hsync_timing_cconfig1 = NULL;
    void *hsync_timing_cconfig2 = NULL;
    void *vsync_timing_cconfig0 = NULL;
    void *vsync_timing_cconfig1 = NULL;
    void *vsync_timing_cconfig2 = NULL;
    void *hdmi_pattern = NULL;

    void *hdmi_reg_base = NULL;
    hdmi_reg_base = ioremap(base, 0x2804);
    if (hdmi_reg_base == NULL) {
        hdmi_dfx_err("hdmi_reg_base remap failed!\n");
        return;
    }

    timing_ctrl = hdmi_reg_base + TIMING_CTRL_OFFSET;
    hsync_timing_cconfig0 = hdmi_reg_base + HSYNC_TIMING_CCONFIG0_OFFSET;
    hsync_timing_cconfig1 = hdmi_reg_base + HSYNC_TIMING_CCONFIG1_OFFSET;
    hsync_timing_cconfig2 = hdmi_reg_base + HSYNC_TIMING_CCONFIG2_OFFSET;
    vsync_timing_cconfig0 = hdmi_reg_base + VSYNC_TIMING_CCONFIG0_OFFSET;
    vsync_timing_cconfig1 = hdmi_reg_base + VSYNC_TIMING_CCONFIG1_OFFSET;
    vsync_timing_cconfig2 = hdmi_reg_base + VSYNC_TIMING_CCONFIG2_OFFSET;
    hdmi_pattern = hdmi_reg_base + HDMI_PATTERN_OFFSET;

    writel(0x2, timing_ctrl);
    writel(0x74041F, hsync_timing_cconfig0);
    writel(0x50348, hsync_timing_cconfig1);
    writel(0x0, hsync_timing_cconfig2);
    writel(0x273, vsync_timing_cconfig0);
    writel(0x525D, vsync_timing_cconfig1);
    writel(0x4, vsync_timing_cconfig2);
    writel(0x3, timing_ctrl);
    writel(0x11, hdmi_pattern);
    mdelay(10U);
    iounmap(hdmi_reg_base);
}

void hdmi_set_pattern_solid(void *hdmi_pattern, void *hdmi_solid_pattern_cfg,
    const drv_hdmi_pattern_mode *hdmi_pattern_mode)
{
    hdmi_debug_trace("hdmi pattern_mode == 0 , squares pattern");
    writel(0x9, hdmi_pattern);

    td_u32 pattern_mode = hdmi_pattern_mode->pattern_mode;
    td_u32 pattern_color_r = hdmi_pattern_mode->pattern_color_r;
    td_u32 pattern_color_g = hdmi_pattern_mode->pattern_color_g;
    td_u32 pattern_color_b = hdmi_pattern_mode->pattern_color_b;

    solid_pattern_cfg solid_pattern_color;
    solid_pattern_color.bits.reserved_1 = 0x0;
    solid_pattern_color.bits.solid_pattern_r = (unsigned int)pattern_color_r;
    solid_pattern_color.bits.solid_pattern_g = (unsigned int)pattern_color_g;
    solid_pattern_color.bits.solid_pattern_b = (unsigned int)pattern_color_b;
    writel(solid_pattern_color.u32, hdmi_solid_pattern_cfg);
}

void hdmi_set_pattern_replace(void *hdmi_pattern, void *hdmi_solid_pattern_cfg,
    const drv_hdmi_pattern_mode *hdmi_pattern_mode)
{
    hdmi_debug_trace("hdmi pattern_mode == 1 , replace pattern");
    writel(0xE01, hdmi_pattern);
    td_u32 pattern_mode = hdmi_pattern_mode->pattern_mode;
    td_u32 pattern_color_r = hdmi_pattern_mode->pattern_color_r;
    td_u32 pattern_color_g = hdmi_pattern_mode->pattern_color_g;
    td_u32 pattern_color_b = hdmi_pattern_mode->pattern_color_b;

    solid_pattern_cfg solid_pattern_color;
    solid_pattern_color.bits.reserved_1 = 0x0;
    solid_pattern_color.bits.solid_pattern_r = (unsigned int)pattern_color_r;
    solid_pattern_color.bits.solid_pattern_g = (unsigned int)pattern_color_g;
    solid_pattern_color.bits.solid_pattern_b = (unsigned int)pattern_color_b;
    writel(solid_pattern_color.u32, hdmi_solid_pattern_cfg);
}

void hdmi_set_pattern_mask(void *hdmi_pattern, void *hdmi_mask_pattern_cfg,
    const drv_hdmi_pattern_mode *hdmi_pattern_mode)
{
    hdmi_debug_trace("hdmi pattern_mode == 2 , mask pattern");
    writel(0x1C1, hdmi_pattern);

    td_u32 pattern_color_r = hdmi_pattern_mode->pattern_color_r;
    td_u32 pattern_color_g = hdmi_pattern_mode->pattern_color_g;
    td_u32 pattern_color_b = hdmi_pattern_mode->pattern_color_b;

    mask_pattern_cfg mask_pattern_color;
    mask_pattern_color.bits.reserved_1 = 0x0;
    mask_pattern_color.bits.mask_pattern_r = (unsigned int)pattern_color_r;
    mask_pattern_color.bits.mask_pattern_g = (unsigned int)pattern_color_g;
    mask_pattern_color.bits.mask_pattern_b = (unsigned int)pattern_color_b;
    writel(mask_pattern_color.u32, hdmi_mask_pattern_cfg);
}


td_s32 hdmi_set_pattern(drv_hdmi_pattern_mode *hdmi_pattern_mode)
{
    td_u32 pattern_mode = hdmi_pattern_mode->pattern_mode;
    td_u32 pattern_color_r = hdmi_pattern_mode->pattern_color_r;
    td_u32 pattern_color_g = hdmi_pattern_mode->pattern_color_g;
    td_u32 pattern_color_b = hdmi_pattern_mode->pattern_color_b;

    hdmi_dfx_info("pattern_mode==%u\n", pattern_mode);
    hdmi_dfx_info("pattern_color_r==%u\n", pattern_color_r);
    hdmi_dfx_info("pattern_color_g==%u\n", pattern_color_g);
    hdmi_dfx_info("pattern_color_b==%u\n", pattern_color_b);

    if (pattern_color_r > 1023U || pattern_color_r > 1023U || pattern_color_b > 1023U) {
        hdmi_dfx_err("hdmi pattern index is out of range");
        return TD_FAILURE;
    }

    if (pattern_mode > HDMI_PATTERN_MODE_MAX) {
        hdmi_dfx_err("hdmi pattern index is out of range");
        return TD_FAILURE;
    }

    td_u64 base = HDMI_TX_REG_BASE_ADDRESS;
    void *hdmi_pattern = NULL;
    void *hdmi_solid_pattern_cfg = NULL;
    void *hdmi_mask_pattern_cfg = NULL;

    void *hdmi_reg_base = NULL;
    hdmi_reg_base = ioremap(base, 0x2804);
    if (hdmi_reg_base == NULL) {
        hdmi_dfx_err("hdmi_reg_base remap failed!\n");
        return TD_FAILURE;
    }

    hdmi_pattern = hdmi_reg_base + HDMI_PATTERN_OFFSET;
    hdmi_solid_pattern_cfg = hdmi_reg_base + HDMI_SOLID_PATTERN_CFG_OFFSET;
    hdmi_mask_pattern_cfg = hdmi_reg_base + HDMI_MASK_PATTERN_CFG_OFFSET;
    writel(0x11, hdmi_pattern);
    mdelay(10U);

    hdmi_set_timing_default(base);

    if (pattern_mode == HDMI_PATTERN_MODE_COLORBAR) {
        hdmi_debug_trace("hdmi pattern_mode == 3 , coloarbar pattern");
        writel(0x11, hdmi_pattern);
    }

    if (pattern_mode == HDMI_PATTERN_MODE_SOLID) {
        hdmi_set_pattern_solid(hdmi_pattern, hdmi_solid_pattern_cfg, hdmi_pattern_mode);
    }

    if (pattern_mode == HDMI_PATTERN_MODE_REPLACE) {
        hdmi_set_pattern_replace(hdmi_pattern, hdmi_solid_pattern_cfg, hdmi_pattern_mode);
    }

    if (pattern_mode == HDMI_PATTERN_MODE_MASK) {
        hdmi_set_pattern_mask(hdmi_pattern, hdmi_mask_pattern_cfg, hdmi_pattern_mode);
    }

    if (pattern_mode == HDMI_PATTERN_MODE_SQUARES) {
        hdmi_debug_trace("hdmi pattern_mode == 4 , mask pattern");
        writel(0x31, hdmi_pattern);
    }
    iounmap(hdmi_reg_base);
    return TD_SUCCESS;
}