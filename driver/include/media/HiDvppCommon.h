/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * Description:
 * Author: huawei
 * Create: 2020-4-1
 */

#ifndef HI_DVPP_COMMON_H
#define HI_DVPP_COMMON_H

#include "HiDvppType.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define ALIGN_NUM 8
#define DEFAULT_ALIGN 32U
#define ATTRIBUTE __attribute__((aligned(ALIGN_NUM)))

#define ALIGN_UP(x, a) ((((x) + ((a) - 1U)) / (a)) * (a))

#define VERSION_NAME_MAXLEN 64

#define HI_ERR_APPID (0x80000000U + 0x20000000U)

#define HI_DEF_ERR(module, level, errid) \
    ((HI_S32)((HI_ERR_APPID) | (((HI_U32)(module)) << 16) | (((HI_U32)(level)) << 13) | ((HI_U32)(errid))))

typedef enum hiOPERATION_MODE_E {
    OPERATION_MODE_AUTO   = 0,
    OPERATION_MODE_MANUAL = 1,
    OPERATION_MODE_BUTT = 2
} OPERATION_MODE_E;

typedef enum hiEN_ERR_CODE_E {
    EN_ERR_INVALID_DEVID = 1, // invlalid device ID
    EN_ERR_INVALID_CHNID = 2, // invlalid channel ID
    EN_ERR_ILLEGAL_PARAM = 3, // at lease one parameter is illagal
                              // eg, an illegal enumeration value
    EN_ERR_EXIST         = 4, // resource exists
    EN_ERR_UNEXIST       = 5, // resource unexists

    EN_ERR_NULL_PTR      = 6, // using a NULL point

    EN_ERR_NOT_CONFIG    = 7, // try to enable or initialize system, device
                              // or channel, before configing attribute

    EN_ERR_NOT_SUPPORT   = 8, // operation or type is not supported by NOW
    EN_ERR_NOT_PERM      = 9, // operation is not permitted
                              // eg, try to change static attribute
    EN_ERR_INVALID_PIPEID = 10, // invlalid pipe ID
    EN_ERR_INVALID_STITCHGRPID  = 11, // invlalid stitch group ID

    EN_ERR_NOMEM         = 12, // failure caused by malloc memory
    EN_ERR_NOBUF         = 13, // failure caused by malloc buffer

    EN_ERR_BUF_EMPTY     = 14, // no data in buffer
    EN_ERR_BUF_FULL      = 15, // no buffer for new data

    EN_ERR_SYS_NOTREADY  = 16, // System is not ready,maybe not initialed or
                               // loaded. Returning the error code when opening
                               // a device file failed.

    EN_ERR_BADADDR       = 17, // bad address,
                               // eg. used for copy_from_user & copy_to_user

    EN_ERR_BUSY          = 18, // resource is busy,
                               // eg. destroy a venc chn without unregister it
    EN_ERR_SIZE_NOT_ENOUGH = 19, // buffer size is smaller than the actual size required
    EN_ERR_TIMEOUT       = 20, // hardware or software timeout
    EN_ERR_SYS_ERROR     = 21, // Internal system error

    EN_ERR_BUTT          = 63, // maxium code, private error code of all modules
                               // must be greater than it
} EN_ERR_CODE_E;

typedef enum hiERR_LEVEL_E {
    EN_ERR_LEVEL_DEBUG = 0, // debug-level
    EN_ERR_LEVEL_INFO, // informational
    EN_ERR_LEVEL_NOTICE, // normal but significant condition
    EN_ERR_LEVEL_WARNING, // warning conditions
    EN_ERR_LEVEL_ERROR, // error conditions
    EN_ERR_LEVEL_CRIT, // critical conditions
    EN_ERR_LEVEL_ALERT, // action must be taken immediately
    EN_ERR_LEVEL_FATAL, // just for compatibility with previous version
    EN_ERR_LEVEL_BUTT
} ERR_LEVEL_E;

typedef struct hiMPP_VERSION_S {
    HI_CHAR aVersion[VERSION_NAME_MAXLEN];
} MPP_VERSION_S;

