/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2009-2020. All rights reserved.
 * Description: ao_ext.h
 * Author: Hisilicon multimedia software group
 * Create: 2009-05-05
 */

#ifndef AO_EXT_H
#define AO_EXT_H

#include "ot_type.h"
#ifdef CONFIG_OT_PLATFORM_V8
#include "ot_inner_common_aio.h"
#else
#include "ot_comm_aio_adapt.h"
#endif
#include "aiao_ext.h"

typedef td_s32 fn_ao_init(ot_audio_dev audio_dev_id);
typedef td_void fn_ao_exit(ot_audio_dev audio_dev_id);
typedef td_s32 fn_ao_trans_init(ot_audio_dev audio_dev_id);
typedef td_s32 fn_ao_trans_begin(ot_audio_dev audio_dev_id);
typedef td_s32 fn_ao_trans_stop(ot_audio_dev audio_dev_id);
typedef td_s32 fn_ao_trans_release(ot_audio_dev audio_dev_id);
typedef aio_dev_ctx *fn_ao_get_dma_inf(ot_audio_dev audio_dev_id);
typedef td_s32 fn_ao_get_data(ot_audio_dev ao_dev, ot_ao_chn ao_chn, ot_audio_frame *frm);
typedef td_void fn_ao_save_ao_rdptr(ot_audio_dev ao_dev);
typedef td_void fn_ao_set_ao_rd_offset_flag(ot_audio_dev ao_dev, ot_ao_chn ao_chn);

/* alsa */
typedef td_s32 fn_ao_set_alsa_dma_attr(ot_audio_dev ao_dev, cir_buf cir_buf_attr, td_u32 trans_len);
typedef td_s32 fn_ao_set_dev_attr(ot_audio_dev ao_dev, const ot_aio_attr *attr);
typedef td_s32 (*fn_ao_alsa_dma_callback)(td_void *substream);
typedef td_void fn_ao_set_dma_ext_isr(ot_audio_dev ao_dev, fn_ao_alsa_dma_callback pf_callback, td_void *substream);
typedef td_void fn_ao_reset_dma_ext_isr(ot_audio_dev ao_dev);

typedef struct {
    fn_ao_init *pfn_init;
    fn_ao_exit *pfn_exit;

    /* trans function */
    fn_ao_trans_init *pfn_trans_init;
    fn_ao_trans_begin *pfn_trans_begin;
    fn_ao_trans_stop *pfn_trans_stop;
    fn_ao_trans_release *pfn_trans_release;

    fn_ao_get_dma_inf *pfn_get_dma_inf;
    fn_ao_get_data *pfn_get_data;
    fn_ao_save_ao_rdptr *pfn_save_ao_rdptr;
    fn_ao_set_ao_rd_offset_flag *pfn_set_ao_rd_offset_flag;

    /* alsa */
    fn_ao_set_alsa_dma_attr *pfn_ao_set_alsa_dma_attr;
    fn_ao_set_dev_attr *pfn_ao_set_dev_attr;
    fn_ao_set_dma_ext_isr *pfn_ao_set_dma_ext_isr;
    fn_ao_reset_dma_ext_isr *pfn_ao_reset_dma_ext_isr;
} ao_export_func;

#endif
