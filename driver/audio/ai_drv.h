/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: ai_drv.h
 * Author: Hisilicon multimedia software group
 * Create: 2012/10/20
 * History: 2012/10/20  Created file
 */

#ifndef AI_DRV_H
#define AI_DRV_H

#include "hi_comm_audio.h"
#include "aio_pub.h"
#include "aiao_ext.h"

typedef td_void (*fn_ai_dma_callback)(ot_audio_dev ai_dev_id);
typedef td_s32 (*fn_ai_alsa_dma_callback)(td_void *substream);

aio_drv_dev_ctx *ai_drv_get_drv_dev_ctx(ot_audio_dev ai_dev);

td_s32 ai_drv_dma_buf_info_init(ot_audio_dev audio_dev_id, td_u32 *dma_buf_bytes);
td_s32 ai_drv_set_dma_buf_addr(ot_audio_dev audio_dev_id, td_u64 dma_phy_addr, td_u8 *dma_vir_addr);
td_s32 ai_drv_clr_dma_buf_addr(ot_audio_dev audio_dev_id);

td_void ai_drv_set_dma_isr(fn_ai_dma_callback pf_callback);
td_s32 ai_drv_set_dev_attr(ot_audio_dev audio_dev_id, const ot_aio_attr *attr);
td_s32 ai_drv_clr_dev_attr(ot_audio_dev audio_dev_id);

td_s32 ai_drv_get_chn_data(ot_audio_dev audio_dev_id, aio_chn_data_addr *chn_data_addr, td_u32 chn_cnt);
td_s32 ai_drv_enable_dev(ot_audio_dev audio_dev_id);
td_void ai_drv_disable_dev(ot_audio_dev audio_dev_id);
td_s32 ai_drv_proc_show(const struct osal_proc_dir_entry *s);

td_s32 ai_drv_dev_init(ot_audio_dev audio_dev_id);
td_s32 ai_drv_dev_deinit(ot_audio_dev audio_dev_id);

td_s32 ai_drv_set_track_mode(ot_audio_dev audio_dev_id, ot_audio_track_mode track_mode);
td_s32 ai_drv_get_track_mode(ot_audio_dev audio_dev_id, ot_audio_track_mode *track_mode);

td_s32 ai_drv_set_clk_dir(ot_audio_dev audio_dev_id, audio_clkdir clk_dir);
td_s32 ai_drv_get_clk_dir(ot_audio_dev audio_dev_id, audio_clkdir *clk_dir);

td_s32 ai_drv_set_acodec_gain(td_s32 gain);
td_s32 ai_drv_get_acodec_gain(td_s32 *gain);

td_s32 ai_drv_check_aio_export_func(td_void);

/* alsa */
td_s32 ai_drv_set_dma_attr(ot_audio_dev ai_dev, cir_buf cir_buf_attr, td_u32 trans_len);
td_void ai_drv_set_dma_ext_isr(ot_audio_dev ai_dev, fn_ai_alsa_dma_callback pf_callback, td_void *substream);
td_void ai_drv_reset_dma_ext_isr(ot_audio_dev ai_dev);

#endif

