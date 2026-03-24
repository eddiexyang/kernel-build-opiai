/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2009-2020. All rights reserved.
 * Description: Header of aenc module driver
 * Author: Hisilicon multimedia software group
 * Create: 2009-06-15
 */

#ifndef AENC_H
#define AENC_H

#include "hi_comm_audio.h"
#include "valg_plat.h"
#include "valg_ext.h"
#include "audio_fb.h"
#include "mkp_aenc.h"
#include "voie_drv.h"

#ifdef __cplusplus
extern "C" {
#endif /* end of #ifdef __cplusplus */

#define  AENC_STATE_STARTED   0
#define  AENC_STATE_STOPPING  1
#define  AENC_STATE_STOPPED   2

#define  AENC_USER_FRM_MAXCNT 16

typedef osal_spinlock_t                         aenc_spin_lock;
#define aenc_spin_lock_init(lock)               osal_spin_lock_init(lock)
#define aenc_spin_lock_destroy(lock)               osal_spin_lock_destroy(lock)

#ifndef AENC_DEBUG
#define aenc_spin_lock_irqsave(lock, flag)      osal_spin_lock_irqsave(lock, &(flag))
#define aenc_spin_unlock_irqrestore(lock, flag) osal_spin_unlock_irqrestore(lock, &(flag))

#else
#define aenc_spin_lock_irqsave(lock, flag) \
    do { \
        osal_spin_lock_irqsave(lock, &flag); \
    } while (0)

#define aenc_spin_unlock_irqrestore(lock, flag) \
    do { \
        osal_spin_unlock_irqrestore(lock, &flag); \
    } while (0)
#endif

typedef struct {
    td_bool             buf_init;
    td_u32              cb_pack_len;  /* 单个码流包的长度: 头+码流数据 */
    valg_crcl_buf     strm_buf;     /* CB控制信息 */
    td_u32              offset;     /* CB控制信息首地址到码流CB首地址的偏移 */
    td_u8               *cb_ctl_vir;  /* CB控制信息首地址-虚拟地址 */
    td_u8               *cb_base_vir; /* 码流CB首地址 */
    td_u32              cb_cur_len;

    struct osal_semaphore    sem;        /* 管理码流BUF */
    osal_wait_t   strm_wait;   /* 用于告知用码流BUF是否有数据供用户获取 */

    audio_frame_buf   ai_frm_buf;   /* 存放AI发送过来的音频帧 */
    osal_wait_t   ai_frm_wait;  /* 用于告知用户态线程是否可获取数据 */

    volatile td_bool    created;
    ot_aenc_chn_attr     chn_attr;
    voie_code_type    voie_code_type;
    aenc_send_frm_dbg send_dbg;

    td_u32              seq;     /* 码流计数 */
    td_u32              usr_que_lost;

    td_bool             mute; /* 是否静音 */

    aenc_spin_lock    spin_lock;
    audio_frame_buf   user_frm_buf;  /* 管理用户态发送下来的音频帧 */
    volatile td_bool    has_frm_in_user_mode; /* 是否有音频帧正在用户态进行处理 */
    aenc_cb_info buf_info;
} aenc_chn_ctx;

aenc_chn_ctx *aenc_get_chn_ctx(td_s32 chn_id);

td_s32 aenc_sub_vb(const audio_frame_combine *audio_frm, td_u32 uid);
td_s32 aenc_wakeup_get_stream(ot_aenc_chn ae_chn);

#ifdef __cplusplus
}
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef AENC_H */

