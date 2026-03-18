/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi debug.
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#ifndef DRV_HDMI_RESET_H
#define DRV_HDMI_RESET_H

#include "ot_type.h"
#include "drv_hdmi_ioctl.h"

#define TX_PWD_RST_CTRL_OFFSET 0x0010

td_s32 hdmi_reset_clock(const drv_hdmi_reset_clk *reset_clk);

#endif /* DRV_HDMI_RESET_H */
