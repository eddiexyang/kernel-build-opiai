/*
 * Copyright (C) Hisilicon Technologies Co., Ltd. 2010-2019. All rights reserved.
 * Description: head file of sys_drv.c
 * Author: Hisilicon multimedia software group
 * Create: 2010-11-19
 */

#ifndef __SYS_DRV_H__
#define __SYS_DRV_H__

#include "hi_osal.h"

#include "hi_comm_sys.h"
#include "hi_inner_sys.h"

#include "sys_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

typedef struct {
    hi_u32 coefficient_lanczos3[64];  /* >1 */
    hi_u32 coefficient8_cubic[64];  /* =1 */
    hi_u32 ive_coefficient8_cubic[3][64];  /* =1由于IVE的缩放系数需求，增加缩放系数字段 */

    hi_u32 coefficient8_lanczos2_8tap[64];  /* 3072/4096 2048/4096 */

    hi_u32 coefficient8_3_m_a19[64];  /* 1365   1024   ... */

    hi_u32 coefficient8_2_m_a05[64];  /* 1024 */

    hi_u32 coefficient8_1_5_m_a05[64];  /* ..... */

    hi_u32 ycoefficient4_5_m_a15[64];  /* >1 3072 2048 */

    hi_u32 ycoefficient4_cubic[64];  /* =1 */

    hi_u32 ycoefficient4_4_m_a15[64];  /* 1365   1024   ... */

    hi_u32 ycoefficient4_2_m_a05[64];  /* 1365 */
    hi_u32 ycoefficient4_1_5_m_a15[64];  /* 1024 ... */

    hi_u32 coefficient6_6_m_a25[64];  /* >1 */

    hi_u32 coefficient6_cubic[64];  /* =1 */

    hi_u32 coefficient6_5_m_a25[64];  /* 3072 */

    hi_u32 coefficient6_4_m_a20[64];  /* 2048 */

    hi_u32 coefficient6_3_m_a15[64];  /* 1365 1024 ... */

    hi_u32 coefficient6_2_m_a05[64];  /* 1024 */
    hi_u32 ive_coefficient6_2_m_a05[64];  /* 1024 由于IVE需求增加字段 */
    hi_u32 coefficient6_1_5_m_a05[64];  /* ... */

    hi_u32 ccoefficient4_5_m_a15[64];  /* >1 3072 2048 */

    hi_u32 ccoefficient4_cubic[64];  /* =1 */

    hi_u32 ccoefficient4_4_m_a15[64];  /* 1365   1024   ... */
    hi_u32 ccoefficient4_2_m_a05[64];  /* 1365 */
    hi_u32 ccoefficient4_1_5_m_a15[64];  /* 1024 ... */

    hi_u32 ive_lcoefficient2_2_m_a05_hor[64];  /* 双线性 由于IVE需求增加字段 */
    hi_u32 ive_lcoefficient2_2_m_a05_vel[64];  /* 双线性 由于IVE需求增加字段 */
    hi_u32 ive_chcoefficient2_2_m_a05_hor[64];  /* 双线性 由于IVE需求增加字段 */
    hi_u32 ive_chcoefficient2_2_m_a05_vel[64];  /* 双线性 由于IVE需求增加字段 */

#ifdef CONFIG_HI_SYS_SCALE_COEF_ONLINE_SUPPORT
    /* VI在线水平亮度 */
    hi_u32 coefficient8_1_5_m_a05_online[68];
    hi_u32 coefficient8_2_m_a05_online[68];
    hi_u32 coefficient8_3_m_a19_online[68];
    hi_u32 coefficient8_lanczos2_8tap_online[68];
    hi_u32 coefficient8_cubic_online[68];

    // VI在线垂直亮度
    hi_u32 coefficient6_1_5_m_a05_online[34];
    hi_u32 coefficient6_2_m_a05_online[34];
    hi_u32 coefficient6_3_m_a15_online[34];
    hi_u32 coefficient6_4_m_a20_online[34];
    hi_u32 coefficient6_5_m_a25_online[34];
    hi_u32 coefficient6_cubic_online[34];
#endif

    hi_u32 coefficient8_lanczos2_8tap_dis[64];
    hi_u32 coefficient6_4_m_a20_dis[64];

    /* 视频缩放系数 */
    hi_u32 hor_luma_coef7_00[64]; /* > 1 */
    hi_u32 hor_luma_coef6_75[64];
    hi_u32 hor_luma_coef6_50[64];
    hi_u32 hor_luma_coef6_25[64];
    hi_u32 hor_luma_coef6_00[64];
    hi_u32 hor_luma_coef5_75[64];
    hi_u32 hor_luma_coef5_50[64];
    hi_u32 hor_luma_coef5_25[64];
    hi_u32 hor_luma_coef5_00[64];
    hi_u32 hor_luma_coef4_50[64];
    hi_u32 hor_luma_coef4_00[64];
    hi_u32 hor_luma_coef3_75[64];
    hi_u32 hor_luma_coef3_375[64];
    hi_u32 hor_luma_coef3_00[64];
    hi_u32 hor_luma_coef2_50[64];
    hi_u32 hor_luma_coef2_00[64];
    hi_u32 hor_luma_coef1_50[64];
    hi_u32 hor_luma_coef2_50_a20[64];
    hi_u32 hor_luma_coef2_00_a15[64];
    hi_u32 hor_luma_coef1_50_a15[64];
    hi_u32 hor_luma_coef1_00_a15[64];
    hi_u32 hor_luma_coef1_00_a10[64];
    hi_u32 hor_luma_coef0_50_a05[64];
    hi_u32 hor_luma_coef0_10_a01[64];

    hi_u32 hor_chroma_coef7_00[64];
    hi_u32 hor_chroma_coef6_75[64];
    hi_u32 hor_chroma_coef6_50[64];
    hi_u32 hor_chroma_coef6_25[64];
    hi_u32 hor_chroma_coef6_00[64];
    hi_u32 hor_chroma_coef5_75[64];
    hi_u32 hor_chroma_coef5_50[64];
    hi_u32 hor_chroma_coef5_25[64];
    hi_u32 hor_chroma_coef5_00[64];
    hi_u32 hor_chroma_coef4_50[64];
    hi_u32 hor_chroma_coef4_00[64];
    hi_u32 hor_chroma_coef3_75[64];
    hi_u32 hor_chroma_coef3_375[64];
    hi_u32 hor_chroma_coef3_00[64];
    hi_u32 hor_chroma_coef2_50[64];
    hi_u32 hor_chroma_coef2_00[64];
    hi_u32 hor_chroma_coef1_50[64];
    hi_u32 hor_chroma_coef1_50_a15[64];
    hi_u32 hor_chroma_coef1_00_a10[64];
    hi_u32 hor_chroma_coef0_50_a05[64];
    hi_u32 hor_chroma_coef0_10_a01[64];

    hi_u32 ver_luma_coef7_00[64];
    hi_u32 ver_luma_coef6_75[64];
    hi_u32 ver_luma_coef6_50[64];
    hi_u32 ver_luma_coef6_25[64];
    hi_u32 ver_luma_coef6_00[64];
    hi_u32 ver_luma_coef5_75[64];
    hi_u32 ver_luma_coef5_50[64];
    hi_u32 ver_luma_coef5_25[64];
    hi_u32 ver_luma_coef5_00[64];
    hi_u32 ver_luma_coef4_50[64];
    hi_u32 ver_luma_coef4_00[64];
    hi_u32 ver_luma_coef3_75[64];
    hi_u32 ver_luma_coef3_75_a15[64];
    hi_u32 ver_luma_coef3_375[64];
    hi_u32 ver_luma_coef3_00[64];
    hi_u32 ver_luma_coef2_50[64];
    hi_u32 ver_luma_coef2_00[64];
    hi_u32 ver_luma_coef1_50[64];
    hi_u32 ver_luma_coef2_50_a20[64];
    hi_u32 ver_luma_coef2_00_a15[64];
    hi_u32 ver_luma_coef1_50_a15[64];
    hi_u32 ver_luma_coef1_00_a10[64];
    hi_u32 ver_luma_coef0_50_a05[64];
    hi_u32 ver_luma_coef0_10_a01[64];

    hi_u32 ver_chroma_coef7_00[64];
    hi_u32 ver_chroma_coef6_75[64];
    hi_u32 ver_chroma_coef6_50[64];
    hi_u32 ver_chroma_coef6_25[64];
    hi_u32 ver_chroma_coef6_00[64];
    hi_u32 ver_chroma_coef5_75[64];
    hi_u32 ver_chroma_coef5_50[64];
    hi_u32 ver_chroma_coef5_25[64];
    hi_u32 ver_chroma_coef5_00[64];
    hi_u32 ver_chroma_coef4_50[64];
    hi_u32 ver_chroma_coef4_00[64];
    hi_u32 ver_chroma_coef3_75[64];
    hi_u32 ver_chroma_coef3_375[64];
    hi_u32 ver_chroma_coef3_00[64];
    hi_u32 ver_chroma_coef2_50[64];
    hi_u32 ver_chroma_coef2_00[64];
    hi_u32 ver_chroma_coef1_50[64];
    hi_u32 ver_chroma_coef1_50_a15[64];
    hi_u32 ver_chroma_coef1_00_a10[64];
    hi_u32 ver_chroma_coef0_50_a05[64];
    hi_u32 ver_chroma_coef0_10_a01[64];

#ifdef CONFIG_HI_SYS_SCALE_COEF_ONLINE_SUPPORT
    /* only AVS use online luma coef */
    /* 水平亮度 */
    hi_u32 online_hor_luma_coef6_75[68];
    hi_u32 online_hor_luma_coef6_50[68];
    hi_u32 online_hor_luma_coef6_25[68];
    hi_u32 online_hor_luma_coef6_00[68];
    hi_u32 online_hor_luma_coef5_75[68];
    hi_u32 online_hor_luma_coef5_50[68];
    hi_u32 online_hor_luma_coef5_25[68];
    hi_u32 online_hor_luma_coef5_00[68];
    hi_u32 online_hor_luma_coef4_50[68];
    hi_u32 online_hor_luma_coef4_00[68];
    hi_u32 online_hor_luma_coef3_75[68];
    hi_u32 online_hor_luma_coef3_375[68];
    hi_u32 online_hor_luma_coef3_00[68];
    hi_u32 online_hor_luma_coef2_50[68];
    hi_u32 online_hor_luma_coef2_00[68];
    hi_u32 online_hor_luma_coef1_50[68];
    hi_u32 online_hor_luma_coef2_50_a20[68];
    hi_u32 online_hor_luma_coef2_00_a15[68];
    hi_u32 online_hor_luma_coef1_50_a15[68];
    hi_u32 online_hor_luma_coef1_00_a15[68];
    hi_u32 online_hor_luma_coef1_00_a10[68];
    hi_u32 online_hor_luma_coef0_50_a05[68];
    hi_u32 online_hor_luma_coef0_10_a01[68];

    /* 垂直亮度 */
    hi_u32 online_ver_luma_coef6_75[64];
    hi_u32 online_ver_luma_coef6_50[64];
    hi_u32 online_ver_luma_coef6_25[64];
    hi_u32 online_ver_luma_coef6_00[64];
    hi_u32 online_ver_luma_coef5_75[64];
    hi_u32 online_ver_luma_coef5_50[64];
    hi_u32 online_ver_luma_coef5_25[64];
    hi_u32 online_ver_luma_coef5_00[64];
    hi_u32 online_ver_luma_coef4_50[64];
    hi_u32 online_ver_luma_coef4_00[64];
    hi_u32 online_ver_luma_coef3_75[64];
    hi_u32 online_ver_luma_coef3_75_a15[64];
    hi_u32 online_ver_luma_coef3_375[64];
    hi_u32 online_ver_luma_coef3_00[64];
    hi_u32 online_ver_luma_coef2_50[64];
    hi_u32 online_ver_luma_coef2_00[64];
    hi_u32 online_ver_luma_coef1_50[64];

    hi_u32 online_ver_luma_coef2_50_a20[64];
    hi_u32 online_ver_luma_coef2_00_a15[64];
    hi_u32 online_ver_luma_coef1_50_a15[64];
    hi_u32 online_ver_luma_coef1_00_a10[64];
    hi_u32 online_ver_luma_coef0_50_a05[64];
    hi_u32 online_ver_luma_coef0_10_a01[64];
#endif

    /* for ive 预乘 */
#ifdef CONFIG_HI_VGS_IVE_PRE_SUPPORT
    hi_u32 ive_coeff_bilinear_8[64];
    hi_u32 ive_coeff_bilinear_6[64];
    hi_u32 ive_coeff_bilinear_4[64];
#endif
} sys_drv_scl_coeff;

