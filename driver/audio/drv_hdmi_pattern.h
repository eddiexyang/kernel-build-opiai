/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi debug.
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#ifndef DRV_HDMI_PATTERN_H
#define DRV_HDMI_PATTERN_H
#include "ot_type.h"
#include "drv_hdmi_ioctl.h"
#include "drv_hdmi_timing.h"

typedef enum {
    HDMI_PATTERN_MODE_SOLID,
    HDMI_PATTERN_MODE_REPLACE,
    HDMI_PATTERN_MODE_MASK,
    HDMI_PATTERN_MODE_COLORBAR,
    HDMI_PATTERN_MODE_SQUARES,
    HDMI_PATTERN_MODE_MAX
} hdmi_pattern_mode_list;

/* define the union solid_pattern_cfg */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int solid_pattern_b : 10; /* [9..0]  */
        unsigned int solid_pattern_g : 10; /* [19..10]  */
        unsigned int solid_pattern_r : 10; /* [29..20]  */
        unsigned int reserved_1 : 2;       /* [31..30]  */
    } bits;

    /* define an unsigned member */
    unsigned int u32;
} solid_pattern_cfg;

/* define the union mask_pattern_cfg */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int mask_pattern_b : 10; /* [9..0]  */
        unsigned int mask_pattern_g : 10; /* [19..10]  */
        unsigned int mask_pattern_r : 10; /* [29..20]  */
        unsigned int reserved_1 : 2;      /* [31..30]  */
    } bits;

    /* define an unsigned member */
    unsigned int u32;
} mask_pattern_cfg;

td_s32 hdmi_set_pattern(drv_hdmi_pattern_mode *hdmi_pattern_mode);
#endif /* DRV_HDMI_DEBUG_H */
