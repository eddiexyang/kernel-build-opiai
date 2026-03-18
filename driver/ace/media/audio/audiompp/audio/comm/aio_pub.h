/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2009-2020. All rights reserved.
 * Description: Header of aio pub.h
 * Author: Hisilicon multimedia software group
 * Create: 2009/5/5
 */

#ifndef AIO_PUB_H
#define AIO_PUB_H

#include "hi_comm_audio.h"
#ifdef CONFIG_OT_PLATFORM_V8
#include "ot_inner_common_aio.h"
#else
#include "ot_comm_aio_adapt.h"
#endif
#include "audio_inner.h"
#include "ot_type.h"
#include "aio_buf.h"
#include "mkp_aio.h"
#include "audio_fb.h"

typedef osal_spinlock_t                        aio_spin_lock_s;
#define aio_spin_lock_init(lock)               osal_spin_lock_init(lock)
#define aio_spin_lock_destroy(lock)            osal_spin_lock_destroy(lock)
#define aio_spin_lock_irqsave(lock, flag)      osal_spin_lock_irqsave(lock, &(flag))
#define aio_spin_unlock_irqrestore(lock, flag) osal_spin_unlock_irqrestore(lock, &(flag))

typedef struct {
    td_u32 ai_get_cnt;
    td_u32 ai_release_cnt;
    td_u32 ao_send_cnt;
} aio_user_dbg_info;

typedef struct {
    struct osal_semaphore sem;

    td_s32 index;   /* 当前通道的数据在DMA buffer中的序号 */
    audio_buf buf; /* audio channel buffer */

    aio_chn_state chn_state;  /* once user send/get frame,set this chn active */
    volatile td_bool dev_enable; /* when AI/AO device enable set it true */

    td_u32 int_lost; /* int lost times (use for debug) */
    td_u32 int_lost_time;

    td_u32 aec_offset;
    td_bool aec_offset_init;

    td_bool vqe_enable;

    ao_vqe_info ao_vqe_dbg;
    aio_resmp_info resmp_dbg;
    ot_audio_save_file_info save_file_info;
} ao_chn_ctx;

typedef struct {
    struct osal_semaphore sem;

    osal_wait_t user_wait; /* wait_queue for user getting frame */
    osal_wait_t aie_wait;  /* wait_queue for ai engine getting frame */

    td_s32 index; /* 当前通道的数据在DMA buffer中的序号 */
    audio_frame_combine audio_frm;
    td_bool frm_valid;
    ot_ai_chn_param chn_param;      /* 可获取图像深度 */

    audio_frame_buf user_frm_buf; /* frame buffer for user */
    td_void *user_buf_virt_addr; /* virt_addr of frame buffer for user */

    audio_frame_buf aie_frm_buf; /* frame buffer for ai engine */
    td_void *aie_buf_virt_addr; /* virt_addr of frame buffer for aie */

    ot_ai_chn_attr chn_attr;

    aio_chn_state chn_state;  /* once user send/get frame,set this chn active */
    volatile td_bool dev_enable; /* when AI/AO device enable set it true */

    ot_audio_dev aec_ao_dev; /* ao device for aec, -1 means aec not enable */
    ot_ao_chn aec_ao_chn;    /* ao chn for aec */
    td_u32 aec_fail;  /* AEC fail times */
    td_u32 int_lost;  /* int lost times (use for debug) */
    td_u32 ai_usr_que_lost;

    td_bool vqe_enable;

    ai_vqe_info ai_vqe_dbg;
    td_s32 ai_vqe_vol;
    aio_resmp_info resmp_dbg;
    ot_audio_save_file_info save_file_info;
    td_bool enable_aec_ref_frame; /* 在AEC功能不开启的情况下是否返回AEC参考帧供用户使用 */

    aio_user_dbg_info aio_user_dbg;
} ai_chn_ctx;

typedef struct {
    td_u32 pool_id; /* 音频私有缓存池的ID */

    volatile td_bool inited; /* 初始化标志 */
    volatile td_bool cfg;    /* 是否设置属性 */
    volatile td_bool enable; /* 设备是否启用标志 */

    ot_aio_attr aio_attr;

    td_u64 dma_phy_addr;
    td_u8 *dma_vir_addr;

    /* 存放各通道音频帧VB的信息，用于传给DRV层 */
    aio_chn_data_addr ast_chn_data_addr[OT_AIO_MAX_CHN_NUM];

    osal_wait_t wait;            /* wait for data */
    struct osal_semaphore sem; /* semphone */
    aio_spin_lock_s spinlock;
} aio_dev_ctx;

aio_spin_lock_s *ao_get_buf_lock(td_void);

td_char *audio_print_work_mode(ot_aio_mode workmode);

td_char *audio_print_sample_rate(ot_audio_sample_rate sample_rate);

td_char *audio_print_bit_width(ot_audio_bit_width bit_width);

static inline td_char *audio_print_sound_mode(ot_audio_snd_mode snd_mode)
{
    if (snd_mode == OT_AUDIO_SOUND_MODE_MONO) {
        return "mono";
    } else if (snd_mode == OT_AUDIO_SOUND_MODE_STEREO) {
        return "stereo";
    } else {
        return "";
    }
}

td_char *audio_print_chn_state(aio_chn_state chn_state);

static inline td_char *audio_print_state(td_bool enable)
{
    if (enable == TD_TRUE) {
        return "Y";
    } else if (enable == TD_FALSE) {
        return "N";
    } else {
        return "";
    }
}

static inline td_char *audio_print_vqe_state(td_bool mannul)
{
    if (mannul == TD_TRUE) {
        return "Y";
    } else {
        return "N";
    }
}

static inline td_char *audio_print_aec_mode(audio_aec_mode aec_mode)
{
    if (aec_mode == AUDIO_AEC_MODE_OPEN) {
        return "open";
    } else {
        return "close";
    }
}

td_char *audio_print_vqe_mode(ot_vqe_work_state vqe_mode);

static inline td_char *audio_print_ai_chn_mode(ot_ai_chn_mode mode)
{
    if (mode == OT_AI_CHN_MODE_NORMAL) {
        return "normal";
    } else if (mode == OT_AI_CHN_MODE_FAST) {
        return "fast";
    } else {
        return "";
    }
}
#endif
