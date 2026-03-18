/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: multimedia common file
 * Author: Hisilicon multimedia software group
 * Create: 2023/03/19
 */
#ifndef MID_MPI_HDMI_H
#define MID_MPI_HDMI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MID_NULL 0L
#define MID_SUCCESS 0
#define MID_FAILURE (-1)
#define mid_void               void

typedef enum {
    MID_FALSE = 0,
    MID_TRUE = 1,
} mid_bool;

typedef enum {
    MID_HDMI_ID_0,
    MID_HDMI_ID_1,
    MID_HDMI_ID_BUTT
} mid_hdmi_id;

typedef enum {
    MID_HDMI_VIDEO_FORMAT_1080P_60,
    MID_HDMI_VIDEO_FORMAT_1080P_50,
    MID_HDMI_VIDEO_FORMAT_1080P_30,
    MID_HDMI_VIDEO_FORMAT_1080P_25,
    MID_HDMI_VIDEO_FORMAT_1080P_24,
    MID_HDMI_VIDEO_FORMAT_1080i_60,
    MID_HDMI_VIDEO_FORMAT_1080i_50,
    MID_HDMI_VIDEO_FORMAT_720P_60,
    MID_HDMI_VIDEO_FORMAT_720P_50,
    MID_HDMI_VIDEO_FORMAT_576P_50,
    MID_HDMI_VIDEO_FORMAT_480P_60,
    MID_HDMI_VIDEO_FORMAT_PAL,
    MID_HDMI_VIDEO_FORMAT_NTSC,
    MID_HDMI_VIDEO_FORMAT_861D_640X480_60,
    MID_HDMI_VIDEO_FORMAT_VESA_800X600_60,
    MID_HDMI_VIDEO_FORMAT_VESA_1024X768_60,
    MID_HDMI_VIDEO_FORMAT_VESA_1280X800_60,
    MID_HDMI_VIDEO_FORMAT_VESA_1280X1024_60,
    MID_HDMI_VIDEO_FORMAT_VESA_1366X768_60,
    MID_HDMI_VIDEO_FORMAT_VESA_1440X900_60,
    MID_HDMI_VIDEO_FORMAT_VESA_1400X1050_60,
    MID_HDMI_VIDEO_FORMAT_VESA_1600X1200_60,
    MID_HDMI_VIDEO_FORMAT_VESA_1680X1050_60,
    MID_HDMI_VIDEO_FORMAT_VESA_1920X1200_60,
    MID_HDMI_VIDEO_FORMAT_2560x1440_30,
    MID_HDMI_VIDEO_FORMAT_2560x1440_60,
    MID_HDMI_VIDEO_FORMAT_2560x1600_60,
    MID_HDMI_VIDEO_FORMAT_1920x2160_30,
    MID_HDMI_VIDEO_FORMAT_3840X2160P_24,
    MID_HDMI_VIDEO_FORMAT_3840X2160P_25,
    MID_HDMI_VIDEO_FORMAT_3840X2160P_30,
    MID_HDMI_VIDEO_FORMAT_3840X2160P_50,
    MID_HDMI_VIDEO_FORMAT_3840X2160P_60,
    MID_HDMI_VIDEO_FORMAT_4096X2160P_24,
    MID_HDMI_VIDEO_FORMAT_4096X2160P_25,
    MID_HDMI_VIDEO_FORMAT_4096X2160P_30,
    MID_HDMI_VIDEO_FORMAT_4096X2160P_50,
    MID_HDMI_VIDEO_FORMAT_4096X2160P_60,
    MID_HDMI_VIDEO_FORMAT_3840X2160P_120,
    MID_HDMI_VIDEO_FORMAT_4096X2160P_120,
    MID_HDMI_VIDEO_FORMAT_7680X4320P_30,
    MID_HDMI_VIDEO_FORMAT_VESA_CUSTOMER_DEFINE,
    MID_HDMI_VIDEO_FORMAT_BUTT
} mid_hdmi_video_format;

typedef enum {
    MID_HDMI_DEEP_COLOR_24BIT, /* HDMI Deep Color 24bit mode */
    MID_HDMI_DEEP_COLOR_30BIT, /* HDMI Deep Color 30bit mode */
    MID_HDMI_DEEP_COLOR_36BIT, /* HDMI Deep Color 36bit mode */
    MID_HDMI_DEEP_COLOR_BUTT
} mid_hdmi_deep_color;

