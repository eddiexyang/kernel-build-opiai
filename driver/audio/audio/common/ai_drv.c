/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: ai_drv.c
 * Author: Hisilicon multimedia software group
 * Create: 2012/10/20
 * History:
 *   1.Date        : 2012/10/20
 *     Modification: Created file
 */

#include "hi_osal.h"
#include "securec.h"

#include "hi_comm_audio.h"
#ifdef CONFIG_OT_PLATFORM_V8
#include "ot_inner_common_aio.h"
#else
#include "ot_comm_aio_adapt.h"
#endif
#include "ai_drv.h"
#include "proc_ext.h"

#include "mod_ext.h"
#include "aiao_ext.h"
#include "sys_ext.h"
#include "mm_ext.h"

aio_drv_dev_ctx g_ai_drv_dev[OT_AI_DEV_MAX_NUM];

static fn_ai_dma_callback g_pf_dma_callback;
static fn_ai_alsa_dma_callback g_pf_dma_ext_callback = TD_NULL;

aio_drv_dev_ctx *ai_drv_get_drv_dev_ctx(ot_audio_dev ai_dev)
{
    return &(g_ai_drv_dev[ai_dev]);
}

static td_s32 ai_get_quadratic_root(td_s32 len)
{
    if (len == 64) { /* 64: fifo len */
        return 6; /* 6: bytes */
    } else if (len == 48) { /* 48: fifo len */
        return 5; /* 5: bytes */
    } else if (len == 32) { /* 32: fifo len */
        return 5; /* 5: bytes */
    } else if (len == 16) { /* 16: fifo len */
        return 4; /* 4: bytes */
    } else if (len == 8) { /* 8: fifo len */
        return 3; /* 3: bytes */
    } else if (len == 4) { /* 4: fifo len */
        return 2; /* 2: bytes */
    } else if (len == 2) { /* 2: fifo len */
        return 1;
    } else {
        return -1;
    }
}

static __inline td_s8 clip_to_8bit(td_s16 x)
{
    td_s16 sign;

    /* clip to [-128, 127] */
    sign = x >> 15; /* 15: offset */
    if (sign != (x >> 7)) { /* 7: offset */
        x = sign ^ ((1 << 7) - 1); /* 7: offset */
    }

    return (td_s8)x;
}

static __inline td_s16 clip_to_16bit(td_s32 x)
{
    td_s32 sign;

    /* clip to [-32768, 32767] */
    sign = x >> 31; /* 31: offset */
    if (sign != (x >> 15)) { /* 15: offset */
        x = sign ^ ((1 << 15) - 1); /* 15: offset */
    }

    return (td_s16)x;
}

static td_u32 ai_drv_get_trans_chn_cnt(const aio_drv_dev_ctx *aio_drv_dev)
{
#ifdef CONFIG_AUDIO_V200_SUPPORT
    return ASP_SUPPORT_MAX_CHN_CNT;
#else
    td_u32 trans_chn_cnt = aio_drv_dev->aio_attr.chn_cnt; /* 总的通道数目 */

    if (aio_drv_dev->aio_attr.chn_cnt == 20) { /* 20: 通道数 */
        if (aio_drv_dev->aio_attr.bit_width == OT_AUDIO_BIT_WIDTH_8) {
            trans_chn_cnt = 32; /* 32: 256/8=32，使用256bit FIFO */
        } else if (aio_drv_dev->aio_attr.bit_width == OT_AUDIO_BIT_WIDTH_16) {
            trans_chn_cnt = 24; /* 24: 384/16=24，使用384bit FIFO */
        } else {
            /* need to modify when 24bit is used */
            ai_err_trace("ai_drv_get_trans_chn_cnt fail, bit_width: %d is invalid.\n",
                aio_drv_dev->aio_attr.bit_width);
        }
    } else {
        trans_chn_cnt = aio_drv_dev->aio_attr.chn_cnt;
    }

    return trans_chn_cnt;
#endif
}