typedef struct {
    sys_drv_scl_coeff *scl_coeff_set;
    hi_u64 scl_coeff_phy_addr;
    hi_u64 scl_coeff_virt2_bus_offset;
} sys_scale_addr;

typedef struct {
    hi_coeff_level hor_lum_coeff_level[SCALE_RANGE_BUTT]; /* 亮度水平方向系数配置 */
    hi_coeff_level hor_chr_coeff_level[SCALE_RANGE_BUTT]; /* 色度水平方向系数配置 */
    hi_coeff_level ver_lum_coeff_level[SCALE_RANGE_BUTT]; /* 亮度垂直方向系数配置 */
    hi_coeff_level ver_chr_coeff_level[SCALE_RANGE_BUTT]; /* 色度垂直方向系数配置 */
} sys_scale_info;

typedef struct {
    hi_u16 tile_input;
    hi_u16 tile_hgt;

    hi_u32 out_width;
    hi_u32 out_height;
} COMPRESS_MOE_EX_PARAM;

typedef struct {
    hi_u16 frm_wth[4];
    hi_u16 frm_hgt[4];
    hi_u16 c_en;
    hi_u16 bit_depth;
    hi_u16 seg_cfg_wth;
    hi_u16 mb_cfg_wth;
    hi_u16 mb_offset_thr;
    hi_u16 mb_offset_enable;
    hi_u16 tol_num;
    hi_u16 tol_special_num;
    hi_u16 tile_input;
    hi_u16 tile_hgt;
    hi_u16 lossy;
    hi_u16 bayer;
    hi_u16 bayer_format;
    hi_u16 rgb;
    hi_u32 slc_cfg_tar_bit;
    hi_u32 slc_spc_tar_bit;
    hi_u32 seg_cfg_max_bit;
    hi_u32 seg_cfg_tar_bit;
    hi_u32 max_qp;
    hi_u32 max_qp_offset;

    hi_u16 dither_qp;
    hi_u16 dither_segots;
    hi_u16 dither_ots;
    hi_u16 dither_glb;
    hi_u16 dither_dts;
    hi_u16 dither_msq;
    hi_u16 esl_en;

    /* tmu */
    hi_u16 grad_ave_thr0;
    hi_u16 grad_ave_thr1;
    hi_u16 grad_ave_thr2;
    hi_u16 simi_fct_thr0;
    hi_u16 simi_fct_thr1;
    hi_u16 simi_fct_thr2;
    hi_u16 rc_qp_offset;
    hi_u16 escape_intreim;
    hi_u16 intreim_table;
    hi_u16 bgn_k;
    hi_u16 max_k;
    hi_u16 adp_k;
    hi_u16 round_en;
    hi_u16 seg_bst_align;
    hi_u16 line_buf_ref_sel;
    hi_u16 mb_independent;
    hi_u16 pred_1ppc;
    hi_u32 frm_bit;
    hi_u16 mb_mode_sel_grad_thr;

#ifndef DVPP_UTST
    struct FILE *error;
#endif
    hi_u32 err_en;
} CMP_IO_CFG;

