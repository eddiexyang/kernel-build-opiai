/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2015-2020. All rights reserved.
 * Description: audio_inner.h
 * Author: Hisilicon multimedia software group
 * Create: 2015/01/19
 */

#ifndef AUDIO_INNER_H
#define AUDIO_INNER_H

#ifdef __LITEOS__
#ifdef __KERNEL__
#undef __KERNEL__
#endif
#endif

#ifndef __KERNEL__
#include <pthread.h>
#endif

#include "ot_inner_common_aio.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

#define AUDIO_DVQE_CHAN_NUM           2        // 算法支持的最大声道数
#define AUDIO_DVQE_BAND_NUM           5        // 算法支持的最大子带数
#define AUDIO_DVQE_SEG_SAMPLE_NUM     81       // 拟合静态特征曲线的线段数
#define AUDIO_DVQE_GAIN_TABLE_SIZE    50       // 拟合limiter2.0曲线点数
#define AUDIO_DVQE_SEG_NUM_MAX 6

#define VQE_PEQ_BAND_NUM 10
#define AI_HIFIVQE_MASK_HPF        0x1
#define AI_HIFIVQE_MASK_RNR        0x2
#define AI_HIFIVQE_MASK_HDR        0x4
#define AI_HIFIVQE_MASK_DRC        0x8
#define AI_HIFIVQE_MASK_PEQ        0x10


/* Max input sample number */
#define AUDIO_DVQE_MAXFRAMESIZE 2048

/* Min input sample number */
#define AUDIO_DVQE_MINFRAMESIZE 80

/* defines the configure parameters of PEQ */
typedef struct {
    td_bool usr_mode;   /* enable user mode or not, sdefault 0: disable user mode,1: user mode. */
    td_u32 band_num;                          /* Band number, range(0, 10] */
    td_u8  filter_type[VQE_PEQ_BAND_NUM]; /* the filter type, range: [0: HP, 1: LS, 2: PK, 3: HS 4: LP] */
    td_s8  gain_db[VQE_PEQ_BAND_NUM];    /* PEQ band gain adjustment, the gain of HP/LP filter are 0dB,
                                            the gain range of other filter are [-15, 15]dB */
    td_u16 frequency[VQE_PEQ_BAND_NUM]; /* center frequency(Hz), range: HP and LS[20, 4000],
                                           PK[20, 22000],HS and LP[4000, 22000] */
    td_u16 q[VQE_PEQ_BAND_NUM];          /* Q value, range: HS and LS[7, 10], PK[5, 100], HP/LP is 7 */
} ai_peq_config;

/* TalkVQEV2的AEC模块结构体，用于通话算法扩展 */
typedef struct {
    td_bool usr_mode;           /* mode 0: auto, mode 1: manual. */

    td_s8 post_filter1_low;     /* post filter 1 low frequency wwitch, default:1, range:[0, 1] */
    td_s8 post_filter1_high;    /* post filter 1 high frequency wwitch, default:1, range:[0, 1] */
    td_s8 post_filter2_low;     /* post filter 2 low frequency wwitch, default:1, range:[0, 1] */
    td_s8 post_filter2_high;    /* post filter 2 high frequency wwitch, default:1, range:[0, 1] */

    td_s8 abf_pf_max1_low;      /* maximum low frequency for post filtered 1, default:50, range:[0, 100] */
    td_s8 abf_pf_max2_low;      /* maximum low frequency for post filtered 2, default:50, range:[0, 100] */
    td_s8 abf_pf_max1_high;     /* maximum high frequency for post filtered 1, default:50, range:[0, 100] */
    td_s8 abf_pf_max2_high;     /* maximum high frequency for post filtered 1, default:50, range:[0, 100] */

    td_u16 p_min1;              /* linear filter 1 minimum, default:100, range:[0, 10000] */
    td_u16 p_min2;              /* linear filter 2 minimum, default:100, range:[0, 10000] */
    td_u16 dtp_min1;            /* linear filter 1 high frequency minimum, default:100, range:[0, 10000] */
    td_u16 dtp_min2;            /* linear filter 2 high frequency minimum, default:100, range:[0, 10000] */

    td_u16 switch_nlp;          /* non-linear filter switch, default:1, range:[0, 1] */

    td_u16 gain_lower_limit1;   /* minimum subband control gain for 0-100Hz, default:0, range:[0, 100] */
    td_u16 gain_lower_limit2;   /* minimum subband control gain for 100-800Hz, default:15, range:[0, 100] */
    td_u16 gain_lower_limit3;   /* minimum subband control gain for 800-3000Hz, default:30, range:[0, 100] */
    td_u16 gain_lower_limit4;   /* minimum subband control gain for 3000-4000Hz, default:30, range:[0, 100] */
    td_u16 gain_lower_limit5;   /* minimum subband control gain for 4000-8000Hz, default:30, range:[0, 100] */

    td_u16 reserved[5];         /* 5: reserved */
} ai_talk_v2_aec_cfg;

