/*
 * Copyright (C) Hisilicon Technologies Co., Ltd. 2016-2019. All rights reserved.
 * Description: mipi tx header file.
 * Author: Hisilicon multimedia software group
 * Create: 2016/06/28
 */

#ifndef __MIPI_TX_H__
#define __MIPI_TX_H__

#include "ot_mipi_tx.h"
#include "mipi_tx_reg.h"
#include "vdp_comm_debug.h"

#define MIPI_TX_MAX_SYNC_VACT  4096
#define MIPI_TX_MIN_SYNC_VACT  100
#define MIPI_TX_MAX_SYNC_VBB   256
#define MIPI_TX_MIN_SYNC_VBB   1
#define MIPI_TX_MAX_SYNC_VFB   256
#define MIPI_TX_MIN_SYNC_VFB   1
#define MIPI_TX_MAX_SYNC_HACT  4096
#define MIPI_TX_MIN_SYNC_HACT  1
#define MIPI_TX_MAX_SYNC_HBB   65535
#define MIPI_TX_MIN_SYNC_HBB   1
#define MIPI_TX_MAX_SYNC_HFB   65535
#define MIPI_TX_MIN_SYNC_HFB   1
#define MIPI_TX_MAX_SYNC_HPW   65535
#define MIPI_TX_MIN_SYNC_HPW   1
#define MIPI_TX_MAX_SYNC_VPW   256
#define MIPI_TX_MIN_SYNC_VPW   1

#define MIPI_TX_MAX_PHY_DATA_RATE 1250 /* Mbps, Only Support 1080P */
#define MIPI_TX_MIN_PHY_DATA_RATE 80 /* Mbps */

#define MIPITX_REG_BASE_ADDRESS 0x400200000
#define OPERATION_MODE_OFFSET 0x0308
#define HBP_HSA_IN_OFFSET 0x0310
#define HFP_HACT_IN_OFFSET 0x030C
#define VIDEOM_VSA_LINES_OFFSET 0x0054
#define VIDEOM_VBP_LINES_OFFSET 0x0058
#define VIDEOM_VFP_LINES_OFFSET 0x005C
#define VIDEOM_VACTIVE_LINES_OFFSET 0x0060
#define VIDEOM_HSA_SIZE_OFFSET 0x0048
#define VIDEOM_HBP_SIZE_OFFSET 0x004C
#define VIDEOM_HLINE_SIZE_OFFSET 0x0050

#define MIPI_TX_MAX_PIXEL_CLK ((MIPI_TX_MAX_PHY_DATA_RATE) * (LANE_MAX_NUM) * 1000 / 8) /* KHz , 8: min 8bit/pixel */
#define MIPI_TX_MIN_PIXEL_CLK ((MIPI_TX_MIN_PHY_DATA_RATE) * (1) * 1000 / 24) /* KHz, 24: max 24bit/pixel */

#define mipi_tx_err(fmt, ...)  \
        VDP_ERR_TRACE("[drv_mipitx][ERR][%s:%d]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__) \

#define mipi_tx_info(fmt, ...)  \
        VDP_INFO_TRACE("[drv_mipitx][INFO][%s:%d]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__) \

#define mipi_tx_check_null_ptr_return(ptr) \
    do { \
        if ((ptr) == NULL) { \
            mipi_tx_err("NULL pointer\n"); \
            return (-1); \
        } \
    } while (0)

#define mipi_tx_desc(x) 1
#define mipi_tx_unused(x) ((void)(x))

typedef struct {
    mipi_tx_work_mode_t work_mode;
    unsigned char       lp_clk_en;
} mipi_tx_work_param;

typedef struct {
    unsigned int        devno;
    mipi_tx_work_param     work_param;
} mipi_tx_pub_info;

typedef struct {
    combo_dev_cfg_t dev_cfg;
    mipi_tx_work_param work_param;
} mipi_tx_dev_ctx_t;

typedef enum {
    MIPI_1080P_60,
    MIPI_800X1280_60
} mipitx_timing;

typedef struct {
    reg_hbp_hsa_in hbp_hsa;
    reg_hfp_hact_in hfp_hact;
    reg_videom_vsa_lines vsa;
    reg_videom_vbp_lines vbp;
    reg_videom_vfp_lines vfp;
    reg_videom_vactive_lines vactive;
    reg_videom_hsa_size hsa;
    reg_videom_hbp_size hbp;
    reg_videom_hline_size hline;
} mipitx_timing_index;

void mipi_tx_set_work_mode(mipi_tx_work_mode_t work_mode);
mipi_tx_work_mode_t mipi_tx_get_work_mode(void);
void mipi_tx_set_lp_clk_en(unsigned char lp_clk_en);
unsigned char mipi_tx_get_lp_clk_en(void);
int mipi_tx_get_lane_num(const short lane_id[], int lane_id_len);

void mipi_check_pid_spin_lock_init(void);
void mipi_check_pid_spin_lock_destory(void);

#endif
