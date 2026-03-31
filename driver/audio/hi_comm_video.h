/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2016-2022. All rights reserved.
 * Create: 2016-09-19
 */

#ifndef HI_COMM_VIDEO_H__
#define HI_COMM_VIDEO_H__

#include "HiDvppVB.h"
#include "hi_type.h"
#include "hi_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define ISP_BAYER_CHN           (4)


typedef enum hiBitWidth {
    BIT_WIDTH_8,
    BIT_WIDTH_10,
} BIT_WIDTH_E;

/* Angle of rotation */
typedef enum hiROTATION_E {
    ROTATION_0   = 0,
    ROTATION_90  = 1,
    ROTATION_180 = 2,
    ROTATION_270 = 3,
    ROTATION_BUTT = 4
} ROTATION_E;

typedef enum hiDATA_RATE_E {
    DATA_RATE_X1 = 0,         /* RW; output 1 pixel per clock */
    DATA_RATE_X2 = 1,         /* RW; output 2 pixel per clock */

    DATA_RATE_BUTT
} DATA_RATE_E;
typedef struct hiBORDER_S {
    HI_U32 u32TopWidth;
    HI_U32 u32BottomWidth;
    HI_U32 u32LeftWidth;
    HI_U32 u32RightWidth;
    HI_U32 u32Color;
} BORDER_S;
typedef struct hiPOINT_S {
    HI_S32 s32X;
    HI_S32 s32Y;
} POINT_S;
typedef struct hiSIZE_S {
    HI_U32 u32Width;
    HI_U32 u32Height;
} SIZE_S;
#ifndef USE_DVPP_FWK
typedef struct hiRECT_S {
    HI_S32 s32X;
    HI_S32 s32Y;
    HI_U32 u32Width;
    HI_U32 u32Height;
} RECT_S;

typedef struct hiVIDEO_REGION_INFO_S {
    HI_U32           u32RegionNum;       /* W; count of the region */
    RECT_S ATTRIBUTE* pstRegion;         /* W; region attribute */
} VIDEO_REGION_INFO_S;

typedef struct hiCROP_INFO_S {
    HI_BOOL bEnable;
    RECT_S  stRect;
} CROP_INFO_S;
#endif
typedef struct hiFRAME_RATE_CTRL_S {
    HI_S32  s32SrcFrameRate;        /* RW; source frame rate */
    HI_S32  s32DstFrameRate;        /* RW; dest frame rate */
} FRAME_RATE_CTRL_S;

typedef enum hiASPECT_RATIO_E {
    ASPECT_RATIO_NONE   = 0,        /* full screen */
    ASPECT_RATIO_AUTO   = 1,        /* ratio no change, 1:1 */
    ASPECT_RATIO_MANUAL = 2,        /* ratio manual set */
    ASPECT_RATIO_BUTT
} ASPECT_RATIO_E;
#ifndef USE_DVPP_FWK
typedef struct hiASPECT_RATIO_S {
    ASPECT_RATIO_E enMode;          /* aspect ratio mode: none/auto/manual */
    HI_U32         u32BgColor;      /* background color, RGB 888 */
    RECT_S         stVideoRect;     /* valid in ASPECT_RATIO_MANUAL mode */
} ASPECT_RATIO_S;
#endif

typedef struct hiLUMA_INFO_S {
    HI_U64 u64LumaPixSum;      /* Luma sum of current frame */
    HI_U32 u32LumaPixAverage;  /* Luma average of current frame */
    HI_U64 u64PTS;             /* PTS of current frame  */
} LUMA_INFO_S;
#define LDCI_STAT_WND_X         24
#define LDCI_STAT_WND_Y         16
#define DCF_DRSCRIPTION_LENGTH  32
#define DCF_CAPTURE_TIME_LENGTH 20

