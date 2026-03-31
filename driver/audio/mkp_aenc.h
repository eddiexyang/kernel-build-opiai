/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2009-2020. All rights reserved.
 * Description: mkp_aenc.h
 * Author: Hisilicon multimedia software group
 * Create: 2009-05-05
 */

#ifndef MKP_AENC_H
#define MKP_AENC_H

#include "ot_inner_common_aenc.h"
#include "mkp_ioctl.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

typedef struct {
    td_u64 time_stamp;
    td_u32 len;
    td_u32 seq;
} aenc_stream_header;

typedef struct {
    td_u64 cb_phy;
    td_u32 cb_pack_len;
    td_u32 cb_all_pack_len;
    td_u32 strm_offset; /* 码流相对所分配空间起始位置的偏移量 */
} aenc_cb_info;

typedef struct {
    td_u32 rcv_frm;  /* 发送编码器的帧数 */
    td_u32 enc_ok;   /* 发送编码器的帧数 */
    td_u32 frm_err;  /* 音频帧错误 */
    td_u32 buf_full; /* 音频码流缓存满 */
    td_u32 get_strm; /* 接收音频码流的次数 */
    td_u32 rls_strm; /* 释放音频码流的次数 */
    ot_g726_bps g726_rate;
    ot_adpcm_type adpcm_type;
} aenc_send_frm_dbg;

typedef struct {
    ot_aenc_chn_attr chn_attr;
    td_bool voie_support;
} aenc_voie_support;

typedef struct {
    td_s32 mill_sec;
    ot_audio_stream stream;
} aenc_stream_get;

/* its value must bigger than sizeof aenc_stream_header */
#define AENC_STRMBUF_ALIGN_BYTE 64

/* 该头文件和mkp_adec.h同时被mpi_sys.c包含，CHECK_NULL_PTR改为AENC_CHECK_NULL_PTR */
#define aenc_check_null_ptr_return(ptr)                        \
    do {                                                       \
        if ((ptr) == TD_NULL) {                                   \
            return OT_ERR_AENC_NULL_PTR;                       \
        }                                                      \
    } while (0)

#define aenc_check_cb_null_ptr_return(cb)                                             \
    do {                                                                              \
        if ((((cb).read_head) == TD_NULL) || (((cb).read_tail) == TD_NULL) ||         \
            (((cb).write_head) == TD_NULL) || (((cb).write_tail) == TD_NULL) ||       \
            (((cb).base) == TD_NULL)) {                                               \
            aenc_err_trace("CB NULL POINTER!\n");                                     \
            return OT_ERR_AENC_NULL_PTR;                                              \
        }                                                                             \
    } while (0)

#define aenc_8to16(src, dst, len)                                 \
    do {                                                          \
        td_u32 index;                                             \
        for (index = 0; index < (len); index++) {                 \
            ((td_u16 *)(dst))[index] = ((td_u8 *)(src))[index];   \
            ((td_u16 *)(dst))[index] <<= 8;                       \
        }                                                         \
    } while (0)

