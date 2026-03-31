/*
* Copyright (c) Hisilicon Technologies Co., Ltd. 2011-2019. All rights reserved.
* Description:
* Author: Hisilicon multimedia software group
* Create: 2011/06/28
 */

#ifndef HI_BUFFER_H__
#define HI_BUFFER_H__

#include "hi_math.h"
#include "hi_type.h"
#include "hi_debug.h"
#include "hi_common.h"
#include "hi_comm_video.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define HI_MAXINUM_LIMIT 100000U

__inline static HI_VOID COMMON_GetPicBufferConfig(HI_U32 u32Width, HI_U32 u32Height, PIXEL_FORMAT_E enPixelFormat,
    DATA_BITWIDTH_E enBitWidth, COMPRESS_MODE_E enCmpMode, HI_U32 u32Align, VB_CAL_CONFIG_S *pstCalConfig)
{
    HI_U32 u32TailInBytes = 0;
    HI_U32 u32BitWidth = 0;
    HI_U32 u32HeadStride = 0;
    HI_U32 u32VBSize = 0;
    HI_U32 u32HeadSize = 0;
    HI_U32 u32AlignHeight;
    HI_U32 u32MainStride = 0;
    HI_U32 u32MainSize = 0;
    HI_U32 u32ExtStride = 0;
    HI_U32 u32ExtSize = 0;
    HI_U32 u32ExtYSize = 0;
    HI_U32 u32HeadYSize = 0;
    HI_U32 u32YSize = 0;

    if ((u32Width > HI_MAXINUM_LIMIT) || (u32Height > HI_MAXINUM_LIMIT)) {
        pstCalConfig->u32VBSize = 0;
    }

    // u32Align: 0 is automatic mode, alignment size following system. Non-0 for specified alignment size
    if (u32Align == 0) {
        u32Align = DEFAULT_ALIGN;
    } else if (u32Align > MAX_ALIGN) {
        u32Align = MAX_ALIGN;
    } else {
        u32Align = (ALIGN_UP(u32Align, DEFAULT_ALIGN));
    }

    switch (enBitWidth) {
        case DATA_BITWIDTH_8: {
            u32BitWidth = 8; // 8:bit
            break;
        }
        case DATA_BITWIDTH_10: {
            u32BitWidth = 10; // 10:bit
            break;
        }
        case DATA_BITWIDTH_12: {
            u32BitWidth = 12; // 12:bit
            break;
        }
        case DATA_BITWIDTH_14: {
            u32BitWidth = 14; // 14:bit
            break;
        }
        case DATA_BITWIDTH_16: {
            u32BitWidth = 16; // 16:bit
            break;
        }
        default:
        {
            u32BitWidth = 0;
            break;
        }
    }

    u32AlignHeight = ALIGN_UP(u32Height, 2U); // 2:algin

    if (enCmpMode == COMPRESS_MODE_NONE) {
        u32MainStride = ALIGN_UP(((u32Width * u32BitWidth) + 7U) >> 3U, u32Align); // 7:offset, 3:offset
        if ((enPixelFormat >= PIXEL_FORMAT_YUYV_PACKED_422) && (enPixelFormat <= PIXEL_FORMAT_VYUY_PACKED_422)) {
            u32MainStride = ALIGN_UP((((u32Width * u32BitWidth) + 7U) >> 3U) * 2U, u32Align);  // width的2倍后对齐
        }
        u32YSize = u32MainStride * u32AlignHeight;

        if ((enPixelFormat == PIXEL_FORMAT_YVU_SEMIPLANAR_420) || (enPixelFormat == PIXEL_FORMAT_YUV_SEMIPLANAR_420)) {
            u32MainSize = (u32MainStride * u32AlignHeight * 3U) >> 1; // 3:algo, 1:algo
        } else if ((enPixelFormat == PIXEL_FORMAT_YVU_SEMIPLANAR_422) ||
                   (enPixelFormat == PIXEL_FORMAT_YUV_SEMIPLANAR_422) ||
                   (enPixelFormat == PIXEL_FORMAT_YVU_SEMIPLANAR_440) ||
                   (enPixelFormat == PIXEL_FORMAT_YUV_SEMIPLANAR_440)) {
            u32MainSize = u32MainStride * u32AlignHeight * 2U;
        } else if (enPixelFormat == PIXEL_FORMAT_YUV_400) {
            u32MainSize = u32MainStride * u32AlignHeight;
        } else if ((enPixelFormat >= PIXEL_FORMAT_YUYV_PACKED_422) && (enPixelFormat <= PIXEL_FORMAT_VYUY_PACKED_422)) {
            u32MainSize = u32MainStride * u32AlignHeight;
        } else {
            u32MainSize = u32MainStride * u32AlignHeight * 3U; // 3:bit
        }
        u32VBSize = u32MainSize;
    } else {
        if (u32Width <= 4096U) { // 4096:width
            u32HeadStride = 16; // 16:stride
        } else if (u32Width <= 8192U) { // 8192:width
            u32HeadStride = 32; // 32:stride
        } else {
            u32HeadStride = 64; // 64:stride
        }

        if (u32BitWidth == 8U) {
            u32MainStride = ALIGN_UP(u32Width, u32Align);
            u32HeadYSize = u32HeadStride * u32AlignHeight;
            u32YSize = u32MainStride * u32AlignHeight;

            if ((enPixelFormat == PIXEL_FORMAT_YVU_SEMIPLANAR_420) ||
                (enPixelFormat == PIXEL_FORMAT_YUV_SEMIPLANAR_420)) {
                u32HeadSize = (u32HeadStride * u32AlignHeight * 3U) >> 1; // 3:bit
                u32MainSize = (u32MainStride * u32AlignHeight * 3U) >> 1; // 3:bit
            } else if ((enPixelFormat == PIXEL_FORMAT_YVU_SEMIPLANAR_422) ||
                       (enPixelFormat == PIXEL_FORMAT_YUV_SEMIPLANAR_422) ||
                       (enPixelFormat == PIXEL_FORMAT_YVU_SEMIPLANAR_440) ||
                       (enPixelFormat == PIXEL_FORMAT_YUV_SEMIPLANAR_440)) {
                u32HeadSize = u32HeadStride * u32AlignHeight * 2U; // 2:bit
                u32MainSize = u32MainStride * u32AlignHeight * 2U; // 2:bit
            } else if (enPixelFormat == PIXEL_FORMAT_YUV_400) {
                u32HeadSize = u32HeadStride * u32AlignHeight;
                u32MainSize = u32MainStride * u32AlignHeight;
            } else {
                u32HeadSize = u32HeadStride * u32AlignHeight * 3U; // 3:bit
                u32MainSize = u32MainStride * u32AlignHeight * 3U; // 3:bit
            }
        } else if (u32BitWidth == 10U) { // 10:width
            u32TailInBytes = DIV_UP(u32Width % SEG_CMP_LENGTH * u32BitWidth, 8U); // 8:algin
            u32MainStride = ALIGN_DOWN(u32Width,
                                       SEG_CMP_LENGTH) +
                            ((u32TailInBytes > SEG_CMP_LENGTH) ? SEG_CMP_LENGTH : u32TailInBytes);
            u32MainStride = ALIGN_UP(u32MainStride, u32Align);
            u32ExtStride = (u32TailInBytes > SEG_CMP_LENGTH) ? (ALIGN_UP(DIV_UP(u32Width, 4U), u32Align)) : // 4:align
                ALIGN_UP((ALIGN_DOWN(u32Width, SEG_CMP_LENGTH) / 4U), u32Align); // 4:align

            u32HeadYSize = u32HeadStride * u32AlignHeight;
            u32YSize = u32MainStride * u32AlignHeight;
            u32ExtYSize = u32ExtStride * u32AlignHeight;

            if ((PIXEL_FORMAT_YVU_SEMIPLANAR_420 == enPixelFormat) ||
                (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == enPixelFormat)) {
                u32HeadSize = (u32HeadStride * u32AlignHeight * 3U) >> 1; // 3:format
                u32MainSize = (u32MainStride * u32AlignHeight * 3U) >> 1; // 3:format
                u32ExtSize = (u32ExtStride * u32AlignHeight * 3U) >> 1; // 3:format
            } else if ((enPixelFormat == PIXEL_FORMAT_YVU_SEMIPLANAR_422) ||
                       (enPixelFormat == PIXEL_FORMAT_YUV_SEMIPLANAR_422) ||
                       (enPixelFormat == PIXEL_FORMAT_YVU_SEMIPLANAR_440) ||
                       (enPixelFormat == PIXEL_FORMAT_YUV_SEMIPLANAR_440)) {
                u32HeadSize = u32HeadStride * u32AlignHeight * 2U; // 2:format
                u32MainSize = u32MainStride * u32AlignHeight * 2U; // 2:format
                u32ExtSize = u32ExtStride * u32AlignHeight * 2U; // 2:format
            } else if (enPixelFormat == PIXEL_FORMAT_YUV_400) {
                u32HeadSize = u32HeadStride * u32AlignHeight;
                u32MainSize = u32MainStride * u32AlignHeight;
                u32ExtSize = u32ExtStride * u32AlignHeight;
            } else {
                u32HeadSize = u32HeadStride * u32AlignHeight * 3U; // 3:format
                u32MainSize = u32MainStride * u32AlignHeight * 3U; // 3:format
                u32ExtSize = u32ExtStride * u32AlignHeight * 3U; // 3:format
            }
        } else {
            u32VBSize = 0;
            u32HeadYSize = 0;
            u32HeadSize = 0;
            u32HeadStride = 0;
            u32MainStride = 0;
            u32YSize = 0;
            u32MainSize = 0;
            u32ExtStride = 0;
            u32ExtYSize = 0;
        }

        u32HeadSize = ALIGN_UP(u32HeadSize, u32Align);

        u32VBSize = u32HeadSize + u32MainSize + u32ExtSize;
    }

    pstCalConfig->u32VBSize = u32VBSize;
    pstCalConfig->u32HeadYSize = u32HeadYSize;
    pstCalConfig->u32HeadSize = u32HeadSize;
    pstCalConfig->u32HeadStride = u32HeadStride;
    pstCalConfig->u32MainStride = u32MainStride;
    pstCalConfig->u32MainYSize = u32YSize;
    pstCalConfig->u32MainSize = u32MainSize;
    pstCalConfig->u32ExtStride = u32ExtStride;
    pstCalConfig->u32ExtYSize = u32ExtYSize;

    return;
}