typedef enum {
    MID_HDMI_SAMPLE_RATE_UNKNOWN, /* unknown sample rate */
    MID_HDMI_SAMPLE_RATE_8K,      /* 8K sample rate */
    MID_HDMI_SAMPLE_RATE_11K,     /* 11.025K sample rate */
    MID_HDMI_SAMPLE_RATE_12K,     /* 12K sample rate */
    MID_HDMI_SAMPLE_RATE_16K,     /* 16K sample rate */
    MID_HDMI_SAMPLE_RATE_22K,     /* 22.050K sample rate */
    MID_HDMI_SAMPLE_RATE_24K,     /* 24K sample rate */
    MID_HDMI_SAMPLE_RATE_32K,     /* 32K sample rate */
    MID_HDMI_SAMPLE_RATE_44K,     /* 44.1K sample rate */
    MID_HDMI_SAMPLE_RATE_48K,     /* 48K sample rate */
    MID_HDMI_SAMPLE_RATE_88K,     /* 88.2K sample rate */
    MID_HDMI_SAMPLE_RATE_96K,     /* 96K sample rate */
    MID_HDMI_SAMPLE_RATE_176K,    /* 176K sample rate */
    MID_HDMI_SAMPLE_RATE_192K,    /* 192K sample rate */
    MID_HDMI_SAMPLE_RATE_768K,    /* 768K sample rate */
    MID_HDMI_SAMPLE_RATE_BUTT
} mid_hdmi_sample_rate;

typedef enum {
    MID_HDMI_BIT_DEPTH_UNKNOWN, /* unknown bit width */
    MID_HDMI_BIT_DEPTH_8,       /* 8 bits width */
    MID_HDMI_BIT_DEPTH_16,      /* 16 bits width */
    MID_HDMI_BIT_DEPTH_18,      /* 18 bits width */
    MID_HDMI_BIT_DEPTH_20,      /* 20 bits width */
    MID_HDMI_BIT_DEPTH_24,      /* 24 bits width */
    MID_HDMI_BIT_DEPTH_32,      /* 32 bits width */
    MID_HDMI_BIT_DEPTH_BUTT
} mid_hdmi_bit_depth;

typedef struct {
    /* Whether to forcibly output the video over the HDMI. */
    mid_bool hdmi_en;
    /* Video norm. This value of the video norm must be consistent with the norm of the video output. */
    mid_hdmi_video_format video_format;
    /* DeepColor output mode.It is OT_HDMI_DEEP_COLOR_24BIT by default. */
    mid_hdmi_deep_color deep_color_mode;
    /* Whether to enable the audio. */
    mid_bool audio_en;
    /* Audio sampling rate. This parameter needs to be consistent with that of the VO. */
    mid_hdmi_sample_rate sample_rate;
    /* Audio bit width. It is 16 by default. This parameter needs to be consistent with that of the VO. */
    mid_hdmi_bit_depth bit_depth;
    /* Whether to enable auth mode. 0: disabled 1: enabled */
    mid_bool auth_mode_en;
    /*
     * Enable flag of deep color mode adapting case of user setting incorrect,
     * default: TD_FALSE.When user have no any adapting strategy,suggestion TD_TRUE
     */
    mid_bool deep_color_adapt_en;
    /*
     * Pixclk of enVideoFmt(unit is kHz).
     * (This param is valid only when enVideoFmt is OT_HDMI_VIDEO_FMT_VESA_CUSTOMER_DEFINE)
     */
    unsigned int pix_clk;
} mid_hdmi_attr;

typedef enum {
    MID_INFOFRAME_TYPE_AVI,
    MID_INFOFRAME_TYPE_AUDIO,
    MID_INFOFRAME_TYPE_VENDORSPEC,
    MID_INFOFRAME_TYPE_BUTT
} mid_hdmi_infoframe_type;