#ifdef __KERNEL__
/* AENC Log Print */
#define aenc_emerg_trace(fmt, ...) \
    OT_EMERG_TRACE(OT_ID_AENC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define aenc_alert_trace(fmt, ...) \
    OT_ALERT_TRACE(OT_ID_AENC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define aenc_crit_trace(fmt, ...) \
    OT_CRIT_TRACE(OT_ID_AENC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define aenc_err_trace(fmt, ...) \
    OT_ERR_TRACE(OT_ID_AENC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define aenc_warn_trace(fmt, ...) \
    OT_WARN_TRACE(OT_ID_AENC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define aenc_notice_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_AENC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define aenc_info_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_AENC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define aenc_debug_trace(fmt, ...) \
    OT_DEBUG_TRACE(OT_ID_AENC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#else
/* 用户态 */
#define AENC_TRACE_LEVEL    OT_DBG_ERR

/* AENC Log Print */
#if (AENC_TRACE_LEVEL >= OT_DBG_EMERG)
#define aenc_emerg_trace(fmt, ...) \
    OT_EMERG_TRACE(OT_ID_AENC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define aenc_emerg_trace(fmt, ...)
#endif

#if (AENC_TRACE_LEVEL >= OT_DBG_ALERT)
#define aenc_alert_trace(fmt, ...) \
    OT_ALERT_TRACE(OT_ID_AENC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define aenc_alert_trace(fmt, ...)
#endif

#if (AENC_TRACE_LEVEL >= OT_DBG_CRIT)
#define aenc_crit_trace(fmt, ...) \
    OT_CRIT_TRACE(OT_ID_AENC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define aenc_crit_trace(fmt, ...)
#endif

#if (AENC_TRACE_LEVEL >= OT_DBG_ERR)
#define aenc_err_trace(fmt, ...) \
    OT_ERR_TRACE(OT_ID_AENC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define aenc_err_trace(fmt, ...)
#endif

#if (AENC_TRACE_LEVEL >= OT_DBG_WARN)
#define aenc_warn_trace(fmt, ...) \
    OT_WARN_TRACE(OT_ID_AENC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define aenc_warn_trace(fmt, ...)
#endif

#if (AENC_TRACE_LEVEL >= OT_DBG_NOTICE)
#define aenc_notice_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_AENC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define aenc_notice_trace(fmt, ...)
#endif

#if (AENC_TRACE_LEVEL >= OT_DBG_INFO)
#define aenc_info_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_AENC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define aenc_info_trace(fmt, ...)
#endif

#if (AENC_TRACE_LEVEL >= OT_DBG_DEBUG)
#define aenc_debug_trace(fmt, ...) \
    OT_DEBUG_TRACE(OT_ID_AENC, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define aenc_debug_trace(fmt, ...)
#endif

#endif

#define aenc_check_chn_return(chan_id)                                 \
    do {                                                        \
        if ((chan_id) < 0 || (chan_id) >= OT_AENC_MAX_CHN_NUM)  \
            return OT_ERR_AENC_INVALID_CHN_ID;                  \
    } while (0)

typedef enum {
    IOC_NR_AENC_CREATE = 0,
    IOC_NR_AENC_DESTROY,

    IOC_NR_AENC_GET_FRAME,
    IOC_NR_AENC_RELEASE_FRAME,

    IOC_NR_AENC_SEND_FRM_DBG,

    IOC_NR_AENC_GET_STRM,
    IOC_NR_AENC_PUT_STRM,

    IOC_NR_AENC_READBUF_INIT,
    IOC_NR_AENC_READBUF_EXIT,

    IOC_NR_AENC_BINDCHN2FD,

    IOC_NR_AENC_SEND_FRM_VOIE,

    IOC_NR_AENC_GET_VOIE_SUPPORT,

    IOC_NR_AENC_UPDATE_STREAM_WR_TAIL,

    IOC_NR_AENC_GETSTREAM,

    IOC_NR_AENC_SET_MUTE,
    IOC_NR_AENC_GET_MUTE,

    IOC_NR_AENC_CLR_BUF
} ioc_nr_aenc;

#define AENC_CREATE_CHN_CTRL  _IOW(IOC_TYPE_AENC, (td_s32)IOC_NR_AENC_CREATE, ot_aenc_chn_attr)
#define AENC_DESTROY_CHN_CTRL _IO(IOC_TYPE_AENC, (td_s32)IOC_NR_AENC_DESTROY)

#define AENC_GET_FRAME     _IOR(IOC_TYPE_AENC, (td_s32)IOC_NR_AENC_GET_FRAME, audio_frame_combine)
#define AENC_RELEASE_FRAME _IOW(IOC_TYPE_AENC, (td_s32)IOC_NR_AENC_RELEASE_FRAME, audio_frame_combine)

#define AENC_SEND_FRM_DBG_CTRL _IOW(IOC_TYPE_AENC, (td_s32)IOC_NR_AENC_SEND_FRM_DBG, aenc_send_frm_dbg)

#define AENC_CHN_GET_STRM_CTRL _IOWR(IOC_TYPE_AENC, (td_s32)IOC_NR_AENC_GET_STRM, aenc_stream_get)
#define AENC_CHN_PUT_STRM_CTRL _IOW(IOC_TYPE_AENC, (td_s32)IOC_NR_AENC_PUT_STRM, ot_audio_stream)

#define AENC_READBUF_INIT_CTRL _IOW(IOC_TYPE_AENC, (td_s32)IOC_NR_AENC_READBUF_INIT, aenc_cb_info)
#define AENC_READBUF_EXIT_CTRL _IO(IOC_TYPE_AENC, (td_s32)IOC_NR_AENC_READBUF_EXIT)

#define AENC_BINDCHN2FD_CTRL _IOW(IOC_TYPE_AENC, (td_s32)IOC_NR_AENC_BINDCHN2FD, td_u32)

#define AENC_SEND_FRM_VOIE_CTRL _IOW(IOC_TYPE_AENC, (td_s32)IOC_NR_AENC_SEND_FRM_VOIE, ot_audio_frame)

#define AENC_GET_VOIE_SUPPORT _IOWR(IOC_TYPE_AENC, (td_s32)IOC_NR_AENC_GET_VOIE_SUPPORT, aenc_voie_support)

#define AENC_UPDATE_STREAM_WR_TAIL _IOW(IOC_TYPE_AENC, (td_s32)IOC_NR_AENC_UPDATE_STREAM_WR_TAIL, td_u32)

#define AENC_WAKEUP_GETSTREAM _IO(IOC_TYPE_AENC, (td_s32)IOC_NR_AENC_GETSTREAM)

#define AENC_SET_MUTE _IOW(IOC_TYPE_AENC, (td_s32)IOC_NR_AENC_SET_MUTE, td_bool)
#define AENC_GET_MUTE _IOWR(IOC_TYPE_AENC, (td_s32)IOC_NR_AENC_GET_MUTE, td_bool)

#define AENC_CLR_BUF _IO(IOC_TYPE_AENC, (td_s32)IOC_NR_AENC_CLR_BUF)

td_s32 mpi_aenc_init(td_void);
td_void mpi_aenc_exit(td_void);

td_s32 mpi_aenc_create_chn(ot_aenc_chn ae_chn, const ot_aenc_chn_attr *attr);
td_s32 mpi_aenc_destroy_chn(ot_aenc_chn ae_chn);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef MKP_AENC_H */
