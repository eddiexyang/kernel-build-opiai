/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: ao_drv.h
 * Author: Hisilicon multimedia software group
 * Create: 2009/5/5
 * History:
 *   1.Date        : 2009/5/5
 *     Modification: Created file
 *   2.Date        : 2019/08/08
 *     Modification: Update file for CSEC
 */

#ifndef AO_DRV_H
#define AO_DRV_H

#include <linux/kernel.h>
#include "aio_pub.h"
#include "aiao_hal_comm.h"
#include "aiao_ext.h"

#ifdef CONFIG_OT_PLATFORM_V8
#include "ot_ao_export.h"
#else
#include "ao_exp.h"
#endif

aio_drv_dev_ctx *ao_drv_get_drv_dev_ctx(ot_audio_dev ao_dev);

typedef td_void (*fn_ao_dma_callback)(ot_audio_dev ao_dev_id);
typedef td_s32 (*fn_ao_alsa_dma_callback)(td_void *substream);

td_void ao_dma_callback(ot_audio_dev audio_dev_id);
td_void ao_drv_notify_user(ot_audio_dev audio_dev_id);

td_void ao_drv_flush_data(ot_audio_dev audio_dev_id);
td_s32 ao_drv_put_zero_data(ot_audio_dev audio_dev_id, td_u32 ao_chn);
td_s32 ao_drv_put_last_data(ot_audio_dev audio_dev_id, td_u32 ao_chn);
td_s32 ao_drv_put_chn_data(ot_audio_dev audio_dev_id, const audio_pack *pack, td_u32 ao_chn);
td_s32 ao_drv_get_chn_data(ot_audio_dev audio_dev_id, td_u32 ao_chn, ot_audio_frame *frm);
td_s32 ao_drv_dma_buf_info_init(ot_audio_dev audio_dev_id, td_u32 *dma_buf_bytes);
td_s32 ao_drv_set_dma_buf_addr(ot_audio_dev audio_dev_id, td_u64 dma_phy_addr, td_u8 *dma_vir_addr);
td_s32 ao_drv_clr_dma_buf_addr(ot_audio_dev audio_dev_id);
td_s32 ao_drv_set_dev_attr(ot_audio_dev ao_dev, const ot_aio_attr *attr);
td_s32 ao_drv_clr_dev_attr(ot_audio_dev audio_dev_id);
td_s32 ao_drv_enable_dev(ot_audio_dev audio_dev_id);
td_void ao_drv_disable_dev(ot_audio_dev audio_dev_id);
td_void ao_drv_set_dma_isr(fn_ao_dma_callback pf_callback);
td_s32 ao_drv_proc_show(struct osal_proc_dir_entry *s);

td_s32 ao_drv_dev_init(ot_audio_dev audio_dev_id);
td_s32 ao_drv_dev_deinit(ot_audio_dev audio_dev_id);

td_s32 ao_drv_set_track_mode(ot_audio_dev audio_dev_id, ot_audio_track_mode track_mode);
td_s32 ao_drv_get_track_mode(ot_audio_dev audio_dev_id, ot_audio_track_mode *track_mode);
td_s32 ao_drv_set_mute(ot_audio_dev audio_dev_id, td_bool enable, const ot_audio_fade *fade);
td_s32 ao_drv_get_mute(ot_audio_dev audio_dev_id, td_bool *enable, ot_audio_fade *fade);
td_s32 ao_drv_set_clk_dir(ot_audio_dev audio_dev_id, audio_clkdir clk_dir);
td_s32 ao_drv_get_clk_dir(ot_audio_dev audio_dev_id, audio_clkdir *clk_dir);

td_s32 ao_drv_set_volume(ot_audio_dev audio_dev_id, td_s32 volume_db);
td_s32 ao_drv_get_volume(ot_audio_dev audio_dev_id, td_s32 *volume_db);

td_void ao_drv_get_dma_rdpt(ot_audio_dev audio_dev_id, td_u32 *dma_phy_addr);
td_s32 ao_drv_get_aec_data(ot_audio_dev audio_dev_id, ot_ao_chn ao_chn, ot_audio_frame *frm, td_u32 rptr_off_set);

td_s32 ao_drv_check_aio_export_func(td_void);

ot_ao_export_callback *ao_drv_get_export_callback(td_void);

td_s32 ao_drv_get_dma_buf_left_len(ot_audio_dev audio_dev_id, td_u32 *left_byte);

/* alsa */
td_s32 ao_drv_set_dma_attr(ot_audio_dev ao_dev, cir_buf cir_buf_attr, td_u32 trans_len);
td_s32 ao_drv_enable_dma(ot_audio_dev ao_dev, td_bool enable);
td_void ao_drv_set_dma_ext_isr(ot_audio_dev ao_dev, fn_ao_alsa_dma_callback pf_callback, td_void* substream);
td_void ao_drv_reset_dma_ext_isr(ot_audio_dev ao_dev);

#endif

