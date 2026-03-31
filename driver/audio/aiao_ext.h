/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: aio_ext.h
 * Author: Hisilicon multimedia software group
 * Create: 2009/5/5
 * History:
 *   1.Date        : 2009/5/5
 *     Modification: Created file
 *   2.Date        : 2019/08/08
 *     Modification: Update file for CSEC
 */

#ifndef AIAO_EXT_H
#define AIAO_EXT_H

#include "audio_inner.h"
#include "aiao_hal_comm.h"

typedef struct {
    td_u32 buf_size;
    td_u64 phy_addr;
    td_u8 *vir_addr;

    td_u32 rptr_offset;
    td_u32 wptr_offset;
} cir_buf;

typedef struct {
    struct {
        td_u32 buf_size; /* 循环buf的大小 */
        td_u64 phys_addr; /* 循环buf物理起始地址 */
        td_u8 *vir_addr; /* 循环buf虚拟起始地址 */

        td_u32 rptr_off_set; /* 循环buf读指针，记录读指针相对于起始地址的偏移量 */
        td_u32 wptr_off_set; /* 循环buf写指针，记录写指针相对于起始地址的偏移量 */
    } cir_buf;

    td_void *alsa_substream;

    td_u32 rptr_offset_aec;

    ot_aio_attr aio_attr;
    td_bool enable;

    td_u64 last_pts;     /* 上帧时间戳 */
    td_u32 frm_time;     /* 帧时间间隔 */
    td_u32 max_frm_time;  /* 帧之间最大时间间隔 */
    td_u32 isr_time;     /* 中断处理时间 */
    td_u32 max_isr_time;  /* 中断处理历史最长时间 */
    td_u32 aio_fifo_len;  /* AIO FIFO 宽度,一次DMA传输的音频数据，aio_fifo_len个字节，最小为8，最大为32 */
    td_u32 fifo_len_base; /* AIO FIFO 宽度的2的幂级，例如8是2的三次幂(用于通道分离合并算法的移位操作) */
    td_u32 fifo_shift;   /* AIO FIFO中数据移位(标准PCM需要移位1位) */
    td_u32 trans_len;    /* 总的dma传输长度,byte为单位(采样点个数*aio_fifo_len) */

    td_s32 chn_index[OT_AIO_MAX_CHN_NUM];

    td_u32 int_cnt;
    td_u32 u32fifo_int_cnt; /* 中断数目 */
    td_u32 u32buff_int_cnt; /* 中断数目 */
    ot_audio_track_mode track_mode;
    audio_clkdir clk_dir;
    td_bool mute;
    ot_audio_fade fade;
    td_s32 volume;
    td_bool mic_inl;
    td_bool mic_inr;
} aio_drv_dev_ctx;

typedef td_s32 fn_aio_init(ot_audio_dev audio_dev_id);
typedef td_void fn_aio_exit(ot_audio_dev audio_dev_id);

