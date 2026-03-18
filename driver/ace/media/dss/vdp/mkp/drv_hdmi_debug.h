/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi debug.
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#ifndef DRV_HDMI_DEBUG_H
#define DRV_HDMI_DEBUG_H

#include "ot_type.h"
#include "hdmi_product_define.h"
#include "drv_hdmi_edid.h"

td_s32 drv_hdmi_debug_source_write(hdmi_proc_entry *file, const char *buf, td_u32 count, const td_s64 * const data);

td_s32 drv_hdmi_debug_edid_raw_read(hdmi_edid_updata_mode mode, td_u8 *data, td_u32 len);

#endif /* DRV_HDMI_DEBUG_H */

