/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi debug.
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#ifndef DRV_HDMI_CAPTURE_H
#define DRV_HDMI_CAPTURE_H
#include "ot_type.h"
#include "drv_hdmi_ioctl.h"

#define PIXEL_CAPTURE_STATE_DONE 0x10
#define PIXEL_CAPTURE_STATE_BUSY 0x20
#define PIXEL_CAPTURE_STATE_ERROR 0x40
#define PIXEL_CAPTURE_STATE (PIXEL_CAPTURE_STATE_BUSY | PIXEL_CAPTURE_STATE_ERROR)

#define PIXEL_CAPTURE_POS_REG_OFFSET 0x910
#define PIXEL_CAPTURE_CTRL_REG_OFFSET 0x90C
#define PIXEL_CAPTURE_G_Y_REG_OFFSET 0x914
#define PIXEL_CAPTURE_B_CB_REG_OFFSET 0x918
#define PIXEL_CAPTURE_C_CR_REG_OFFSET 0x91C

struct color {
    td_u32 g_y;
    td_u32 b_cb;
    td_u32 r_cr;
};

struct pixel_param {
    void *pos;
    void *ctrl;
    void *g_y;
    void *b_cb;
    void *r_cr;
};

td_s32 hdmi_set_capture(drv_hdmi_capture *capture_index);

#endif /* DRV_HDMI_CAPTURE_H */