// 内存配置结构体, 12B
typedef struct {
    td_s32 sample_rate;     /* 数据采样率:48000, 16000, 8000 */
    td_s32 channels;        /* 声道数: 0, 1 */
    td_s32 frame_len;       /* 帧长: ms */
    td_s32 band_num;        /* 子带数: 1 - 5 */
}ao_vqe_mbc_stru_mem_config;

/** Defines the configure parameters of HIFI VQE. */
typedef struct {
    td_u32            open_mask;

    td_s32               work_sample_rate;  /* sample rate:48kHz */
    td_s32               frame_sample;      /* VQE frame length:80-4096 */
    ot_vqe_work_state    work_state;

    ot_audio_hpf_cfg  hpf_cfg;
    ot_ai_rnr_cfg     rnr_cfg;
    ot_ai_hdr_cfg     hdr_cfg;
    ot_ai_drc_cfg     drc_cfg;
    ai_peq_config        peq_cfg;
} ai_hifivqe_config;

//  抗削波模块结构体, 12B
typedef struct {
    td_s32 peak;
    td_s32 hold;
    td_s16 gain_old;
    td_s16 timer;
} ao_vqe_mbc_stru_limiter;

// 子带参数配置, 104B
typedef struct {
    // 滤波器系数
    td_s32 low_filter_b[2];               /* 2:低通滤波器b系数 */
    td_s32 high_filter_b[2];              /* 2:高通滤波器b系数 */
    td_s32 filter_a[2];                   /* 2:高低通滤波器a系数 */

    // 静态特征曲线
    td_s16 line_x[AUDIO_DVQE_SEG_NUM_MAX];  /* 线段起点 */
    td_s16 line_k[AUDIO_DVQE_SEG_NUM_MAX];  /* 线段斜率, Q8 */
    td_s16 line_b[AUDIO_DVQE_SEG_NUM_MAX];  /* 线段截距 */
    td_s16 in_min;                          /* 输入下限 */

    td_s16 align;                           /* 对齐 */

    // 动态特征参数
    td_s32 attack_rate[AUDIO_DVQE_CHAN_NUM];   /* Q26，用采样点数计算，范围(0,1] */
    td_s32 release_rate[AUDIO_DVQE_CHAN_NUM];  /* Q26，用采样点数计算，范围(0,1] */
    td_s32 hold_on_time[AUDIO_DVQE_CHAN_NUM];  /* 用采样点表示，大于0 */

    // 控制参数
    td_s32 top_freq;                      /* 子带上界频率, 范围[100,21000]Hz */
    td_s32 rms_size;                      /* RMS的采样点数，各采样率1ms, 2ms, 5ms, 10ms对应的样点数。 */
    td_s32 mute_flag;                     /* 子带静音标志,0或1 */
    td_s32 band_pass_flag;                /* 子带增益不调整标志,0或1 */
} ao_vqe_mbc_stru_band_config;