typedef struct hiISP_DCF_CONST_INFO_S {
    HI_U8       au8ImageDescription[DCF_DRSCRIPTION_LENGTH]; /* Describes image */
    HI_U8       au8Make[DCF_DRSCRIPTION_LENGTH]; /* Shows manufacturer of digital cameras */
    HI_U8       au8Model[DCF_DRSCRIPTION_LENGTH]; /* Shows model number of digital cameras */
    /* Shows firmware (internal software of digital cameras)version number */
    HI_U8       au8Software[DCF_DRSCRIPTION_LENGTH];
  /* Light source, actually this means white balance setting. '0' means unknown, '1' daylight, '2'
    fluorescent, '3' tungsten, '10' flash, '17' standard light A, '18' standard light B, '19' standard light
    C, '20' D55, '21' D65, '22' D75, '255' other */
    HI_U8       u8LightSource;
    HI_U32      u32FocalLength; /* Focal length of lens used to take image. Unit is millimeter */
    HI_U8       u8SceneType; /* Indicates the type of scene. Value '0x01'
                                means that the image was directly photographed. */
    HI_U8       u8CustomRendered; /* Indicates the use of special processing on image data,
                                    such as rendering geared to output.0 = Normal process  1 = Custom process   */
    HI_U8       u8FocalLengthIn35mmFilm; /* Indicates the equivalent focal length assuming a 35mm film camera, in mm */
    HI_U8       u8SceneCaptureType; /* Indicates the type of scene that was shot. 0 = Standard,
                                        1 = Landscape,2 = Portrait,3 = Night scene. */
    HI_U8       u8GainControl; /* Indicates the degree of overall image gain adjustment.
                                    0 = None,1 = Low gain up,2 = High gain up,3 = Low gain down,4 = High gain down. */
    HI_U8       u8Contrast; /* Indicates the direction of contrast processing applied
                                by the camera when the image was shot.0 = Normal,1 = Soft,2 = Hard */
    HI_U8       u8Saturation; /* Indicates the direction of saturation processing applied by the camera
                                when the image was shot.0 = Normal,1 = Low saturation,2 = High saturation */
    HI_U8       u8Sharpness; /* Indicates the direction of sharpness processing applied by the camera when the image
                                was shot. 0 = Normal,1 = Soft,2 = Hard . */
    HI_U8       u8MeteringMode; /* Exposure metering method. '0' means unknown, '1' average, '2' center weighted
                                average, '3' spot, '4' multi-spot, '5' multi-segment, '6' partial, '255' other */
} ISP_DCF_CONST_INFO_S;
typedef struct hiISP_DCF_UPDATE_INFO_S {
    HI_U32      u32ISOSpeedRatings; /* CCD sensitivity equivalent to Ag-Hr film speedrate */
    HI_U32      u32ExposureTime; /* Exposure time (reciprocal of shutter speed). */
    HI_U32      u32ExposureBiasValue; /* Exposure bias (compensation) value of taking picture */
    HI_U8       u8ExposureProgram; /* Exposure program that the camera used when image was taken. '1' means manual
                                    control, '2' program normal, '3' aperture priority, '4' shutter priority, '5'
                                    program creative (slow program), '6' program action(high-speed program),
                                    '7' portrait mode, '8' landscape mode */
    HI_U32      u32FNumber; /* The actual F-number (F-stop) of lens when the image was taken */
    HI_U32      u32MaxApertureValue; /* Maximum aperture value of lens. */
    HI_U8       u8ExposureMode; /* Indicates the exposure mode set when the image was shot.
                                    0 = Auto exposure,1 = Manual exposure, 2 = Auto bracket */
    HI_U8       u8WhiteBalance; /* Indicates the white balance mode set when the image was shot.
                                    0 = Auto white balance ,1 = Manual white balance */
} ISP_DCF_UPDATE_INFO_S;
typedef struct hiISP_DCF_INFO_S {
    ISP_DCF_CONST_INFO_S  stIspDCFConstInfo;
    ISP_DCF_UPDATE_INFO_S stIspDCFUpdateInfo;
} ISP_DCF_INFO_S;

typedef struct hiJPEG_DCF_S {
    HI_U8           au8CaptureTime[DCF_CAPTURE_TIME_LENGTH]; /* The date and time when the picture data was generated */
    HI_BOOL         bFlash; /* whether the picture is captured when a flash lamp is on */
    HI_U32          u32DigitalZoomRatio; /* Indicates the digital zoom ratio when the image was shot.If the numerator
                                        of the recorded value is 0, this indicates that digital zoom was not used. */
    ISP_DCF_INFO_S  stIspDCFInfo;
} JPEG_DCF_S;
typedef struct hiISP_FRAME_INFO_S {
    HI_U32      u32ISO;                    /* ISP internal ISO : Again*Dgain*ISPgain */
    HI_U32      u32ExposureTime;           /* Exposure time (reciprocal of shutter speed),unit is us */
    HI_U32      u32IspDgain;
    HI_U32      u32Again;
    HI_U32      u32Dgain;
    HI_U32      au32Ratio[3];
    HI_U32      u32IspNrStrength;
    HI_U32      u32FNumber;                /* The actual F-number (F-stop) of lens when the image was taken */
    HI_U32      u32SensorID;               /* which sensor is used */
    HI_U32      u32SensorMode;
    HI_U32      u32HmaxTimes;              /* Sensor HmaxTimes,unit is ns */
    HI_U32      u32Vmax;                   /* Sensor Vmax,unit is line */
    HI_U32      u32VcNum;                  /* when dump wdr frame, which is long or short  exposure frame. */
} ISP_FRAME_INFO_S;
#define HI_CFACOLORPLANE     (3)
#define DNG_NP_SIZE       (6)
typedef struct hiDNG_RATIONAL_S {
    HI_U32 u32Numerator; /* represents the numerator of a fraction, */
    HI_U32 u32Denominator; /* the denominator. */
} DNG_RATIONAL_S;

