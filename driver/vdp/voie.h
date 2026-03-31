/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2009-2020. All rights reserved.
 * Description: Header of voie driver
 * Author: Hisilicon audio software group
 * Create: 2009/9/6
 */

#ifndef VOIE_H
#define VOIE_H

#include "hi_comm_audio.h"
#include "aenc.h"

#define VOIE_CHN_MAX_NUM  OT_AENC_MAX_CHN_NUM

#define AENC_CHN_ILLEGAL_ID (-1)

/* VOIE Log Print */
#define voie_emerg_trace(fmt, ...)                                                                              \
    do {                                                                                                        \
        OT_EMERG_TRACE(OT_ID_VOIE, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);   \
    } while (0)

#define voie_alert_trace(fmt, ...)                                                                              \
    do {                                                                                                        \
        OT_ALERT_TRACE(OT_ID_VOIE, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);   \
    } while (0)

#define voie_crit_trace(fmt, ...)                                                                               \
    do {                                                                                                        \
        OT_CRIT_TRACE(OT_ID_VOIE, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);    \
    } while (0)

#define voie_err_trace(fmt, ...)                                                                                \
    do {                                                                                                        \
        OT_ERR_TRACE(OT_ID_VOIE, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);     \
    } while (0)

#define voie_warn_trace(fmt, ...)                                                                               \
    do {                                                                                                        \
        OT_WARN_TRACE(OT_ID_VOIE, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);    \
    } while (0)

#define voie_notice_trace(fmt, ...)                                                                             \
    do {                                                                                                        \
        OT_INFO_TRACE(OT_ID_VOIE, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);  \
    } while (0)

#define voie_info_trace(fmt, ...)                                                                               \
    do {                                                                                                        \
        OT_INFO_TRACE(OT_ID_VOIE, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);    \
    } while (0)

#define voie_debug_trace(fmt, ...)                                                                              \
    do {                                                                                                        \
        OT_DEBUG_TRACE(OT_ID_VOIE, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);   \
    } while (0)


typedef enum {
    VOIE_STATE_FREE    = 0,
    VOIE_STATE_BUSY,
    VOIE_STATE_STOPPING,
    VOIE_STATE_BUTT
} voie_state;

typedef struct {
    /* ping-pong队列，每个分配VOIE_CHN_MAX_NUM个voie_list_node */
    voie_lli_node *apst_lli_list[2]; /* 2: ping-pong队列 */
    volatile td_u32 au32_lli_phy_addr[2]; /* 2: ping-pong队列 */
    volatile ot_aenc_chn a_aenc_chn[2][VOIE_CHN_MAX_NUM]; /* 2: ping-pong队列 */
    audio_frame_combine ast_audio_frm[2][VOIE_CHN_MAX_NUM]; /* 2: ping-pong队列 */
    volatile td_s32 free_index;
    volatile voie_state state;

    td_u32 state_phy_addr;
    td_u32 state_vir_addr;
    osal_atomic_t ref_cnt;

    aenc_spin_lock spin_lock;
}voie_ctx;

td_s32 _voie_init(td_void);

td_void _voie_exit(td_void);

td_s32 _voie_open(td_void);

td_void _voie_close(ot_aenc_chn aenc_chn);

#endif /* end of #ifndef VOIE_H */