typedef enum {
    MID_HDMI_COLOR_SPACE_RGB444,
    MID_HDMI_COLOR_SPACE_YCBCR422,
    MID_HDMI_COLOR_SPACE_YCBCR444,
    /* following is new featrue of CEA-861-F */
    MID_HDMI_COLOR_SPACE_YCBCR420,
    MID_HDMI_COLOR_SPACE_BUTT
} mid_hdmi_color_space;

typedef enum {
    MID_HDMI_BAR_INFO_NOT_VALID, /* Bar Data not valid */
    MID_HDMI_BAR_INFO_V,         /* Vertical bar data valid */
    MID_HDMI_BAR_INFO_H,         /* Horizontal bar data valid */
    MID_HDMI_BAR_INFO_VH,        /* Horizontal and Vertical bar data valid */
    MID_HDMI_BAR_INFO_BUTT
} mid_hdmi_bar_info;

typedef enum {
    MID_HDMI_SCAN_INFO_NO_DATA,      /* No Scan information */
    MID_HDMI_SCAN_INFO_OVERSCANNED,  /* Scan information, Overscanned (for television) */
    MID_HDMI_SCAN_INFO_UNDERSCANNED, /* Scan information, Underscanned (for computer) */
    MID_HDMI_SCAN_INFO_BUTT
} mid_hdmi_scan_info;

typedef enum {
    MID_HDMI_COMMON_COLORIMETRY_NO_DATA, /* Colorimetry No Data option */
    MID_HDMI_COMMON_COLORIMETRY_ITU601,  /* Colorimetry ITU601 option */
    MID_HDMI_COMMON_COLORIMETRY_ITU709,  /* Colorimetry ITU709 option */
    MID_HDMI_COMMON_COLORIMETRY_BUTT     /* Colorimetry extended option */
} mid_hdmi_colorimetry;

typedef enum {
    MID_HDMI_COMMON_COLORIMETRY_XVYCC_601,               /* Colorimetry xvYCC601 extended option */
    MID_HDMI_COMMON_COLORIMETRY_XVYCC_709,               /* Colorimetry xvYCC709 extended option */
    MID_HDMI_COMMON_COLORIMETRY_S_YCC_601,               /* Colorimetry S YCC 601 extended option */
    MID_HDMI_COMMON_COLORIMETRY_ADOBE_YCC_601,           /* Colorimetry ADOBE YCC 601 extended option */
    MID_HDMI_COMMON_COLORIMETRY_ADOBE_RGB,               /* Colorimetry ADOBE RGB extended option */
    MID_HDMI_COMMON_COLORIMETRY_2020_CONST_LUMINOUS,     /* Colorimetry ITU2020 extended option */
    MID_HDMI_COMMON_COLORIMETRY_2020_NON_CONST_LUMINOUS, /* Colorimetry ITU2020 extended option */
    MID_HDMI_COMMON_COLORIMETRY_EXT_BUTT
} mid_hdmi_ex_colorimetry;

typedef enum {
    MID_HDMI_PIC_ASPECT_RATIO_NO_DATA,
    MID_HDMI_PIC_ASPECT_RATIO_4TO3,
    MID_HDMI_PIC_ASPECT_RATIO_16TO9,
    MID_HDMI_PIC_ASPECT_RATIO_64TO27,
    MID_HDMI_PIC_ASPECT_RATIO_256TO135,
    MID_HDMI_PIC_ASPECT_RATIO_BUTT
} mid_pic_aspect_ratio;

typedef enum {
    MID_HDMI_ACTIVE_ASPECT_RATIO_16TO9_TOP = 2,
    MID_HDMI_ACTIVE_ASPECT_RATIO_14TO9_TOP,
    MID_HDMI_ACTIVE_ASPECT_RATIO_16TO9_BOX_CENTER,
    MID_HDMI_ACTIVE_ASPECT_RATIO_SAME_PIC = 8,
    MID_HDMI_ACTIVE_ASPECT_RATIO_4TO3_CENTER,
    MID_HDMI_ACTIVE_ASPECT_RATIO_16TO9_CENTER,
    MID_HDMI_ACTIVE_ASPECT_RATIO_14TO9_CENTER,
    MID_HDMI_ACTIVE_ASPECT_RATIO_4TO3_14_9 = 13,
    MID_HDMI_ACTIVE_ASPECT_RATIO_16TO9_14_9,
    MID_HDMI_ACTIVE_ASPECT_RATIO_16TO9_4_3,
    MID_HDMI_ACTIVE_ASPECT_RATIO_BUTT
} mid_hdmi_active_aspect_ratio;