typedef struct hiISP_HDR_INFO_S {
    HI_U32 u32ColorTemp;
    HI_U16 au16CCM[9];
    HI_U8  u8Saturation;
} ISP_HDR_INFO_S;

typedef struct hiISP_ATTACH_INFO_S {
    ISP_HDR_INFO_S     stIspHdr;
    HI_U32 u32ISO;
    HI_U32 u32InitISO;
    HI_U8 u8SnsWDRMode;
} ISP_ATTACH_INFO_S;
typedef enum hiFRAME_FLAG_E {
    FRAME_FLAG_SNAP_FLASH  = 0x1 << 0,
    FRAME_FLAG_SNAP_CUR    = 0x1 << 1,
    FRAME_FLAG_SNAP_REF    = 0x1 << 2,
    FRAME_FLAG_SNAP_END    = 0x1 << 3,
    FRAME_FLAG_MIRROR      = 0x1 << 4,
    FRAME_FLAG_FLIP        = 0x1 << 5,
    FRAME_FLAG_DGAIN_BYPASS = 0x1 << 6,
    FRAME_FLAG_BUTT
} FRAME_FLAG_E;

#define FRAME_DEC_SUCCESS 0U
#define FRAME_DEC_FAIL 1U
#define FRAME_DEC_NO_PICTURE 2U
#define FRAME_DEC_NOT_ENOUGH_REF_NUM 3U
#define FRAME_DEC_NOT_ENOUGH_PIC_BUF_SIZE 4U
#define FRAME_DEC_INVALID_PARA 5U
#define FRAME_DEC_NEED_FREE_VB_BUFF 6U

/* RGGB=4 */
#define ISP_WB_GAIN_NUM         4
/* 3*3=9 matrix */
#define ISP_CAP_CCM_NUM         9
typedef struct hiISP_CONFIG_INFO_S {
    HI_U32 u32ISO;
    HI_U32 u32IspDgain;
    HI_U32 u32ExposureTime;
    HI_U32 au32WhiteBalanceGain[ISP_WB_GAIN_NUM];
    HI_U32 u32ColorTemperature;
    HI_U16 au16CapCCM[ISP_CAP_CCM_NUM];
} ISP_CONFIG_INFO_S;


typedef struct hiISP_COLORGAMMUT_INFO_S {
        COLOR_GAMUT_E enColorGamut;
} ISP_COLORGAMMUT_INFO_S;


typedef struct hiVB_CAL_CONFIG_S {
    HI_U32 u32VBSize;

    HI_U32 u32HeadStride;
    HI_U32 u32HeadSize;
    HI_U32 u32HeadYSize;

    HI_U32 u32MainStride;
    HI_U32 u32MainSize;
    HI_U32 u32MainYSize;

    HI_U32 u32ExtStride;
    HI_U32 u32ExtYSize;
} VB_CAL_CONFIG_S;

typedef struct hiBITMAP_S {
    PIXEL_FORMAT_E enPixelFormat;  /* Bitmap's pixel format */
    HI_U32 u32Width;               /* Bitmap's width */
    HI_U32 u32Height;              /* Bitmap's height */
    HI_VOID* ATTRIBUTE pData;      /* Address of Bitmap's data */
} BITMAP_S;