__inline static HI_U32 COMMON_GetPicBufferSize(HI_U32 u32Width, HI_U32 u32Height, PIXEL_FORMAT_E enPixelFormat,
    DATA_BITWIDTH_E enBitWidth, COMPRESS_MODE_E enCmpMode, HI_U32 u32Align)
{
    VB_CAL_CONFIG_S stCalConfig;

    COMMON_GetPicBufferConfig(u32Width, u32Height, enPixelFormat, enBitWidth, enCmpMode, u32Align, &stCalConfig);

    return stCalConfig.u32VBSize;
}

__inline static HI_U32 VI_GetRawBufferSizeEx(HI_U32 u32Width, HI_U32 u32Height,
                                             PIXEL_FORMAT_E enPixelFormat,
                                             COMPRESS_MODE_E enCmpMode, HI_U32 u32CmpRatio, HI_U32 u32Align)
{
    HI_U32 u32BitWidth;
    HI_U32 u32Size = 0;
    HI_U32 u32Stride = 0;
    HI_U32 u32RawCmpRatio = 1600; // 1600:ratio

    if ((u32Width > HI_MAXINUM_LIMIT) || (u32Height > HI_MAXINUM_LIMIT)) {
        return 0;
    }

#ifndef USE_DVPP_FWK
    if (enCmpMode == COMPRESS_MODE_LINE) {
        u32RawCmpRatio = 1600; // 1600:ratio
    } else if (enCmpMode == COMPRESS_MODE_FRAME) {
        if (u32CmpRatio == 0) {
            u32RawCmpRatio = 2000; // 2000:ratio
        } else {
            u32RawCmpRatio = u32CmpRatio;
        }
    }
#endif
    /* u32Align: 0 is automatic mode, alignment size following system. Non-0 for specified alignment size */
    if (u32Align == 0) {
        u32Align = DEFAULT_ALIGN;
    } else if (u32Align > MAX_ALIGN) {
        u32Align = MAX_ALIGN;
    } else {
        u32Align = (ALIGN_UP(u32Align, DEFAULT_ALIGN));
    }

    switch (enPixelFormat) {
#ifndef USE_DVPP_FWK
        case PIXEL_FORMAT_RGB_BAYER_8BPP: {
            u32BitWidth = 8; // 8:bit
            break;
        }

        case PIXEL_FORMAT_RGB_BAYER_10BPP: {
            u32BitWidth = 10; // 10:bit
            break;
        }

        case PIXEL_FORMAT_RGB_BAYER_12BPP: {
            u32BitWidth = 12; // 12:bit
            break;
        }

        case PIXEL_FORMAT_RGB_BAYER_14BPP: {
            u32BitWidth = 14; // 14:bit
            break;
        }

        case PIXEL_FORMAT_RGB_BAYER_16BPP: {
            u32BitWidth = 16; // 16:bit
            break;
        }
#endif
        default:
        {
            u32BitWidth = 0;
            break;
        }
    }

    if (enCmpMode == COMPRESS_MODE_NONE) {
        u32Stride = ALIGN_UP(ALIGN_UP((u32Width * u32BitWidth), 8U) / 8U, u32Align); // 8:bit
        u32Size = u32Stride * u32Height;
#ifndef USE_DVPP_FWK
    } else if (enCmpMode == COMPRESS_MODE_LINE) {
        HI_U32 u32Tmp;
        // 16:,1000:,8192:,127:,128:,2:algo param
        u32Tmp = ALIGN_UP((16 + u32Width * u32BitWidth * 1000UL / u32RawCmpRatio + 8192 + 127) / 128, 2);
        u32Stride = ALIGN_UP(u32Tmp * 16, u32Align); // 16:format
        u32Size = u32Stride * u32Height;
    } else if (enCmpMode == COMPRESS_MODE_FRAME) {
        // 1000:,8:algo param
        u32Size = ALIGN_UP(u32Height * u32Width * u32BitWidth * 1000UL / (u32RawCmpRatio * 8), u32Align);
#endif
    }
    return u32Size;
}

