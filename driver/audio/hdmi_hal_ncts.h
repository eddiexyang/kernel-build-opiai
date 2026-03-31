/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi ncts
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#ifndef HDMI_HAL_NCTS_H
#define HDMI_HAL_NCTS_H

#include "ot_type.h"

typedef struct {
    td_u32 audio_smp_rate;
    td_u32 tmds_clk;
    td_u32 n_value;
    td_u32 cts_value;
} hdmi_audio_cts_n;

td_u32 hal_hdmi_n_value_get(td_u32 sample_rate, td_u32 tmds_clk);

td_u32 hal_hdmi_cts_value_get(td_u32 sample_rate, td_u32 tmds_clk);

#endif /* HDMI_HAL_NCTS_H */

