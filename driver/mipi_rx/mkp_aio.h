/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2009-2020. All rights reserved.
 * Description: mkp_aio.h
 * Author: Hisilicon multimedia software group
 * Create: 2009-05-05
 */

#ifndef MKP_AIO_H
#define MKP_AIO_H

#include "mkp_ioctl.h"
#include "audio_inner.h"
#include "ot_debug.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/* channel state:
    enable <----> pause --> disable
    ^                             ^
    |-----------------------------|
*/
typedef enum {
    AIO_CHN_ORIGINAL = 0,
    AIO_CHN_ENABLE,
    AIO_CHN_PAUSE,
    AIO_CHN_DISABLE,
    AIO_CHN_BUTT
} aio_chn_state;

typedef struct {
    td_u64 pts;
    td_u32 seq;
    td_u32 len;
    void *data;
} aio_frame_info;

typedef struct {
    aio_frame_info *ai_frame;
    aio_frame_info *ref_frame;
    td_bool ref_data_valid;
} ai_frame_combine;

typedef struct {
    audio_frame_combine audio_frm;
    td_s32 milli_sec; /* TimeOut */
} aio_frame;

typedef struct {
    ot_audio_dev ao_dev;
    ot_ao_chn ao_chn;
} ai_enable_aec;

typedef struct {
    ot_audio_dev ai_dev;
    ot_ai_chn ai_chn;
} ao_bind_ai_args;

typedef struct {
    ot_audio_dev ai_dev;
    ot_ai_chn ai_chn;
    ot_audio_dev ao_dev;
    ot_ao_chn ao_chn;
} aiao_bind_args;

typedef struct {
    ot_adec_chn ad_chn;
    ot_audio_dev ao_dev;
    ot_ao_chn ao_chn;
} adecao_bind_args;

/* 存放通道音频数据buf的地址信息 */
typedef struct {
    td_u64 phys_addr;
    td_u8 *virt_addr;

    td_bool valid; /* whether frame is valid */
} aio_chn_data_addr;

typedef struct {
    td_bool enable;
    ot_audio_fade fade;
} audio_mute;

#define aio_check_null_ptr_return(ptr)     \
    do {                                   \
        if ((td_u8 *)(ptr) == TD_NULL) {   \
            return OT_ERR_AIO_NULL_PTR;    \
        }                                  \
    } while (0)

#define ai_check_null_ptr_return(ptr)      \
    do {                                   \
        if ((td_u8 *)(ptr) == TD_NULL) {   \
            return OT_ERR_AI_NULL_PTR;     \
        }                                  \
    } while (0)

#define ao_check_null_ptr_return(ptr)      \
    do {                                   \
        if ((td_u8 *)(ptr) == TD_NULL) {   \
            return OT_ERR_AO_NULL_PTR;     \
        }                                  \
    } while (0)

#define check_sample_signal_16bit(val) ((val) < -8 || (val) > 8)

#define ot_check_signal_16bit(data) (check_sample_signal_16bit(((td_s16 *)(data))[0]))