typedef struct {
    /* ---------------basic cfg parameter--------------------------------------- */
    hi_s32 frm_hgt; /* height */
    hi_s32 frm_wth; /* width */
    hi_s32 bit_depth; /* 8,10,12,14 */
    hi_s32 lossy;
    hi_s32 format; /* 1:420 2:422 3:bayer */
    hi_s32 bayer_format; /* RANGE:0-2; TYPICAL 0:GRBG; 1:GBRG; 2:RGGB; 3:BGGR */
    hi_s32 esl_en;
    hi_s32 esl_mode_qp;
    hi_s32 cmp_ratio_pct_y;
    hi_s32 cmp_ratio_pct;
    hi_s32 type;
    hi_s32 c_en;
    hi_s32 tile_input;
    hi_s32 til_hgt;
} CMP_PARA_INPUT;

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
} SYS_GLOBAL_CMP_INPUT;

/*
 * end : segment compress parameter data definition.
 */
hi_s32 sys_drv_init(hi_void);
hi_s32 sys_drv_exit(hi_void);

hi_s32 sys_drv_set_align(hi_u32 align);
hi_s32 sys_drv_get_stride(hi_u32 width, hi_u32 *stride);

hi_u32 sys_drv_get_chip_version(hi_void);

hi_s32 sys_drv_drv_ioctrl(hi_mpp_chn *mpp_chn, hi_u32 func_id, hi_void *io_args);