typedef enum {
    MID_HDMI_PIC_NON_UNIFORM_SCALING, /* No Known, non-uniform picture scaling */
    MID_HDMI_PIC_SCALING_H,           /* Picture has been scaled horizontally */
    MID_HDMI_PIC_SCALING_V,           /* Picture has been scaled Vertically */
    MID_HDMI_PIC_SCALING_HV,          /* Picture has been scaled horizontally and Vertically */
    MID_HDMI_PIC_SCALING_BUTT
} mid_hdmi_pic_scaline;

typedef enum {
    MID_HDMI_RGB_QUANT_DEFAULT_RANGE, /* Default range, it depends on the video format */
    MID_HDMI_RGB_QUANT_LIMITED_RANGE, /* Limited quantization range of 220 levels when receiving a CE video format */
    MID_HDMI_RGB_QUANT_FULL_RANGE,    /* Full quantization range of 256 levels when receiving an IT video format */
    MID_HDMI_RGB_QUANT_FULL_BUTT
} mid_hdmi_rgb_quant_range;

typedef enum {
    MID_HDMI_PIXEL_REPET_NO,
    MID_HDMI_PIXEL_REPET_2_TIMES,
    MID_HDMI_PIXEL_REPET_3_TIMES,
    MID_HDMI_PIXEL_REPET_4_TIMES,
    MID_HDMI_PIXEL_REPET_5_TIMES,
    MID_HDMI_PIXEL_REPET_6_TIMES,
    MID_HDMI_PIXEL_REPET_7_TIMES,
    MID_HDMI_PIXEL_REPET_8_TIMES,
    MID_HDMI_PIXEL_REPET_9_TIMES,
    MID_HDMI_PIXEL_REPET_10_TIMES,
    MID_HDMI_PIXEL_REPET_BUTT
} mid_hdmi_pixel_repetition;

typedef enum {
    MID_HDMI_CONTNET_GRAPHIC,
    MID_HDMI_CONTNET_PHOTO,
    MID_HDMI_CONTNET_CINEMA,
    MID_HDMI_CONTNET_GAME,
    MID_HDMI_CONTNET_BUTT
} mid_hdmi_content_type;

typedef enum {
    MID_HDMI_YCC_QUANT_LIMITED_RANGE, /* Limited quantization range of 220 levels when receiving a CE video format */
    MID_HDMI_YCC_QUANT_FULL_RANGE,    /* Full quantization range of 256 levels when receiving an IT video format */
    MID_HDMI_YCC_QUANT_BUTT
} mid_hdmi_ycc_quant_range;

typedef struct {
    mid_hdmi_video_format timing_mode;
    mid_hdmi_color_space color_space;
    mid_bool active_info_present;
    mid_hdmi_bar_info bar_info;
    mid_hdmi_scan_info scan_info;
    mid_hdmi_colorimetry colorimetry;
    mid_hdmi_ex_colorimetry ex_colorimetry;
    mid_pic_aspect_ratio aspect_ratio;
    mid_hdmi_active_aspect_ratio active_aspect_ratio;
    mid_hdmi_pic_scaline pic_scaling;
    mid_hdmi_rgb_quant_range rgb_quant;
    mid_bool is_it_content;
    mid_hdmi_pixel_repetition pixel_repetition;
    mid_hdmi_content_type content_type;
    mid_hdmi_ycc_quant_range ycc_quant;
    unsigned short line_n_end_of_top_bar;
    unsigned short line_n_start_of_bot_bar;
    unsigned short pixel_n_end_of_left_bar;
    unsigned short pixel_n_start_of_right_bar;
} mid_hdmi_avi_infoframe;

typedef enum {
    MID_HDMI_AUDIO_CHN_CNT_STREAM,
    MID_HDMI_AUDIO_CHN_CNT_2,
    MID_HDMI_AUDIO_CHN_CNT_3,
    MID_HDMI_AUDIO_CHN_CNT_4,
    MID_HDMI_AUDIO_CHN_CNT_5,
    MID_HDMI_AUDIO_CHN_CNT_6,
    MID_HDMI_AUDIO_CHN_CNT_7,
    MID_HDMI_AUDIO_CHN_CNT_8,
    MID_HDMI_AUDIO_CHN_CNT_BUTT
} mid_hdmi_audio_chn_cnt;

