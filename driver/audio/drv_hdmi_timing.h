/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi debug.
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#ifndef DRV_HDMI_TIMING_H
#define DRV_HDMI_TIMING_H

#include "ot_type.h"

#define TIMING_CTRL_OFFSET 0X800
#define HSYNC_TIMING_CCONFIG0_OFFSET 0x804
#define HSYNC_TIMING_CCONFIG1_OFFSET 0x808
#define HSYNC_TIMING_CCONFIG2_OFFSET 0x80C
#define VSYNC_TIMING_CCONFIG0_OFFSET 0x810
#define VSYNC_TIMING_CCONFIG1_OFFSET 0x814
#define VSYNC_TIMING_CCONFIG2_OFFSET 0x818
#define HDMI_PATTERN_OFFSET 0x840
#define HDMI_SOLID_PATTERN_CFG_OFFSET 0x844
#define HDMI_MASK_PATTERN_CFG_OFFSET 0x848

td_s32 hdmi_set_timing(td_u32 timing_width, td_u32 timing_height, td_u32 fps, td_u32 id, td_u32 pattern_color_space);
typedef struct {
    td_u32 hsync_timing_config0;
    td_u32 hsync_timing_config1;
    td_u32 hsync_timing_config2;
    td_u32 vsync_timing_config0;
    td_u32 vsync_timing_config1;
    td_u32 vsync_timing_config2;
} hdmi_timing_reg;


typedef union {
    struct {
        unsigned int tpg_enable : 1; /* [0] */
        unsigned int video_format : 2; /* [2:1] */
        unsigned int solid_pattern_en : 1; /* [3] */
        unsigned int colorbar_en : 1; /* [4] */
        unsigned int square_pattern_en : 1; /* [5] */
        unsigned int mask_pattern_en : 3; /* [8:6] */
        unsigned int replace_pattern_en : 3; /* [11:9] */
        unsigned int bar_pattern_extmode : 1; /* [12] */
        unsigned int cbar_pattern_sel : 2; /* [14:13] */
        unsigned int mix_color_en : 1; /* [15] */
        unsigned int increase_en : 1; /* [16] */
        unsigned int reserved : 15; /* [31:17] */
    } bits;
    unsigned int u32;
} hdmi_pattern_gen_ctrl;

#endif /* DRV_HDMI_TIMING_H */