typedef td_s32 fn_aip_hal_set_dev_attr(ot_audio_dev ai_dev_id, const ot_aio_attr *attr);
typedef td_s32 fn_aip_hal_dev_enable(ot_audio_dev ai_dev_id);
typedef td_void fn_aip_hal_dev_disable(ot_audio_dev ai_dev_id);
typedef td_void fn_aip_hal_set_buff_rptr(ot_audio_dev ai_dev_id, td_u32 value);
typedef td_u32 fn_aip_hal_get_buff_rptr(ot_audio_dev ai_dev_id);
typedef td_void fn_aip_hal_set_buff_wptr(ot_audio_dev ai_dev_id, td_u32 value);
typedef td_u32 fn_aip_hal_get_buff_wptr(ot_audio_dev ai_dev_id);
typedef td_void fn_aip_hal_set_buffer_addr(ot_audio_dev ai_dev_id, td_u64 phy_add);
typedef td_void fn_aip_hal_set_buffer_size(ot_audio_dev ai_dev_id, td_u32 value);
typedef td_void fn_aip_hal_set_trans_size(ot_audio_dev ai_dev_id, td_u32 value);
typedef td_s32 fn_aip_hal_get_int_status(ot_audio_dev ai_dev_id, td_u32 *int_status);
typedef td_s32 fn_aip_hal_clr_int_status(ot_audio_dev ai_dev_id);
typedef td_void fn_aip_hal_set_isr(fn_aio_callback pf_callback);
typedef td_s32 fn_aip_hal_clr_attr(ot_audio_dev ai_dev_id);
typedef td_s32 fn_aip_hal_set_track_mode(ot_audio_dev ai_dev_id, ot_audio_track_mode track_mode);
typedef td_s32 fn_aip_hal_get_track_mode(ot_audio_dev ai_dev_id, ot_audio_track_mode *track_mode);
typedef td_s32 fn_aip_hal_set_clk_dir(ot_audio_dev ai_dev_id, audio_clkdir clk_dir);
typedef td_s32 fn_aip_hal_get_clk_dir(ot_audio_dev ai_dev_id, audio_clkdir *clk_dir);
typedef td_s32 fn_aip_hal_set_acodec_gain(td_s32 gain);
typedef td_s32 fn_aip_hal_get_acodec_gain(td_s32 *gain);
typedef td_s32 fn_aip_hal_set_acodec_soft_diff(td_bool enable);
typedef td_s32 fn_aip_hal_get_acodec_soft_diff(td_bool *enable);

typedef td_s32 fn_aop_hal_set_attr(ot_audio_dev ao_dev_id, const ot_aio_attr *attr);
typedef td_s32 fn_aop_hal_dev_enable(ot_audio_dev ao_dev_id);
typedef td_void fn_aop_hal_dev_disable(ot_audio_dev ao_dev_id);
typedef td_void fn_aop_hal_set_buff_wptr(ot_audio_dev ao_dev_id, td_u32 value);
typedef td_u32 fn_aop_hal_get_buff_wptr(ot_audio_dev ao_dev_id);
typedef td_void fn_aop_hal_set_buff_rptr(ot_audio_dev ao_dev_id, td_u32 value);
typedef td_u32 fn_aop_hal_get_buff_rptr(ot_audio_dev ao_dev_id);
typedef td_void fn_aop_hal_set_buffer_addr(ot_audio_dev ao_dev_id, td_u64 phy_add);
typedef td_void fn_aop_hal_set_buffer_size(ot_audio_dev ao_dev_id, td_u32 value);
typedef td_void fn_aop_hal_set_trans_size(ot_audio_dev ao_dev_id, td_u32 value);
typedef td_s32 fn_aop_hal_get_int_status(ot_audio_dev ao_dev_id, td_u32 *int_status);
typedef td_s32 fn_aop_hal_clr_int_status(ot_audio_dev ao_dev_id);
typedef td_void fn_aop_hal_set_isr(fn_aio_callback pf_callback);
typedef td_s32 fn_aop_hal_clr_attr(ot_audio_dev ao_dev_id);
typedef td_s32 fn_aop_hal_set_track_mode(ot_audio_dev ao_dev_id, ot_audio_track_mode track_mode);
typedef td_s32 fn_aop_hal_get_track_mode(ot_audio_dev ao_dev_id, ot_audio_track_mode *track_mode);
typedef td_s32 fn_aop_hal_set_clk_dir(ot_audio_dev ao_dev_id, audio_clkdir clk_dir);
typedef td_s32 fn_aop_hal_get_clk_dir(ot_audio_dev ao_dev_id, audio_clkdir *clk_dir);
typedef td_s32 fn_aop_hal_set_mute(ot_audio_dev ao_dev_id, td_bool mute, const ot_audio_fade *fade);
typedef td_s32 fn_aop_hal_get_mute(ot_audio_dev ao_dev_id, td_bool *mute, ot_audio_fade *fade);
typedef td_s32 fn_aop_hal_set_volume(ot_audio_dev ao_dev_id, td_s32 volumedb);
typedef td_s32 fn_aop_hal_get_volume(ot_audio_dev ao_dev_id, td_s32 *volumedb);
typedef td_s32 fn_aop_hal_get_tx_multislot(td_bool *multislot_enable);