hi_s32 sys_drv_get_custom_code(hi_u32 *custom_code);

hi_s32 sys_drv_check_vo_wbc_bind(hi_mpp_chn *src_chn,
                                 hi_mpp_chn *dest_chn);
#ifdef CONFIG_HISI_SNAPSHOT_BOOT
hi_s32 sys_drv_restore(osal_dev_t *pdev);
#endif

hi_s32 sys_drv_malloc_scale_coef_buf(sys_scale_addr *scale_coef, hi_char *pch_name);
hi_s32 sys_drv_free_scale_coef_buffer(hi_void);
hi_void sys_drv_init_scale_coef(sys_drv_scl_coeff *scl_coeff_set, hi_bool deflicker);
hi_void sys_drv_get_vb_cfg(hi_void *info, hi_void *config);
hi_void sys_drv_get_vdec_buffer_cfg(hi_payload_type type, hi_void *info, hi_void *config);

hi_void sys_drv_init_coef(hi_void);

hi_void sys_drv_init_default_coef_level(hi_void);
hi_s32 sys_drv_check_coeff_info(hi_scale_coeff_info *coeff_info);
hi_void sys_drv_set_coeff_info(hi_scale_coeff_info *coeff_info);
hi_s32 sys_drv_get_coeff_info(hi_scale_coeff_info *coeff_info);
hi_void *sys_drv_get_hor_lum_scale_coef(hi_u32 yhrat, sys_drv_scl_coeff *scl_coeff_set);
hi_void *sys_drv_get_hor_chr_scale_coef(hi_u32 chrat, sys_drv_scl_coeff *scl_coeff_set);
hi_void *sys_drv_get_ver_lum_scale_coef(hi_u32 yvrat, sys_drv_scl_coeff *scl_coeff_set);
hi_void *sys_drv_get_ver_chr_scale_coef(hi_u32 cvrat, sys_drv_scl_coeff *scl_coeff_set);