typedef enum hiMOD_ID_E {
    HI_ID_CMPI = 0,
    HI_ID_VB = 1,
    HI_ID_SYS = 2,
    HI_ID_RGN = 3,
    HI_ID_CHNL = 4,
    HI_ID_VDEC = 5,
    HI_ID_AVS = 6,
    HI_ID_VPC = 7,
    HI_ID_VENC = 8,
    HI_ID_SVP = 9,
    HI_ID_H264E = 10,
    HI_ID_JPEGE = 11,
    HI_ID_MPEG4E = 12,
    HI_ID_H265E = 13,
    HI_ID_JPEGD = 14,
    HI_ID_VO = 15,
    HI_ID_VI = 16,
    HI_ID_DIS = 17,
    HI_ID_VALG = 18,
    HI_ID_RC = 19,
    HI_ID_AIO = 20,
    HI_ID_AI = 21,
    HI_ID_AO = 22,
    HI_ID_AENC = 23,
    HI_ID_ADEC = 24,
    HI_ID_VPU = 25,
    HI_ID_PCIV = 26,
    HI_ID_PCIVFMW = 27,
    HI_ID_ISP = 28,
    HI_ID_IVE = 29,
    HI_ID_USER = 30,
    HI_ID_DCCM = 31,
    HI_ID_DCCS = 32,
    HI_ID_PROC = 33,
    HI_ID_LOG = 34,
    HI_ID_VFMW = 35,
    HI_ID_H264D = 36,
    HI_ID_GDC = 37,
    HI_ID_PHOTO = 38,
    HI_ID_FB = 39,
    HI_ID_HDMI = 40,
    HI_ID_VOIE = 41,
    HI_ID_TDE = 42,
    HI_ID_HDR = 43,
    HI_ID_PRORES = 44,
    HI_ID_VGS = 45,

    HI_ID_FD = 47,
    HI_ID_ODT = 48, // Object detection trace
    HI_ID_VQA = 49, // Video quality analysis
    HI_ID_LPR = 50, // Object detection trace
    HI_ID_SVP_NNIE = 51,
    HI_ID_SVP_DSP = 52,
    HI_ID_DPU_RECT = 53,
    HI_ID_DPU_MATCH = 54,

    HI_ID_MOTIONSENSOR = 55,
    HI_ID_MOTIONFUSION = 56,

    HI_ID_GYRODIS = 57,
    HI_ID_PM = 58,
    HI_ID_SVP_ALG = 59,
    HI_ID_IVP = 60,
    HI_ID_MCF = 61,
    HI_ID_VPSS = 62,
    HI_ID_DRV_VPC = 63,
    HI_ID_PNGD = 64,

    HI_ID_VDEC_ADAPT   = 65,
    HI_ID_DCC          = 66,
    HI_ID_VDEC_SERVER  = 67,

    HI_ID_DVPP_COMM  = 68,
    HI_ID_VFMW_SERVER = 69,

    HI_ID_BUTT = 0x100,
} MOD_ID_E;

// We just coyp this value of payload type from RTP/RTSP definition
typedef enum {
    PT_PCMU = 0,
    PT_1016 = 1,
    PT_G721 = 2,
    PT_GSM = 3,
    PT_G723 = 4,
    PT_DVI4_8K = 5,
    PT_DVI4_16K = 6,
    PT_LPC = 7,
    PT_PCMA = 8,
    PT_G722 = 9,
    PT_S16BE_STEREO = 10,
    PT_S16BE_MONO = 11,
    PT_QCELP = 12,
    PT_CN = 13,
    PT_MPEGAUDIO = 14,
    PT_G728 = 15,
    PT_DVI4_3 = 16,
    PT_DVI4_4 = 17,
    PT_G729 = 18,
    PT_G711A = 19,
    PT_G711U = 20,
    PT_G726 = 21,
    PT_G729A = 22,
    PT_LPCM = 23,
    PT_CelB = 25,
    PT_JPEG = 26,
    PT_CUSM = 27,
    PT_NV = 28,
    PT_PICW = 29,
    PT_CPV = 30,
    PT_H261 = 31,
    PT_MPEGVIDEO = 32,
    PT_MPEG2TS = 33,
    PT_H263 = 34,
    PT_SPEG = 35,
    PT_MPEG2VIDEO = 36,
    PT_AAC = 37,
    PT_WMA9STD = 38,
    PT_HEAAC = 39,
    PT_PCM_VOICE = 40,
    PT_PCM_AUDIO = 41,
    PT_MP3 = 43,
    PT_ADPCMA = 49,
    PT_AEC = 50,
    PT_X_LD = 95,
    PT_H264 = 96,
    PT_D_GSM_HR = 200,
    PT_D_GSM_EFR = 201,
    PT_D_L8 = 202,
    PT_D_RED = 203,
    PT_D_VDVI = 204,
    PT_D_BT656 = 220,
    PT_D_H263_1998 = 221,
    PT_D_MP1S = 222,
    PT_D_MP2P = 223,
    PT_D_BMPEG = 224,
    PT_MP4VIDEO = 230,
    PT_MP4AUDIO = 237,
    PT_VC1 = 238,
    PT_JVC_ASF = 255,
    PT_D_AVI = 256,
    PT_DIVX3 = 257,
    PT_AVS = 258,
    PT_REAL8 = 259,
    PT_REAL9 = 260,
    PT_VP6 = 261,
    PT_VP6F = 262,
    PT_VP6A = 263,
    PT_SORENSON  = 264,
    PT_H265 = 265,
    PT_VP8 = 266,
    PT_MVC = 267,
    PT_PNG = 268,
    // add by hisilicon
    PT_AMR = 1001,
    PT_MJPEG = 1002,
    PT_AMRWB = 1003,
    PT_PRORES = 1006,
    PT_OPUS = 1007,
    PT_VPC = 2000,
    PT_BUTT
} PAYLOAD_TYPE_E;