__inline static HI_U32 VI_GetRawBufferSize(HI_U32 u32Width, HI_U32 u32Height,
                                           PIXEL_FORMAT_E enPixelFormat, COMPRESS_MODE_E enCmpMode, HI_U32 u32Align)
{
    return VI_GetRawBufferSizeEx(u32Width, u32Height, enPixelFormat, enCmpMode, 0, u32Align);
}

__inline static HI_U32 AVS_GetPicBufferSize(HI_U32 u32Width, HI_U32 u32Height, COMPRESS_MODE_E enCmpMode,
                                            HI_U32 u32Align)
{
    if ((u32Width > HI_MAXINUM_LIMIT) || (u32Height > HI_MAXINUM_LIMIT)) {
        return 0;
    }

    return ALIGN_UP(u32Width, 128U) * ALIGN_UP(u32Height, 64U) * 2U; // 128:align, 64:algin, 2:format
}

// 由于内部ko也会用到此接口，所以VDEC_InnerGetTmvBufferSize作为内部使用。
// Notice 注意！！！对外提供的接口为VDEC_GetTmvBufferSize,需要同步修改，保持实现一致。
__inline static HI_U32 VDEC_InnerGetTmvBufferSize(PAYLOAD_TYPE_E enType, HI_U32 u32Width, HI_U32 u32Height)
{
    HI_U32 WidthInMb, HeightInMb;
    HI_U32 ColMbSize;
    HI_U32 u32Size = 0;

    if ((u32Width > HI_MAXINUM_LIMIT) || (u32Height > HI_MAXINUM_LIMIT)) {
        return 0;
    }

    if (enType == PT_H264) {
        WidthInMb = ALIGN_UP(u32Width, 16U) >> 4; // 16:algin, 4:offset
        HeightInMb = ALIGN_UP(u32Height, 16U) >> 4; // 16:algin, 4:offset
        ColMbSize = 16 * 4; // 16:algin, 4:format
        u32Size = ALIGN_UP((ColMbSize * WidthInMb * HeightInMb), 128U); // 128:align
    } else if (enType == PT_H265) {
        WidthInMb = ALIGN_UP(u32Width, 64U) >> 4; // 64:algin, 4:offset
        HeightInMb = ALIGN_UP(u32Height, 64U) >> 4; // 64:algin, 4:offset
        ColMbSize = 4 * 4; // 4:format
        u32Size = ALIGN_UP((ColMbSize * WidthInMb * HeightInMb), 128U); // 128:algin
    } else {
        u32Size = 0;
    }

    return u32Size;
}