// 配置参数结构体定义, 976B
typedef struct {
    // 子带参数
    ao_vqe_mbc_stru_band_config str_bands[AUDIO_DVQE_BAND_NUM];
    // 通道变量大小相关参数
    ao_vqe_mbc_stru_mem_config  str_chan_size_cfg;

    // 小信号阈值
    td_s32                      rms_threshold;

    // MBDRC 2.0新增
    // 增加limiter
    ao_vqe_mbc_stru_limiter str_limiter[AUDIO_DVQE_CHAN_NUM];
    td_s32                  gain_map_tbl_in[AUDIO_DVQE_GAIN_TABLE_SIZE];  /* 幅值 */
    td_s16                  gain_map_tbl_out[AUDIO_DVQE_GAIN_TABLE_SIZE]; /* 增益,Q14 */
    td_s32                  max_amp;                               /* 压缩后的最大幅值 */
    td_s32                  limiter_flag;                          /* limiter开关 */

    // 长短时保护开关及阈值
    td_s32                  long_flag[AUDIO_DVQE_BAND_NUM + 1];
    td_s32                  instant_flag[AUDIO_DVQE_BAND_NUM + 1];
    td_s32                  long_threshold[AUDIO_DVQE_BAND_NUM + 1];
    td_s32                  instant_threshold[AUDIO_DVQE_BAND_NUM + 1];
    td_s32                  long_smooth_len;                        /* 长时跟踪帧数 */

    td_u8                   reserve[4]; /* 4:保留变量 */
} ao_vqe_mbc_stru_params;

/* Defines the configure parameters of MBC. */
typedef struct {
    ao_vqe_mbc_stru_params mbc_param;   /* MBC配置参数 */

    td_s32                 channels;    /* 声道数，其中0为mono，1为stereo */
    td_s32                 band_num;    /* 子带数，取值范围为1-5 */
}ao_vqe_mbc_config;

/* Defines the configure parameters of MBC. */
typedef struct {
    td_s32            mbc_enable;         /* MBC功能使能值，其中0为关闭，1为打开 */

    td_s32            work_sample_rate;   /* 采样频率：范围为 8000/16000 */
    td_s32            frame_sample;       /* DVQE帧长：使用可变帧长时，该值为0-4096，不使用可变帧长时，该值为10ms采样点的整数倍 */

    ot_vqe_work_state     work_state; /* UpVQE Work state, 根据用户场景配置，默认场景配置为UPVQE_WORKSTATE_COMMON. */

    ao_vqe_mbc_config mbc_cfg;
} ao_mbc_config;

/* Defines the configure parameters of UPVQE. */
typedef struct {
    td_s32            hpf_open;
    td_s32            aec_open;
    td_s32            anr_open; /* Anr and Rnr can't enable at the same time,anr used in voice noise reducing */
    td_s32            rnr_open; /* Rnr only used in Action Camera ,in other situation, please set rnr_enable = 0 */
    td_s32            agc_open;
    td_s32            eq_open;
    td_s32            hdr_open;
    td_s32            drc_open;
    td_s32            peq_open;

    td_s32            in_sample_rate;   /* 采样频率：范围为 8000/16000/48000。默认值为8000 */
    td_s32            work_sample_rate; /* 采样频率：范围为 8000/16000/48000。默认值为8000 */
    td_s32            out_sample_rate;  /* 采样频率：范围为 8000/11025/16000/22500。默认值为8000 */

    td_s32            frame_sample;     /* UPVQE帧长：80-4096 */

    td_s32            in_chn_num;
    td_s32            out_chn_num;
    ot_vqe_record_type   record_type;

    ot_vqe_work_state     work_state; /* UpVQE Work state, 根据用户场景配置，默认场景配置为UPVQE_WORKSTATE_COMMON. */

    ot_audio_hpf_cfg  hpf_cfg;
    ot_ai_aec_cfg     aec_cfg;
    ot_audio_anr_cfg  anr_cfg;
    ot_ai_rnr_cfg     rnr_cfg;
    ot_audio_agc_cfg  agc_cfg;
    ot_audio_eq_cfg   eq_cfg;
    ot_ai_hdr_cfg     hdr_cfg;
    ot_ai_drc_cfg     drc_cfg;
    ai_peq_config     peq_cfg;

    /* talk v2 vqe */
    td_s32 talkv2_pnr_open;
    td_s32 talkv2_nr_open;
    td_s32 talkv2_agc_open;
    td_s32 talkv2_eq_open;
    td_s32 talkv2_drc_open;
    td_s32 talkv2_limiter_open;
    td_s32 talkv2_dereverb_open;
    td_s32 talkv2_fmp_open;
    td_s32 talkv2_aec_open;

    ot_ai_talk_v2_pnr_cfg   talkv2_pnr_cfg;
    ot_ai_talk_v2_nr_cfg    talkv2_nr_cfg;
    ot_ai_talk_v2_agc_cfg   talkv2_agc_cfg;
    ot_ai_talk_v2_eq_cfg    talkv2_eq_cfg;
    ai_talk_v2_aec_cfg      talkv2_aec_cfg;

    td_u8 talkv2_reserved[8];   /* 8: reserved */

    td_s32 wnr_open;
    ot_ai_talk_v2_wnr_cfg wnr_cfg;
} ai_upvqe_config;

