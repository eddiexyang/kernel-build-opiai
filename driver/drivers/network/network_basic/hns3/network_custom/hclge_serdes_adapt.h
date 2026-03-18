/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Create: 2023-2-7
 */
#ifndef HCLGE_SERDES_ADAPT_H
#define HCLGE_SERDES_ADAPT_H

#include <linux/types.h>
#include "hclge_main.h"
#include "hclge_serdes_common.h"

typedef enum {
    LMS_LOS = 0,
    CDR_LOS
} SERDES_LOS_E;

typedef enum {
    EVB_BOARD  = 0x0,
    PCIe_BOARD = 0x1,
    PoD_BOARD  = 0x2,
    A_K_BOARD  = 0x3,
    PoD_BUSINESS_BOARD = 0x4,
    A_X_BOARD  = 0x5,
    UNKOWN_BOARD_TYPE = 0xff
} SERDES_BOARD_TYPE_E;

typedef enum {
    HAM1_NPU0 = 0,
    HAM1_NPU1,
    HAM2_NPU0,
    HAM2_NPU1
} SERDES_HAM_NPU_ID_E;

typedef enum {
    DEVICE_0 = 0,
    DEVICE_1,
    DEVICE_2,
    DEVICE_3,
    DEVICE_4,
    DEVICE_5,
    DEVICE_6,
    DEVICE_7,
    DEVICE_8,
    DEVICE_9,
    DEVICE_10,
    DEVICE_11,
    DEVICE_12,
    DEVICE_13,
    DEVICE_14,
    DEVICE_15,
    MAX_DEVICE_NUM
} DEVICE_ID_E;

#define EVB_RX_INSERTION_LOSS 30
#define PoD_RX_INSERTION_LOSS 24
#define PoD_TX_FFE_PRE_VAL (-15)
#define PoD_TX_FFE_POST_VAL (-3)

#define A_K_TX_FFE_PRE_VAL (-9)
#define A_K_TX_FFE_POST_VAL (0)

#define A_X_TX_FFE_PRE_VAL (-15)
#define A_X_TX_FFE_POST_VAL 0

#define PoD_BUSINESS_TX_FFE_PRE_VAL (-9)
#define PoD_BUSINESS_TX_FFE_POST_VAL 0

#define HAM1_NPU0_ETH_M0_RX_INSERTION_LOSS 28
#define HAM1_NPU1_ETH_M0_RX_INSERTION_LOSS 27
#define HAM2_NPU0_ETH_M0_RX_INSERTION_LOSS 30
#define HAM2_NPU1_ETH_M0_RX_INSERTION_LOSS 28
#define BOARD_TYPE_OFFSET 4
#define BOARD_TYPE_MASK 0x7

#define A_X_BOARD_INSERT_LOSS_CYCLE_NUM 4

typedef enum {
    SERDES_ADAPT_STEP_INIT = 0,
    SERDES_ADAPT_STEP_CONT,
    SERDES_ADAPT_STEP_CTLE,
    SERDES_ADAPT_STEP_DFE,
    SERDES_ADAPT_FAST_SEARCH,
    SERDES_ADAPT_FAST_REFINE,
    SERDES_ADAPT_STEP_END
} SERDES_ADAPT_STEP_E;

#define H60_DS_STATE_INIT_ADAPT  3
#define H60_DS_STATE_CONT_ADAPT  4
#define H60_DS_STATE_ABIST       5

#define H60_ADAPT_INIT_MASK 0xf
#define H60_ADAPT_TIMEOUT   5000  /* 5s */

int hclge_do_h60_adapt(struct hclge_dev *hdev);
int hclge_h60_check_los(struct hclge_dev *hdev, u32 *los_status, SERDES_LOS_E los_type);
void set_tx_fir(struct hclge_dev *hdev, CHIP_INFO_S *chip_info, int txpre, int txpost);
u32 hisds_ds_freeze_adaptation_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info);
u32 hisds_ds_unfreeze_adaptation_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info);
#endif