// 由于内部ko也会用到此接口，所以VDEC_InnerGetPicBufferSize作为内部使用。
// Notice 注意！！！对外提供的接口为VDEC_GetPicBufferSize,需要同步修改，保持实现一致。
__inline static HI_U32 VDEC_InnerGetPicBufferSize(PAYLOAD_TYPE_E enType, HI_U32 u32Width,
    HI_U32 u32Height, PIXEL_FORMAT_E enPixelFormat, DATA_BITWIDTH_E enBitWidth, HI_U32 u32Align)
{
    HI_U32 u32AlignWidth, u32AlignHeight;
    HI_U32 u32HeaderSize = 0;
    HI_U32 u32HeaderStride = 0;
    HI_U32 u32Size = 0;
    HI_U32 u32BlockNum = 0;

    if ((u32Width > HI_MAXINUM_LIMIT) || (u32Height > HI_MAXINUM_LIMIT)) {
        return 0;
    }

    if (enType == PT_H264) {
        u32Width = ALIGN_UP(u32Width, 16U); // 16:align
        u32Height = ALIGN_UP(u32Height, 16U); // 16:algin
        u32AlignWidth  = ALIGN_UP(u32Width, 64U); // 64:HFBC数据块宽64对齐
        u32AlignHeight = ALIGN_UP(u32Height, 8U); // 8:HFBC数据块高8对齐
        u32BlockNum = (u32AlignWidth * u32AlignHeight) / (64U * 8U); // 64:algin, 8:align
        u32HeaderStride = ((u32AlignWidth / 64U * 4U) + 127U) & (~127U) ; // 64:,4:,127:HeaderStride 128对齐
        u32HeaderSize = u32HeaderStride * u32AlignHeight / 8U * 2U; // 8:format, 2:format
        u32Size = (u32BlockNum * (512U + 256U)) + u32HeaderSize; // 512:algo param, 256:algo param
    } else if (enType == PT_H265) {
        u32Width = ALIGN_UP(u32Width, 64U); // 64:algin
        u32Height = ALIGN_UP(u32Height, 64U); // 64:algin
        if (enBitWidth == DATA_BITWIDTH_8) {
            u32AlignWidth  = ALIGN_UP(u32Width, 64U); // 64:HFBC数据块宽64对齐
            u32AlignHeight = ALIGN_UP(u32Height, 8U); // 8:HFBC数据块高8对齐
            u32BlockNum = (u32AlignWidth * u32AlignHeight) / (64U * 8U); // 64:algin, 8:algin
            u32HeaderStride = ((u32AlignWidth / 64U * 4U) + 127U) & (~127U) ; // 64:,4:,127:HeaderStride 128对齐
            u32HeaderSize = u32HeaderStride * u32AlignHeight / 8U * 2U; // 8:algin, 2:algin
            u32Size = (u32BlockNum * (512U + 256U)) + u32HeaderSize; // 512:algo param, 256:algo param
        } else if (enBitWidth == DATA_BITWIDTH_10) {
            if ((enPixelFormat == PIXEL_FORMAT_YUV_SEMI_PLANNER_420_10BIT) ||
                (enPixelFormat == PIXEL_FORMAT_YVU_SEMI_PLANNER_420_10BIT)) {
                u32AlignWidth  = ALIGN_UP(u32Width, 64U);
                u32AlignHeight = ALIGN_UP(u32Height, 64U);
                u32Size = u32AlignWidth * u32AlignHeight * 3U;
            } else {
                u32AlignWidth  = ALIGN_UP(u32Width, 64U); // 64:HFBC数据块宽64对齐
                u32AlignHeight = ALIGN_UP(u32Height, 8U); // 8:HFBC数据块高8对齐
                u32BlockNum = (u32AlignWidth * u32AlignHeight) / (64U * 8U); // 64:algin, 8:algin
                u32HeaderStride = ((u32AlignWidth / 64U * 4U) + 127U) & (~127U) ; // 64:,4:,127:HeaderStride 128对齐
                u32HeaderSize = u32HeaderStride * u32AlignHeight / 8U * 2U; // 8:algin, 2:algin
                u32Size = (u32BlockNum * 1024U) + u32HeaderSize; // 1024:format
            }
        }
    } else if (enType == PT_JPEG) {
       if ((enPixelFormat == PIXEL_FORMAT_RGB_888) || (enPixelFormat == PIXEL_FORMAT_BGR_888)) {
            u32AlignWidth = ALIGN_UP(u32Width * 3U, JPEGD_RGB_ALIGN);
            u32AlignHeight = ALIGN_UP(u32Height, JPEGD_RGB_ALIGN);
            u32Size = u32AlignWidth * u32AlignHeight;
        } else if ((enPixelFormat == PIXEL_FORMAT_ARGB_8888) || (enPixelFormat == PIXEL_FORMAT_ABGR_8888)) {
            u32AlignWidth = ALIGN_UP(u32Width * 4U, JPEGD_RGB_ALIGN); // 4:algo param
            u32AlignHeight = ALIGN_UP(u32Height, JPEGD_RGB_ALIGN);
            u32Size = u32AlignWidth * u32AlignHeight;
        } else if ((enPixelFormat == PIXEL_FORMAT_YVU_SEMIPLANAR_420) ||
                   (enPixelFormat == PIXEL_FORMAT_YUV_SEMIPLANAR_420)) {
            u32AlignWidth = ALIGN_UP(u32Width, JPEGD_ALIGN_W);
            u32AlignHeight = ALIGN_UP(u32Height, JPEGD_ALIGN_H);
            u32Size = (u32AlignWidth * u32AlignHeight * 3U) >> 1; // 3:format
        } else if ((enPixelFormat == PIXEL_FORMAT_YUV_SEMIPLANAR_422) ||
                   (enPixelFormat == PIXEL_FORMAT_YVU_SEMIPLANAR_422) ||
                   (enPixelFormat == PIXEL_FORMAT_YUV_SEMIPLANAR_440) ||
                   (enPixelFormat == PIXEL_FORMAT_YVU_SEMIPLANAR_440)) {
            u32AlignWidth = ALIGN_UP(u32Width, JPEGD_ALIGN_W);
            u32AlignHeight = ALIGN_UP(u32Height, JPEGD_ALIGN_H);
            u32Size = (u32AlignWidth * u32AlignHeight * 2U); // 2:algin
        } else if ((enPixelFormat == PIXEL_FORMAT_YUV_SEMIPLANAR_444) ||
                   (enPixelFormat == PIXEL_FORMAT_YVU_SEMIPLANAR_444)) {
            u32AlignWidth = ALIGN_UP(u32Width, JPEGD_ALIGN_W);
            u32AlignHeight = ALIGN_UP(u32Height, JPEGD_ALIGN_H);
            u32Size = (u32AlignWidth * u32AlignHeight * 3U); // 3:algo param
        } else if (enPixelFormat == PIXEL_FORMAT_YUV_400) {
            u32AlignWidth = ALIGN_UP(u32Width, JPEGD_ALIGN_W);
            u32AlignHeight = ALIGN_UP(u32Height, JPEGD_ALIGN_H);
            u32Size = u32AlignWidth * u32AlignHeight;
        } else if (enPixelFormat == PIXEL_FORMAT_UNKNOWN) {
            HI_TRACE(HI_DBG_WARN, HI_ID_SYS, "can't calculate unknown pic buffer size!\n");
            u32Size = 0;
        }
    } else {
        HI_TRACE(HI_DBG_WARN, HI_ID_SYS, "enType(%d) is not support!\n", enType);
        u32Size = 0;
    }
    return u32Size;
}