typedef enum hiDATA_BITWIDTH_E {
    DATA_BITWIDTH_8 = 0,
    DATA_BITWIDTH_10,
    DATA_BITWIDTH_12,
    DATA_BITWIDTH_14,
    DATA_BITWIDTH_16,
    DATA_BITWIDTH_BUTT
} DATA_BITWIDTH_E;

// we ONLY define picture format used, all unused will be deleted!
typedef enum hiPIXEL_FORMAT_E {
    PIXEL_FORMAT_YUV_400 = 0,
    PIXEL_FORMAT_YUV_SEMIPLANAR_420 = 1,
    PIXEL_FORMAT_YVU_SEMIPLANAR_420 = 2,
    PIXEL_FORMAT_YUV_SEMIPLANAR_422 = 3,
    PIXEL_FORMAT_YVU_SEMIPLANAR_422 = 4,
    PIXEL_FORMAT_YUV_SEMIPLANAR_444 = 5,
    PIXEL_FORMAT_YVU_SEMIPLANAR_444 = 6,
    PIXEL_FORMAT_YUYV_PACKED_422 = 7,
    PIXEL_FORMAT_UYVY_PACKED_422 = 8,
    PIXEL_FORMAT_YVYU_PACKED_422 = 9,
    PIXEL_FORMAT_VYUY_PACKED_422 = 10,
    PIXEL_FORMAT_YUV_PACKED_444 = 11,
    PIXEL_FORMAT_RGB_888 = 12,
    PIXEL_FORMAT_BGR_888 = 13,
    PIXEL_FORMAT_ARGB_8888 = 14,
    PIXEL_FORMAT_ABGR_8888 = 15,
    PIXEL_FORMAT_RGBA_8888 = 16,
    PIXEL_FORMAT_BGRA_8888 = 17,
    PIXEL_FORMAT_YUV_SEMI_PLANNER_420_10BIT = 18,
    PIXEL_FORMAT_YVU_SEMI_PLANNER_420_10BIT = 19,
    PIXEL_FORMAT_YVU_PLANAR_420 = 20,
    PIXEL_FORMAT_YVU_PLANAR_422 = 21,
    PIXEL_FORMAT_YVU_PLANAR_444 = 22,
    PIXEL_FORMAT_RGB_444 = 23,
    PIXEL_FORMAT_BGR_444 = 24,
    PIXEL_FORMAT_ARGB_4444 = 25,
    PIXEL_FORMAT_ABGR_4444 = 26,
    PIXEL_FORMAT_RGBA_4444 = 27,
    PIXEL_FORMAT_BGRA_4444 = 28,
    PIXEL_FORMAT_RGB_555 = 29,
    PIXEL_FORMAT_BGR_555 = 30,
    PIXEL_FORMAT_RGB_565 = 31,
    PIXEL_FORMAT_BGR_565 = 32,
    PIXEL_FORMAT_ARGB_1555 = 33,
    PIXEL_FORMAT_ABGR_1555 = 34,
    PIXEL_FORMAT_RGBA_1555 = 35,
    PIXEL_FORMAT_BGRA_1555 = 36,
    PIXEL_FORMAT_ARGB_8565 = 37,
    PIXEL_FORMAT_ABGR_8565 = 38,
    PIXEL_FORMAT_RGBA_8565 = 39,
    PIXEL_FORMAT_BGRA_8565 = 40,
    PIXEL_FORMAT_ARGB_CLUT2 = 41,
    PIXEL_FORMAT_ARGB_CLUT4 = 42,

    PIXEL_FORMAT_RGB_BAYER_8BPP = 50,
    PIXEL_FORMAT_RGB_BAYER_10BPP = 51,
    PIXEL_FORMAT_RGB_BAYER_12BPP = 52,
    PIXEL_FORMAT_RGB_BAYER_14BPP = 53,
    PIXEL_FORMAT_RGB_BAYER_16BPP = 54,
    PIXEL_FORMAT_YUV_PLANAR_420 = 55,
    PIXEL_FORMAT_YUV_PLANAR_422 = 56,
    PIXEL_FORMAT_YUV_PLANAR_444 = 57,
    PIXEL_FORMAT_YVU_PACKED_444 = 58,
    PIXEL_FORMAT_XYUV_PACKED_444 = 59,
    PIXEL_FORMAT_XYVU_PACKED_444 = 60,
    PIXEL_FORMAT_YUV_SEMIPLANAR_411 = 61,
    PIXEL_FORMAT_YVU_SEMIPLANAR_411 = 62,
    PIXEL_FORMAT_YUV_PLANAR_411 = 63,
    PIXEL_FORMAT_YVU_PLANAR_411 = 64,
    PIXEL_FORMAT_YUV_PLANAR_440 = 65,
    PIXEL_FORMAT_YVU_PLANAR_440 = 66,

    PIXEL_FORMAT_RGB_888_PLANAR = 69,
    PIXEL_FORMAT_BGR_888_PLANAR = 70,
    PIXEL_FORMAT_HSV_888_PACKAGE = 71,
    PIXEL_FORMAT_HSV_888_PLANAR = 72,
    PIXEL_FORMAT_LAB_888_PACKAGE = 73,
    PIXEL_FORMAT_LAB_888_PLANAR = 74,
    PIXEL_FORMAT_S8C1 = 75,
    PIXEL_FORMAT_S8C2_PACKAGE = 76,
    PIXEL_FORMAT_S8C2_PLANAR = 77,
    PIXEL_FORMAT_S16C1 = 78,
    PIXEL_FORMAT_U8C1 = 79,
    PIXEL_FORMAT_U16C1 = 80,
    PIXEL_FORMAT_S32C1 = 81,
    PIXEL_FORMAT_U32C1 = 82,
    PIXEL_FORMAT_U64C1 = 83,
    PIXEL_FORMAT_S64C1 = 84,

    PIXEL_FORMAT_RGB_888_INT8 = 110,
    PIXEL_FORMAT_BGR_888_INT8 = 111,
    PIXEL_FORMAT_RGB_888_INT16 = 112,
    PIXEL_FORMAT_BGR_888_INT16 = 113,
    PIXEL_FORMAT_RGB_888_INT32 = 114,
    PIXEL_FORMAT_BGR_888_INT32 = 115,
    PIXEL_FORMAT_RGB_888_UINT16 = 116,
    PIXEL_FORMAT_BGR_888_UINT16 = 117,
    PIXEL_FORMAT_RGB_888_UINT32 = 118,
    PIXEL_FORMAT_BGR_888_UINT32 = 119,
    PIXEL_FORMAT_RGB_888_PLANAR_INT8  = 120,
    PIXEL_FORMAT_BGR_888_PLANAR_INT8  = 121,
    PIXEL_FORMAT_RGB_888_PLANAR_INT16 = 122,
    PIXEL_FORMAT_BGR_888_PLANAR_INT16 = 123,
    PIXEL_FORMAT_RGB_888_PLANAR_INT32 = 124,
    PIXEL_FORMAT_BGR_888_PLANAR_INT32 = 125,
    PIXEL_FORMAT_RGB_888_PLANAR_UINT16 = 126,
    PIXEL_FORMAT_BGR_888_PLANAR_UINT16 = 127,
    PIXEL_FORMAT_RGB_888_PLANAR_UINT32 = 128,
    PIXEL_FORMAT_BGR_888_PLANAR_UINT32 = 129,
    PIXEL_FORMAT_YUV400_UINT16 = 130,
    PIXEL_FORMAT_YUV400_UINT32 = 131,
    PIXEL_FORMAT_YUV400_UINT64 = 132,
    PIXEL_FORMAT_YUV400_INT8   = 133,
    PIXEL_FORMAT_YUV400_INT16  = 134,
    PIXEL_FORMAT_YUV400_INT32  = 135,
    PIXEL_FORMAT_YUV400_INT64  = 136,
    PIXEL_FORMAT_YUV400_FP16 = 137,
    PIXEL_FORMAT_YUV400_FP32 = 138,
    PIXEL_FORMAT_YUV400_FP64 = 139,
    PIXEL_FORMAT_YUV400_BF16 = 140,

    PIXEL_FORMAT_YUV_SEMIPLANAR_440 = 1000,
    PIXEL_FORMAT_YVU_SEMIPLANAR_440 = 1001,
    PIXEL_FORMAT_FLOAT32 = 1002,
    PIXEL_FORMAT_BUTT = 1003,

    PIXEL_FORMAT_RGB_888_PLANAR_FP16 = 1004,
    PIXEL_FORMAT_BGR_888_PLANAR_FP16 = 1005,
    PIXEL_FORMAT_RGB_888_PLANAR_FP32 = 1006,
    PIXEL_FORMAT_BGR_888_PLANAR_FP32 = 1007,
    PIXEL_FORMAT_RGB_888_PLANAR_BF16 = 1008,
    PIXEL_FORMAT_BGR_888_PLANAR_BF16 = 1009,
    PIXEL_FORMAT_RGB_888_FP16 = 1010,
    PIXEL_FORMAT_BGR_888_FP16 = 1011,
    PIXEL_FORMAT_RGB_888_FP32 = 1012,
    PIXEL_FORMAT_BGR_888_FP32 = 1013,
    PIXEL_FORMAT_RGB_888_BF16 = 1014,
    PIXEL_FORMAT_BGR_888_BF16 = 1015,

    PIXEL_FORMAT_UNKNOWN = 10000
} PIXEL_FORMAT_E;

