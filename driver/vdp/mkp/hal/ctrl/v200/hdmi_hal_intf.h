/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi hal intf
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#ifndef HDMI_HAL_INTF_H
#define HDMI_HAL_INTF_H

#include "drv_hdmi_common.h"
#include "hdmi_hal_ctrl.h"
#include "hdmi_hal_ddc.h"
#include "hdmi_hal_scdc.h"
#include "drv_hdmi_compatibility.h"

#define HDMI_PHY_POWER_LENGTH 6
#define HDMI0_TMDS_DRV_CFG_CH_ID1 0
#define HDMI0_TMDS_DRV_CFG_CH_ID2 1
#define HDMI0_TMDS_DRV_CFG_CH_ID3 2
#define HDMI1_TMDS_DRV_CFG_CH_ID1 3
#define HDMI1_TMDS_DRV_CFG_CH_ID2 4
#define HDMI1_TMDS_DRV_CFG_CH_ID3 5

typedef struct {
    td_u32 hdmi_power_config_val[HDMI_PHY_POWER_LENGTH];
} hdmi_power_config_info;

typedef struct {
    td_bool init;
    td_u32  tmds_clk;
    hdmi_tx_capability_data tx_capability;
} hdmi_hal_cfg;

typedef struct {
    hdmi_callback event_callback;
    td_void      *event_handle;
} hdmi_hal_event;

#endif /* HDMI_HAL_INTF_H */

