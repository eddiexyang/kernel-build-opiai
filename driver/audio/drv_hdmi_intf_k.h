/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi intf kernel.
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#ifndef DRV_HDMI_INTF_K_H
#define DRV_HDMI_INTF_K_H

#include "drv_hdmi_common.h"

td_s32 drv_hdmi_kernel_event_callback(td_void *data, hdmi_event event);

td_s32 drv_hdmi_csc_param_set_export(hdmi_dev_id hdmi, const hdmi_csc_param *csc_param);

td_s32 drv_hdmi_video_param_set_export(hdmi_dev_id hdmi, const hdmi_video_param *video_param);

td_s32 drv_hdmi_stop_export(hdmi_dev_id hdmi);

#endif /* DRV_HDMI_INTF_K_H */