typedef enum hiVIDEO_MODE_E {
    VIDEO_MODE_STREAM = 0, // send by stream
    VIDEO_MODE_FRAME, // send by frame
    VIDEO_MODE_COMPAT, // One frame supports multiple packets sending.
    VIDEO_MODE_BUTT
} VIDEO_MODE_E;

typedef enum hiVIDEO_FORMAT_E {
    VIDEO_FORMAT_LINEAR = 0, // nature video line
    VIDEO_FORMAT_TILE_64x16, // tile cell: 64pixel x 16line
    VIDEO_FORMAT_BUTT
} VIDEO_FORMAT_E;

typedef enum hiCOMPRESS_MODE_E {
    COMPRESS_MODE_NONE = 0, // no compress
    COMPRESS_MODE_SEG, // compress unit is 256x1 bytes as a segment
    COMPRESS_MODE_TILE, // compress unit is a tile
    COMPRESS_MODE_HFBC,
    COMPRESS_MODE_LINE,
    COMPRESS_MODE_FRAME,
    COMPRESS_MODE_BUTT
} COMPRESS_MODE_E;

typedef enum hiCOLOR_GAMUT_E {
    COLOR_GAMUT_BT601 = 0,
    COLOR_GAMUT_BT709,
    COLOR_GAMUT_BT2020,
    COLOR_GAMUT_USER,
    COLOR_GAMUT_BUTT
} COLOR_GAMUT_E;