__inline static HI_U32 VPC_GetOutPicBufferSize(HI_U32 u32Width, HI_U32 u32Height,
    int enPixelFormat)
{
    HI_U32 u32AlignWidth, u32AlignHeight;
    HI_U32 u32Size = 0;

    if ((enPixelFormat == 0) || (enPixelFormat == 1)) {
        u32AlignWidth  = ALIGN_UP(u32Width * 3U, VPC_ALIGN_W); // 3:algo param
        u32AlignHeight = ALIGN_UP(u32Height,  VPC_ALIGN_H);
        u32Size = u32AlignWidth * u32AlignHeight;
    } else if ((enPixelFormat == 2) || (enPixelFormat == 3)) { // 2:format 3:format
        u32AlignWidth  = ALIGN_UP(u32Width * 4U, VPC_ALIGN_W); // 4:algo param
        u32AlignHeight = ALIGN_UP(u32Height,  VPC_ALIGN_H);
        u32Size = u32AlignWidth * u32AlignHeight;
    } else if (enPixelFormat == 4) { // 4:format
        /* for PIXEL_FORMAT_YVU_SEMIPLANAR_420 */
        u32AlignWidth  = ALIGN_UP(u32Width, VPC_ALIGN_W);
        u32AlignHeight = ALIGN_UP(u32Height, VPC_ALIGN_H);
        u32Size = (u32AlignWidth * u32AlignHeight * 3U) >> 1; // 3:algin
    } else if (enPixelFormat == 5) { // 5:format
        u32AlignWidth  = ALIGN_UP(u32Width, VPC_ALIGN_W);
        u32AlignHeight = ALIGN_UP(u32Height, VPC_ALIGN_H);
        u32Size = u32AlignWidth * u32AlignHeight * 2U; // 2:algin
    } else if (enPixelFormat == 6) { // 6:format
        u32AlignWidth  = ALIGN_UP(u32Width, VPC_ALIGN_W);
        u32AlignHeight = ALIGN_UP(u32Height, VPC_ALIGN_H);
        u32Size = u32AlignWidth * u32AlignHeight;
    } else {
        u32Size = 0;
    }
    return u32Size;
}