hi_s32 sys_drv_set_compress_param(sys_compress_param *compress_param);
hi_s32 sys_drv_get_compress_param(sys_compress_param *compress_param);
hi_void sys_drv_get_cmp_cfg(hi_void *video_frame, hi_void *y_cmp_cfg, hi_void *c_cmp_cfg, hi_void *cmp_mode_ex_param);

hi_s32 sys_drv_get_vpss_venc_wrap_buffer_line(hi_vpss_venc_wrap_param *wrap_param, hi_u32 *buf_line);

hi_s32 sys_drv_set_raw_frame_compress(hi_raw_frame_compress_param *compress_param);
hi_s32 sys_drv_get_raw_frame_compress(hi_raw_frame_compress_param *compress_param);

hi_s32 sys_drv_set_compress_v2_param(sys_compress_v2_param *compress_param);
hi_s32 sys_drv_get_compress_v2_param(sys_compress_v2_param *compress_param);

hi_void sys_drv_get_cmp_bayer_cfg(hi_void *sys_cmp_in, hi_void *bayer_cmp_cfg, hi_void *bayer_dcmp_cfg);
hi_void sys_drv_get_cmp_3dnr_cfg(hi_void *sys_cmp_in, hi_void *y_cmp_cfg, hi_void *c_cmp_cfg);
hi_void sys_drv_get_dcmp_muv1_cfg(hi_void *sys_cmp_in, hi_void *cmp_out);
hi_void sys_drv_get_cmp_tde_cfg(hi_void *sys_cmp_in, hi_void *cmp_out);
hi_void sys_drv_get_cmp_seg_cfg(const hi_void *sys_cmp_in, hi_void *y_cmp_cfg, hi_void *c_cmp_cfg);

#ifdef CONFIG_HI_TZASC_SUPPORT
hi_s32 sys_drv_get_security_version(sys_security_version *version);
#endif

hi_s32 sys_drv_scale_coef_init(void);
hi_s32 sys_drv_scale_coef_exit(void);

hi_void *sys_drv_get_online_hor_lum_scale_coef(hi_u32 yhrat, sys_drv_scl_coeff *scl_coeff_set);
hi_void *sys_drv_get_online_hor_chr_scale_coef(hi_u32 chrat, sys_drv_scl_coeff *scl_coeff_set);
hi_void *sys_drv_get_online_ver_lum_scale_coef(hi_u32 yvrat, sys_drv_scl_coeff *scl_coeff_set);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef __SYS_DRV_H__ */

