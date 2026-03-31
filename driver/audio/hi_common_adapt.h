/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2016-2022. All rights reserved.
 * Create: 2016-09-19
 */

#ifndef HI_COMMON_ADAPT_H__
#define HI_COMMON_ADAPT_H__

#include "autoconf.h"
#include "hi_type.h"
#include "hi_defines.h"
#include "hi_common.h"

#ifndef __IGNORE_HWSEC__

#ifndef __KERNEL__
#include "securec.h"
#endif

#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

typedef struct {
    hi_char version[VERSION_NAME_MAXLEN];
} hi_mpp_version;

typedef hi_s32 hi_ai_chn;
typedef hi_s32 hi_ao_chn;
typedef hi_s32 hi_aenc_chn;
typedef hi_s32 hi_adec_chn;
typedef hi_s32 hi_audio_dev;
typedef hi_s32 hi_vi_dev;
typedef hi_s32 hi_vi_pipe;
typedef hi_s32 hi_vi_chn;
typedef hi_s32 hi_vi_stitch_grp;
typedef hi_s32 hi_vi_mcf_grp;
typedef hi_s32 hi_vo_dev;
typedef hi_s32 hi_vo_layer;
typedef hi_s32 hi_vo_chn;
typedef hi_s32 hi_vo_wbc;
typedef hi_s32 hi_graphic_layer;
typedef hi_s32 hi_venc_chn;
typedef hi_s32 hi_vdec_chn;
typedef hi_s32 hi_vpc_chn;
typedef hi_s32 hi_pngd_chn;
typedef hi_s32 hi_isp_dev;
typedef hi_s32 hi_blk_dev;
typedef hi_s32 hi_sensor_id;
typedef hi_s32 hi_mipi_dev;
typedef hi_s32 hi_slave_dev;
typedef hi_s32 hi_vpss_grp;
typedef hi_s32 hi_vpss_grp_pipe;
typedef hi_s32 hi_vpss_chn;

typedef MOD_ID_E hi_mod_id;
typedef CSC_MATRIX_E hi_csc_matrix_mode;
typedef CSC_COEFFICIENT_S hi_user_csc_coefficient;

typedef struct coefficient {
    hi_u64 coefficient_0;
    hi_u64 coefficient_1;
    hi_u64 coefficient_2;
    hi_u64 coefficient_3;
    hi_u64 coefficient_4;
    hi_u64 coefficient_5;
    hi_u64 coefficient_6;
    hi_u64 coefficient_7;
    hi_u64 coefficient_8;
    hi_u64 offset_0;
    hi_u64 offset_1;
    hi_u64 offset_2;
} hi_coefficient;

typedef struct csc_coefficient {
    hi_coefficient yuv_to_rgb_coefficient;
    hi_coefficient rgb_to_yuv_coefficient;
} hi_csc_coefficient;

typedef struct {
    hi_mod_id    mod_id;
    hi_s32      dev_id;
    hi_s32      chn_id;
} hi_mpp_chn;

typedef PAYLOAD_TYPE_E hi_payload_type;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* HI_COMMON_ADAPT_H__ */