typedef enum hiDYNAMIC_RANGE_E {
    DYNAMIC_RANGE_SDR8 = 0,
    DYNAMIC_RANGE_SDR10,
    DYNAMIC_RANGE_HDR10,
    DYNAMIC_RANGE_HLG,
    DYNAMIC_RANGE_SLF,
    DYNAMIC_RANGE_XDR,
    DYNAMIC_RANGE_BUTT
} DYNAMIC_RANGE_E;

typedef enum hiVIDEO_FIELD_E {
    VIDEO_FIELD_TOP = 0x1, // even field
    VIDEO_FIELD_BOTTOM = 0x2, // odd field
    VIDEO_FIELD_INTERLACED = 0x3, // two interlaced fields
    VIDEO_FIELD_FRAME = 0x4, // frame

    VIDEO_FIELD_BUTT
} VIDEO_FIELD_E;

typedef enum hiVIDEO_OUTPUT_ORDER_E {
    VIDEO_OUTPUT_ORDER_DISP = 0,
    VIDEO_OUTPUT_ORDER_DEC,
    VIDEO_OUTPUT_ORDER_BUTT
} VIDEO_OUTPUT_ORDER_E;

typedef enum hiJPEG_RAW_FORMAT_E {
    JPEG_RAW_FORMAT_YUV444 = 0,
    JPEG_RAW_FORMAT_YUV422 = 1,
    JPEG_RAW_FORMAT_YUV420 = 2,
    JPEG_RAW_FORMAT_YUV440 = 3,
    JPEG_RAW_FORMAT_YUV400 = 4,
    JPEG_RAW_FORMAT_YUV411 = 5,
    JPEG_RAW_FORMAT_MAX = 100
} JPEG_RAW_FORMAT_E;