/* 根据设备的属性初始化DMA buf的信息，并返回DMA buf的总长度 */
td_s32 ai_drv_dma_buf_info_init(ot_audio_dev audio_dev_id, td_u32 *dma_buf_bytes)
{
    td_u32 i;
    td_s32 fifo_len_base;
    td_u32 aio_fifo_len;
    aio_drv_dev_ctx *aio_drv_dev = &g_ai_drv_dev[audio_dev_id];
    td_u32 trach_chn_cnt; /* 总的通道数目 */
    td_u32 bit_cnt;
    td_s32 track_bit;

    trach_chn_cnt = ai_drv_get_trans_chn_cnt(aio_drv_dev);
    if (trach_chn_cnt == 0) {
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    /* 一个通道的位宽 */
    if ((aio_drv_dev->aio_attr.bit_width == OT_AUDIO_BIT_WIDTH_8) &&
        (aio_drv_dev->aio_attr.expand_flag == OT_AI_EXPAND)) {
        bit_cnt = aio_get_bit_cnt(OT_AUDIO_BIT_WIDTH_16);
    } else {
        bit_cnt = aio_get_bit_cnt(aio_drv_dev->aio_attr.bit_width);
    }

    /* 所有通道的总位宽 */
    track_bit = (td_s32)bit_cnt * trach_chn_cnt;

    aio_fifo_len = track_bit / 8; /* 8: 1字节的bit数 */

    /* 初始化通道在DMA BUF中的位置索引 */
    for (i = 0; i < aio_drv_dev->aio_attr.chn_cnt; i++) {
        g_ai_drv_dev[audio_dev_id].chn_index[i] = i;
    }

    /* 获取fifo位宽 */
    aio_drv_dev->aio_fifo_len = aio_fifo_len;
    fifo_len_base = ai_get_quadratic_root((td_s32)aio_fifo_len);
    if (fifo_len_base <= 0) {
        ai_err_trace("aio_get_quadratic_root fail, aio_fifo_len: %u is invalid\n", aio_fifo_len);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    aio_drv_dev->fifo_len_base = fifo_len_base;
    aio_drv_dev->trans_len = aio_drv_dev->aio_attr.point_num_per_frame * aio_drv_dev->aio_fifo_len;

    aio_drv_dev->cir_buf.rptr_off_set = 0;
    aio_drv_dev->cir_buf.wptr_off_set = 0;
    /*
     * buff满中断上报条件有原先的(3*128)bit变为((3+8)*128)bit，且dma搬移每次搬128_byte，
     * 因此buff长度应满足:首次上报完成中断时buff剩余空间不小于((3+8)*128)bit，
     * 否则声音异常。当buff长度为2帧时，场景为单通道80p时不满足此要求
     */
#ifdef CONFIG_AUDIO_V200_SUPPORT
    aio_drv_dev->cir_buf.buf_size = (aio_drv_dev->aio_fifo_len) *
                                    (aio_drv_dev->aio_attr.point_num_per_frame) * (2); /* 2: 16bit扩大为32bit, 3: 3frame */
#else
    aio_drv_dev->cir_buf.buf_size = (aio_drv_dev->aio_fifo_len) *
                                    (aio_drv_dev->aio_attr.point_num_per_frame) *
                                    (aio_drv_dev->aio_attr.chn_cnt) * (2) * (3); /* 2: 16bit扩大为32bit, 3: 3frame */
#endif
    /* 需要对buffer大小做128字节对齐 */
    if (aio_drv_dev->cir_buf.buf_size % 128 != 0) { /* 128: 128字节对齐 */
#ifndef CONFIG_AUDIO_V200_SUPPORT
        aio_drv_dev->cir_buf.buf_size = (aio_drv_dev->cir_buf.buf_size / 128 + 1) * 128; /* 128: 128字节对齐 */
#endif
    }

    *dma_buf_bytes = aio_drv_dev->cir_buf.buf_size;
    return TD_SUCCESS;
}

td_s32 ai_drv_set_dma_buf_addr(ot_audio_dev audio_dev_id, td_u64 dma_phy_addr, td_u8 *dma_vir_addr)
{
    aio_drv_dev_ctx *aio_drv_dev = &g_ai_drv_dev[audio_dev_id];

    /* init cir_buf addr */
    aio_drv_dev->cir_buf.phys_addr = dma_phy_addr;
    aio_drv_dev->cir_buf.vir_addr = dma_vir_addr;

    return TD_SUCCESS;
}

td_s32 ai_drv_clr_dma_buf_addr(ot_audio_dev audio_dev_id)
{
    aio_drv_dev_ctx *aio_drv_dev = &g_ai_drv_dev[audio_dev_id];

    /* clear cir_buf addr */
    aio_drv_dev->cir_buf.phys_addr = 0;
    aio_drv_dev->cir_buf.vir_addr = TD_NULL;

    return TD_SUCCESS;
}

td_void ai_drv_set_dma_isr(fn_ai_dma_callback pf_callback)
{
    g_pf_dma_callback = pf_callback;
}

#ifdef CONFIG_OT_AUDIO_ALSA_SUPPORT
/* 设置DMA外部回调，供ALSA使用 */
td_void ai_drv_set_dma_ext_isr(ot_audio_dev ai_dev, fn_ai_alsa_dma_callback pf_callback, td_void *substream)
{
    aio_drv_dev_ctx *aio_drv_dev = &g_ai_drv_dev[ai_dev];
    aio_drv_dev->alsa_substream = substream;
    g_pf_dma_ext_callback = pf_callback;
}

/* 复位DMA外部回调，供ALSA使用 */
td_void ai_drv_reset_dma_ext_isr(ot_audio_dev ai_dev)
{
    aio_drv_dev_ctx *aio_drv_dev = &g_ai_drv_dev[ai_dev];
    aio_drv_dev->alsa_substream = TD_NULL;
    g_pf_dma_ext_callback = TD_NULL;
}
#endif

/* 以导出接口的方式设置AIP设备的属性 */
td_s32 ai_drv_set_dev_attr(ot_audio_dev audio_dev_id, const ot_aio_attr *attr)
{
    td_s32 ret;

    aio_drv_dev_ctx *ai_drv_dev = &g_ai_drv_dev[audio_dev_id];
    aio_export_func *aio_exp_func = TD_NULL;

    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if ((aio_exp_func == TD_NULL) || (aio_exp_func->pfn_set_ai_attr == TD_NULL)) {
        ai_emerg_trace("ai_set_dev_attr fail, aio mode may not insmod\n");
        return OT_ERR_AI_NOT_READY;
    }

    ret = aio_exp_func->pfn_set_ai_attr(audio_dev_id, attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ai_drv_dev->track_mode = OT_AUDIO_TRACK_NORMAL;
    ai_drv_dev->mute = TD_FALSE;
    ai_drv_dev->fade.fade = TD_FALSE;
    ai_drv_dev->fade.fade_in_rate = OT_AUDIO_FADE_RATE_1;
    ai_drv_dev->fade.fade_out_rate = OT_AUDIO_FADE_RATE_1;
    ai_drv_dev->volume = 0;

    ret = memcpy_s(&ai_drv_dev->aio_attr, sizeof(ai_drv_dev->aio_attr), attr, sizeof(*attr));
    if (ret != EOK) {
        ai_err_trace("ai_dev %d attr memcpy_s fail, ret = 0x%x.\n", audio_dev_id, ret);
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

td_s32 ai_drv_clr_dev_attr(ot_audio_dev audio_dev_id)
{
    td_s32 ret;
    aio_export_func *aio_exp_func = TD_NULL;

    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if ((aio_exp_func == TD_NULL) || (aio_exp_func->pfn_clr_ai_attr == TD_NULL)) {
        ai_emerg_trace("ai_clr_cfg fail, aio mode may not insmod\n");
        return OT_ERR_AI_NOT_READY;
    }

    ret = aio_exp_func->pfn_clr_ai_attr(audio_dev_id);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

/* 所有芯片都使用该接口，启动DMA及设备 */
td_s32 ai_drv_enable_dev(ot_audio_dev audio_dev_id)
{
    aio_drv_dev_ctx *aio_drv_dev = &g_ai_drv_dev[audio_dev_id];
    aio_export_func *aio_exp_func = TD_NULL;

    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if ((aio_exp_func == TD_NULL) ||
        (aio_exp_func->pfn_set_ai_dev_buf_rptr == TD_NULL) ||
        (aio_exp_func->pfn_enable_ai_dev == TD_NULL)) {
        ai_emerg_trace("ai_drv_enable_dev fail, aio mode may not insmod\n");
        return OT_ERR_AI_NOT_READY;
    }

    /* 为传输完成中断设置上报中断的传输长度，即软件上所有通道一帧数据的长度 */
    aio_exp_func->pfn_set_ai_dev_buf_rptr(audio_dev_id, 0);

    /* 使能中断，清除原始中断，启动设备 */
    aio_exp_func->pfn_enable_ai_dev(audio_dev_id);

    aio_drv_dev->enable = TD_TRUE;

    return TD_SUCCESS;
}

td_void ai_drv_disable_dev(ot_audio_dev audio_dev_id)
{
    aio_export_func *aio_exp_func = TD_NULL;

    g_ai_drv_dev[audio_dev_id].enable = TD_FALSE;
    g_ai_drv_dev[audio_dev_id].int_cnt = 0;
    g_ai_drv_dev[audio_dev_id].u32fifo_int_cnt = 0;
    g_ai_drv_dev[audio_dev_id].u32buff_int_cnt = 0;
    g_ai_drv_dev[audio_dev_id].frm_time = 0;
    g_ai_drv_dev[audio_dev_id].max_frm_time = 0;

    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if ((aio_exp_func == TD_NULL) || (aio_exp_func->pfn_disable_ai_dev == TD_NULL)) {
        ai_emerg_trace("ai_drv_disable_dev fail, aio mode may not insmod\n");
        return;
    }

    /* 配置设备启动/停止寄存器，停止设备 */
    aio_exp_func->pfn_disable_ai_dev(audio_dev_id);
}

#ifdef CONFIG_OT_AUDIO_ALSA_SUPPORT
/* 单独设置DMA属性，供ALSA使用 */
td_s32 ai_drv_set_dma_attr(ot_audio_dev ai_dev, cir_buf cir_buf_attr, td_u32 trans_len)
{
    aio_export_func *aio_exp_func = TD_NULL;

    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if ((aio_exp_func == TD_NULL) ||
        (aio_exp_func->pfn_set_ai_dev_buf_rptr == TD_NULL) ||
        ai_emerg_trace("ai_drv_set_dma_attr fail, aio mode may not insmod\n");
        return OT_ERR_AI_NOT_READY;
    }

    aio_exp_func->pfn_set_ai_dev_buf_rptr(ai_dev, 0);

    return TD_SUCCESS;
}
#endif

static td_void ai_get_chn_data_8bit(td_u8 *dest_buf, const td_u8 *src_buf, td_u32 point_num,
    td_u32 base, td_s32 index, td_bool soft_diff)
{
    td_u32 k, excess_point;

    const td_s8 *tmp_buf = (td_s8 *)src_buf + index;
    td_s8 *data = (td_s8 *)dest_buf;

    const td_s8 *other_tmp_buf = (td_s8 *)src_buf + index + 1;

    if (point_num == 0) {
        /* 无须处理，直接返回 */
        return;
    }

    if (soft_diff == TD_TRUE) {
        if (index % 2) { /* 2: stereo */
            other_tmp_buf = (td_s8 *)src_buf + index - 1;
        }
    }

    excess_point = point_num % 4; /* 用于处理不能被4整除的采样点 */

    /* 先处理不能被4整除的采样点 */
    for (k = 0; k < excess_point; k++) {
        data[k] = (*(tmp_buf + (k << base)));

        if (soft_diff == TD_TRUE) {
            /* 软差分模式下需要减去相邻采样点 */
            data[k] = clip_to_8bit((td_s16)(data[k]) - (td_s16)(*(other_tmp_buf + (k << base))));
        }
    }

    /* 再处理能被4整除的采样点 */
    for (k = excess_point; k < point_num; k += 4) { /* 4: period */
        data[k] = (*(tmp_buf + (k << base)));
        data[k + 1] = (*(tmp_buf + ((k + 1) << base)));
        data[k + 2] = (*(tmp_buf + ((k + 2) << base))); /* 2: offset */
        data[k + 3] = (*(tmp_buf + ((k + 3) << base))); /* 3: offset */

        if (soft_diff == TD_TRUE) {
            /* 软差分模式下需要减去相邻采样点 */
            data[k] = clip_to_8bit((td_s16)(data[k]) - (td_s16)(*(other_tmp_buf + (k << base))));
            data[k + 1] = clip_to_8bit((td_s16)(data[k + 1]) -
                (td_s16)(*(other_tmp_buf + ((k + 1) << base))));
            data[k + 2] = clip_to_8bit((td_s16)(data[k + 2]) - /* 2: offset */
                (td_s16)(*(other_tmp_buf + ((k + 2) << base)))); /* 2: offset */
            data[k + 3] = clip_to_8bit((td_s16)(data[k + 3]) - /* 3: offset */
                (td_s16)(*(other_tmp_buf + ((k + 3) << base)))); /* 3: offset */
        }
    }
}

static td_void ai_get_chn_data_16bit(td_u16 *dest_buf, const td_u8 *src_buf, td_u32 point_num,
    td_u32 base, td_s32 index, td_bool soft_diff)
{
    td_u32 excess_point, k;
    ot_unused(soft_diff);
    const td_u8 *tmp_buf = src_buf + index * 4 + 2; /* 4: 硬件底层按照32bit传输，2: 每次取高16bit储存 */
    const td_u8 *other_tmp_buf = src_buf + index * 2 + 2; /* 2: 16bit使用2byte储存 */
    td_s16 *data = (td_s16 *)dest_buf;
    if (point_num == 0) {
        /* 无须处理，直接返回 */
        return;
    }

    excess_point = point_num % 4; /* 用于处理不能被4整除的采样点 */

    /* 先处理不能被4整除的采样点 */
    for (k = 0; k < excess_point; k++) {
        data[k] = (*(td_s16 *)(tmp_buf + (k << base)));
    }

    /* 再处理能被4整除的采样点 */
    for (k = excess_point; k < point_num; k += 4) { /* 4: period */
        data[k] = (*(td_s16 *)(tmp_buf + (k << base)));
        data[k + 1] = (*(td_s16 *)(tmp_buf + ((k + 1) << base)));
        data[k + 2] = (*(td_s16 *)(tmp_buf + ((k + 2) << base))); /* 2: offset */
        data[k + 3] = (*(td_s16 *)(tmp_buf + ((k + 3) << base))); /* 3: offset */
    }
}

static td_void ai_get_chn_data_16bit_24chn(td_u16 *dest_buf, const td_u8 *src_buf, td_u32 point_num,
    td_u32 base, td_s32 index, td_bool soft_diff)
{
    td_u32 k, excess_point;

    /* 24传输通道处理,每个采样点偏移24*16/2Byte,分解成32+16,((j<<base)+j<<4),base为5(2^5=32) */
    const td_u8 *tmp_buf = src_buf + index * 2; /* 2: 16bit使用2byte储存 */
    td_u16 *data = dest_buf;

    const td_u8 *other_tmp_buf = src_buf + index * 2 + 2; /* 2: 16bit使用2byte储存 */

    if (point_num == 0) {
        /* 无须处理，直接返回 */
        return;
    }

    if (soft_diff == TD_TRUE) {
        if (index % 2) { /* 2: stereo */
            other_tmp_buf = src_buf + index * 2 - 2; /* 2: 16bit使用2byte储存 */
        }
    }

    excess_point = point_num % 4; /* 用于处理不能被4整除的采样点 */

    /* 先处理不能被4整除的采样点 */
    for (k = 0; k < excess_point; k++) {
        data[k] = (*(td_u16 *)(tmp_buf + (k << base) + (k << 4))); /* 4: offset */

        if (soft_diff == TD_TRUE) {
            /* 软差分模式下需要减去相邻采样点 */
            data[k] -= (*(td_u16 *)(other_tmp_buf + (k << base) + (k << 4))); /* 4: offset */
        }
    }

    /* 再处理能被4整除的采样点 */
    for (k = excess_point; k < point_num; k += 4) { /* 4: period */
        data[k] = (*(td_u16 *)(tmp_buf + (k << base) + (k << 4))); /* 4: offset */
        data[k + 1] = (*(td_u16 *)(tmp_buf + ((k + 1) << base) + ((k + 1) << 4))); /* 4: offset */
        data[k + 2] = (*(td_u16 *)(tmp_buf + ((k + 2) << base) + ((k + 2) << 4))); /* 2,4: offset */
        data[k + 3] = (*(td_u16 *)(tmp_buf + ((k + 3) << base) + ((k + 3) << 4))); /* 3,4: offset */

        if (soft_diff == TD_TRUE) {
            /* 软差分模式下需要减去相邻采样点 */
            data[k] -= (*(td_u16 *)(other_tmp_buf + (k << base) + (k << 4))); /* 4: offset */
            data[k + 1] -= (*(td_u16 *)(other_tmp_buf + ((k + 1) << base) + ((k + 1) << 4))); /* 4: offset */
            data[k + 2] -= (*(td_u16 *)(other_tmp_buf + ((k + 2) << base) + ((k + 2) << 4))); /* 2,4: offset */
            data[k + 3] -= (*(td_u16 *)(other_tmp_buf + ((k + 3) << base) + ((k + 3) << 4))); /* 3,4: offset */
        }
    }
}

static td_void ai_get_chn_data_24bit(td_u32 *dest_buf, const td_u8 *src_buf, td_u32 point_num,
    td_u32 base, td_s32 index, td_bool soft_diff)
{
    td_u32 k, j, tmp;
    ot_unused(soft_diff);
    const td_u8 *tmp_buf = src_buf + index * 4; /* 4: 24bit使用4byte储存 */
    td_u8 *data = (td_u8 *)dest_buf;

    const td_u8 *other_tmp_buf = src_buf + index * 4 + 4; /* 4: 24bit使用4byte储存 */

    if (point_num == 0) {
        /* 无须处理，直接返回 */
        return;
    }

    /* 再处理能被4整除的采样点 */
    for (k = 0, j = 0; k < point_num; k++, j += 3) { /* 3: period */
        tmp = (*(td_u32 *)(tmp_buf + (k  << base)));
        data[j] = (tmp >> 8) & 0xff;  // 8: bits
        data[j + 1] = (tmp >> 16) & 0xff;  // 16 bits
        data[j + 2] = (tmp >> 24) & 0xff;  // 24 bits
    }
}

static td_u32 ai_cir_buf_get_spare_point_num(const aio_drv_dev_ctx *ai_drv_dev)
{
    td_u32 bit_cnt, trans_chn_cnt, len, point_num_to_end;

    if ((ai_drv_dev->aio_attr.bit_width == OT_AUDIO_BIT_WIDTH_8) &&
        (ai_drv_dev->aio_attr.expand_flag == OT_AI_EXPAND)) {
        bit_cnt = aio_get_bit_cnt(OT_AUDIO_BIT_WIDTH_16);
    } else {
        bit_cnt = aio_get_bit_cnt(ai_drv_dev->aio_attr.bit_width);
    }

    /* 获取传输的通道数 */
    trans_chn_cnt = ai_drv_get_trans_chn_cnt(ai_drv_dev);

    /* 计算当前读指针到循环buffer结尾的长度 */
    len = ai_drv_dev->cir_buf.buf_size - ai_drv_dev->cir_buf.rptr_off_set;
    point_num_to_end = (len * 8) / (bit_cnt * trans_chn_cnt); /* 8: 1Byte的bit数 */
    if (point_num_to_end > ai_drv_dev->aio_attr.point_num_per_frame) {
        point_num_to_end = ai_drv_dev->aio_attr.point_num_per_frame;
    }

    return point_num_to_end;
}

static td_s32 ai_update_buf_rptr_after_read(ot_audio_dev ai_dev_id)
{
    td_u32 len;
    aio_export_func *aio_exp_func = TD_NULL;
    aio_drv_dev_ctx *ai_drv_dev = &g_ai_drv_dev[ai_dev_id];

    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if ((aio_exp_func == TD_NULL) || (aio_exp_func->pfn_set_ai_dev_buf_rptr == TD_NULL)) {
        ai_err_trace("ai_get_chn_data fail, aio mode may not insmod\n");
        return OT_ERR_AI_NOT_READY;
    }

    /* 计算当前读指针到循环buffer结尾的长度 */
    len = ai_drv_dev->cir_buf.buf_size - ai_drv_dev->cir_buf.rptr_off_set;

    /* 更新读指针，并更新读地址寄存器中的读指针 */
    if ((len != 0) && (len < ai_drv_dev->trans_len)) {
        ai_drv_dev->cir_buf.rptr_off_set = ai_drv_dev->trans_len - len;
    } else {
        ai_drv_dev->cir_buf.rptr_off_set += ai_drv_dev->trans_len;
    }
    if (ai_drv_dev->cir_buf.rptr_off_set == ai_drv_dev->cir_buf.buf_size) {
        ai_drv_dev->cir_buf.rptr_off_set = 0;
    }

    /* 更新读地址寄存器中的读指针偏移量 */
    aio_exp_func->pfn_set_ai_dev_buf_rptr(ai_dev_id,
                                          (ai_drv_dev->cir_buf.rptr_off_set & (~(16 - 1)))); /* 16: free>=32byte */

    return TD_SUCCESS;
}

static td_s32 ai_drv_get_buf_info(aio_drv_dev_ctx *ai_drv_dev, td_u8 **dma_buf,
    td_u8 **start_buf, td_bool *soft_diff, td_u32 *trans_chn_cnt)
{
    aio_export_func *aio_exp_func = TD_NULL;

    if (ai_drv_dev->cir_buf.rptr_off_set == ai_drv_dev->cir_buf.buf_size) {
        ai_drv_dev->cir_buf.rptr_off_set = 0;
    }

    if (ai_drv_dev->cir_buf.vir_addr == TD_NULL) {
        ai_err_trace("AI CB buffer is NULL\n");
        return OT_ERR_AI_NOT_ENABLED;
    }

    *dma_buf = ai_drv_dev->cir_buf.vir_addr + ai_drv_dev->cir_buf.rptr_off_set;
    if (*dma_buf > ai_drv_dev->cir_buf.vir_addr + ai_drv_dev->cir_buf.buf_size) {
        ai_err_trace("buffer addr is illegal!\n");
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    *start_buf = ai_drv_dev->cir_buf.vir_addr;
    if (*start_buf == TD_NULL) {
        ai_err_trace("AI not init(CB buffer is NULL)\n");
        return OT_ERR_AI_NOT_ENABLED;
    }

    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if ((aio_exp_func == TD_NULL) || (aio_exp_func->pfn_get_acodec_soft_diff == TD_NULL)) {
        ai_err_trace("ai_get_chn_data fail, aio mode may not insmod\n");
        return OT_ERR_AI_NOT_READY;
    }

    /* 获取是否需要进行软件差分处理，目前仅HI3516CV500使用，并且HI3516CV500只有一个AIP0，如要支持多AIP则须重构代码 */
    aio_exp_func->pfn_get_acodec_soft_diff(soft_diff);

    /* 获取传输的通道数 */
    *trans_chn_cnt = ai_drv_get_trans_chn_cnt(ai_drv_dev);

    return TD_SUCCESS;
}

td_s32 ai_drv_get_chn_data(ot_audio_dev audio_dev_id, aio_chn_data_addr *chn_data_addr, td_u32 chn_cnt)
{
    td_s32 index, ret;
    td_u8 *dma_buf = TD_NULL;
    td_u8 *start_buf = TD_NULL;
    td_u32 i, pt_num_to_end, trans_chn_cnt;
    aio_drv_dev_ctx *aio_drv_dev = &g_ai_drv_dev[audio_dev_id];
    td_u32 base = aio_drv_dev->fifo_len_base;
    td_u32 point_num = aio_drv_dev->aio_attr.point_num_per_frame;
    td_u8 *data = TD_NULL;
    td_bool soft_diff = TD_FALSE;

    /* 获取dma_buf、start_buf、软差分、传输通道数信息 */
    ret = ai_drv_get_buf_info(aio_drv_dev, &dma_buf, &start_buf, &soft_diff, &trans_chn_cnt);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    /* 计算当前读指针到循环buffer结尾的长度 */
    pt_num_to_end = ai_cir_buf_get_spare_point_num(aio_drv_dev);

    for (i = 0; i < chn_cnt; ++i) {
        if (chn_data_addr[i].valid == TD_FALSE) {
            continue;
        }
        index = aio_drv_dev->chn_index[i];
        data = (td_u8 *)chn_data_addr[i].virt_addr;
        if ((aio_drv_dev->aio_attr.bit_width == OT_AUDIO_BIT_WIDTH_8) && (aio_drv_dev->aio_attr.expand_flag == 0)) {
            ai_get_chn_data_8bit(data, dma_buf, pt_num_to_end, base, index, soft_diff);

            /* 必要时处理需要折回的情况 */
            ai_get_chn_data_8bit(data + pt_num_to_end, start_buf, point_num - pt_num_to_end,
                base, index, soft_diff);
        } else if (((aio_drv_dev->aio_attr.bit_width == OT_AUDIO_BIT_WIDTH_8) &&
                   (aio_drv_dev->aio_attr.expand_flag == OT_AI_EXPAND)) ||
                   (aio_drv_dev->aio_attr.bit_width == OT_AUDIO_BIT_WIDTH_16)) {
            if (trans_chn_cnt != 24) { /* 24: 20chn时使用384bit FIFO */
                /* 非24传输通道处理 */
                ai_get_chn_data_16bit((td_u16 *)data, dma_buf, pt_num_to_end, base, index, soft_diff);
#ifndef CONFIG_AUDIO_V200_SUPPORT
                /* 必要时处理需要折回的情况 */
                ai_get_chn_data_16bit((td_u16 *)data + pt_num_to_end, start_buf, point_num - pt_num_to_end,
                    base, index, soft_diff);
#endif
            } else {
                /* 24传输通道处理 */
                ai_get_chn_data_16bit_24chn((td_u16 *)data, dma_buf, pt_num_to_end, base, index, soft_diff);

                /* 必要时处理需要折回的情况 */
                ai_get_chn_data_16bit_24chn((td_u16 *)data + pt_num_to_end, start_buf, point_num - pt_num_to_end,
                    base, index, soft_diff);
            }
        } else if (aio_drv_dev->aio_attr.bit_width == OT_AUDIO_BIT_WIDTH_24) {
            ai_get_chn_data_24bit((td_u32 *)data, dma_buf, pt_num_to_end, base, index, soft_diff);

            /* 必要时处理需要折回的情况 */
            ai_get_chn_data_24bit((td_u32 *)data + pt_num_to_end, start_buf, point_num - pt_num_to_end,
                base, index, soft_diff);
        } else {
            ai_info_trace("bit_width error\n");
            return OT_ERR_AI_ILLEGAL_PARAM;
        }
    }

    /* 更新读指针，并更新读地址寄存器中的读指针 */
    return ai_update_buf_rptr_after_read(audio_dev_id);
}

#ifndef CONFIG_AUDIO_V200_SUPPORT
static td_void ai_drv_isr_buf_full_process(ot_audio_dev dev_id, const aio_export_func *aio_exp_func,
                                           aio_drv_dev_ctx *aio_drv_dev)
{
    td_u32 rptr_off_set, len;

    /* 异常中断处理，将读指针更新，丢掉一帧 */
    rptr_off_set = aio_exp_func->pfn_get_ai_dev_buf_rptr(dev_id);
    len = aio_drv_dev->cir_buf.buf_size - rptr_off_set;
    if ((len != 0) && (len < aio_drv_dev->trans_len)) {
        rptr_off_set = aio_drv_dev->trans_len - len;
    } else {
        rptr_off_set += aio_drv_dev->trans_len;
    }
    if (rptr_off_set == aio_drv_dev->cir_buf.buf_size) {
        rptr_off_set = 0;
    }

    aio_exp_func->pfn_set_ai_dev_buf_rptr(dev_id, (rptr_off_set & (~(16 - 1)))); /* 16: 1字节对齐 */

    /* 更新软件记录的读指针，与硬件寄存器中的读指针保持一致，用于异常中断处理 */
    aio_drv_dev->cir_buf.rptr_off_set = aio_exp_func->pfn_get_ai_dev_buf_rptr(dev_id);
}
#endif

static td_void ai_drv_isr_core(ot_audio_dev ai_dev, aio_drv_dev_ctx *ai_drv_ctx)
{
    if (g_pf_dma_ext_callback == TD_NULL) {
        if (g_pf_dma_callback == TD_NULL) {
            ai_err_trace("g_pf_dma_callback is null\n");
        } else {
            g_pf_dma_callback(ai_dev);
        }
    } else {
        g_pf_dma_ext_callback(ai_drv_ctx->alsa_substream); /* update read in alsa dma isr */
    }
}

static void ai_drv_isr(ot_audio_dev audio_dev_id)
{
    td_u32 int_status;
    struct osal_timeval time1, time2;
    td_u64 pts;
    aio_export_func *aio_exp_func = TD_NULL;
    aio_drv_dev_ctx *aio_drv_dev = &g_ai_drv_dev[audio_dev_id];
#ifndef CONFIG_AUDIO_V200_SUPPORT
    /* get int status */
    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if ((aio_exp_func == TD_NULL) || (aio_exp_func->pfn_get_ai_int_status == TD_NULL) ||
        (aio_exp_func->pfn_clr_ai_int_status == TD_NULL) || (aio_exp_func->pfn_get_ai_dev_buf_rptr == TD_NULL) ||
        (aio_exp_func->pfn_set_ai_dev_buf_rptr == TD_NULL)) {
        ai_emerg_trace("ai_drv_isr fail, aio module may not insmod\n");
        return;
    }
    (td_void)aio_exp_func->pfn_get_ai_int_status(audio_dev_id, &int_status);

    if (int_status & 0x08) { /* bit[3], fifo over */
        aio_drv_dev->u32fifo_int_cnt++;
        ai_err_trace("AI dev%d int status : 0x%x, fifo over cnt:%d \n",
                     audio_dev_id, int_status, aio_drv_dev->u32fifo_int_cnt);
    }
    if (int_status & 0x02) { /* bit[1]，buffer is full */
        aio_drv_dev->u32buff_int_cnt++;

        /* 异常中断处理，将读指针更新，丢掉一帧 */
        ai_drv_isr_buf_full_process(audio_dev_id, aio_exp_func, aio_drv_dev);

        ai_err_trace("AI dev%d int status : 0x%x, buffer full cnt:%u \n",
                     audio_dev_id, int_status, aio_drv_dev->u32buff_int_cnt);
    }

    (td_void)aio_exp_func->pfn_clr_ai_int_status(audio_dev_id);

    if (int_status & 0x01) { /* bit[0]，transfer finish */
        aio_drv_dev->int_cnt++;

        /* clac frame time */
        if ((ckfn_sys_entry() == TD_TRUE) && (ckfn_sys_get_time_stamp() == TD_TRUE)) {
            pts = call_sys_get_time_stamp();
            aio_drv_dev->frm_time = (td_u32)(pts - aio_drv_dev->last_pts);
            aio_drv_dev->last_pts = pts;
        }

        /* get max frame time */
        if ((aio_drv_dev->int_cnt != 1) && (aio_drv_dev->frm_time > aio_drv_dev->max_frm_time)) {
            aio_drv_dev->max_frm_time = aio_drv_dev->frm_time;
        }

        osal_gettimeofday(&time1);

        ai_drv_isr_core(audio_dev_id, aio_drv_dev);

        osal_gettimeofday(&time2);
        aio_drv_dev->isr_time = (td_u32)aio_time_diff_us(time2, time1);

        /* 获取最长的中断时间 */
        if (aio_drv_dev->isr_time > aio_drv_dev->max_isr_time) {
            aio_drv_dev->max_isr_time = aio_drv_dev->isr_time;
        }
    }
#endif
    osal_gettimeofday(&time1);

    ai_drv_isr_core(audio_dev_id, aio_drv_dev);

    osal_gettimeofday(&time2);
    aio_drv_dev->isr_time = (td_u32)aio_time_diff_us(time2, time1);

    /* 获取最长的中断时间 */
    if (aio_drv_dev->isr_time > aio_drv_dev->max_isr_time) {
        aio_drv_dev->max_isr_time = aio_drv_dev->isr_time;
    }
}

td_s32 ai_drv_dev_init(ot_audio_dev audio_dev_id)
{
    aio_export_func *aio_exp_func = TD_NULL;
    aio_drv_dev_ctx *aio_drv_dev = &g_ai_drv_dev[audio_dev_id];

    (td_void)memset_s(aio_drv_dev, sizeof(aio_drv_dev_ctx), 0, sizeof(aio_drv_dev_ctx));
    aio_drv_dev->enable = TD_FALSE;

    /* 注册DMA传输完成中断函数 */
    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if ((aio_exp_func == TD_NULL) || (aio_exp_func->pfn_set_ai_isr == TD_NULL)) {
        ai_emerg_trace("aio module may not insmod\n");
        return OT_ERR_AI_NOT_READY;
    }
    aio_exp_func->pfn_set_ai_isr(ai_drv_isr);

    return TD_SUCCESS;
}

td_s32 ai_drv_dev_deinit(ot_audio_dev audio_dev_id)
{
    aio_drv_dev_ctx *aio_drv_dev = &g_ai_drv_dev[audio_dev_id];

    aio_drv_dev->enable = TD_FALSE;
    return TD_SUCCESS;
}

td_s32 ai_drv_check_aio_export_func(td_void)
{
    aio_export_func *aio_exp_func = TD_NULL;
    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if (aio_exp_func == TD_NULL) {
        ai_emerg_trace("aio module may not insmod\n");
        return OT_ERR_AI_NOT_READY;
    }

    return TD_SUCCESS;
}

td_s32 ai_drv_set_track_mode(ot_audio_dev audio_dev_id, ot_audio_track_mode track_mode)
{
    aio_export_func *aio_exp_func = TD_NULL;
    aio_drv_dev_ctx *aio_drv_dev = TD_NULL;

    aio_drv_dev = &g_ai_drv_dev[audio_dev_id];

    if ((aio_drv_dev->aio_attr.work_mode != OT_AIO_MODE_I2S_SLAVE) &&
        (aio_drv_dev->aio_attr.work_mode != OT_AIO_MODE_I2S_MASTER)) {
        ai_err_trace("only i2s work_mode support trackmode\n");
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if ((aio_exp_func == TD_NULL)) {
        ai_emerg_trace("aio module may not insmod\n");
        return OT_ERR_AI_NOT_READY;
    }

    return TD_SUCCESS;
}

td_s32 ai_drv_get_track_mode(ot_audio_dev audio_dev_id, ot_audio_track_mode *track_mode)
{
    aio_export_func *aio_exp_func = TD_NULL;
    aio_drv_dev_ctx *aio_drv_dev = TD_NULL;
    ot_audio_track_mode tmp_track = OT_AUDIO_TRACK_NORMAL;

    aio_drv_dev = &g_ai_drv_dev[audio_dev_id];

    if ((aio_drv_dev->aio_attr.work_mode != OT_AIO_MODE_I2S_MASTER) &&
        (aio_drv_dev->aio_attr.work_mode != OT_AIO_MODE_I2S_SLAVE)) {
        ai_err_trace("only i2s work_mode support trackmode\n");
        return OT_ERR_AI_ILLEGAL_PARAM;
    }
    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if (aio_exp_func == TD_NULL) {
        ai_emerg_trace("aio module may not insmod\n");
        return OT_ERR_AI_NOT_READY;
    }

    *track_mode = tmp_track;

    return TD_SUCCESS;
}

td_s32 ai_drv_set_clk_dir(ot_audio_dev audio_dev_id, audio_clkdir clk_dir)
{
    aio_export_func *aio_exp_func = TD_NULL;
    aio_drv_dev_ctx *aio_drv_dev = TD_NULL;

    aio_drv_dev = &g_ai_drv_dev[audio_dev_id];

    aio_exp_func = (aio_export_func *)cmpi_get_module_func_by_id(OT_ID_AIO);
    if (aio_exp_func == TD_NULL) {
        ai_emerg_trace("aio module may not insmod\n");
        return OT_ERR_AI_NOT_READY;
    }

    return TD_SUCCESS;
}

td_s32 ai_drv_get_clk_dir(ot_audio_dev audio_dev_id, audio_clkdir *clk_dir)
{
    aio_export_func *aio_exp_func = TD_NULL;

    aio_exp_func = (aio_export_func *)cmpi_get_module_func_by_id(OT_ID_AIO);
    if (aio_exp_func == TD_NULL) {
        ai_emerg_trace("aio module may not insmod\n");
        return OT_ERR_AI_NOT_READY;
    }
    return TD_SUCCESS;
}

td_s32 ai_drv_set_acodec_gain(td_s32 gain)
{
    aio_export_func *aio_exp_func = TD_NULL;

    aio_exp_func = (aio_export_func *)cmpi_get_module_func_by_id(OT_ID_AIO);
    if (aio_exp_func == TD_NULL) {
        ai_emerg_trace("aio module may not insmod\n");
        return OT_ERR_AI_NOT_READY;
    }

    if (aio_exp_func->pfn_set_acodec_gain == TD_NULL) {
        ai_err_trace("aio module has no pfn_set_acodec_gain\n");
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    return aio_exp_func->pfn_set_acodec_gain(gain);
}

td_s32 ai_drv_get_acodec_gain(td_s32 *gain)
{
    aio_export_func *aio_exp_func = TD_NULL;

    aio_exp_func = (aio_export_func *)cmpi_get_module_func_by_id(OT_ID_AIO);
    if (aio_exp_func == TD_NULL) {
        ai_emerg_trace("aio module may not insmod\n");
        return OT_ERR_AI_NOT_READY;
    }

    if (aio_exp_func->pfn_get_acodec_gain == TD_NULL) {
        ai_err_trace("aio module has no pfn_get_acodec_gain\n");
        return OT_ERR_AI_ILLEGAL_PARAM;
    }

    return aio_exp_func->pfn_get_acodec_gain(gain);
}

