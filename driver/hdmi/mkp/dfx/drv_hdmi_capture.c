/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi debug.
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#include "drv_hdmi_capture.h"
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

static unsigned g_pw;
static unsigned g_ph;
static const unsigned g_timeout = 100U;
struct pixel_param g_pixel;

void format_detect(void)
{
    drv_hdmi_detection hdmi_detection;
    hdmi_set_detection(&hdmi_detection);
    g_pw = hdmi_detection.timing_width;
    g_ph = hdmi_detection.timing_height;
    hdmi_dfx_info("format: [%u %u]\n", g_pw, g_ph);
}

td_s32 pixel_capture(td_u32 y, td_u32 x, struct color *sample)
{
    td_u32 value = (y << 13) + x;
    writel(0x4, g_pixel.ctrl);
    writel(value, g_pixel.pos);
    writel(0x6, g_pixel.ctrl);
    td_u32 count = g_timeout;
    while (count--) {
        msleep(30U);
        td_u32 state = readl(g_pixel.ctrl);
        switch (state & PIXEL_CAPTURE_STATE) {
            case PIXEL_CAPTURE_STATE_ERROR:
                hdmi_dfx_info(KERN_ALERT "[%u %u] capture error\n", y, x);
                return TD_FAILURE;
            case PIXEL_CAPTURE_STATE_BUSY:
                break;
            default:
                sample->g_y = readl(g_pixel.g_y);
                sample->r_cr = readl(g_pixel.r_cr);
                sample->b_cb = readl(g_pixel.b_cb);
                return TD_SUCCESS;
        }
    }
    hdmi_dfx_info(KERN_ALERT "[%u %u] capture timeout\n", y, x);
    return TD_FAILURE;
}

td_s32 hdmi_set_capture(drv_hdmi_capture *capture_index)
{
    td_u32 pos_x = capture_index->pos_x;
    td_u32 pos_y = capture_index->pos_y;
    hdmi_dfx_info("capture_index->pos_x==%u\n", pos_x);
    hdmi_dfx_info("capture_index->pos_y==%u\n", pos_y);

    format_detect();

    if (g_pw == 0 || g_ph == 0) {
        hdmi_dfx_err("there is no video");
        return TD_FAILURE;
    }

    if (pos_x > g_pw || pos_y > g_ph) {
        hdmi_dfx_err("position is out of range");
        return TD_FAILURE;
    }

    void *hdmi_reg_base = NULL;
    td_u64 base = HDMI_TX_REG_BASE_ADDRESS;
    hdmi_reg_base = ioremap(base, 0x2804);
    if (hdmi_reg_base == NULL) {
        hdmi_dfx_err("hdmi_reg_base remap failed!\n");
        return TD_FAILURE;
    }

    g_pixel.pos = hdmi_reg_base + PIXEL_CAPTURE_POS_REG_OFFSET;
    g_pixel.ctrl = hdmi_reg_base + PIXEL_CAPTURE_CTRL_REG_OFFSET;
    g_pixel.g_y = hdmi_reg_base + PIXEL_CAPTURE_G_Y_REG_OFFSET;
    g_pixel.b_cb = hdmi_reg_base + PIXEL_CAPTURE_B_CB_REG_OFFSET;
    g_pixel.r_cr = hdmi_reg_base + PIXEL_CAPTURE_C_CR_REG_OFFSET;

    struct color sample;
    td_s32 ret = pixel_capture(pos_y, pos_x, &sample);
    hdmi_dfx_info("pixel_capture result , color: [%3u %3u %3u]\n", sample.r_cr, sample.g_y, sample.b_cb);
    capture_index->color_r = sample.r_cr;
    capture_index->color_g = sample.g_y;
    capture_index->color_b = sample.b_cb;
    iounmap(hdmi_reg_base);

    return ret;
}