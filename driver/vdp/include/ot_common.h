/*
 * ot_common.h - OT common definitions (mediabase compatibility layer)
 *
 * This is a thin compatibility wrapper derived from IDA Pro reverse engineering
 * of ascend_vdp_hifb.ko. Module ID values have been precisely confirmed via
 * binary analysis of cmpi_get_module_func_by_id() call sites.
 */

#ifndef OT_COMMON_H
#define OT_COMMON_H

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_video_adapt.h"
#include "hi_common_adapt.h"
#include "ot_type.h"

/* Version string - confirmed from IDA: gfbg_init prints version "Aug  1 2023" */
#ifndef OT_MPP_VERSION
#define OT_MPP_VERSION "1.0"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*
 * Module IDs confirmed via IDA Pro analysis:
 *   cmpi_get_module_func_by_id(0, 0)  -> SYS
 *   cmpi_get_module_func_by_id(1, 0)  -> VB
 *   cmpi_get_module_func_by_id(5, 0)  -> VO
 *   cmpi_get_module_func_by_id(42, 0) -> TDE
 *   cmpi_get_module_func_by_id(44, 0) -> FB
 *   cmpi_get_module_func_by_id(45, 0) -> HDMI
 *   cmpi_get_module_func_by_id(46, 0) -> VGS
 */
typedef enum {
    OT_ID_SYS   = 0,
    OT_ID_VB    = 1,
    OT_ID_VO    = 5,
    OT_ID_AI    = 16,
    OT_ID_AO    = 17,
    OT_ID_AIO   = 18,
    OT_ID_AENC  = 20,
    OT_ID_ADEC  = 21,
    OT_ID_TDE   = 42,
    OT_ID_FB    = 44,
    OT_ID_HDMI  = 45,
    OT_ID_VGS   = 46,
    OT_ID_VOIE  = 52,
    OT_ID_BUTT,
} ot_mod_id;

/*
 * VO/GFX layer type aliases - confirmed via IDA:
 *   gfbg_init() uses these as td_s32 indices into capability arrays
 *   is_4k_layer(), is_hd_layer(), is_sd_layer() etc. take td_s32 (== td_u32)
 */
typedef td_s32 ot_vo_dev;
typedef td_s32 ot_vo_layer;
typedef td_s32 ot_vo_chn;
typedef td_s32 ot_gfx_layer;

/*
 * Struct/type mappings: ot_xxx = hi_xxx
 * Confirmed via field usage in gfbg_rotate_vgs.c, gfbg_graphics_drv.c, gfbg_main.c
 */
typedef hi_mpp_chn          ot_mpp_chn;
typedef hi_video_frame_info ot_video_frame_info;
typedef hi_video_frame      ot_video_frame;
typedef hi_size             ot_size;
typedef hi_pixel_format     ot_pixel_format;

/* Enum value mappings - confirmed via source usage patterns */
#define OT_PIXEL_FORMAT_ARGB_1555   PIXEL_FORMAT_ARGB_1555
#define OT_PIXEL_FORMAT_ARGB_4444   PIXEL_FORMAT_ARGB_4444
#define OT_PIXEL_FORMAT_BUTT        PIXEL_FORMAT_BUTT
#define OT_VIDEO_FIELD_FRAME        VIDEO_FIELD_FRAME
#define OT_VIDEO_FORMAT_LINEAR      VIDEO_FORMAT_LINEAR
#define OT_COMPRESS_MODE_NONE       COMPRESS_MODE_NONE

/* VGS handle type - confirmed via IDA: simple td_s32 */
typedef td_s32 ot_vgs_handle;

/* VO defines from ot_defines.h (ascend310Brc values) */
#ifndef OT_VO_MAX_PHYS_DEV_NUM
#define OT_VO_MAX_PHYS_DEV_NUM            1
#define OT_VO_MAX_VIRT_DEV_NUM            0
#define OT_VO_MAX_CAS_DEV_NUM             0
#define OT_VO_MAX_DEV_NUM                 (OT_VO_MAX_PHYS_DEV_NUM + OT_VO_MAX_VIRT_DEV_NUM + OT_VO_MAX_CAS_DEV_NUM)
#define OT_VO_MAX_PHYS_VIDEO_LAYER_NUM    1
#define OT_VO_MAX_GFX_LAYER_NUM           1
#define OT_VO_MAX_PHYS_LAYER_NUM          (OT_VO_MAX_PHYS_VIDEO_LAYER_NUM + OT_VO_MAX_GFX_LAYER_NUM)
#define OT_VO_MAX_LAYER_NUM               (OT_VO_MAX_PHYS_LAYER_NUM + OT_VO_MAX_VIRT_DEV_NUM + OT_VO_MAX_CAS_DEV_NUM)
#define OT_VO_LAYER_V0                    0
#define OT_VO_LAYER_G0                    1
#define OT_VO_MAX_CHN_NUM                 4
#endif

/* Legacy VO_MAX_* aliases used by drv/adp/ */
#ifndef VO_MAX_DEV_NUM
#define VO_MAX_DEV_NUM       OT_VO_MAX_DEV_NUM
#define VO_MAX_PHY_DEV_NUM   OT_VO_MAX_PHYS_DEV_NUM
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* OT_COMMON_H */
