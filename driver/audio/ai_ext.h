/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2009-2020. All rights reserved.
 * Description: ai_ext.h
 * Author: Hisilicon multimedia software group
 * Create: 2009-05-05
 */

#ifndef AI_EXT_H
#define AI_EXT_H

#include "ot_type.h"
#ifdef CONFIG_OT_PLATFORM_V8
#include "ot_inner_common_aio.h"
#else
#include "ot_comm_aio_adapt.h"
#endif
#include "aiao_ext.h"

typedef td_s32 fn_ai_init(ot_audio_dev audio_dev_id);
typedef td_void fn_ai_exit(ot_audio_dev audio_dev_id);
typedef td_s32 fn_ai_trans_init(ot_audio_dev audio_dev_id);
typedef td_s32 fn_ai_trans_begin(ot_audio_dev audio_dev_id);
typedef td_s32 fn_ai_trans_stop(ot_audio_dev audio_dev_id);
typedef td_s32 fn_ai_trans_release(ot_audio_dev audio_dev_id);

/* alsa */
typedef td_s32 fn_ai_set_alsa_dma_attr(ot_audio_dev ai_dev, cir_buf cir_buf_attr, td_u32 trans_len);
typedef td_s32 fn_ai_set_dev_attr(ot_audio_dev ai_dev, const ot_aio_attr *attr);
typedef td_s32 (*fn_ai_alsa_dma_callback)(td_void *substream);
typedef td_void fn_ai_set_dma_ext_isr(ot_audio_dev ai_dev, fn_ai_alsa_dma_callback pf_callback, td_void *substream);
typedef td_void fn_ai_reset_dma_ext_isr(ot_audio_dev ai_dev);

typedef struct {
    fn_ai_init *pfn_init;
    fn_ai_exit *pfn_exit;
    /* trans function */
    fn_ai_trans_init *pfn_trans_init;
    fn_ai_trans_begin *pfn_trans_begin;
    fn_ai_trans_stop *pfn_trans_stop;
    fn_ai_trans_release *pfn_trans_release;

    /* alsa */
    fn_ai_set_alsa_dma_attr *pfn_ai_set_alsa_dma_attr;
    fn_ai_set_dev_attr *pfn_ai_set_dev_attr;
    fn_ai_set_dma_ext_isr *pfn_ai_set_dma_ext_isr;
    fn_ai_reset_dma_ext_isr *pfn_ai_reset_dma_ext_isr;
} ai_export_func;

#endif

