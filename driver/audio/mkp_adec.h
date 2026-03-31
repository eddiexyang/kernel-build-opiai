/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2009-2020. All rights reserved.
 * Description: mkp_adec.h
 * Author: Hisilicon multimedia software group
 * Create: 2009-05-05
 */

#ifndef MKP_ADEC_H
#define MKP_ADEC_H

#include "ot_inner_common_adec.h"
#include "mkp_ioctl.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/* its value must bigger than sizeof adec_stream_header */
#define ADEC_STRMBUF_ALIGN_BYTE 64

typedef struct {
    td_u64 time_stamp;
    td_u32 len;
    td_u32 seq;
} adec_stream_header;

typedef struct {
    td_u32 cb_phy;
    td_u32 cb_pack_len;
    td_u32 cb_all_pack_len;
} adec_cb_info;

typedef struct {
    td_s32 ret;
    adec_cb_info cb_info;
} adec_dcc_createchn_ret;

typedef struct {
    ot_adec_chn_attr chn_attr;
    adec_cb_info cb_info;
} adec_create_args;

typedef struct {
    ot_audio_dev ao_dev;
    ot_ao_chn ao_chn;
} adec_bind_ao_args;

typedef struct {
    td_u32 block_flag;
    ot_audio_stream stream;
} adec_send_stream_args;

typedef struct {
    td_s32 ori_send_cnt;
    td_s32 send_cnt;
    td_s32 get_cnt;
    td_s32 put_cnt;
    ot_g726_bps g726_rate;
    ot_adpcm_type adpcm_type;
    td_bool stream;
} adec_dbg_info;

#define adec_check_null_ptr_return(ptr)         \
    do {                                        \
        if ((ptr) == TD_NULL) {                 \
            return OT_ERR_ADEC_NULL_PTR;        \
        }                                       \
    } while (0)

#ifdef __KERNEL__
/* ADEC Log Print */
#define adec_emerg_trace(fmt, ...) \
    OT_EMERG_TRACE(OT_ID_ADEC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define adec_alert_trace(fmt, ...) \
    OT_ALERT_TRACE(OT_ID_ADEC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define adec_crit_trace(fmt, ...) \
    OT_CRIT_TRACE(OT_ID_ADEC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define adec_err_trace(fmt, ...) \
    OT_ERR_TRACE(OT_ID_ADEC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define adec_warn_trace(fmt, ...) \
    OT_WARN_TRACE(OT_ID_ADEC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define adec_notice_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_ADEC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define adec_info_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_ADEC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define adec_debug_trace(fmt, ...) \
    OT_DEBUG_TRACE(OT_ID_ADEC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#else
/* 用户态 */
#define ADEC_TRACE_LEVEL    OT_DBG_ERR

/* ADEC Log Print */
#if (ADEC_TRACE_LEVEL >= OT_DBG_EMERG)
#define adec_emerg_trace(fmt, ...) \
    OT_EMERG_TRACE(OT_ID_ADEC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define adec_emerg_trace(fmt, ...)
#endif

#if (ADEC_TRACE_LEVEL >= OT_DBG_ALERT)
#define adec_alert_trace(fmt, ...) \
    OT_ALERT_TRACE(OT_ID_ADEC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define adec_alert_trace(fmt, ...)
#endif

#if (ADEC_TRACE_LEVEL >= OT_DBG_CRIT)
#define adec_crit_trace(fmt, ...) \
    OT_CRIT_TRACE(OT_ID_ADEC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define adec_crit_trace(fmt, ...)
#endif

#if (ADEC_TRACE_LEVEL >= OT_DBG_ERR)
#define adec_err_trace(fmt, ...) \
    OT_ERR_TRACE(OT_ID_ADEC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define adec_err_trace(fmt, ...)
#endif

#if (ADEC_TRACE_LEVEL >= OT_DBG_WARN)
#define adec_warn_trace(fmt, ...) \
    OT_WARN_TRACE(OT_ID_ADEC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define adec_warn_trace(fmt, ...)
#endif

#if (ADEC_TRACE_LEVEL >= OT_DBG_NOTICE)
#define adec_notice_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_ADEC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define adec_notice_trace(fmt, ...)
#endif

#if (ADEC_TRACE_LEVEL >= OT_DBG_INFO)
#define adec_info_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_ADEC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define adec_info_trace(fmt, ...)
#endif

#if (ADEC_TRACE_LEVEL >= OT_DBG_DEBUG)
#define adec_debug_trace(fmt, ...) \
    OT_DEBUG_TRACE(OT_ID_ADEC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define adec_debug_trace(fmt, ...)
#endif

#endif

#define adec_check_chn_return(chan_id)                              \
    do {                                                            \
        if ((chan_id) < 0 || (chan_id) >= OT_ADEC_MAX_CHN_NUM)      \
            return OT_ERR_ADEC_INVALID_CHN_ID;                      \
    } while (0)

typedef enum {
    IOC_NR_ADEC_BINDCHN2FD = 0,
    IOC_NR_ADEC_CREATE,
    IOC_NR_ADEC_DESTROY,
    IOC_NR_ADEC_SET_DBGINFO
} ioc_nr_adec;

#define ADEC_BINDCHN2FD_CTRL  _IOW(IOC_TYPE_ADEC, (td_s32)IOC_NR_ADEC_BINDCHN2FD, td_u32)
#define ADEC_CREATE_CHN_CTRL  _IOW(IOC_TYPE_ADEC, (td_s32)IOC_NR_ADEC_CREATE, ot_adec_chn_attr)
#define ADEC_DESTROY_CHN_CTRL _IO(IOC_TYPE_ADEC, (td_s32)IOC_NR_ADEC_DESTROY)
#define ADEC_SET_DBGINFO_CTRL _IOW(IOC_TYPE_ADEC, (td_s32)IOC_NR_ADEC_SET_DBGINFO, adec_dbg_info)

td_s32 mpi_adec_init(td_void);
td_void mpi_adec_exit(td_void);

td_s32 mpi_adec_create_chn(ot_adec_chn ad_chn, const ot_adec_chn_attr *attr);
td_s32 mpi_adec_destroy_chn(ot_adec_chn ad_chn);
td_s32 mpi_adec_clear_chn_buf(ot_adec_chn ad_chn);
td_s32 mpi_adec_send_stream(ot_adec_chn ad_chn,
    const ot_audio_stream *stream, td_bool block);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef MKP_ADEC_H */