__inline static HI_U32 VENC_GetRefPicInfoBufferSize(PAYLOAD_TYPE_E enType, HI_U32 u32Width, HI_U32 u32Height,
                                                    HI_U32 u32Align)
{
    HI_U32 u32Size;
    HI_U32 u32AlignWidth, u32AlignHeight;
    HI_U32 u32TmvSize, u32PmeSize, u32PmeInfoSize;

    if ((u32Width > HI_MAXINUM_LIMIT) || (u32Height > HI_MAXINUM_LIMIT)) {
        return 0;
    }

    if (enType == PT_H265) {
        u32AlignWidth = ALIGN_UP(u32Width, 64U) >> 6; // 64:algin, 6:offset
        u32AlignHeight = ALIGN_UP(u32Height, 64U) >> 6; // 64:algin, 6:offset

        u32TmvSize = (u32AlignWidth * u32AlignHeight) << 7; // 7:offset
        u32PmeSize = (u32AlignWidth << 4) * (u32AlignHeight << 4); // 4: algin

        u32AlignWidth = ALIGN_UP(u32Width, 128U) >> 7; // 128:algin, 7:offset
        u32AlignHeight = ALIGN_UP(u32Height, 256U) >> 8; // 256:algin, 8:offset
        u32PmeInfoSize = (u32AlignWidth * u32AlignHeight) << 5; // 5:offset

        u32Size = u32TmvSize + u32PmeSize + u32PmeInfoSize;
    } else if (enType == PT_H264) {
        u32AlignWidth = ALIGN_UP(u32Width, 16U) >> 4; // 16:algin, 4:offset
        u32AlignHeight = ALIGN_UP(u32Height, 16U) >> 4; // 16:algin, 4:offset
        u32TmvSize = (u32AlignWidth * u32AlignHeight) << 5; // 5:offset

        u32AlignWidth = ALIGN_UP(u32Width, 64U) >> 6; // 64:algin, 6:offset
        u32AlignHeight = ALIGN_UP(u32Height, 64U) >> 6; // 64:algin, 6:offset
        u32PmeSize = (u32AlignWidth << 4) * (u32AlignHeight << 4); // 4:offset

        u32AlignWidth = ALIGN_UP(u32Width, 4096U) >> 12; // 4096:format, 12:offset
        u32AlignHeight = ALIGN_UP(u32Height, 16U) >> 4; // 16:format, 4:offset
        u32PmeInfoSize = (u32AlignWidth * u32AlignHeight) << 5; // 5:offset

        u32Size = u32TmvSize + u32PmeSize + u32PmeInfoSize;
    } else {
        u32Size = 0;
    }
    return u32Size;
}

