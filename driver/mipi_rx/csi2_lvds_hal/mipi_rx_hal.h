/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: mipi hal header file
 * Author: Hisilicon multimedia software group
 * Create: 2023-03-08
 */
#ifndef MIPI_RX_HAL_H
#define MIPI_RX_HAL_H

#include "hi_mipi_rx.h"
#include "mipi_rx_reg.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MIPI_RX_IRQ 160

#define MIPI_RX_MAX_PHY_NUM                    2U
#define MIPI_RX_PHY_LANE_NUM                   4U
#define MIPI_RX_MAX_EXT_DATA_TYPE_BIT_WIDTH    16
#define MIPI_RX_MIN_EXT_DATA_TYPE_BIT_WIDTH    8

#define MIPI_CIL_INT_MASK      0x00001f1f
#define MIPI_CTRL_INT_MASK     0x00030013           // 打开帧同步信号中断，用于每帧监视mipi故障状态
#define LVDS_CTRL_INT_MASK     0x0f770fff
#define ALIGN0_INT_MASK        0x00001fff

typedef struct {
    unsigned int clk2_fsm_timeout_err_cnt;
    unsigned int clk_fsm_timeout_err_cnt;
    unsigned int d0_fsm_timeout_err_cnt;
    unsigned int d1_fsm_timeout_err_cnt;
    unsigned int d2_fsm_timeout_err_cnt;
    unsigned int d3_fsm_timeout_err_cnt;

    unsigned int clk2_fsm_escape_err_cnt;
    unsigned int clk_fsm_escape_err_cnt;
    unsigned int d0_fsm_escape_err_cnt;
    unsigned int d1_fsm_escape_err_cnt;
    unsigned int d2_fsm_escape_err_cnt;
    unsigned int d3_fsm_escape_err_cnt;
} phy_err_int_cnt_t;

typedef struct {
    /* Crc_intr err */
    unsigned int vc0_err_crc_cnt;
    unsigned int vc1_err_crc_cnt;
    unsigned int vc2_err_crc_cnt;
    unsigned int vc3_err_crc_cnt;
    unsigned int vc0_err_ecc_corrected_cnt;
    unsigned int vc1_err_ecc_corrected_cnt;
    unsigned int vc2_err_ecc_corrected_cnt;
    unsigned int vc3_err_ecc_corrected_cnt;
    unsigned int err_ecc_double_cnt;

    unsigned int vc0_err_frame_num_cnt;
    unsigned int vc1_err_frame_num_cnt;
    unsigned int vc2_err_frame_num_cnt;
    unsigned int vc3_err_frame_num_cnt;
    unsigned int vc0_err_frame_s_e_num_mismatch_cnt;
    unsigned int vc1_err_frame_s_e_num_mismatch_cnt;
    unsigned int vc2_err_frame_s_e_num_mismatch_cnt;
    unsigned int vc3_err_frame_s_e_num_mismatch_cnt;

    /* Ctrl err */
    unsigned int data_fifo_rderr_cnt;
    unsigned int cmd_fifo_rderr_cnt;
    unsigned int vsync_cnt;
    unsigned int data_fifo_wrerr_cnt;
    unsigned int cmd_fifo_wrerr_cnt;
} mipi_err_int_cnt_t;

typedef struct {
    unsigned int lvds_vsync_cnt;
    unsigned int cmd_rd_err_cnt;
    unsigned int cmd_wr_err_cnt;
    unsigned int pop_err_cnt;
    unsigned int lvds_state_err_cnt;

    unsigned int link2_rd_err_cnt;
    unsigned int link1_rd_err_cnt;
    unsigned int link0_rd_err_cnt;
    unsigned int link2_wr_err_cnt;
    unsigned int link1_wr_err_cnt;
    unsigned int link0_wr_err_cnt;

    /* Lane sync err */
    unsigned int lane7_sync_err_cnt;
    unsigned int lane6_sync_err_cnt;
    unsigned int lane5_sync_err_cnt;
    unsigned int lane4_sync_err_cnt;
    unsigned int lane3_sync_err_cnt;
    unsigned int lane2_sync_err_cnt;
    unsigned int lane1_sync_err_cnt;
    unsigned int lane0_sync_err_cnt;
} lvds_err_int_cnt_t;

typedef struct {
    unsigned int lane7_align_err_cnt;
    unsigned int lane6_align_err_cnt;
    unsigned int lane5_align_err_cnt;
    unsigned int lane4_align_err_cnt;
    unsigned int lane3_align_err_cnt;
    unsigned int lane2_align_err_cnt;
    unsigned int lane1_align_err_cnt;
    unsigned int lane0_align_err_cnt;
    unsigned int fifo_full_err_cnt;
} align_err_int_cnt_t;

void mipi_rx_drv_set_irq_num(unsigned int irq_num);
void mipi_rx_drv_set_regs(mipi_rx_regs_type_t *regs);
int isp_subctrl_reg_remap(void);
phy_err_int_cnt_t *mipi_rx_drv_get_phy_err_int_cnt(unsigned int phy_id);
mipi_err_int_cnt_t *mipi_rx_drv_get_mipi_err_int_cnt(unsigned int devno);
lvds_err_int_cnt_t *mipi_rx_drv_get_lvds_err_int_cnt(unsigned int devno);
align_err_int_cnt_t *mipi_rx_drv_get_align_err_int_cnt(unsigned int devno);

/* sensor function */
int sensor_drv_enable_clock(unsigned int sns_clk_source);
int sensor_drv_disable_clock(unsigned int sns_clk_source);
int sensor_drv_config_clock(sns_clk_cfg_t sns_clk_config);