/* Defines the configure parameters of DNVQE. */
typedef struct {
    td_s32            hpf_open;
    td_s32            anr_open; /* anr and rnr can't enable at the same time,anr used in voice noise reducing */
    td_s32            agc_open;
    td_s32            eq_open;
    td_s32            mbc_open;

    td_s32            in_sample_rate;   /* 采样频率：范围为 8000/16000/48000。默认值为8000 */
    td_s32            work_sample_rate; /* 采样频率：范围为 8000/16000/48000。默认值为8000 */
    td_s32            out_sample_rate;  /* 采样频率：范围为 8000/11025/16000/22500。默认值为8000 */

    td_s32            frame_sample;     /* UPVQE帧长：80-4096 */

    ot_vqe_work_state     work_state; /* UpVQE Work state, 根据用户场景配置，默认场景配置为UPVQE_WORKSTATE_COMMON. */

    ot_audio_hpf_cfg  hpf_cfg;
    ot_audio_anr_cfg  anr_cfg;
    ot_audio_agc_cfg  agc_cfg;
    ot_audio_eq_cfg   eq_cfg;
    ao_vqe_mbc_config mbc_cfg;
} ao_dnvqe_config;

typedef struct {
    td_void *vqe;
    td_bool  vqe_enable;
    td_bool  resmp_enable;
    td_u32   vb_pool;
#ifndef __KERNEL__
    pthread_mutex_t         lock;
#endif
} audio_vqe_state;

/* obsolete */
typedef enum {
    AUDIO_CLKDIR_RISE = 0,
    AUDIO_CLKDIR_FALL = 1,

    AUDIO_CLKDIR_BUTT
} AUDIO_CLKDIR_E;

typedef AUDIO_CLKDIR_E audio_clkdir;

/* obsolete */
typedef struct {
    ot_audio_frame frm;                 /* audio frame */
    ot_aec_frame   ref_frm;             /* AEC reference audio frame */
    td_bool        enable_vqe;          /* whether is enable vqe */
} audio_frame_combine;

/* obsolete */
typedef struct {
    td_u32                   in_point_num;       /* input point number of frame */
    ot_audio_sample_rate     in_sample_rate;     /* input sample rate */
    ot_audio_sample_rate     out_sample_rate;    /* output sample rate */
} audio_resample_attr;

/* obsolete */
typedef struct {
    td_bool               resmp_enable;      /* resample enable or disable */
    audio_resample_attr   resmp_attr;
} aio_resmp_info;

/* obsolete */
typedef enum {
    AUDIO_AEC_MODE_CLOSE = 0,
    AUDIO_AEC_MODE_OPEN = 1,

    AUDIO_AEC_MODE_BUTT
} audio_aec_mode;

/* obsolete */
typedef struct {
    td_bool             vqe_enable;      /* vqe enable or disable */
    ai_upvqe_config     ai_vqe_cfg;
} ai_vqe_info;

/* obsolete */
typedef struct {
    td_bool             vqe_enable;      /* vqe enable or disable */
    ao_dnvqe_config     ao_vqe_cfg;
} ao_vqe_info;

td_s32 mpi_ai_get_pool_addr(td_u32 pool_id, td_void **pool_virt_addr, td_phys_addr_t *phys_addr);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif  /* AUDIO_INNER_H */