__inline static HI_U32 VENC_GetRefBufferSize(PAYLOAD_TYPE_E enType, HI_U32 u32Width, HI_U32 u32Height,
                                             DATA_BITWIDTH_E enBitWidth, HI_U32 u32Align)
{
    HI_U32 u32Size = 0;
    HI_U32 u32AlignWidth, u32AlignHeight, u32BitWidth;
    HI_U32 u32YHeaderSize, u32CHeaderSize, u32YSize, u32CSize;

    if ((u32Width > HI_MAXINUM_LIMIT) || (u32Height > HI_MAXINUM_LIMIT)) {
        return 0;
    }

    if (enBitWidth == DATA_BITWIDTH_8) {
        u32BitWidth = 8; // 8:bit
    } else if (enBitWidth == DATA_BITWIDTH_10) {
        u32BitWidth = 10; // 10:bit
    } else {
        return 0;
    }

    if (enType == PT_H265) {
        u32AlignWidth = ALIGN_UP(u32Width, 128U); // 128:bit
        u32AlignHeight = ALIGN_UP(u32Height, 64U) >> 6; // 64:algin,6:offset

        u32YHeaderSize = u32AlignWidth / 64U * 32U * u32AlignHeight; // 64:,32:algo param
        u32CHeaderSize = u32YHeaderSize;

        u32AlignWidth = ALIGN_UP(u32Width, 64U); // 64:align
        u32AlignHeight = ALIGN_UP(u32Height, 16U); // 16:align
        u32YSize = (u32AlignWidth * u32AlignHeight * u32BitWidth) >> 3; // 3:offset
        u32CSize = u32YSize >> 1;

        u32Size = u32YHeaderSize + u32CHeaderSize + u32YSize + u32CSize;
    } else if (enType == PT_H264) {
        u32AlignWidth = ALIGN_UP(u32Width, 512U); // 512:align
        u32AlignHeight = ALIGN_UP(u32Height, 16U) >> 4; // 16:align, 4:offset
        u32YHeaderSize = ((u32AlignWidth >> 8) << 4) * u32AlignHeight; // 8:,4:offset
        u32CHeaderSize = u32YHeaderSize;

        u32AlignWidth = ALIGN_UP(u32Width, 64U); // 64:align
        u32AlignHeight = ALIGN_UP(u32Height, 16U); // 16:align
        u32YSize = u32AlignWidth * u32AlignHeight;
        u32CSize = u32YSize >> 1;

        u32Size = u32YHeaderSize + u32CHeaderSize + u32YSize + u32CSize;
    } else {
        u32Size = 0;
    }

    return u32Size;
}

