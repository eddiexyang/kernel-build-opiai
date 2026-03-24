/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi debug.
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#include "drv_hdmi_timing.h"
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
#include "ot_osal.h"
#include "hdmi_product_define.h"
#include "drv_hdmi_debug.h"
#include "drv_hdmi_detection.h"
#include "drv_avi_infoframe.h"

td_u32 g_timing_list[][3] = {
    /* timing_height,timing_width,timing_fps */
    {800, 600, 60},
    {1024, 768, 60},
    {1280, 800, 60},
    {1366, 768, 60},
    {1440, 900, 60},
    {1280, 720, 50},
    {1280, 720, 60},
    {1280, 1024, 60},
    {720, 480, 60},
    {720, 576, 50},
    {1920, 1080, 24},
    {1920, 1080, 25},
    {1920, 1080, 30},
    {1920, 1080, 50},
    {1920, 1080, 60},
    {1680, 1050, 60},
    {1920, 1200, 60},
    {2560, 1600, 60},
    {3840, 2160, 24},
    {3840, 2160, 25},
    {3840, 2160, 30},
    {3840, 2160, 50},
    {3840, 2160, 60},
    {640, 480, 60},
    {1600, 1200, 60}
};

td_u32 g_timing_index_list[][6] = {
    /* HSYNC_TIMING_CONFIG0, HSYNC_TIMING_CONFIG1, HSYNC_TIMING_CONFIG2,
    VSYNC_TIMING_CONFIG0, VSYNC_TIMING_CONFIG1, VSYNC_TIMING_CONFIG2 */
    {0x74041F, 0x50348, 0x0, 0x273, 0x525D, 0x4},
    {0x97053F, 0x30418, 0x0, 0x325, 0x9309, 0x6},
    {0xB0059F, 0x60530, 0x0, 0x336, 0x9329, 0x6},
    {0xCE26FF, 0x8C59C, 0x0, 0x31D, 0x6306, 0x3},
    {0xC4063F, 0x605D0, 0x0, 0x39D, 0x938D, 0x6},
    {0xF287BB, 0x3706B8, 0x0, 0x2ED, 0xA2DA, 0x5},
    {0xC94671, 0xDC56E, 0x0, 0x2ED, 0xA2DA, 0x5},
    {0xC50697, 0x60530, 0x0, 0x429, 0x4404, 0x3},
    {0x638359, 0x202E0, 0x0, 0x20C, 0xF1EF, 0x6},
    {0x64035F, 0x182DC, 0x0, 0x270, 0xA24A, 0x5},
    {0x1524ABD, 0x4FC9FE, 0x0, 0x464, 0x9441, 0x5},
    {0x1448A4F, 0x420990, 0x0, 0x464, 0x9441, 0x5},
    {0x10D8897, 0xB07D8, 0x0, 0x464, 0x9441, 0x5},
    {0x1448A4F, 0x420990, 0x0, 0x464, 0x9441, 0x5},
    {0x10D8897, 0xB07D8, 0x0, 0x464, 0x9441, 0x5},
    {0xE2072F, 0x606C0, 0x0, 0x437, 0x9423, 0x6},
    {0x100081F, 0x607B0, 0x0, 0x4D2, 0x94B9, 0x6},
    {0x1500A9F, 0x60A30, 0x0, 0x66D, 0x9649, 0x6},
    {0x2A4957B, 0x9F93FC, 0x0, 0x8C9, 0x12882, 0xA},
    {0x289149F, 0x841320, 0x0, 0x8C9, 0x12882, 0xA},
    {0x21B112F, 0x160FB0, 0x0, 0x8C9, 0x12882, 0xA},
    {0x289149F, 0x841320, 0x0, 0x8C9, 0x12882, 0xA},
    {0x21B112F, 0x160FB0, 0x0, 0x8C9, 0x12882, 0xA},
    {0x58031F, 0x20290, 0x0, 0x20C, 0xC1EC, 0x2},
    {0xF6086F, 0x80680, 0x0, 0x4E1, 0x44B4, 0x3}

};

td_s32 GetListNum(td_u32 timing_width, td_u32 timing_height, td_u32 fps)
{
    hdmi_dfx_info("timing_width==%u\n", timing_width);
    hdmi_dfx_info("timing_height==%u\n", timing_height);
    hdmi_dfx_info("fps==%u\n", fps);
    td_u32 list_len = (td_u32)(sizeof(g_timing_list) / sizeof(g_timing_list[0]));
    td_s32 timing_num = -1;
    td_u32 i;
    for (i = 0; i < list_len; i++) {
        if (timing_width == g_timing_list[i][0] && timing_height == g_timing_list[i][1U] &&
            fps == g_timing_list[i][2U]) {
            timing_num = i;
            break;
        }
    }

    return timing_num;
}

