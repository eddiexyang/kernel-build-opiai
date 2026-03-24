/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi debug.
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#ifndef DRV_HDMI_HOTPLUG_H
#define DRV_HDMI_HOTPLUG_H

#include "ot_type.h"
#include "drv_hdmi_ioctl.h"

#define HDMI_HOTPLUG_STATE_OFFSET 0x4028

typedef union {
    struct {
        unsigned int hotplug_state : 1; /* [0] */
        unsigned int phy_rx_sense : 1;  /* [1] */
        unsigned int rsv_4 : 30;        /* [31:2] */
    } bits;
    unsigned int u32;
} hdmi_tx_aon_state;

td_s32 hdmi_set_hotplug(drv_hdmi_hotplug_status *hotplug_status);
#endif /* DRV_HDMI_HOTPLUG_H */
