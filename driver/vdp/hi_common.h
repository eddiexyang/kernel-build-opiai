/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2016-2022. All rights reserved.
 * Create: 2016-09-19
 */

#ifndef HI_COMMON_H__
#define HI_COMMON_H__

#ifndef __IGNORE_HWSEC__
#if !defined(__KERNEL__) || defined(__LITEOS__)

#include "securec.h"
#include <stdarg.h>
#endif
#endif

#include "autoconf.h"

#include "HiDvppCommon.h"
#ifndef USE_DVPP_FWK
#include "HiCommonInternal.h"
#endif
#include "hi_type.h"
#include "hi_defines.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#ifndef VER_X
#define VER_X                  1
#endif

#ifndef VER_Y
#define VER_Y                  0
#endif

#ifndef VER_Z
#define VER_Z                  0
#endif

#ifndef VER_P
#define VER_P                  0
#endif

#ifndef VER_B
#define VER_B                  0
#endif

#ifdef HI_DEBUG
#define VER_D                  " Debug"
#else
#define VER_D                  " Release"
#endif
#define ATTRIBUTE              __attribute__((aligned(ALIGN_NUM)))
#define hi_unused(x)           ((hi_void)(x))

#define COMPAT_POINTER(ptr, type) \
    do { \
        HI_UL ulAddr = (HI_UINTPTR_T)(ptr); \
        HI_U32 u32Addr = (HI_U32)ulAddr; \
        (ptr) = (type)(HI_UINTPTR_T)u32Addr; \
    } while (0)

#define MPP_VERSION            "HiDVPP"

#define VERSION_NAME_MAXLEN    64

typedef HI_S32 AI_CHN;
typedef HI_S32 AO_CHN;
typedef HI_S32 AENC_CHN;
typedef HI_S32 ADEC_CHN;
typedef HI_S32 AUDIO_DEV;
typedef HI_S32 VI_DEV;
typedef HI_S32 VI_PIPE;
typedef HI_S32 VI_CHN;
typedef HI_S32 VI_STITCH_GRP;
typedef HI_S32 VI_MCF_GRP;
typedef HI_S32 VO_DEV;
typedef HI_S32 VO_LAYER;
typedef HI_S32 VO_CHN;
typedef HI_S32 VO_WBC;
typedef HI_S32 GRAPHIC_LAYER;
typedef HI_S32 VENC_CHN;
typedef HI_S32 ISP_DEV;
typedef HI_S32 BLK_DEV;
typedef HI_S32 SENSOR_ID;
typedef HI_S32 MIPI_DEV;
typedef HI_S32 SLAVE_DEV;
typedef HI_S32 VPSS_GRP;
typedef HI_S32 VPSS_GRP_PIPE;
typedef HI_S32 VPSS_CHN;
typedef HI_S32 AVS_GRP;
typedef HI_S32 AVS_PIPE;
typedef HI_S32 AVS_CHN;
typedef HI_S32 MCF_GRP;
typedef HI_S32 MCF_PIPE;
typedef HI_S32 MCF_CHN;

#define HI_INVALID_CHN         (-1)
#define HI_INVALID_DEV         (-1)
#define HI_INVALID_HANDLE      (-1)
#define CCM_MATRIX_SIZE        (9)
#define CCM_MATRIX_NUM         (7)

typedef struct hiMPP_CHN_S {
    MOD_ID_E    enModId;
    HI_S32      s32DevId;
    HI_S32      s32ChnId;
} MPP_CHN_S;

#define MPP_MOD_VI             "vi"
#define MPP_MOD_VO             "vo"
#define MPP_MOD_AVS            "avs"
#define MPP_MOD_HDMI           "hdmi"
#define MPP_MOD_VGS            "vgs"
#define MPP_MOD_GDC            "gdc"
#define MPP_MOD_DIS            "dis"
#define MPP_MOD_GYRODIS        "gyrodis"

#define MPP_MOD_CHNL           "chnl"
#define MPP_MOD_VENC           "venc"
#define MPP_MOD_VPSS           "vpss"
#define MPP_MOD_VENC_REG0      "venc_reg0"
#define MPP_MOD_VENC_REG1      "venc_reg1"
#define MPP_MOD_VENC_REG2      "venc_reg2"

#define MPP_MOD_H264E          "h264e"
#define MPP_MOD_H265E          "h265e"
#define MPP_MOD_JPEGE          "jpege"

#define MPP_MOD_VEDU           "vedu"
#define MPP_MOD_VDEC           "vdec"
#define MPP_MOD_VPSS           "vpss"
#define MPP_MOD_H264D          "h264d"
#define MPP_MOD_JPEGD          "jpegd"
#define MPP_MOD_VPC            "vpc"
#define MPP_MOD_DRV_VPC         "drv_vpc"

#define MPP_MOD_AI             "ai"
#define MPP_MOD_AO             "ao"
#define MPP_MOD_AENC           "aenc"
#define MPP_MOD_ADEC           "adec"
#define MPP_MOD_AIO            "aio"
#define MPP_MOD_ACODEC         "acodec"

#define MPP_MOD_VB             "vb"
#define MPP_MOD_SYS            "sys"
#define MPP_MOD_PROC           "proc"
#define MPP_MOD_LOG            "logmpp"
#define MPP_MOD_FB             "fb"
#define MPP_MOD_RC             "rc"
#define MPP_MOD_TDE            "tde"
#define MPP_MOD_ISP            "isp"
#define MPP_MOD_PNGD           "pngd"
#define MPP_MOD_DCC            "dcc"
#define MPP_MOD_VDEC_SERVER    "vdec_server"
#define MPP_MOD_VDEC_ADAPT     "vdec_adapt"
#define MPP_MOD_MFUSION        "motionfusion"

#define MPP_MOD_RGN            "rgn"

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* HI_COMMON_H__ */
