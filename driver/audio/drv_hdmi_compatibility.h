/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi compatibility.
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#ifndef DRV_HDMI_COMPATIBILITY_H
#define DRV_HDMI_COMPATIBILITY_H

#include "drv_hdmi_common.h"
#include "drv_hdmi_intf.h"

typedef struct {
    td_bool setmute_send; /* whether send AVMute */
    td_bool avi_use_gen5;
    td_bool avi_use_gen5_debug;
    td_u8   frl_compress_mode;
} compatibility_info;

td_s32 drv_hdmi_compat_avmute_get(hdmi_device_id hdmi_id, td_bool avmute, hdmi_avmute_cfg *avmute_cfg);

td_s32 drv_hdmi_compat_delay_get(hdmi_device_id hdmi_id, hdmi_delay *delay);

td_s32 drv_hdmi_compat_stop_delay(hdmi_device_id hdmi_id);

compatibility_info *compat_info_get(hdmi_device_id hdmi_id);

td_s32 compatibility_info_update(hdmi_device_id hdmi_id);

td_void compatibility_info_default_set(hdmi_device_id hdmi_id);

#endif  /* DRV_HDMI_COMPATIBILITY_H */

