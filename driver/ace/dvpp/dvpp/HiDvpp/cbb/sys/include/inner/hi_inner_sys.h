/*
 * Copyright (C) Hisilicon Technologies Co., Ltd. 2014-2019. All rights reserved.
 * Description: The mpp inner common defination, not release to customer
 * Author: Hisilicon multimedia software group
 * Create: 2014-08-02
 */

#ifndef __HI_INNER_SYS_H__
#define __HI_INNER_SYS_H__

#include "hi_common_adapt.h"
#include "hi_comm_video_adapt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

typedef enum {
    COMPRESS_LOSSLESS = 0,
    COMPRESS_LOSSY,
    COMPRESS_BUTT,
} compress_strategy;

typedef struct {
    compress_strategy comp_strategy; /* RW; compress strategy */
    hi_s32 type;
    hi_u32 comp_ratio_8_bit; /* RW, range[0, 100]; lossy compression ratio of 8 bit    */
    hi_u32 comp_ratio_10_bit; /* RW, range[0, 100]; lossy compression ratio  of 10 bit */
} compress_parameter;

typedef struct {
    compress_parameter luma_comp_param;
    compress_parameter chroma_comp_param;
} yuv_compress_param;

typedef struct {
    compress_strategy comp_strategy; /* RW; compress strategy */
    hi_s32 type;
    hi_u32 comp_ratio_8_bit; /* RW, range[0, 100]; lossy compression ratio of 8 bit    */
    hi_u32 comp_ratio_10_bit; /* RW, range[0, 100]; lossy compression ratio of 10 bit */
    hi_u32 comp_ratio_12_bit; /* RW, range[0, 100]; lossy compression ratio of 12 bit */
    hi_u32 comp_ratio_14_bit; /* RW, range[0, 100]; lossy compression ratio of 14 bit */
    hi_u32 comp_ratio_16_bit; /* RW, range[0, 100]; lossy compression ratio of 16 bit */
} raw_compress_param;

typedef struct {
    hi_operation_mode operation_mode;
    raw_compress_param raw_comp_param;
    yuv_compress_param vi_comp_param;
    yuv_compress_param vpss_comp_param;
    yuv_compress_param vgs_comp_param;
    yuv_compress_param gdc_comp_param;
    yuv_compress_param avs_comp_param;
    yuv_compress_param vo_comp_param;
} sys_compress_param;

typedef struct {
    hi_operation_mode operation_mode;
    raw_compress_param raw_line_comp_param;
    raw_compress_param raw_frame_comp_param;
    yuv_compress_param vi_comp_param;
    yuv_compress_param vpss_comp_param;
    yuv_compress_param vgs_comp_param;
    yuv_compress_param gdc_comp_param;
    yuv_compress_param avs_comp_param;
    yuv_compress_param vo_comp_param;
    yuv_compress_param vi_tile_comp_param;
    yuv_compress_param vpss_tile_comp_param;
    yuv_compress_param nr_comp_param;
} sys_compress_v2_param;

typedef struct {
    hi_mod_id mod_id;
    hi_pixel_format pixel_format;
    hi_compress_mode compress_mode;
    hi_data_bitwidth bit_width;
    hi_u32 bayer_mode;
    hi_u32 width;
    hi_u32 height;
    hi_bool mirror;
    hi_bool tile;  /* only compress by GDC is 1. */
    hi_u32 tile_height;  /* GDC */
} sys_global_cmp_input;

typedef struct {
    hi_u64 yh_coef_phy_addr;
    hi_u64 yv_coef_phy_addr;
    hi_u64 ch_coef_phy_addr;
    hi_u64 cv_coef_phy_addr;
    hi_void *yh_coef_vir_addr;
    hi_void *yv_coef_vir_addr;
    hi_void *ch_coef_vir_addr;
    hi_void *cv_coef_vir_addr;
    hi_s32 yh_avg;
    hi_s32 yv_avg;
    hi_s32 ch_avg;
    hi_s32 cv_avg;
    hi_u32 yhstep;
    hi_u32 yvstep;
    hi_u32 chstep;
    hi_u32 cvstep;
    hi_u32 yhrat;
    hi_u32 yvrat;
    hi_u32 chrat;
    hi_u32 cvrat;
} scl_cfg;

typedef struct {
    hi_u32 yhrat;
    hi_u32 yvrat;
    hi_u32 chrat;
    hi_u32 cvrat;
} sys_scl_rate;

typedef enum {
    SCLCOEF_NORMAL = 0,
    SCLCOEF_IVE_TAP2 = 1,
    SCLCOEF_IVE_TAP4 = 2,
    SCLCOEF_IVE_TAP6 = 3,
    SCLCOEF_IVE_TAP8 = 4,
    SCLCOEF_DIS_LINEAR = 5,
    SCLCOEF_BUTT
} sclcoef_mode;

typedef struct {
    hi_bool online;
    hi_bool deflicker;
    hi_bool osd_mode;
    sclcoef_mode ive_sclcoef_mode;
} scale_coef_opt;
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* __HI_INNER_SYS_H__ */

