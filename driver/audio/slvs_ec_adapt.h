/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: slvs_ec reg header file
 * Author: Hisilicon multimedia software group
 * Create: 2023-03-08
 */
#include "hi_mipi_rx.h"

#ifndef SLVS_EC_ADAPT_H
#define SLVS_EC_ADAPT_H

#define SLVS_MAX_LINK_NUM       2U   /* has 2 links per phy */
#define SLVS_MAX_DEV_NUM        2U   /* suppor 2 slvs-ec dev */
#define SLVS_DEV_NUM_START      0U
#define SLVS_PHY_NUM            2U   /* uppor 2 SLVS HPY */
#define SLVS_REG_BASE_NUM       1   /* 1 SLVS reg base add */
#define SLVS_IRQ_NUM            1   /* 1 SLVS interrupt */

#define MIPI_CHECK_SLVS_DEV_RETURN(combo_dev)                              \
    do {                                                                   \
        if (((combo_dev) < SLVS_DEV_NUM_START) || ((combo_dev) >= (SLVS_DEV_NUM_START + SLVS_MAX_DEV_NUM))) {      \
            mipi_rx_err_trace("Err slvs-ec combo_dev %u, should be in [%u, %u)!\n", \
                (combo_dev), (SLVS_DEV_NUM_START), (SLVS_DEV_NUM_START + SLVS_MAX_DEV_NUM));   \
            return HI_FAILURE;                                              \
        }                                                                  \
    } while (0)

#define MIPI_CHECK_PHY_ID_RETURN_NULL(phy_id)                              \
    do {                                                                   \
        if (((phy_id) < 0) || ((phy_id) >= SLVS_PHY_NUM)) {      \
            mipi_rx_err_trace("Err phy_id %u, should be in [0, %u)!\n", \
                (phy_id), (SLVS_PHY_NUM));   \
            return HI_NULL;                                              \
        }                                                                  \
    } while (0)

typedef enum {
    SLVS_HD_CRC_ERR         = 0x1 << 0,         /* header crc err */
    SLVS_PLD_CRC_ERR        = 0x1 << 1,         /* payload crc err */
    SLVS_ECC_ERR            = 0x1 << 2,         /* ECC err */
    SLVS_DATA_FIFO_W_ERR    = 0x1 << 3,         /* data fifo write err */
    SLVS_DATA_FIFO_R_ERR    = 0x1 << 4,         /* data fifo read err */
    SLVS_CMD_FIFO_FULL      = 0x1 << 5,         /* command fifo full */
    SLVS_SKEW_ERR           = 0x1 << 6,         /* skew err */
    SLVS_VSYNC_RAW          = 0x1 << 7,         /* VSYNC cnt */
} SLVS_LINK_INT_STAT;

typedef struct {
    unsigned int afifo_align_cnt[SLVS_LANE_NUM];
    unsigned int code_err_cnt[SLVS_LANE_NUM];
    unsigned int disp_err_cnt[SLVS_LANE_NUM];
} slvs_phy_int_state_t;

typedef struct {
    unsigned int header_crc_err_cnt;
    unsigned int payload_crc_err_cnt;
    unsigned int ecc_err_cnt;
    unsigned int data_fifo_w_err_cnt;
    unsigned int data_fifo_r_err_cnt;
    unsigned int cmd_fifo_full_err_cnt;
    unsigned int skew_err_cnt;
    unsigned int vsync_cnt;
} slvs_link_int_state_t;

#endif // SLVS_EC_ADAPT_H