typedef struct hiLDC_ATTR_S {
    HI_BOOL    bAspect;             /* RW;Range: [0, 1];Whether aspect ration  is keep */
    HI_S32     s32XRatio;           /* RW; Range: [0, 100]; field angle ration of  horizontal,valid when bAspect=0. */
    HI_S32     s32YRatio;           /* RW; Range: [0, 100]; field angle ration of  vertical,valid when bAspect=0. */
    HI_S32     s32XYRatio;          /* RW; Range: [0, 100]; field angle ration of  all,valid when bAspect=1. */
    HI_S32 s32CenterXOffset;        /* RW; Range: [-511, 511]; horizontal offset of the image distortion center relative
                                        to image center. */
    HI_S32 s32CenterYOffset;        /* RW; Range: [-511, 511]; vertical offset of the image distortion center relative
                                        to image center. */
    HI_S32 s32DistortionRatio;      /* RW; Range: [-300, 500]; LDC Distortion ratio.When spread on,s32DistortionRatio
                                        range should be [0, 500] */
} LDC_ATTR_S;

typedef enum hiLDC_VIEW_TYPE_E {
    LDC_VIEW_TYPE_ALL  = 0,  /* View scale all but x and y independtly,
                                this will keep both x and y axis ,but corner maybe lost */
    LDC_VIEW_TYPE_CROP = 1,  /* Not use view scale, this will lost some side and corner */

    LDC_VIEW_TYPE_BUTT,
} LDC_VIEW_TYPE_E;

typedef struct hiLDCV3_ATTR_S {
    LDC_VIEW_TYPE_E enViewType; /* RW; Range: [0, 1], 0: all mode, 1: crop mode. */
    HI_S32 s32CenterXOffset;    /* RW; Range: ABS(s32CenterXOffset)->[0, min(128, width*0.08)],
                                    horizontal offset of the image distortion center relative to image center. */
    HI_S32 s32CenterYOffset;    /* RW; Range: ABS(s32CenterXOffset)->[0, min(128, height*0.08)],
                                    vertical offset of the image distortion center relative to image center. */
    HI_S32 s32DistortionRatio;  /* RW; Range: [-300, 300], LDC Distortion ratio. */
    HI_S32 s32MinRatio;         /* RW; Range: [-300, 300], to remove the black region around when performing
                                    pinchusion distortion correction with crop mode. */
} LDCV3_ATTR_S;

typedef enum hiROTATION_VIEW_TYPE_E {
    ROTATION_VIEW_TYPE_ALL      = 0, /* View all source Image,no lose */
    ROTATION_VIEW_TYPE_TYPICAL  = 1, /* View from rotation Image with source size,same lose */
    ROTATION_VIEW_TYPE_INSIDE   = 2, /* View with no black section,all  in dest Image */
    ROTATION_VIEW_TYPE_BUTT,
} ROTATION_VIEW_TYPE_E;

typedef struct hiROTATION_EX_S {
    ROTATION_VIEW_TYPE_E enViewType;       /* RW;Range: [0, 2];Rotation mode */
    HI_U32               u32Angle;         /* RW;Range: [0,360];Rotation Angle:[0,360] */
    HI_S32               s32CenterXOffset; /* RW;Range: [-511, 511];Horizontal offset of the image distortion
                                            center relative to image center. */
    HI_S32               s32CenterYOffset; /* RW;Range: [-511, 511];Vertical offset of the image distortion
                                            center relative to image center. */
    SIZE_S               stDestSize;       /* RW;Dest size of any angle rotation */
} ROTATION_EX_S;
typedef enum hiWDR_MODE_E {
    WDR_MODE_NONE = 0,
    WDR_MODE_BUILT_IN,
    WDR_MODE_QUDRA,

    WDR_MODE_2To1_LINE,
    WDR_MODE_2To1_FRAME,

    WDR_MODE_3To1_LINE,
    WDR_MODE_3To1_FRAME,

    WDR_MODE_4To1_LINE,
    WDR_MODE_4To1_FRAME,

    WDR_MODE_BUTT,
} WDR_MODE_E;

typedef enum hiFRAME_INTERRUPT_TYPE_E {
    FRAME_INTERRUPT_START,
    FRAME_INTERRUPT_EARLY,
    FRAME_INTERRUPT_EARLY_END,
    FRAME_INTERRUPT_EARLY_ONE_BUF,
    FRAME_INTERRUPT_EARLY_END_ONE_BUF,
    FRAME_INTERRUPT_BUTT,
} FRAME_INTERRUPT_TYPE_E;

typedef struct hiFRAME_INTERRUPT_ATTR_S {
    FRAME_INTERRUPT_TYPE_E enIntType;
    HI_U32 u32EarlyLine;
} FRAME_INTERRUPT_ATTR_S;
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* HI_COMM_VIDEO_H__ */
