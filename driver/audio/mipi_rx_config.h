/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: mipi config header file
 * Author: Hisilicon multimedia software group
 * Create: 2023-03-08
 */
#ifndef MIPI_RX_CONFIG_H
#define MIPI_RX_CONFIG_H

#include "hi_mipi_rx.h"

#define COMBO_DEV_MAX_NUM  4U
/* global variables definition */

#define MIPI_RX_STATUS_RUNNING                0U
#define MIPI_RX_STATUS_STOPPING               1U
#define MIPI_RX_STATUS_STOPPED                2U
#define MIPI_RX_STATUS_SUSPENDING             3U // 开始休眠
#define MIPI_RX_STATUS_SUSPENDED              4U // 休眠结束
#define MIPI_RX_STATUS_RESUMING               5U // 开始唤醒, 唤醒结束则恢复休眠前的状态

typedef enum {
    DISABLE = 0,
    ENABLE
} clock_status_e;

typedef enum {
    RESET = 0,
    UNRESET
} reset_status_e;

typedef struct {
    lane_divide_mode_t lane_divide_mode;
    unsigned char hs_mode_cfged;
    combo_dev_attr_t combo_dev_attr[COMBO_DEV_MAX_NUM];
    unsigned char dev_valid[COMBO_DEV_MAX_NUM];
    unsigned char dev_cfged[COMBO_DEV_MAX_NUM];
    unsigned int lane_bitmap[COMBO_DEV_MAX_NUM];

    clock_status_e mipi_clock_status[COMBO_DEV_MAX_NUM];
    reset_status_e reset_mipi_status[COMBO_DEV_MAX_NUM];
    clock_status_e slvs_clock_status[COMBO_DEV_MAX_NUM];
    reset_status_e reset_slvs_status[COMBO_DEV_MAX_NUM];
    clock_status_e sns_clk_source_cfged[SNS_MAX_CLK_SOURCE_NUM];
    reset_status_e sns_rst_source_cfged[SNS_MAX_RST_SOURCE_NUM];
    unsigned char sns_clk_config_cfged[SNS_MAX_CLK_SOURCE_NUM];
    sns_clk_freq_t clk_freq[SNS_MAX_CLK_SOURCE_NUM];
} mipi_dev_ctx_t;

#define MIPI_CHECK_MIPI_DEV_RETURN(combo_dev)                              \
    do {                                                                   \
        if ((combo_dev) >= (COMBO_DEV_MAX_NUM)) {      \
            mipi_rx_err_trace("Err MIPI combo_dev %u, should be less than %u!\n",   \
                (combo_dev), (COMBO_DEV_MAX_NUM));   \
            return HI_FAILURE;                                              \
        }                                                                  \
    } while (0)

extern mipi_dev_ctx_t g_mipi_dev_ctx;
extern osal_spinlock_t g_mipi_ctx_spinlock;

#endif