typedef enum hiPNG_COLOR_FORMAT_E {
    PNG_COLOR_FORMAT_GRAY  = 0x0, // gray bitmap
    PNG_COLOR_FORMAT_RGB   = 0x2, // RGB bitmap
    PNG_COLOR_FORMAT_CLUT  = 0x3, // clut
    PNG_COLOR_FORMAT_AGRAY = 0x4, // gray bitmap with alpha
    PNG_COLOR_FORMAT_ARGB  = 0x6, // RGB bitmap with alpha
    PNG_COLOR_FORMAT_BUTT  = 0x100
} PNG_COLOR_FORMAT_E;

typedef enum hiCSC_MATRIX_E {
   CSC_MATRIX_BT601_WIDE = 0,
   CSC_MATRIX_BT601_NARROW,
   CSC_MATRIX_BT709_WIDE,
   CSC_MATRIX_BT709_NARROW,
   CSC_MATRIX_BT2020_WIDE,
   CSC_MATRIX_BT2020_NARROW,
   CSC_MATRIX_USER = 100,
   CSC_MATRIX_BUTT
} CSC_MATRIX_E;

typedef struct hiCOEFFICIENT_S {
   HI_DOUBLE cscMatrixR0C0;
   HI_DOUBLE cscMatrixR0C1;
   HI_DOUBLE cscMatrixR0C2;
   HI_DOUBLE cscMatrixR1C0;
   HI_DOUBLE cscMatrixR1C1;
   HI_DOUBLE cscMatrixR1C2;
   HI_DOUBLE cscMatrixR2C0;
   HI_DOUBLE cscMatrixR2C1;
   HI_DOUBLE cscMatrixR2C2;
   HI_DOUBLE cscBiasR0;
   HI_DOUBLE cscBiasR1;
   HI_DOUBLE cscBiasR2;
} COEFFICIENT_S;

typedef struct hiCSC_COEFFICIENT_S {
    COEFFICIENT_S yuvToRgbCoefficient;
    COEFFICIENT_S rgbToYuvCoefficient;
} CSC_COEFFICIENT_S;

typedef struct hiVIDEO_SUPPLEMENT_S {
    HI_U64 u64JpegDCFPhyAddr;
    HI_U64 u64IspInfoPhyAddr;
    HI_U64 u64LowDelayPhyAddr;
    HI_U64 u64MotionDataPhyAddr;
    HI_U64 u64FrameDNGPhyAddr;

    HI_VOID* ATTRIBUTE pJpegDCFVirAddr; // JPEG_DCF_S, used in JPEG DCF
    HI_VOID* ATTRIBUTE pIspInfoVirAddr; // ISP_FRAME_INFO_S, used in ISP debug, when get raw and send raw
    HI_VOID* ATTRIBUTE pLowDelayVirAddr; // used in low delay
    HI_VOID* ATTRIBUTE pMotionDataVirAddr; // vpss 3dnr use: gme motion data, Filter motion data, gyro data.
    HI_VOID* ATTRIBUTE pFrameDNGVirAddr;
} VIDEO_SUPPLEMENT_S;

