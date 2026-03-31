/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * Description:
 * Author: huawei
 * Create: 2019-10-15
 */

#ifndef SDK_HILINK_INIT_H
#define SDK_HILINK_INIT_H

#include "sdk_hilink_pub.h"
#include "sdk_hilink_common.h"

#define EVB_BOARD_ID_BOM1_20T   0x900
#define EVB_BOARD_ID_BOM2_20T_0 0x901
#define EVB_BOARD_ID_BOM2_20T_1 0x903

#define RECOGNIZED_UNSUPPORTED_BOARD_ID 1

#define DTS_BOARD_ID_ARRAY_SIZE 6
#define DTS_BOARD_ID_OFFSET 4

#define HILINK_CRG_MODE_OFFSET 2
#define HILINK_SDS_MODE_OFFSET 4
#define HILINK_MCLK_DIV_OFFSET 2
#define HILINK_DS_ADAPT_API_REG_NUM 8

// default: clk0-Gen3/Gen4 clk1-Gen1/Gen2
#define LINK_MODE_MCLK0 0
#define LINK_MODE_MCLK1 1

#define MODE_CTRL_PIN       0
#define MODE_CTRL_MSG_BUS   1
#define LANE_MODE_PCIE      0
#define LANE_MODE_SAS       1

/* HILINK_DsDataRateSwitch */
#define DS_DATA_WIDTH_OFFSET 6
#define DS_DATA_RATE_SWITCH_TIMEOUT_CNT 12

#define AO_REG_WRITE_TIMEOUT_CNT 200
#define AO_REG_WRITING 1
#define AO_REG_WRITE_SUCCESS 0
#define AO_REG_WRITE_FAILED (-1)

/* RunAdaption */
#define RUN_ADAPTION_TIMEOUT_CNT 600
#define RUN_CALIBRATION_TIMEOUT_CNT 100

typedef struct {
    u32 ctrlMode;
    u32 laneMode;
} MessageBus_t;

typedef struct {
    u32 macro;
    u32 val;
} SerdesPowerOnThreadPara_t;

#ifdef CFG_SOC_PLATFORM_MDC_V11
#define PCIE_MODE   0
#define ETH_MODE    1
struct serdes_dts_param {
    u32 mode;
    u32 tx_deemph[LANE_MAX];
    u32 tx_margin[LANE_MAX];
};
#endif

int hilink_module_suspend(void);
int hilink_module_resume(void);

#endif