int sensor_drv_reset(unsigned int sns_reset_source);
int sensor_drv_unreset(unsigned int sns_reset_source);

/* mipi_rx function */
void mipi_rx_drv_set_work_mode(unsigned int devno, input_mode_t input_mode);
void mipi_rx_drv_set_mipi_image_rect(unsigned int devno, const img_rect_t *p_img_rect);
void mipi_rx_drv_set_mipi_crop_en(unsigned int devno, unsigned int enable);
void mipi_rx_drv_set_mipi_yuv_dt(unsigned int devno, data_type_t input_data_type);
void mipi_rx_drv_set_mipi_wdr_user_dt(unsigned int devno, data_type_t input_data_type,
                                      const short data_type[WDR_VC_NUM]);
void mipi_rx_drv_set_mipi_dol_id(unsigned int devno, data_type_t input_data_type, const short dol_id[]);
void mipi_rx_drv_set_mipi_wdr_mode(unsigned int devno, mipi_wdr_mode_t wdr_mode);
unsigned int mipi_rx_drv_get_phy_data(int phy_id, int lane_id);
unsigned int mipi_rx_drv_get_phy_mipi_link_data(int phy_id, int lane_id);
unsigned int mipi_rx_drv_get_phy_freq(int phy_id);
unsigned int mipi_rx_drv_get_phy_lvds_link_data(int phy_id, int lane_id);

void mipi_rx_drv_set_data_rate(unsigned int devno, mipi_data_rate_t data_rate);
void mipi_rx_drv_set_link_lane_id(unsigned int devno, input_mode_t input_mode, const short *p_lane_id);
void mipi_rx_drv_set_mem_cken(unsigned int devno, unsigned int enable);
void mipi_rx_drv_set_clr_cken(unsigned int devno, unsigned int enable);
void mipi_rx_drv_set_phy_config(input_mode_t input_mode, unsigned int lane_bitmap, unsigned int lane_mask);

void mipi_rx_drv_set_lvds_image_rect(unsigned int devno, const img_rect_t *p_img_rect, unsigned int total_lane_num);
void mipi_rx_drv_set_lvds_crop_en(unsigned int devno, unsigned int enable);

int mipi_rx_drv_set_lvds_wdr_mode(unsigned int devno, lvds_wdr_mode_t wdr_mode,
                                  const lvds_vsync_attr_t *vsync_attr, const lvds_fid_attr_t *fid_attr);
void mipi_rx_drv_set_lvds_ctrl_mode(unsigned int devno, lvds_sync_mode_t sync_mode,
                                    data_type_t input_data_type,
                                    lvds_bit_endian_t data_endian,
                                    lvds_bit_endian_t sync_code_endian);

void mipi_rx_drv_set_lvds_data_rate(unsigned int devno, mipi_data_rate_t data_rate);

void mipi_rx_drv_set_dol_line_information(unsigned int devno, lvds_wdr_mode_t wdr_mode);
void mipi_rx_drv_set_lvds_sync_code(unsigned int devno, unsigned int lane_cnt, const short lane_id[LVDS_LANE_NUM],
                                    const unsigned short sync_code[][WDR_VC_NUM][SYNC_CODE_NUM]);

void mipi_rx_drv_set_lvds_nxt_sync_code(unsigned int devno, unsigned int lane_cnt, const short lane_id[LVDS_LANE_NUM],
                                        const unsigned short sync_code[][WDR_VC_NUM][SYNC_CODE_NUM]);

void mipi_rx_drv_set_phy_sync_config(const lvds_dev_attr_t *p_attr, unsigned int lane_bitmap,
                                     const unsigned short nxt_sync_code[][WDR_VC_NUM][SYNC_CODE_NUM]);

unsigned int mipi_rx_drv_get_lane_mask(unsigned int devno, lane_divide_mode_t mode);
int mipi_rx_drv_is_lane_valid(unsigned int devno, short lane_id, lane_divide_mode_t mode);
void mipi_rx_drv_set_hs_mode(lane_divide_mode_t mode);

void mipi_rx_drv_get_mipi_imgsize_statis(unsigned int devno, short vc, img_size_t *p_size);
void mipi_rx_drv_get_lvds_imgsize_statis(unsigned int devno, short vc, img_size_t *p_size);
void mipi_rx_drv_get_lvds_lane_imgsize_statis(unsigned int devno, short lane, img_size_t *p_size);

void mipi_rx_drv_set_chn_int_mask(unsigned int devno);
void mipi_rx_drv_set_lvds_ctrl_int_mask(unsigned int devno, unsigned int mask);
void mipi_rx_drv_set_mipi_ctrl_int_mask(unsigned int devno, unsigned int mask);
void mipi_rx_drv_set_align_int_mask(unsigned int devno, unsigned int mask);

int mipi_rx_drv_enable_clock(hi_u32 combo_dev, lane_divide_mode_t mode);
int mipi_rx_drv_disable_clock(hi_u32 combo_dev, lane_divide_mode_t mode);

void mipi_rx_drv_core_reset(hi_u32 combo_dev);
void mipi_rx_drv_core_unreset(hi_u32 combo_dev);

int mipi_rx_drv_init(void);
void mipi_rx_drv_exit(void);

int mipi_rx_drv_hw_init(void);
int mipi_rx_drv_hw_exit(void);

hi_void isp_subctrl_reg_unremap(void);

hi_ulong get_isp_subctrl_base(hi_void);

hi_void write_reg32(hi_ulong addr, hi_u32 value, hi_u32 mask);
hi_u32 read_reg32(hi_ulong addr);
hi_s32 check_reg(hi_ulong addr, hi_u32 value, hi_u32 mask);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MIPI_RX_HAL_H */