typedef enum {
    MID_HDMI_AUDIO_CODING_REFER_STREAM_HEAD,
    MID_HDMI_AUDIO_CODING_PCM,
    MID_HDMI_AUDIO_CODING_AC3,
    MID_HDMI_AUDIO_CODING_MPEG1,
    MID_HDMI_AUDIO_CODING_MP3,
    MID_HDMI_AUDIO_CODING_MPEG2,
    MID_HDMI_AUDIO_CODING_AACLC,
    MID_HDMI_AUDIO_CODING_DTS,
    MID_HDMI_AUDIO_CODING_ATRAC,
    MID_HDMI_AUDIO_CODIND_ONE_BIT_AUDIO,
    MID_HDMI_AUDIO_CODING_ENAHNCED_AC3,
    MID_HDMI_AUDIO_CODING_DTS_HD,
    MID_HDMI_AUDIO_CODING_MAT,
    MID_HDMI_AUDIO_CODING_DST,
    MID_HDMI_AUDIO_CODING_WMA_PRO,
    MID_HDMI_AUDIO_CODING_BUTT
} mid_hdmi_coding_type;

typedef enum {
    MID_HDMI_AUDIO_SAMPLE_SIZE_STREAM,
    MID_HDMI_AUDIO_SAMPLE_SIZE_16,
    MID_HDMI_AUDIO_SAMPLE_SIZE_20,
    MID_HDMI_AUDIO_SAMPLE_SIZE_24,
    MID_HDMI_AUDIO_SAMPLE_SIZE_BUTT
} mid_hdmi_audio_sample_size;

typedef enum {
    MID_HDMI_AUDIO_SAMPLE_FREQ_STREAM,
    MID_HDMI_AUDIO_SAMPLE_FREQ_32000,
    MID_HDMI_AUDIO_SAMPLE_FREQ_44100,
    MID_HDMI_AUDIO_SAMPLE_FREQ_48000,
    MID_HDMI_AUDIO_SAMPLE_FREQ_88200,
    MID_HDMI_AUDIO_SAMPLE_FREQ_96000,
    MID_HDMI_AUDIO_SAMPLE_FREQ_176400,
    MID_HDMI_AUDIO_SAMPLE_FREQ_192000,
    MID_HDMI_AUDIO_SAMPLE_FREQ_BUTT
} mid_hdmi_audio_sample_freq;

typedef enum {
    MID_HDMI_LEVEL_SHIFT_VAL_0_DB,
    MID_HDMI_LEVEL_SHIFT_VAL_1_DB,
    MID_HDMI_LEVEL_SHIFT_VAL_2_DB,
    MID_HDMI_LEVEL_SHIFT_VAL_3_DB,
    MID_HDMI_LEVEL_SHIFT_VAL_4_DB,
    MID_HDMI_LEVEL_SHIFT_VAL_5_DB,
    MID_HDMI_LEVEL_SHIFT_VAL_6_DB,
    MID_HDMI_LEVEL_SHIFT_VAL_7_DB,
    MID_HDMI_LEVEL_SHIFT_VAL_8_DB,
    MID_HDMI_LEVEL_SHIFT_VAL_9_DB,
    MID_HDMI_LEVEL_SHIFT_VAL_10_DB,
    MID_HDMI_LEVEL_SHIFT_VAL_11_DB,
    MID_HDMI_LEVEL_SHIFT_VAL_12_DB,
    MID_HDMI_LEVEL_SHIFT_VAL_13_DB,
    MID_HDMI_LEVEL_SHIFT_VAL_14_DB,
    MID_HDMI_LEVEL_SHIFT_VAL_15_DB,
    MID_HDMI_LEVEL_SHIFT_VAL_BUTT
} mid_hdmi_level_shift_val;

typedef enum {
    MID_HDMI_LFE_PLAYBACK_NO,
    MID_HDMI_LFE_PLAYBACK_0_DB,
    MID_HDMI_LFE_PLAYBACK_10_DB,
    MID_HDMI_LFE_PLAYBACK_BUTT
} mid_hdmi_lfe_playback_level;


