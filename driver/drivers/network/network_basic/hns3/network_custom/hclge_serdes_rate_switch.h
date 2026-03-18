/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Create: 2023-2-7
 */
#ifndef HCLGE_SERDES_RATE_SWITCH_H
#define HCLGE_SERDES_RATE_SWITCH_H

#include <linux/types.h>
#include "hclge_main.h"
#include "hclge_serdes_reg_macro.h"
#include "hclge_serdes_common.h"

typedef enum {
    SELECT_PLL_ONLY,
    CHANGE_PLL_FREQ,
    UNSUPPORT_SWITCH
} SWITCH_STRATEGY_E;

/* Cs Calibration Mode */
typedef enum {
    H60_CS_FULL_CAL,
    H60_CS_PLL0_VCO_CAL,
    H60_CS_PLL1_VCO_CAL,
    H60_CS_PLL0_PL1_VCO_CAL,
    H60_CS_CAL_BYPASS,
    H60_CS_PLL0_CAL_COMPLETE,
    H60_CS_PLL1_CAL_COMPLETE,
    H60_CS_CALIB_MODE_END
} H60_CS_CALIB_MODE;

struct mac_cfg_info {
    u8 speed_mode;   /* 10G-400G */
    u8 fec_mode;     /* NOFEC/BASEFEC/RSFEC/RSFEC272 */
    u8 lane_num;     /* 1/2/4/8 */
    u8 sds_speed;    /* 10/25/26/51/53 */
};

enum H60_SERDES_SPEED {
    H60_ETH_10G = 12,
    H60_ETH_25G = 13,
    H60_ETH_26G = 14,
    H60_ETH_51G = 15,
    H60_ETH_53G = 16
};

struct mag_serdes_speed_table {
    u8 sds_speed;
    u8 h60_speed;
};

enum eth_sds_rate_mode {
    ETH_SDS_RATE_UNKNOWN = 0,
    ETH_SDS_RATE_1P25G,
    ETH_SDS_RATE_10P3125G,
    ETH_SDS_RATE_25P78125G,
    ETH_SDS_RATE_26P5625G,
    ETH_SDS_RATE_51P5625G,
    ETH_SDS_RATE_53P125G
};

#define H60_DATA_RATE_GB_SCALAR    1000

enum h60_sds_rate_mode {
    H60_DATA_RATE_UNKNOWN   = 0,
    H60_DATA_RATE_25P78125G = 25,
    H60_DATA_RATE_26P5625G  = 26,
    H60_DATA_RATE_51P5625G  = 51,
    H60_DATA_RATE_53P125G   = 53,
};

typedef struct {
    u8 eth_speed;
    u8 h60_speed;
} h60_sds_speed_table;

typedef struct tagSERDES_CFG_REG {
    SERDES_REG_FIELD_S pll_reg[HILINK_SERDES_MAX_CS_NUM]; /* 2 pll */
    u32 value;
} SERDES_CFG_REG_S;

/* array index */
#define IDX_0       0
#define IDX_1       1
#define IDX_2       2
#define IDX_3       3
#define IDX_4       4
#define IDX_5       5
#define IDX_6       6
#define IDX_7       7
#define IDX_8       8
#define IDX_9       9
#define IDX_10      10
#define IDX_11      11
#define IDX_12      12
#define IDX_13      13
#define IDX_14      14
#define IDX_15      15
#define IDX_16      16
#define IDX_17      17
#define IDX_18      18
#define IDX_19      19
#define G_PRE_CFG_REG_SIZE          20
#define G_CFG_FPLL_REG_SIZE         11
#define G_POST_CFG_REG_SIZE         4

int hclge_do_h60_datarate_switch(struct hclge_dev *hdev, u8 eth_speed);
#endif