__inline static HI_U32 VENC_GetQpmapSizeStride(HI_U32 u32Width)
{
    if (u32Width > HI_MAXINUM_LIMIT) {
        return 0;
    }

    return DIV_UP(u32Width, 512U) * 32U; // 512:,32:alog param
}

__inline static HI_U32 VENC_GetQpmapSize(HI_U32 u32Width, HI_U32 u32Height)
{
    HI_U32 u32Stride, u32AlignHeight;

    if ((u32Width > HI_MAXINUM_LIMIT) || (u32Height > HI_MAXINUM_LIMIT)) {
        return 0;
    }

    u32Stride = VENC_GetQpmapSizeStride(u32Width);
    u32AlignHeight = DIV_UP(u32Height, 16U); // 16:align

    return u32Stride * u32AlignHeight;
}

__inline static HI_U32 VENC_GetSkipWeightSizeStride(PAYLOAD_TYPE_E enType, HI_U32 u32Width)
{
    HI_U32 u32Stride;

    if (u32Width > HI_MAXINUM_LIMIT) {
        return 0;
    }

    if (enType == PT_H265) {
        u32Stride = DIV_UP(u32Width, 2048U) * 16U; // 2048:align, 16:offset
    } else if (enType == PT_H264) {
        u32Stride = DIV_UP(u32Width, 512U) * 16U; // 512:algin, 16:offset
    } else {
        u32Stride = 0;
    }

    return u32Stride;
}
__inline static HI_U32 VENC_GetSkipWeightSize(PAYLOAD_TYPE_E enType, HI_U32 u32Width, HI_U32 u32Height)
{
    HI_U32 u32Stride, u32AlignHeight;

    if ((u32Width > HI_MAXINUM_LIMIT) || (u32Height > HI_MAXINUM_LIMIT)) {
        return 0;
    }

    u32Stride = VENC_GetSkipWeightSizeStride(enType, u32Width);

    if (enType == PT_H265) {
        u32AlignHeight = DIV_UP(u32Height, 64U); // 64:align
    } else if (enType == PT_H264) {
        u32AlignHeight = DIV_UP(u32Height, 16U); // 16:align
    } else {
        u32AlignHeight = 0;
    }

    return u32Stride * u32AlignHeight;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* HI_BUFFER_H__ */