typedef struct {
    mid_hdmi_audio_chn_cnt chn_cnt;
    mid_hdmi_coding_type coding_type;
    mid_hdmi_audio_sample_size sample_size;
    mid_hdmi_audio_sample_freq sampling_freq;
    unsigned char chn_alloc; /* Channel/Speaker Allocation.Range [0,255] */
    mid_hdmi_level_shift_val level_shift;
    mid_hdmi_lfe_playback_level lfe_playback_level;
    mid_bool down_mix_inhibit;
} mid_hdmi_audio_infoframe;

#define OT_HDMI_VENDOR_USER_DATA_MAX_LEN 22
typedef struct {
    unsigned char data_len;
    unsigned char user_data[OT_HDMI_VENDOR_USER_DATA_MAX_LEN];
} mid_hdmi_vendorspec_infoframe;

typedef union {
    mid_hdmi_avi_infoframe avi_infoframe;                /* AUTO:ot_hdmi_infoframe_type:OT_INFOFRAME_TYPE_AVI; */
    mid_hdmi_audio_infoframe audio_infoframe;            /* AUTO:ot_hdmi_infoframe_type:OT_INFOFRAME_TYPE_AUDIO; */
    mid_hdmi_vendorspec_infoframe vendor_spec_infoframe; /* AUTO:ot_hdmi_infoframe_type:OT_INFOFRAME_TYPE_VENDORSPEC; */
} mid_hdmi_infoframe_unit;

typedef struct {
    mid_hdmi_infoframe_type infoframe_type; /* InfoFrame type */
    mid_hdmi_infoframe_unit infoframe_unit; /* InfoFrame date */
} mid_hdmi_infoframe;

typedef struct {
    mid_bool is_connected; /* Whether the devices are connected. */
    /* Whether the HDMI is supported by the device. If the HDMI is not supported by the device, the device is DVI. */
    mid_bool support_hdmi;
    /* Whether to support HDMI2.0. */
    mid_bool support_hdmi_2_0;
} mid_hdmi_sink_capability;

typedef enum {
    MID_HDMI_EVENT_HOTPLUG = 0x10, /* HDMI hot-plug event */
    MID_HDMI_EVENT_NO_PLUG,        /* HDMI cable disconnection event */
    MID_HDMI_EVENT_EDID_FAIL,      /* HDMI EDID read failure event */
    MID_HDMI_EVENT_BUTT
} mid_hdmi_event_type;

typedef void (*mid_hdmi_callback)(mid_hdmi_event_type event, void *private_data);

typedef struct {
    mid_hdmi_callback hdmi_event_callback; /* Event handling callback function */
    void *private_data;                /* Private data of the callback functions and parameters */
} mid_hdmi_callback_func;

int mid_mpi_hdmi_init(mid_void);
int mid_mpi_hdmi_deinit(mid_void);
int mid_mpi_hdmi_open(mid_hdmi_id hdmi);
int mid_mpi_hdmi_close(mid_hdmi_id hdmi);
int mid_mpi_hdmi_set_attr(mid_hdmi_id hdmi, const mid_hdmi_attr *attr);
int mid_mpi_hdmi_get_attr(mid_hdmi_id hdmi, mid_hdmi_attr *attr);
int mid_mpi_hdmi_start(mid_hdmi_id hdmi);
int mid_mpi_hdmi_stop(mid_hdmi_id hdmi);
int mid_mpi_hdmi_set_infoframe(mid_hdmi_id hdmi, const mid_hdmi_infoframe *infoframe);
int mid_mpi_hdmi_get_infoframe(mid_hdmi_id hdmi, mid_hdmi_infoframe_type infoframe_type,
                               const mid_hdmi_infoframe *infoframe);

int mid_mpi_hdmi_get_sink_capability(mid_hdmi_id hdmi, mid_hdmi_sink_capability *capability);

int mid_mpi_hdmi_register_callback(mid_hdmi_id hdmi, const mid_hdmi_callback_func *callback_func);

#ifdef __cplusplus
}
#endif
#endif /* __MID_MPI_HDMI_H__ */