td_s32 hdmi_get_reg_base(td_u32 id, td_u64 *base)
{
    td_u64 hdmi_base_addr = 0;
    if (id == OT_HDMI_ID_0) {
        hdmi_base_addr = HDMI_TX_REG_BASE_ADDRESS;
    } else if (id == OT_HDMI_ID_1) {
        hdmi_base_addr = HDMI_TX_1_REG_BASE_ADDRESS;
    } else {
        return TD_FAILURE;
    }
    base = &hdmi_base_addr;
    return TD_SUCCESS;
}

td_u32 hdmi_get_colorsapce_index(td_u32 pattern_color_space)
{
    hdmi_pattern_gen_ctrl pattern_gen_ctrl;
    pattern_gen_ctrl.u32 = 0;
    pattern_gen_ctrl.bits.tpg_enable = 0x1;
    pattern_gen_ctrl.bits.colorbar_en = 0x1;

    if (pattern_color_space == HDMI_AVI_INFOFRAME_RGB) {
        pattern_gen_ctrl.bits.video_format = 0;
        return pattern_gen_ctrl.u32;
    } else if (pattern_color_space == HDMI_AVI_INFOFRAME_YUV422) {
        pattern_gen_ctrl.bits.video_format = 0x1;
    } else if (pattern_color_space == HDMI_AVI_INFOFRAME_YUV420) {
        pattern_gen_ctrl.bits.video_format = 0x2;
    } else if (pattern_color_space == HDMI_AVI_INFOFRAME_YUV444) {
        pattern_gen_ctrl.bits.video_format = 0x0;
    }
    pattern_gen_ctrl.bits.cbar_pattern_sel = 0x1;

    return pattern_gen_ctrl.u32;
}

td_s32 hdmi_set_timing(td_u32 timing_width, td_u32 timing_height, td_u32 fps, td_u32 id, td_u32 pattern_color_space)
{
    td_u64 base;
    td_s32 ret = hdmi_get_reg_base(id, &base);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

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
        return TD_FAILURE;
    }

    timing_ctrl = hdmi_reg_base + TIMING_CTRL_OFFSET;
    hsync_timing_cconfig0 = hdmi_reg_base + HSYNC_TIMING_CCONFIG0_OFFSET;
    hsync_timing_cconfig1 = hdmi_reg_base + HSYNC_TIMING_CCONFIG1_OFFSET;
    hsync_timing_cconfig2 = hdmi_reg_base + HSYNC_TIMING_CCONFIG2_OFFSET;
    vsync_timing_cconfig0 = hdmi_reg_base + VSYNC_TIMING_CCONFIG0_OFFSET;
    vsync_timing_cconfig1 = hdmi_reg_base + VSYNC_TIMING_CCONFIG1_OFFSET;
    vsync_timing_cconfig2 = hdmi_reg_base + VSYNC_TIMING_CCONFIG2_OFFSET;
    hdmi_pattern = hdmi_reg_base + HDMI_PATTERN_OFFSET;

    td_s32 timing_num = GetListNum(timing_width, timing_height, fps);

    hdmi_dfx_info("timing_num==%d\n", timing_num);

    if (timing_num == -1) {
        hdmi_dfx_err("hdmi timing index is out of range");
        iounmap(hdmi_reg_base);
        return TD_FAILURE;
    }

    hdmi_timing_reg timing_reg;

    timing_reg.hsync_timing_config0 = g_timing_index_list[timing_num][0];
    timing_reg.hsync_timing_config1 = g_timing_index_list[timing_num][1];
    timing_reg.hsync_timing_config2 = g_timing_index_list[timing_num][2U];
    timing_reg.vsync_timing_config0 = g_timing_index_list[timing_num][3U];
    timing_reg.vsync_timing_config1 = g_timing_index_list[timing_num][4U];
    timing_reg.vsync_timing_config2 = g_timing_index_list[timing_num][5U];

    writel(0x2, timing_ctrl); // close hdmi timing
    writel(timing_reg.hsync_timing_config0, hsync_timing_cconfig0);
    writel(timing_reg.hsync_timing_config1, hsync_timing_cconfig1);
    writel(timing_reg.hsync_timing_config2, hsync_timing_cconfig2);
    writel(timing_reg.vsync_timing_config0, vsync_timing_cconfig0);
    writel(timing_reg.vsync_timing_config1, vsync_timing_cconfig1);
    writel(timing_reg.vsync_timing_config2, vsync_timing_cconfig2);
    writel(0x3, timing_ctrl);   // open hdmi timing
    td_u32 colorsapce_index = hdmi_get_colorsapce_index(pattern_color_space);
    writel(colorsapce_index, hdmi_pattern); // open colorbar pattern
    mdelay(10U);

    iounmap(hdmi_reg_base);

    return TD_SUCCESS;
}