typedef struct hiVIDEO_FRAME_S {
    HI_U32 u32Width;
    HI_U32 u32Height;
    VIDEO_FIELD_E enField;
    PIXEL_FORMAT_E enPixelFormat;
    VIDEO_FORMAT_E enVideoFormat;
    COMPRESS_MODE_E enCompressMode;
    DYNAMIC_RANGE_E enDynamicRange;
    COLOR_GAMUT_E enColorGamut;
    HI_U32 u32HeaderStride[3];
    HI_U32 u32Stride[3];
    HI_U32 u32ExtStride[3];

    HI_U64 u64HeaderPhyAddr[3];
    HI_U64 u64HeaderVirAddr[3];
    HI_U64 u64PhyAddr[3];
    HI_U64 u64VirAddr[3];
    HI_U64 u64ExtPhyAddr[3];
    HI_U64 u64ExtVirAddr[3];

    HI_S16 s16OffsetTop; // top offset of show area
    HI_S16 s16OffsetBottom; // bottom offset of show area
    HI_S16 s16OffsetLeft; // left offset of show area
    HI_S16 s16OffsetRight; // right offset of show area

    HI_U32 u32MaxLuminance;
    HI_U32 u32MinLuminance;

    HI_U32 u32TimeRef;
    HI_U64 u64PTS;

    HI_U64 u64PrivateData;
    HI_U32 u32FrameFlag; // FRAME_FLAG_E, 0:Decode Success. 1:Decode Fail. 2:No Picture, For Field Stream
    VIDEO_SUPPLEMENT_S  stSupplement;
} VIDEO_FRAME_S;

typedef struct hiVIDEO_FRAME_INFO_S {
    VIDEO_FRAME_S stVFrame;
    HI_U32        u32PoolId;
    MOD_ID_E      enModId;
} VIDEO_FRAME_INFO_S;

typedef struct hiVIDEO_SIZE_S {
    HI_U32 u32Width;
    HI_U32 u32Height;
} VIDEO_SIZE_S;

typedef struct hiIMG_INFO_S {
    HI_U32 u32Width;
    HI_U32 u32Height;
    HI_U32 u32WidthStride;
    HI_U32 u32HeightStride;
    HI_U32 u32ImgBufSize;
    union {
        JPEG_RAW_FORMAT_E enPixelFormat;
        PNG_COLOR_FORMAT_E pngPixelFormat;
    };
    HI_U32 u32Reserved[4];
} IMG_INFO_S;

typedef struct hiIMAGE_STREAM_S {
    PAYLOAD_TYPE_E enType;
    HI_U8 *ATTRIBUTE pu8Addr;
    HI_U32 u32Len;
    HI_U64 u64Pts;
    HI_U32 reserved[2];
} IMG_STREAM_S;

typedef enum hiVIDEO_DISPLAY_MODE_E {
    VIDEO_DISPLAY_MODE_PREVIEW  = 0x0,
    VIDEO_DISPLAY_MODE_PLAYBACK = 0x1,

    VIDEO_DISPLAY_MODE_BUTT = 0x2
} VIDEO_DISPLAY_MODE_E;

/*
 * @brief alloc device memory for dvpp
 * @param [in] devId: the device id, set 0 in 1p device
 * @param [in] size: memory size
 * @param [out] devPtr: memory pointer
 * @return success: return 0
 *         fail: return error number
 */
HI_S32 HI_MPI_DvppMalloc(HI_U32 devId, HI_VOID **devPtr, HI_U64 size);

/*
 * @brief free the memory requested through the HI_MPI_DvppMalloc interface
 * @param [in] devPtr: memory pointer
 * @return success: return 0
 *         fail: return error number
 */
HI_S32 HI_MPI_DvppFree(HI_VOID *devPtr);

/**
 * @brief query DVPP interface version
 * @param [out] majorVersion: DVPP interface major version
 * @param [out] minorVersion: DVPP interface minor version
 * @param [out] patchVersion: DVPP interface patch version
 * @return success: return 0
 *         fail: return error number
 */
HI_S32 HI_MPI_DvppGetVersion(HI_S32 *majorVersion, HI_S32 *minorVersion, HI_S32 *patchVersion);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // #ifndef HI_DVPP_COMMON_H
