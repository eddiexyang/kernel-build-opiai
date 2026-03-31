/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: slvs hal header file.
 * Author: Hisilicon multimedia software group
 * Create: 2023-03-08
 */
#include "hi_type.h"
#include "slvs_ec_adapt.h"
#include "slvs_ec_reg.h"
#ifndef SLVS_HAL
#define SLVS_HAL

#define SLVS0_BASE_ADDR          0x3002C0000

#define SLVS_EC0_IRQ                 291

#define SLVS_PHY_PCS_INT_MASK   (0x00000000)
#define SLVS_EC_INT_MASK        (0x00000000)
#define SLVS_LINK_INT_MASK      (0x00000000) // if colorbar-fe-mask header crc; others ff

#define SLVS_ECC_OPT_MODE_NONE                  (0x0)
#define SLVS_ECC_OPT_MODE_2BYTE                 (0x1)
#define SLVS_ECC_OPT_MODE_4BYTE                 (0x2)

#define SLVS_ECC_BASIC_BLOCK_SIZE_2BYTE         (0xE2)
#define SLVS_ECC_BASIC_BLOCK_SIZE_4BYTE         (0xE4)

#define SLVS_ECC_DATA_BLOCK_SIZE                (0xE0)

#define SLVS_ECC_ATTACH_EXTRA_BLOCK_SIZE_2BYTE  (0x2)
#define SLVS_ECC_ATTACH_EXTRA_BLOCK_SIZE_4BYTE  (0x4)

#define SLVS_ECC_EXTRA_BLOCK_SIZE_ALIGN         (16)

#define SLVS_EC_RESET_REG_OFFSET                (0xA58U)
#define SLVS_EC_UNRESET_REG_OFFSET              (0xA5CU)
#define SLVS_EC_RESET_STATUS_REG_OFFSET         (0x5A58U)

void slvs_drv_set_irq_num(unsigned int irq_num);

hi_s32 slvs_drv_enable_clock(hi_u32 combo_dev);
hi_s32 slvs_drv_disable_clock(hi_u32 combo_dev);

void slvs_drv_apb_reset(unsigned int dev);

void slvs_drv_apb_unreset(unsigned int dev);

void slvs_drv_core_reset(hi_u32 combo_dev);

void slvs_drv_core_unreset(hi_u32 combo_dev);

void slvs_drv_cdr_en(hi_u32 phy_id, hi_u32 en);

void slvs_drv_phy_eq_ctrl(hi_u32 phy_id, unsigned int eq);


void slvs_drv_phy_ctrl_test(unsigned int devno, unsigned int value);

void slvs_drv_phy_pdac_ldo(hi_u32 phy_id, unsigned int value);

hi_u32 slvs_drv_is_lane_valid(unsigned int devno, short lane);

void slvs_drv_set_lane_num(unsigned int devno, unsigned int lane_num);

void slvs_drv_set_phy_en(unsigned int devno, hi_u32 en);

hi_void slvs_drv_set_lane_cken(unsigned int devno, const short* p_lane_id, hi_u32 en);

hi_void slvs_drv_lane_reset(unsigned int devno, const short* p_lane_id);

hi_void slvs_drv_lane_unreset(unsigned int devno, const short* p_lane_id);

hi_void slvs_drv_set_lane_en(unsigned int devno, const short* p_lane_id, hi_u32 en);

void slvs_drv_set_bist_en(unsigned int devno);

void slvs_drv_set_lane_rate(unsigned int devno, slvs_lane_rate_t lane_rate);

hi_void slvs_drv_set_lane_sel(unsigned int devno, const short* p_lane_id);

void slvs_drv_set_raw_type(unsigned int devno, data_type_t input_data_type);

void slvs_drv_set_data_rate(unsigned int devno, mipi_data_rate_t data_rate);

void slvs_drv_set_wdr_mode(unsigned int devno, wdr_mode_t wdr_mode);

void slvs_drv_set_deskew_symbol(unsigned int devno, unsigned int symbol);

void slvs_drv_set_clear_en(unsigned int devno, hi_u32 en);

void slvs_drv_set_mem_ck_en(unsigned int devno, hi_u32 en);

void slvs_drv_set_sensor_avalid_width(unsigned int devno, int width);

void slvs_drv_set_image_rect(unsigned int devno, const img_rect_t *p_img_rect);

void slvs_drv_set_crop_en(unsigned int devno, hi_u32 en);

void slvs_drv_set_crc_enable(unsigned int devno, hi_u32 enable);

void slvs_drv_set_ecc_basic_block_size(unsigned int devno, unsigned int basic_block_size);

void slvs_drv_set_ecc_extra_block_size(unsigned int devno, unsigned int extra_block_size);

void slvs_drv_set_ecc_basic_block_num(unsigned int devno, unsigned int basic_block_num);

void slvs_drv_set_ecc_option(unsigned int devno, unsigned int ecc_option);

hi_void slvs_drv_set_link_lane_order(unsigned int devno, const short* p_lane_id);

void slvs_drv_phy_pcs_int_mask(hi_u32 phy_id, unsigned int mask);

void slvs_drv_set_slvs_mask(unsigned int mask);

void slvs_drv_set_link_int_mask(unsigned int devno, unsigned int mask);

unsigned int slvs_drv_get_phy_raw_int(hi_u32 phy_id);

void slvs_drv_clear_phy_raw_int(hi_u32 phy_id, unsigned int intclr);

unsigned int slvs_drv_get_slvs_int(hi_u32 phy_id);

void slvs_drv_clear_slvs_int(hi_u32 phy_id, unsigned int intclr);

unsigned int slvs_drv_get_link_raw_int(unsigned int devno);

void slvs_drv_clear_link_raw_int(unsigned int devno, unsigned int intclr);

void slvs_drv_get_imgsize_statis(unsigned int devno, short vc, img_size_t *p_size);

void slvs_drv_get_phy_data(unsigned int devno, short lane_id, unsigned int* phy_data);

void slvs_drv_get_phy_aligned_data(unsigned int devno, short lane_id, unsigned int* phy_data);

unsigned int slvs_drv_get_phy_valid_lane(unsigned int devno);

void slvs_clear_config(unsigned int devno, const short *p_lane_id);
void slvs_ec_drv_set_regs(slvs_regs_type_t *regs);
int slvs_drv_init(void);
void slvs_rx_drv_exit(void);
void slvs_drv_exit(void);
void proc_show_slvs_phy_info(const osal_proc_entry_t* s);
void proc_show_slvs_dev_err_info(const osal_proc_entry_t* s);
#endif

