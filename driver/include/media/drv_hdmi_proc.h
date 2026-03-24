/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2022. All rights reserved.
 * Description: hdmi proc.
 * Author: Hisilicon multimedia software group
 * Create: 2019/09/06
 */
#ifndef DRV_HDMI_PROC_H
#define DRV_HDMI_PROC_H

#include "ot_type.h"
#include "hdmi_product_define.h"

td_s32 drv_hdmi_proc_show(hdmi_proc_entry *file);

td_s32 drv_hdmi_vo_proc_show(hdmi_proc_entry *file);

td_s32 drv_hdmi_ao_proc_show(hdmi_proc_entry *file);

td_s32 drv_hdmi_sink_proc_show(hdmi_proc_entry *file);

#endif /* DRV_HDMI_PROC_H */