#ifdef __KERNEL__
/* AIO Log Print */
#define aio_emerg_trace(fmt, ...) \
    OT_EMERG_TRACE(OT_ID_AIO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define aio_alert_trace(fmt, ...) \
    OT_ALERT_TRACE(OT_ID_AIO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define aio_crit_trace(fmt, ...) \
    OT_CRIT_TRACE(OT_ID_AIO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define aio_err_trace(fmt, ...) \
    OT_ERR_TRACE(OT_ID_AIO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define aio_warn_trace(fmt, ...) \
    OT_WARN_TRACE(OT_ID_AIO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define aio_notice_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_AIO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define aio_info_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_AIO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define aio_debug_trace(fmt, ...) \
    OT_DEBUG_TRACE(OT_ID_AIO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

/* AI Log Print */
#define ai_emerg_trace(fmt, ...) \
    OT_EMERG_TRACE(OT_ID_AI, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define ai_alert_trace(fmt, ...) \
    OT_ALERT_TRACE(OT_ID_AI, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define ai_crit_trace(fmt, ...) \
    OT_CRIT_TRACE(OT_ID_AI, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define ai_err_trace(fmt, ...) \
    OT_ERR_TRACE(OT_ID_AI, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define ai_warn_trace(fmt, ...) \
    OT_WARN_TRACE(OT_ID_AI, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define ai_notice_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_AI, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define ai_info_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_AI, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define ai_debug_trace(fmt, ...) \
    OT_DEBUG_TRACE(OT_ID_AI, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)


/* AO Log Print */
#define ao_emerg_trace(fmt, ...) \
    OT_EMERG_TRACE(OT_ID_AO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define ao_alert_trace(fmt, ...) \
    OT_ALERT_TRACE(OT_ID_AO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define ao_crit_trace(fmt, ...) \
    OT_CRIT_TRACE(OT_ID_AO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define ao_err_trace(fmt, ...) \
    OT_ERR_TRACE(OT_ID_AO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define ao_warn_trace(fmt, ...) \
    OT_WARN_TRACE(OT_ID_AO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define ao_notice_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_AO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define ao_info_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_AO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define ao_debug_trace(fmt, ...) \
    OT_DEBUG_TRACE(OT_ID_AO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
/* 用户态 */
#define AIO_TRACE_LEVEL OT_DBG_ERR
#define AI_TRACE_LEVEL  OT_DBG_ERR
#define AO_TRACE_LEVEL  OT_DBG_ERR

/* AIO Log Print */
#if (AIO_TRACE_LEVEL >= OT_DBG_EMERG)
#define aio_emerg_trace(fmt, ...) \
    OT_EMERG_TRACE(OT_ID_AIO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define aio_emerg_trace(fmt, ...)
#endif

#if (AIO_TRACE_LEVEL >= OT_DBG_ALERT)
#define aio_alert_trace(fmt, ...) \
    OT_ALERT_TRACE(OT_ID_AIO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define aio_alert_trace(fmt, ...)
#endif

#if (AIO_TRACE_LEVEL >= OT_DBG_CRIT)
#define aio_crit_trace(fmt, ...) \
    OT_CRIT_TRACE(OT_ID_AIO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define aio_crit_trace(fmt, ...)
#endif

#if (AIO_TRACE_LEVEL >= OT_DBG_ERR)
#define aio_err_trace(fmt, ...) \
    OT_ERR_TRACE(OT_ID_AIO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define aio_err_trace(fmt, ...)
#endif

#if (AIO_TRACE_LEVEL >= OT_DBG_WARN)
#define aio_warn_trace(fmt, ...) \
    OT_WARN_TRACE(OT_ID_AIO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define aio_warn_trace(fmt, ...)
#endif

#if (AIO_TRACE_LEVEL >= OT_DBG_NOTICE)
#define aio_notice_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_AIO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define aio_notice_trace(fmt, ...)
#endif

#if (AIO_TRACE_LEVEL >= OT_DBG_INFO)
#define aio_info_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_AIO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define aio_info_trace(fmt, ...)
#endif

#if (AIO_TRACE_LEVEL >= OT_DBG_DEBUG)
#define aio_debug_trace(fmt, ...) \
    OT_DEBUG_TRACE(OT_ID_AIO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define aio_debug_trace(fmt, ...)
#endif


/* AI Log Print */
#if (AI_TRACE_LEVEL >= OT_DBG_EMERG)
#define ai_emerg_trace(fmt, ...) \
    OT_EMERG_TRACE(OT_ID_AI, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ai_emerg_trace(fmt, ...)
#endif

#if (AI_TRACE_LEVEL >= OT_DBG_ALERT)
#define ai_alert_trace(fmt, ...) \
    OT_ALERT_TRACE(OT_ID_AI, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ai_alert_trace(fmt, ...)
#endif

#if (AI_TRACE_LEVEL >= OT_DBG_CRIT)
#define ai_crit_trace(fmt, ...) \
    OT_CRIT_TRACE(OT_ID_AI, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ai_crit_trace(fmt, ...)
#endif

#if (AI_TRACE_LEVEL >= OT_DBG_ERR)
#define ai_err_trace(fmt, ...) \
    OT_ERR_TRACE(OT_ID_AI, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ai_err_trace(fmt, ...)
#endif

#if (AI_TRACE_LEVEL >= OT_DBG_WARN)
#define ai_warn_trace(fmt, ...) \
    OT_WARN_TRACE(OT_ID_AI, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ai_warn_trace(fmt, ...)
#endif

#if (AI_TRACE_LEVEL >= OT_DBG_NOTICE)
#define ai_notice_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_AI, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ai_notice_trace(fmt, ...)
#endif

#if (AI_TRACE_LEVEL >= OT_DBG_INFO)
#define ai_info_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_AI, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ai_info_trace(fmt, ...)
#endif

#if (AI_TRACE_LEVEL >= OT_DBG_DEBUG)
#define ai_debug_trace(fmt, ...) \
    OT_DEBUG_TRACE(OT_ID_AI, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ai_debug_trace(fmt, ...)
#endif


/* AO Log Print */
#if (AO_TRACE_LEVEL >= OT_DBG_EMERG)
#define ao_emerg_trace(fmt, ...) \
    OT_EMERG_TRACE(OT_ID_AO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ao_emerg_trace(fmt, ...)
#endif

#if (AO_TRACE_LEVEL >= OT_DBG_ALERT)
#define ao_alert_trace(fmt, ...) \
    OT_ALERT_TRACE(OT_ID_AO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ao_alert_trace(fmt, ...)
#endif

#if (AO_TRACE_LEVEL >= OT_DBG_CRIT)
#define ao_crit_trace(fmt, ...) \
    OT_CRIT_TRACE(OT_ID_AO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ao_crit_trace(fmt, ...)
#endif

#if (AO_TRACE_LEVEL >= OT_DBG_ERR)
#define ao_err_trace(fmt, ...) \
    OT_ERR_TRACE(OT_ID_AO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ao_err_trace(fmt, ...)
#endif

#if (AO_TRACE_LEVEL >= OT_DBG_WARN)
#define ao_warn_trace(fmt, ...) \
    OT_WARN_TRACE(OT_ID_AO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ao_warn_trace(fmt, ...)
#endif

#if (AO_TRACE_LEVEL >= OT_DBG_NOTICE)
#define ao_notice_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_AO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ao_notice_trace(fmt, ...)
#endif

#if (AO_TRACE_LEVEL >= OT_DBG_INFO)
#define ao_info_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_AO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ao_info_trace(fmt, ...)
#endif

#if (AO_TRACE_LEVEL >= OT_DBG_DEBUG)
#define ao_debug_trace(fmt, ...) \
    OT_DEBUG_TRACE(OT_ID_AO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ao_debug_trace(fmt, ...)
#endif

#endif

#define ai_check_chn_id_return(chn_id)                          \
    do {                                                        \
        if ((chn_id) < 0 || (chn_id) >= OT_AI_MAX_CHN_NUM) {    \
            ai_err_trace("ai chnid %d is invalid\n", (chn_id)); \
            return OT_ERR_AI_INVALID_CHN_ID;                    \
        }                                                       \
    } while (0)

#define ai_check_chn_id_for_dev_return(chn_id, dev_id, chn_cnt, chn_per_frm)                           \
    do {                                                                                               \
        if ((chn_id) >= ((td_s32)(chn_cnt) / (chn_per_frm))) {                                         \
            ai_err_trace("no aichn %d for aidev %d, or can't handle right channels for stereo mode\n", \
                (chn_id), (dev_id));                                                                   \
            return OT_ERR_AI_INVALID_CHN_ID;                                                           \
        }                                                                                              \
    } while (0)

#define ao_check_chn_id_return(chn_id)                          \
    do {                                                        \
        if ((chn_id) < 0 || (chn_id) >= OT_AO_MAX_CHN_NUM) {    \
            ao_err_trace("ao chnid %d is invalid\n", (chn_id)); \
            return OT_ERR_AO_INVALID_CHN_ID;                    \
        }                                                       \
    } while (0)

#define ao_check_chn_id_for_dev_return(chn_id, dev_id, chn_cnt, chn_per_frm)                           \
    do {                                                                                               \
        if (((chn_id) != OT_AO_SYS_CHN_ID) && ((chn_id) >= ((td_s32)(chn_cnt) / (chn_per_frm)))) {     \
            ao_err_trace("no aochn %d for aodev %d, or can't handle right channels for stereo mode\n", \
                (chn_id), (dev_id));                                                                   \
            return OT_ERR_AO_INVALID_CHN_ID;                                                           \
        }                                                                                              \
    } while (0)

#define ai_get_chn_id(dev, i)       ((dev) * OT_AI_MAX_CHN_NUM + (i))
#define ai_get_dev_by_chn_id(chn_id) ((chn_id) / OT_AI_MAX_CHN_NUM)
#define ai_get_chn_by_chn_id(chn_id) ((chn_id) % OT_AI_MAX_CHN_NUM)

#define ao_get_chn_id(dev, i)       ((dev) * OT_AO_MAX_CHN_NUM + (i))
#define ao_get_dev_by_chn_id(chn_id) ((chn_id) / OT_AO_MAX_CHN_NUM)
#define ao_get_chn_by_chn_id(chn_id) ((chn_id) % OT_AO_MAX_CHN_NUM)

#define is_ai_extern(attr) \
    ((attr)->bit_width == 0 && (attr)->expand_flag)

/* 立体声每帧有两个通道的音频数据 */
#define aio_chn_num_per_frm(snd_mode) ((OT_AUDIO_SOUND_MODE_STEREO == (snd_mode)) ? 2 : 1)

typedef enum {
    IOC_NR_AI_BINDFD = 0,
    IOC_NR_AI_SET_ATTR,
    IOC_NR_AI_GET_ATTR,
    IOC_NR_AI_EN_DEV,
    IOC_NR_AI_DIS_DEV,
    IOC_NR_AI_GET_POOLID,
    IOC_NR_AI_GET_CHNPARAM,
    IOC_NR_AI_SET_CHNPARAM,
    IOC_NR_AI_GET_FRM,
    IOC_NR_AI_RELEASE_FRM,
    IOC_NR_AI_EN_VQE,
    IOC_NR_AI_DIS_VQE,
    IOC_NR_AI_EN_CHN,
    IOC_NR_AI_DIS_CHN,
    IOC_NR_AI_PAUSE_CHN,
    IOC_NR_AI_RESUME_CHN,
    IOC_NR_AI_EN_RESMP,
    IOC_NR_AI_DIS_RESMP,
    IOC_NR_AI_SET_RESMP,
    IOC_NR_AI_SET_VQE,
    IOC_NR_AI_SET_VQEGAIN,
    IOC_NR_AI_SET_TRACK_MODE,
    IOC_NR_AI_GET_TRACK_MODE,
    IOC_NR_AI_SET_CLKDIR,
    IOC_NR_AI_GET_CLKDIR,
    IOC_NR_AI_CLR_ATTR,
    IOC_NR_AI_SET_SAVEFILE,
    IOC_NR_AI_GET_SAVEFILE,
    IOC_NR_AI_EN_AECREFFRAME,
    IOC_NR_AI_DIS_AECREFFRAME,
    IOC_NR_AI_SET_ACODECGAIN,
    IOC_NR_AI_GET_ACODECGAIN,
    IOC_NR_AI_PUT_AIEFRM,
    IOC_NR_AI_GET_AIEFRM,
    IOC_NR_AI_SET_CHN_ATTR,
    IOC_NR_AI_GET_CHN_ATTR,
    IOC_NR_AI_GET_POOL_USER_CNT,
    IOC_NR_AI_GET_RESMP,
    IOC_NR_AI_VB_GETINFO,

    IOC_NR_AI_BUTT
} ioc_nr_ai;

typedef enum {
    IOC_NR_AO_BINDFD = 0,
    IOC_NR_AO_SET_ATTR,
    IOC_NR_AO_GET_ATTR,
    IOC_NR_AO_EN_DEV,
    IOC_NR_AO_DIS_DEV,
    IOC_NR_AO_PUT_FRM,
    IOC_NR_AO_EN_CHN,
    IOC_NR_AO_DIS_CHN,
    IOC_NR_AO_PAUSE_CHN,
    IOC_NR_AO_RESUME_CHN,
    IOC_NR_AO_EN_RESMP,
    IOC_NR_AO_DIS_RESMP,
    IOC_NR_AO_CLEAR_BUF,
    IOC_NR_AO_QUERY_CHN,
    IOC_NR_AO_SET_RESMP,
    IOC_NR_AO_SET_VQE,
    IOC_NR_AO_SET_TRACK_MODE,
    IOC_NR_AO_GET_TRACK_MODE,
    IOC_NR_AO_SET_CLKDIR,
    IOC_NR_AO_GET_CLKDIR,
    IOC_NR_AO_SET_VOLUME,
    IOC_NR_AO_GET_VOLUME,
    IOC_NR_AO_SET_MUTE,
    IOC_NR_AO_GET_MUTE,
    IOC_NR_AO_CLR_ATTR,
    IOC_NR_AO_SET_SAVEFILE,
    IOC_NR_AO_GET_SAVEFILE,
    IOC_NR_AO_GET_CHN_STATE,
    IOC_NR_AO_GET_CHN_DELAY,
    IOC_NR_AO_GET_RESMP,

    IOC_NR_AO_BUTT
} ioc_nr_ao;

/* AI IOC */
#define AI_BIND_CHANNEL2FD   _IOW(IOC_TYPE_AI, (td_s32)IOC_NR_AI_BINDFD, td_u32)
#define GET_AI_ATTR          _IOR(IOC_TYPE_AI, (td_s32)IOC_NR_AI_GET_ATTR, ot_aio_attr)         /* 获取设备属性 */
#define SET_AI_ATTR          _IOW(IOC_TYPE_AI, (td_s32)IOC_NR_AI_SET_ATTR, ot_aio_attr)         /* 设置设备属性 */
#define ENABLE_AI_TRANS      _IO(IOC_TYPE_AI, (td_s32)IOC_NR_AI_EN_DEV)                         /* 启用设备 */
#define DISABLE_AI_TRANS     _IO(IOC_TYPE_AI, (td_s32)IOC_NR_AI_DIS_DEV)                        /* 禁用设备 */
#define GET_AI_POOLID        _IOR(IOC_TYPE_AI, (td_s32)IOC_NR_AI_GET_POOLID, td_u32)            /* 获取AI的私有缓存池ID */
#define GET_AI_VB_POOLINFO   _IOWR(IOC_TYPE_AI, (td_s32)IOC_NR_AI_VB_GETINFO, vb_pool_info)     /* 获取AI的缓存池信息 */
#define GET_AI_CHNPARAM      _IOR(IOC_TYPE_AI, (td_s32)IOC_NR_AI_GET_CHNPARAM, ot_ai_chn_param) /* 获取通道参数 */
#define SET_AI_CHNPARAM      _IOW(IOC_TYPE_AI, (td_s32)IOC_NR_AI_SET_CHNPARAM, ot_ai_chn_param) /* 设置通道参数 */
#define GET_AI_FRAME         _IOWR(IOC_TYPE_AI, (td_s32)IOC_NR_AI_GET_FRM, aio_frame)           /* 获取音频帧 */
#define RELEASE_AI_FRAME     _IOW(IOC_TYPE_AI, (td_s32)IOC_NR_AI_RELEASE_FRM, audio_frame_combine)  /* 释放音频帧 */
#define AI_ENABLE_VQE        _IOW(IOC_TYPE_AI, (td_s32)IOC_NR_AI_EN_VQE, ai_enable_aec)           /* 启动VQE */
#define AI_DISABLE_VQE       _IO(IOC_TYPE_AI, (td_s32)IOC_NR_AI_DIS_VQE)                          /* 禁止VQE */
#define ENABLE_AI_CHN        _IO(IOC_TYPE_AI, (td_s32)IOC_NR_AI_EN_CHN)                           /* 启用音频输入通道 */
#define DISABLE_AI_CHN       _IO(IOC_TYPE_AI, (td_s32)IOC_NR_AI_DIS_CHN)                          /* 禁用音频输入通道 */
#define AI_ENABLE_RESMP      _IOW(IOC_TYPE_AI, IOC_NR_AI_EN_RESMP, audio_resample_attr)   /* 启用重采样 */
#define AI_DISABLE_RESMP     _IO(IOC_TYPE_AI, IOC_NR_AI_DIS_RESMP)                        /* 禁用用重采样 */
#define SET_AI_CHN_RESMP     _IOW(IOC_TYPE_AI, (td_s32)IOC_NR_AI_SET_RESMP, aio_resmp_info)       /* 设置通道的重采样信息 */
#define GET_AI_CHN_RESMP     _IOR(IOC_TYPE_AI, (td_s32)IOC_NR_AI_GET_RESMP, aio_resmp_info)       /* 获取通道的重采样信息 */
#define SET_AI_CHN_VQE       _IOW(IOC_TYPE_AI, (td_s32)IOC_NR_AI_SET_VQE, ai_vqe_info)            /* 设置通道的VQE信息 */
#define SET_AI_CHN_VQEGAIN   _IOW(IOC_TYPE_AI, (td_s32)IOC_NR_AI_SET_VQEGAIN, td_s32)             /* 设置通道的VQEGAIN信息 */

#define PUT_AI_AIE_FRAME _IOWR(IOC_TYPE_AI, (td_s32)IOC_NR_AI_PUT_AIEFRM, audio_frame_combine)  /* 输入用户音频帧 */
#define GET_AI_AIE_FRAME _IOWR(IOC_TYPE_AI, (td_s32)IOC_NR_AI_GET_AIEFRM, aio_frame)            /* 获取用户音频帧 */

#define AI_SET_TRACK_MODE _IOW(IOC_TYPE_AI, (td_s32)IOC_NR_AI_SET_TRACK_MODE, ot_audio_track_mode) /* 设置track mode */
#define AI_GET_TRACK_MODE _IOR(IOC_TYPE_AI, (td_s32)IOC_NR_AI_GET_TRACK_MODE, ot_audio_track_mode) /* 获取track mode */
#define AI_SET_CLKDIR     _IOW(IOC_TYPE_AI, (td_s32)IOC_NR_AI_SET_CLKDIR, audio_clkdir)         /* 设置clock direction */
#define AI_GET_CLKDIR     _IOR(IOC_TYPE_AI, (td_s32)IOC_NR_AI_GET_CLKDIR, audio_clkdir)         /* 获取clock direction */

#define AI_CLR_ATTR _IO(IOC_TYPE_AI, (td_s32)IOC_NR_AI_CLR_ATTR) /* 清除aiconfig标示 */

#define SET_AI_SAVEFILE _IOW(IOC_TYPE_AI, (td_s32)IOC_NR_AI_SET_SAVEFILE, ot_audio_save_file_info)
#define GET_AI_SAVEFILE _IOR(IOC_TYPE_AI, (td_s32)IOC_NR_AI_GET_SAVEFILE, ot_audio_save_file_info)

#define AI_ENABLE_AECREFFRAME  _IOW(IOC_TYPE_AI, (td_s32)IOC_NR_AI_EN_AECREFFRAME, ai_enable_aec)   /* 启动回声抵消 */
#define AI_DISABLE_AECREFFRAME _IO(IOC_TYPE_AI, (td_s32)IOC_NR_AI_DIS_AECREFFRAME)                  /* 禁止回声抵消 */

#define AI_SET_ACODECGAIN _IOW(IOC_TYPE_AI, (td_s32)IOC_NR_AI_SET_ACODECGAIN, td_s32)   /* 设置ACODEC模拟增益 */
#define AI_GET_ACODECGAIN _IOR(IOC_TYPE_AI, (td_s32)IOC_NR_AI_GET_ACODECGAIN, td_s32)   /* 获取ACODEC模拟增益 */

#define GET_AI_CHN_ATTR     _IOR(IOC_TYPE_AI, (td_s32)IOC_NR_AI_GET_CHN_ATTR, ot_ai_chn_attr)      /* 获取AI通道属性 */
#define SET_AI_CHN_ATTR     _IOW(IOC_TYPE_AI, (td_s32)IOC_NR_AI_SET_CHN_ATTR, ot_ai_chn_attr)      /* 设置AI通道属性 */

#define GET_AI_POOL_USER_CNT _IOR(IOC_TYPE_AI, (td_s32)IOC_NR_AI_GET_POOL_USER_CNT, td_u32) /* 获取AI被user占用的VB块数 */

/* AO IOC */
#define SET_AO_SAVEFILE _IOW(IOC_TYPE_AO, (td_s32)IOC_NR_AO_SET_SAVEFILE, ot_audio_save_file_info)
#define GET_AO_SAVEFILE _IOR(IOC_TYPE_AO, (td_s32)IOC_NR_AO_GET_SAVEFILE, ot_audio_save_file_info)

#define AO_BIND_CHANNEL2FD _IOW(IOC_TYPE_AO, (td_s32)IOC_NR_AO_BINDFD, td_u32)
#define GET_AO_ATTR        _IOR(IOC_TYPE_AO, (td_s32)IOC_NR_AO_GET_ATTR, ot_aio_attr)   /* 获取设备属性 */
#define SET_AO_ATTR        _IOW(IOC_TYPE_AO, (td_s32)IOC_NR_AO_SET_ATTR, ot_aio_attr)   /* 设置设备属性 */
#define ENABLE_AO_TRANS    _IO(IOC_TYPE_AO, (td_s32)IOC_NR_AO_EN_DEV)                   /* 启用设备 */
#define DISABLE_AO_TRANS   _IO(IOC_TYPE_AO, (td_s32)IOC_NR_AO_DIS_DEV)                  /* 禁用设备 */

#define PUT_AO_FRAME     _IOWR(IOC_TYPE_AO, (td_s32)IOC_NR_AO_PUT_FRM, aio_frame)           /* 获取音频帧buf */
#define ENABLE_AO_CHN    _IO(IOC_TYPE_AO, (td_s32)IOC_NR_AO_EN_CHN)                         /* 启用音频输出通道 */
#define DISABLE_AO_CHN   _IO(IOC_TYPE_AO, (td_s32)IOC_NR_AO_DIS_CHN)                        /* 禁用音频输出通道 */
#define PAUSE_AO_CHN     _IO(IOC_TYPE_AO, (td_s32)IOC_NR_AO_PAUSE_CHN)                      /* 暂停音频输出通道 */
#define RESUME_AO_CHN    _IO(IOC_TYPE_AO, (td_s32)IOC_NR_AO_RESUME_CHN)                     /* 恢复音频输出通道 */
#define AO_GET_CHN_STATE _IOR(IOC_TYPE_AO, (td_s32)IOC_NR_AO_GET_CHN_STATE, aio_chn_state)  /* 获取通道状态 */

#define AO_ENABLE_RESMP    _IOW(IOC_TYPE_AO, IOC_NR_AO_EN_RESMP, audio_resample_attr)   /* 启用重采样 */
#define AO_DISABLE_RESMP   _IO(IOC_TYPE_AO, IOC_NR_AO_DIS_RESMP)                        /* 禁用用重采样 */
#define AO_CLEAR_BUF_CTRL  _IO(IOC_TYPE_AO, (td_s32)IOC_NR_AO_CLEAR_BUF)                        /* 清空通道缓存 */
#define AO_QUERY_CHN_STATE _IOR(IOC_TYPE_AO, (td_s32)IOC_NR_AO_QUERY_CHN, ot_ao_chn_state)      /* 获取通道状态 */
#define AO_GET_CHN_DELAY   _IOR(IOC_TYPE_AO, (td_s32)IOC_NR_AO_GET_CHN_DELAY, td_u32)           /* 获取通道delay */
#define SET_AO_CHN_RESMP   _IOW(IOC_TYPE_AO, (td_s32)IOC_NR_AO_SET_RESMP, aio_resmp_info)       /* 设置通道的重采样信息 */
#define GET_AO_CHN_RESMP   _IOR(IOC_TYPE_AO, (td_s32)IOC_NR_AO_GET_RESMP, aio_resmp_info)       /* 获取通道的重采样信息 */
#define SET_AO_CHN_VQE     _IOW(IOC_TYPE_AO, (td_s32)IOC_NR_AO_SET_VQE, ao_vqe_info)            /* 设置通道的VQE信息 */

#define AO_SET_TRACK_MODE _IOW(IOC_TYPE_AO, (td_s32)IOC_NR_AO_SET_TRACK_MODE, ot_audio_track_mode) /* 设置track mode */
#define AO_GET_TRACK_MODE _IOR(IOC_TYPE_AO, (td_s32)IOC_NR_AO_GET_TRACK_MODE, ot_audio_track_mode) /* 获取track mode */
#define AO_SET_CLKDIR     _IOW(IOC_TYPE_AO, (td_s32)IOC_NR_AO_SET_CLKDIR, audio_clkdir)          /* 设置clock direction */
#define AO_GET_CLKDIR     _IOR(IOC_TYPE_AO, (td_s32)IOC_NR_AO_GET_CLKDIR, audio_clkdir)          /* 获取clock direction */

#define AO_SET_VOLUME _IOW(IOC_TYPE_AO, (td_s32)IOC_NR_AO_SET_VOLUME, td_s32)       /* 设置音量 */
#define AO_GET_VOLUME _IOR(IOC_TYPE_AO, (td_s32)IOC_NR_AO_GET_VOLUME, td_s32)       /* 获取音量 */
#define AO_SET_MUTE   _IOW(IOC_TYPE_AO, (td_s32)IOC_NR_AO_SET_MUTE, audio_mute)     /* 设置音量 */
#define AO_GET_MUTE   _IOWR(IOC_TYPE_AO, (td_s32)IOC_NR_AO_GET_MUTE, audio_mute)    /* 获取音量 */
#define AO_CLR_ATTR   _IO(IOC_TYPE_AO, (td_s32)IOC_NR_AO_CLR_ATTR)                  /* 清除aoconfig标示 */

typedef enum {
    IOC_NR_AIAO_SETMODPARAM = 0,
    IOC_NR_AIAO_GETMODPARAM
} ioc_nr_aiao;

/* AIO IOC */
#define AIAO_SETMODPARAM _IOW(IOC_TYPE_AIO, IOC_NR_AIAO_SETMODPARAM, ot_audio_mod_param)
#define AIAO_GETMODPARAM _IOR(IOC_TYPE_AIO, IOC_NR_AIAO_GETMODPARAM, ot_audio_mod_param)

td_s32 mpi_aio_init(td_void);
td_s32 mpi_aio_exit(td_void);

td_s32 mpi_ao_init(td_void);
td_s32 mpi_ao_exit(td_void);

td_s32 mpi_ai_init(td_void);
td_s32 mpi_ai_exit(td_void);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef MKP_AIO_H */