typedef struct {
    fn_aip_hal_set_dev_attr *pfn_set_ai_attr;
    fn_aip_hal_dev_enable *pfn_enable_ai_dev;
    fn_aip_hal_dev_disable *pfn_disable_ai_dev;
    fn_aip_hal_set_buff_rptr *pfn_set_ai_dev_buf_rptr;
    fn_aip_hal_get_buff_rptr *pfn_get_ai_dev_buf_rptr;
    fn_aip_hal_set_buff_wptr *pfn_set_ai_dev_buf_wptr;
    fn_aip_hal_get_buff_wptr *pfn_get_ai_dev_buf_wptr;
    fn_aip_hal_set_buffer_addr *pfn_set_ai_dev_buf_addr;
    fn_aip_hal_set_buffer_size *pfn_set_ai_dev_buf_size;
    fn_aip_hal_set_trans_size *pfn_set_ai_dev_trans_size;
    fn_aip_hal_get_int_status *pfn_get_ai_int_status;
    fn_aip_hal_clr_int_status *pfn_clr_ai_int_status;
    fn_aip_hal_set_isr *pfn_set_ai_isr;
    fn_aip_hal_clr_attr *pfn_clr_ai_attr;
    fn_aip_hal_set_track_mode *pfn_set_ai_track_mode;
    fn_aip_hal_get_track_mode *pfn_get_ai_track_mode;
    fn_aip_hal_set_clk_dir *pfn_set_ai_clk_dir;
    fn_aip_hal_get_clk_dir *pfn_get_ai_clk_dir;
    fn_aip_hal_set_acodec_gain *pfn_set_acodec_gain;
    fn_aip_hal_get_acodec_gain *pfn_get_acodec_gain;
    fn_aip_hal_set_acodec_soft_diff *pfn_set_acodec_soft_diff;
    fn_aip_hal_get_acodec_soft_diff *pfn_get_acodec_soft_diff;

    fn_aop_hal_set_attr *pfn_set_ao_attr;
    fn_aop_hal_dev_enable *pfn_enable_ao_dev;
    fn_aop_hal_dev_disable *pfn_disable_ao_dev;
    fn_aop_hal_set_buff_wptr *pfn_set_ao_dev_buf_wptr;
    fn_aop_hal_get_buff_wptr *pfn_get_ao_dev_buf_wptr;
    fn_aop_hal_set_buff_rptr *pfn_set_ao_dev_buf_rptr;
    fn_aop_hal_get_buff_rptr *pfn_get_ao_dev_buf_rptr;
    fn_aop_hal_set_buffer_addr *pfn_set_ao_dev_buf_addr;
    fn_aop_hal_set_buffer_size *pfn_set_ao_dev_buf_size;
    fn_aop_hal_set_trans_size *pfn_set_ao_dev_trans_size;
    fn_aop_hal_get_int_status *pfn_get_ao_int_status;
    fn_aop_hal_clr_int_status *pfn_clr_ao_int_status;
    fn_aop_hal_set_isr *pfn_set_ao_isr;
    fn_aop_hal_clr_attr *pfn_clr_ao_attr;
    fn_aop_hal_set_track_mode *pfn_set_ao_track_mode;
    fn_aop_hal_get_track_mode *pfn_get_ao_track_mode;
    fn_aop_hal_set_clk_dir *pfn_set_ao_clk_dir;
    fn_aop_hal_get_clk_dir *pfn_get_ao_clk_dir;
    fn_aop_hal_set_mute *pfn_set_ao_mute;
    fn_aop_hal_get_mute *pfn_get_ao_mute;
    fn_aop_hal_set_volume *pfn_set_ao_volume;
    fn_aop_hal_get_volume *pfn_get_ao_volume;
    fn_aop_hal_get_tx_multislot *pfn_get_ao_tx_multislot;
} aio_export_func;

td_void *aiao_get_acodec_base(td_void);
td_void *aiao_get_aio_base(td_void);

#endif
