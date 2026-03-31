/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: ao_drv.c
 * Author: Hisilicon multimedia software group
 * Create: 2009/5/5
 * History:
 *   1.Date        : 2009/5/5
 *     Modification: Created file
 */

#include "hi_osal.h"
#include "securec.h"

#include "hi_comm_audio.h"
#include "ao_drv.h"
#include "proc_ext.h"
#include "ao_ext.h"

#include "mod_ext.h"
#include "aiao_ext.h"
#include "sys_ext.h"
#include "mm_ext.h"
#include "osal_mmz.h"
#include "linux/interrupt.h"
#ifdef CONFIG_OT_PLATFORM_V8
#include "ot_inner_common_aio.h"
#else
#include "ot_comm_aio_adapt.h"
#endif

#define DATA_SIZE 7681

static fn_ao_dma_callback g_pf_dma_callback;
static fn_ao_alsa_dma_callback g_pf_dma_ext_callback = TD_NULL;
aio_drv_dev_ctx g_ao_drv_dev[OT_AO_DEV_MAX_NUM];

ot_ao_export_callback g_ao_export_callback = { 0 };

ot_ao_export_callback *ao_drv_get_export_callback(td_void)
{
    return &g_ao_export_callback;
}

td_void ao_drv_notify_user(ot_audio_dev audio_dev_id)
{
    if (g_ao_export_callback.ao_notify != TD_NULL) {
        /* AO DMA回调 */
        g_ao_export_callback.ao_notify(audio_dev_id);
    }
}

aio_drv_dev_ctx *ao_drv_get_drv_dev_ctx(ot_audio_dev ao_dev)
{
    return &(g_ao_drv_dev[ao_dev]);
}

static td_s32 ao_get_quadratic_root(td_s32 a)
{
    if (a == 64) { /* 64: fifo length */
        return 6; /* 6: bytes */
    } else if (a == 48) { /* 48: fifo length */
        return 5; /* 5: bytes */
    } else if (a == 32) { /* 32: fifo length */
        return 5; /* 5: bytes */
    } else if (a == 16) { /* 16: fifo length */
        return 4; /* 4: bytes */
    } else if (a == 8) { /* 8: fifo length */
        return 3; /* 3: bytes */
    } else if (a == 4) { /* 4: fifo length */
        return 2; /* 2: bytes */
    } else if (a == 2) { /* 2: fifo length */
        return 1;
    } else {
        return -1;
    }
}

static td_void ao_put_same_data_8bit(td_u8 *tmp_buf, td_u32 point_num, td_u32 base, td_u8 data)
{
    td_u32 k, excess_point;

    if (point_num == 0) {
        /* 无须处理，直接返回 */
        return;
    }

    excess_point = point_num % 4; /* 用于处理不能被4整除的采样点 */

    /* 先处理不能被4整除的采样点 */
    for (k = 0; k < excess_point; k++) {
        *(tmp_buf + (k << base)) = data;
    }

    /* 再处理能被4整除的采样点 */
    for (k = excess_point; k < point_num; k += 4) { /* 4: period */
        *(tmp_buf + ((k + 0) << base)) = data;
        *(tmp_buf + ((k + 1) << base)) = data;
        *(tmp_buf + ((k + 2) << base)) = data; /* 2: offset */
        *(tmp_buf + ((k + 3) << base)) = data; /* 3: offset */
    }
}

static td_void ao_put_same_data_16bit(td_u8 *tmp_buf, td_u32 point_num, td_u32 base, td_u16 data)
{
    td_u32 k, excess_point;

    if (point_num == 0) {
        /* 无须处理，直接返回 */
        return;
    }

    excess_point = point_num % 4; /* 用于处理不能被4整除的采样点 */

    /* 先处理不能被4整除的采样点 */
    for (k = 0; k < excess_point; k++) {
        *(td_u16 *)(td_void *)(tmp_buf + (k << base)) = data;
    }

    /* 再处理能被4整除的采样点 */
    for (k = excess_point; k < point_num; k += 4) { /* 4: period */
        *(td_u16 *)(tmp_buf + ((k + 0) << base)) = data;
        *(td_u16 *)(tmp_buf + ((k + 1) << base)) = data;
        *(td_u16 *)(tmp_buf + ((k + 2) << base)) = data; /* 2: offset */
        *(td_u16 *)(tmp_buf + ((k + 3) << base)) = data; /* 3: offset */
    }
}

static td_void ao_put_same_data_24bit(td_u8 *tmp_buf, td_u32 point_num, td_u32 base, td_u32 data)
{
    td_u32 k, excess_point;

    if (point_num == 0) {
        /* 无须处理，直接返回 */
        return;
    }

    excess_point = point_num % 4; /* 用于处理不能被4整除的采样点 */

    /* 先处理不能被4整除的采样点 */
    for (k = 0; k < excess_point; k++) {
        *(td_u32 *)(tmp_buf + (k << base)) = data;
    }

    /* 再处理能被4整除的采样点 */
    for (k = excess_point; k < point_num; k += 4) { /* 4: period */
        *(td_u32 *)(td_void *)(tmp_buf + ((k + 0) << base)) = data;
        *(td_u32 *)(td_void *)(tmp_buf + ((k + 1) << base)) = data;
        *(td_u32 *)(td_void *)(tmp_buf + ((k + 2) << base)) = data; /* 2: offset */
        *(td_u32 *)(td_void *)(tmp_buf + ((k + 3) << base)) = data; /* 3: offset */
    }
}

static td_s32 check_dma_buf(const td_u8 *dma_buf, const aio_drv_dev_ctx *aio_drv_dev)
{
    if (dma_buf == TD_NULL) {
        ao_err_trace("AO not init(CB buffer is NULL)\n");
        return OT_ERR_AO_NOT_ENABLED;
    }

    if (dma_buf > aio_drv_dev->cir_buf.vir_addr + aio_drv_dev->cir_buf.buf_size) {
        ao_err_trace("buffer addr is illegal!\n");
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_u32 ao_cir_buf_get_spare_point_num(const aio_drv_dev_ctx *ao_drv_dev)
{
    td_u32 bit_width = (td_u32)ao_drv_dev->aio_attr.bit_width;
    td_u32 len, point_num_to_end;

    len = ao_drv_dev->cir_buf.buf_size - ao_drv_dev->cir_buf.wptr_off_set;
    point_num_to_end = (len * 8U) / ((8U << bit_width) * ao_drv_dev->aio_attr.chn_cnt); /* 8: 1字节的bit数 */
    if (point_num_to_end > ao_drv_dev->aio_attr.point_num_per_frame) {
        point_num_to_end = ao_drv_dev->aio_attr.point_num_per_frame;
    }

    return point_num_to_end;
}

td_s32 ao_drv_put_zero_data(ot_audio_dev audio_dev_id, td_u32 ao_chn)
{
    td_s32 index, ret;
    aio_drv_dev_ctx *aio_drv_dev = &g_ao_drv_dev[audio_dev_id];
    td_u32 bit_width = (td_u32)aio_drv_dev->aio_attr.bit_width;
    td_u32 point_num = aio_drv_dev->aio_attr.point_num_per_frame;
    td_u32 base = aio_drv_dev->fifo_len_base;
    td_u32 point_num_to_end;
    td_u8 *dma_buf = aio_drv_dev->cir_buf.vir_addr + aio_drv_dev->cir_buf.wptr_off_set;
    td_u8 *tmp_buf = TD_NULL;

    ret = check_dma_buf(dma_buf, aio_drv_dev);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    /* 获取环形buf末端的空闲数据点数 */
    point_num_to_end = ao_cir_buf_get_spare_point_num(aio_drv_dev);

    index = aio_drv_dev->chn_index[ao_chn];

    switch (bit_width) {
        case OT_AUDIO_BIT_WIDTH_8:
            tmp_buf = dma_buf + index;
            ao_put_same_data_8bit(tmp_buf, point_num_to_end, base, 0);

            /* 必要时处理需要折回的情况 */
            tmp_buf = aio_drv_dev->cir_buf.vir_addr + index;
            ao_put_same_data_8bit(tmp_buf, point_num - point_num_to_end, base, 0);
            break;

        case OT_AUDIO_BIT_WIDTH_16:
            tmp_buf = dma_buf + (2 * (td_slong)index); /* 2: 16bit使用2byte储存 */
            ao_put_same_data_16bit(tmp_buf, point_num_to_end, base, 0);

            /* 必要时处理需要折回的情况 */
            tmp_buf = aio_drv_dev->cir_buf.vir_addr + (2 * (td_slong)index); /* 2: 16bit使用2byte储存 */
            ao_put_same_data_16bit(tmp_buf, point_num - point_num_to_end, base, 0);
            break;

        case OT_AUDIO_BIT_WIDTH_24:
            tmp_buf = dma_buf + (4 * (td_slong)index); /* 4: 24bit使用4byte储存 */
            ao_put_same_data_24bit(tmp_buf, point_num_to_end, base, 0);

            /* 必要时处理需要折回的情况 */
            tmp_buf = aio_drv_dev->cir_buf.vir_addr + (4 * (td_slong)index); /* 4: 24bit使用4byte储存 */
            ao_put_same_data_24bit(tmp_buf, point_num - point_num_to_end, base, 0);
            break;

        default:
            ao_info_trace("bit_width error\n");
            return OT_ERR_AO_ILLEGAL_PARAM;
    }
    return TD_SUCCESS;
}

static td_s32 ao_get_last_data(ot_audio_dev dev_id, td_u32 ao_chn, td_u8 **last_buf, td_u32 *end_point_num)
{
    aio_drv_dev_ctx *aio_drv_dev = &g_ao_drv_dev[dev_id];
    td_u32 bit_width = (td_u32)aio_drv_dev->aio_attr.bit_width;
    td_u32 point_num = aio_drv_dev->aio_attr.point_num_per_frame;
    td_u32 len;
    td_u32 last_point_num_to_end;
    td_u32 last_wptr_offset;
    td_u32 bytes_per_sample = 0;
    td_s32 index = aio_drv_dev->chn_index[ao_chn];

    /* 上一帧数据的策略，来自当前写指针的前一帧数据，将当前写指针后退一帧数据长度，作为上一阵数据的起始地址 */
    if (aio_drv_dev->cir_buf.wptr_off_set >= aio_drv_dev->trans_len) {
        last_wptr_offset = aio_drv_dev->cir_buf.wptr_off_set - aio_drv_dev->trans_len;
    } else {
        last_wptr_offset = aio_drv_dev->cir_buf.buf_size -
            (aio_drv_dev->trans_len - aio_drv_dev->cir_buf.wptr_off_set);
    }

    len = aio_drv_dev->cir_buf.buf_size - last_wptr_offset;
    last_point_num_to_end = (len * 8) / ((8U << bit_width) * aio_drv_dev->aio_attr.chn_cnt); /* 8: 1Byte的bit数 */
    if (last_point_num_to_end > point_num) {
        last_point_num_to_end = point_num;
    }

    switch (bit_width) {
        case OT_AUDIO_BIT_WIDTH_8:
            bytes_per_sample = 1;
            break;
        case OT_AUDIO_BIT_WIDTH_16:
            bytes_per_sample = 2; /* 2: 16bit使用2byte储存 */
            break;
        case OT_AUDIO_BIT_WIDTH_24:
            bytes_per_sample = 4; /* 4: 24bit使用4byte储存 */
            break;
        default:
            ao_info_trace("bit_width error\n");
            return OT_ERR_AO_ILLEGAL_PARAM;
    }

    if (last_point_num_to_end == point_num) {
        /* 上一帧数据不需要掉头处理 */
        *last_buf = (td_u8 *)(aio_drv_dev->cir_buf.vir_addr + last_wptr_offset +
                    ((td_s64)index * bytes_per_sample));
        *end_point_num = point_num - 1;
    } else {
        /* 上一帧数据需要掉头处理 */
        *last_buf = (td_u8 *)(td_void *)(aio_drv_dev->cir_buf.vir_addr + (td_u32)index * bytes_per_sample);
        *end_point_num = (point_num - last_point_num_to_end) - 1;
    }

    return TD_SUCCESS;
}

td_s32 ao_drv_put_last_data(ot_audio_dev audio_dev_id, td_u32 ao_chn)
{
    td_s32 index, ret;
    aio_drv_dev_ctx *aio_drv_dev = &g_ao_drv_dev[audio_dev_id];
    td_u32 bit_width = (td_u32)aio_drv_dev->aio_attr.bit_width;
    td_u32 point_num = aio_drv_dev->aio_attr.point_num_per_frame;
    td_u32 base = aio_drv_dev->fifo_len_base;
    td_u8 *last_buf = TD_NULL; /* 上一帧数据buf的起始指针 */
    td_u8 *start_buf = aio_drv_dev->cir_buf.vir_addr;
    td_u8 *dma_buf = aio_drv_dev->cir_buf.vir_addr + aio_drv_dev->cir_buf.wptr_off_set;
    td_u32 pt_num_to_end;
    td_u32 end_point_num = 0; /* 上一帧数据最后一个采样点的偏移 */
    td_u8 data_8bit;
    td_u16 data_16bit;
    td_u32 data_24bit;

    /* 当前帧数据的起始地址 */
    ret = check_dma_buf(dma_buf, aio_drv_dev);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    /* 获取环形buf末端的空闲数据点数 */
    pt_num_to_end = ao_cir_buf_get_spare_point_num(aio_drv_dev);

    /* 获取上一帧的最后一个采样点数据地址及偏移 */
    ret = ao_get_last_data(audio_dev_id, ao_chn, &last_buf, &end_point_num);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    /*
     * 根据buf分配规则，当前帧与上一帧在某一时刻，只有其中一帧需要掉头处理或者都不需要掉头处理，
     * 获取需要掉头处理的是上一帧还是当前帧，或是不需要掉头处理
     */
    index = aio_drv_dev->chn_index[ao_chn];

    switch (bit_width) {
        case OT_AUDIO_BIT_WIDTH_8:
            data_8bit = *(last_buf + (end_point_num << base));
            ao_put_same_data_8bit(dma_buf + index, pt_num_to_end, base, data_8bit);

            /* 必要时处理需要折回的情况 */
            ao_put_same_data_8bit(start_buf + index, point_num - pt_num_to_end, base, data_8bit);
            break;
        case OT_AUDIO_BIT_WIDTH_16:
            data_16bit = *(td_u16 *)(td_void *)(last_buf + ((td_slong)end_point_num << base));
            ao_put_same_data_16bit(dma_buf + (td_u32)index * 2U, pt_num_to_end, base, data_16bit); /* 2: 16bit */

            /* 必要时处理需要折回的情况 2: 16bit */
            ao_put_same_data_16bit(start_buf + (td_u32)index * 2U, point_num - pt_num_to_end, base, data_16bit);
            break;
        case OT_AUDIO_BIT_WIDTH_24:
            data_24bit = *(td_u32 *)(td_void *)(last_buf + (end_point_num << base));
            ao_put_same_data_24bit(dma_buf + (td_u32)index * 4U, pt_num_to_end, base, data_24bit); /* 4: 24bit */

            /* 必要时处理需要折回的情况 4: 24bit */
            ao_put_same_data_24bit((start_buf + (td_u64)index * 4), point_num - pt_num_to_end, base, data_24bit);
            break;
        default:
            ao_info_trace("bit_width error\n");
            return OT_ERR_AO_ILLEGAL_PARAM;
    }
    return TD_SUCCESS;
}

td_void ao_drv_flush_data(ot_audio_dev audio_dev_id)
{
// remove mmz code
#ifndef CONFIG_AUDIO_V200_SUPPORT
    aio_drv_dev_ctx *aio_drv_dev = &g_ao_drv_dev[audio_dev_id];

    ot_mmb_flush_dcache_byaddr(aio_drv_dev->cir_buf.vir_addr, (unsigned long)aio_drv_dev->cir_buf.phys_addr,
        aio_drv_dev->cir_buf.buf_size);
#endif
    return;
}

static td_void ao_put_chn_data_8bit(td_u8 *dest_buf, td_u32 point_num, const td_u8 *src_buf,
    td_u32 base, td_s32 index)
{
    td_u32 excess_point, k;
    td_u8 *tmp_buf = dest_buf + index;

    if (point_num == 0) {
        /* 无须处理，直接返回 */
        return;
    }

    excess_point = point_num % 4; /* 用于处理掉头前不能被4整除的采样点 */

    /* 先处理不能被4整除的采样点 */
    for (k = 0; k < excess_point; k++) {
        *(tmp_buf + (k << base)) = src_buf[k];
    }

    /* 再处理能被4整除的采样点 */
    for (k = excess_point; k < point_num; k += 4) { /* 4: offset */
        *(tmp_buf + (k << base)) = src_buf[k];
        *(tmp_buf + ((k + 1) << base)) = src_buf[k + 1];
        *(tmp_buf + ((k + 2) << base)) = src_buf[k + 2]; /* 2: offset */
        *(tmp_buf + ((k + 3) << base)) = src_buf[k + 3]; /* 3: offset */
    }
}

static td_void ao_put_chn_data_16bit(td_u8 *dest_buf, td_u32 point_num, const td_u8 *src_buf,
    td_u32 base, td_s32 index)
{
    td_u32 excess_point, k;
    td_u8 *tmp_buf = dest_buf + index * 4; /* 4: 32bit使用4byte储存 */

    if (point_num == 0) {
        /* 无须处理，直接返回 */
        return;
    }

    excess_point = point_num % 4; /* 用于处理掉头前不能被4整除的采样点 */

    /* 先处理不能被4整除的采样点 */
    for (k = 0; k < excess_point; k++) {
        *(td_u32 *)(tmp_buf + (k << base)) = (td_u32)(((td_u16 *)src_buf)[k] << 16U);
    }

    /* 再处理能被4整除的采样点 */
    for (k = excess_point; k < point_num; k += 4) { /* 4: offset */
        *(td_u32 *)(tmp_buf + (k << base)) = (td_u32)(((td_u16 *)src_buf)[k] << 16U); /* 16:左移16bit */
        *(td_u32 *)(tmp_buf + ((k + 1) << base)) = (td_u32)(((td_u16 *)src_buf)[k + 1] << 16U); /* 16:左移16bit */
        *(td_u32 *)(tmp_buf + ((k + 2) << base)) = (td_u32)(((td_u16 *)src_buf)[k + 2] << 16U); /* 2 16:左移16bit */
        *(td_u32 *)(tmp_buf + ((k + 3) << base)) = (td_u32)(((td_u16 *)src_buf)[k + 3] << 16U); /* 3 16:左移16bit */
    }
}

static td_void ao_put_chn_data_24bit(td_u8 *dest_buf, td_u32 point_num, const td_u8 *src_buf,
    td_u32 base, td_s32 index)
{
    td_u32 excess_point, k;
    td_u8 *tmp_buf = dest_buf + index * 4; /* 4: 24bit使用4byte储存 */

    if (point_num == 0) {
        /* 无须处理，直接返回 */
        return;
    }

    excess_point = point_num % 4; /* 用于处理掉头前不能被4整除的采样点 */

    /* 先处理不能被4整除的采样点 */
    for (k = 0; k < excess_point; k++) {
        *(td_u32 *)(tmp_buf + (k << base)) = ((td_u32 *)src_buf)[k];
    }

    /* 再处理能被4整除的采样点 */
    for (k = excess_point; k < point_num; k += 4) { /* 4: offset */
        *(td_u32 *)(tmp_buf + (k << base)) = ((td_u32 *)src_buf)[k];
        *(td_u32 *)(tmp_buf + ((k + 1) << base)) = ((td_u32 *)src_buf)[k + 1];
        *(td_u32 *)(tmp_buf + ((k + 2) << base)) = ((td_u32 *)src_buf)[k + 2]; /* 2: offset */
        *(td_u32 *)(tmp_buf + ((k + 3) << base)) = ((td_u32 *)src_buf)[k + 3]; /* 3: offset */
    }
}

td_s32 ao_drv_put_chn_data(ot_audio_dev audio_dev_id, const audio_pack *pack, td_u32 ao_chn)
{
    td_s32 index;
    aio_drv_dev_ctx *aio_drv_dev = &g_ao_drv_dev[audio_dev_id];

    td_u32 point_num = aio_drv_dev->aio_attr.point_num_per_frame;
    td_u32 base = aio_drv_dev->fifo_len_base;
    td_u32 point_num_to_end;
    td_u8 *dma_buf = TD_NULL;

    dma_buf = aio_drv_dev->cir_buf.vir_addr + aio_drv_dev->cir_buf.wptr_off_set;
    if (dma_buf == TD_NULL) {
        ao_err_trace("AO not init(CB buffer is NULL)\n");
        return OT_ERR_AO_NOT_ENABLED;
    }
    if (dma_buf > aio_drv_dev->cir_buf.vir_addr + aio_drv_dev->cir_buf.buf_size) {
        ao_err_trace("buffer addr is illegal!\n");
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    /* 获取环形buf末端的空闲数据点数 */
    point_num_to_end = ao_cir_buf_get_spare_point_num(aio_drv_dev);

    index = aio_drv_dev->chn_index[ao_chn];

    switch (aio_drv_dev->aio_attr.bit_width) {
        case OT_AUDIO_BIT_WIDTH_8:
            ao_put_chn_data_8bit(dma_buf, point_num_to_end, pack->data, base, index);

            /* 必要时处理需要折回的情况 */
            ao_put_chn_data_8bit(aio_drv_dev->cir_buf.vir_addr, point_num - point_num_to_end,
                pack->data + point_num_to_end, base, index);
            break;

        case OT_AUDIO_BIT_WIDTH_16:
            ao_put_chn_data_16bit(dma_buf, point_num_to_end, pack->data, base, index);
            /* 必要时处理需要折回的情况 */
            ao_put_chn_data_16bit(aio_drv_dev->cir_buf.vir_addr, point_num - point_num_to_end,
                pack->data + point_num_to_end * 2, base, index); /* 2: offset */
            break;

        case OT_AUDIO_BIT_WIDTH_24:
            ao_put_chn_data_24bit(dma_buf, point_num_to_end, pack->data, base, index);

            /* 必要时处理需要折回的情况 */
            ao_put_chn_data_24bit(aio_drv_dev->cir_buf.vir_addr, point_num - point_num_to_end,
                pack->data + point_num_to_end * 4, base, index); /* 4: offset */
            break;

        default:
            ao_info_trace("bit_width error\n");
            return OT_ERR_AO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 dma_buf_info_init_chn_index(ot_audio_dev ao_dev_id, const ot_aio_attr *ao_attr)
{
    td_s32 i, sd_num;
    td_s32 ret;
    aio_export_func *aio_exp_func = TD_NULL;
    td_bool tx_multislot_support = TD_FALSE;

    /* 小于和等于2通道时，无须考虑多根数据线和时分复用输出的特殊情况 */
    if (ao_attr->chn_cnt <= 2) {
        for (i = 0; i < (td_s32)(ASP_SUPPORT_MAX_CHN_CNT); i++) {
            g_ao_drv_dev[ao_dev_id].chn_index[i] = i;
        }
        return TD_SUCCESS;
    }
#ifndef CONFIG_AUDIO_V200_SUPPORT
    /* 对于通道数>2的场景，先判断芯片是否支持TX方向的时分复用输出 */
    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if (aio_exp_func == TD_NULL) {
        ao_emerg_trace("init chn index fail, aio mode may not insmod\n");
        return OT_ERR_AO_NOT_READY;
    }

    ret = aio_exp_func->pfn_get_ao_tx_multislot(&tx_multislot_support);
    if (ret != TD_SUCCESS) {
        ao_err_trace("get ao tx multislot fail\n");
        return OT_ERR_AO_NOT_READY;
    }

    if (tx_multislot_support == TD_TRUE) {
        /* 对于时分复用的情况，其DDR数据从0通道开始递增排布 */
        for (i = 0; i < (td_s32)(ao_attr->chn_cnt); i++) {
            g_ao_drv_dev[ao_dev_id].chn_index[i] = i;
        }
    } else {
        /* 对于多根数据线发送的情况，其DDR数据按sd0_left/sd0_right/sd1_l/sd1_r/.../sdn_l/sdn_r的方式进行排布 */
        sd_num = (td_s32)ao_attr->chn_cnt / 2; /* 2: 1根数据线仅传输左右声道 */
        for (i = 0; i < sd_num; i++) {
            g_ao_drv_dev[ao_dev_id].chn_index[i] = 2 * i; /* 2: 左右声道 */
            g_ao_drv_dev[ao_dev_id].chn_index[i + sd_num] = 2 * i + 1; /* 2: 左右声道 */
        }
    }
#endif
    return TD_SUCCESS;
}

/* 根据设备的属性初始化DMA buf的信息，并返回DMA buf的总长度 */
td_s32 ao_drv_dma_buf_info_init(ot_audio_dev audio_dev_id, td_u32 *dma_buf_bytes)
{
    td_s32 ret;
    td_s32 fifo_len_base;
    td_u32 aio_fifo_len;
    aio_drv_dev_ctx *aio_drv_dev = &g_ao_drv_dev[audio_dev_id];
    td_s32 trach_chn_cnt = (td_s32)ASP_SUPPORT_MAX_CHN_CNT; /* 总的通道数目 V200上只支持2个chn */
    td_u32 bit_cnt = aio_get_bit_cnt(aio_drv_dev->aio_attr.bit_width); /* 一个通道的位宽 */
    td_s32 track_bit = (((td_s32)bit_cnt) * trach_chn_cnt);                       /* 所有通道的总位宽 */

    aio_fifo_len = (td_u32)(track_bit / 8); /* 8: 1Byte的bit数 */

    /* 初始化通道在DMA BUF中的位置索引 */
    ret = dma_buf_info_init_chn_index(audio_dev_id, &(aio_drv_dev->aio_attr));
    if (ret != TD_SUCCESS) {
        return ret;
    }

    aio_drv_dev->aio_fifo_len = aio_fifo_len;
    fifo_len_base = ao_get_quadratic_root((td_s32)aio_fifo_len);
    if (fifo_len_base <= 0) {
        ao_err_trace("aio_get_quadratic_root fail, aio_fifo_len: %u is invalid\n", aio_fifo_len);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    aio_drv_dev->fifo_len_base = (td_u32)fifo_len_base;

    aio_drv_dev->trans_len = aio_drv_dev->aio_attr.point_num_per_frame * aio_drv_dev->aio_fifo_len ;
    aio_drv_dev->cir_buf.rptr_off_set = 0;
    aio_drv_dev->cir_buf.wptr_off_set = aio_drv_dev->trans_len * 2; /* 2: 2帧 */

    /*
     * 在初始化时将写指针指向2帧长度处，让硬件认为有数据从而启动传输并启动中断，分配2帧长度的循环buffer时， 第一次
     * 中断到来后，读指针与写指针更新，如果循环buffer刚好是两帧长度，则写指针更新后掉头指向第一帧长度处，而读指针
     * 更新后也指向第一帧长度处，导致硬件认为循环buffer空，一直上报buffer空中断，实际上第二帧的数据还在等待硬件读取
     * 故分配3帧长度的循环buffer，解决此问题。
     */
    aio_drv_dev->cir_buf.buf_size = aio_drv_dev->trans_len * 2; /* 2: 2帧 */
    /* 需要对buffer大小做128字节对齐 */

    if (aio_drv_dev->cir_buf.buf_size % 128 != 0) { /* 128: 128字节对齐 */
#ifndef CONFIG_AUDIO_V200_SUPPORT
        aio_drv_dev->cir_buf.buf_size = (aio_drv_dev->cir_buf.buf_size / 128 + 1) * 128; /* 128: 128字节对齐 */
#endif
    }
    *dma_buf_bytes = aio_drv_dev->cir_buf.buf_size;

    ao_info_trace("trans_len %u, point_num_per_frame = %u, aio_fifo_len = %u\n",
                  aio_drv_dev->trans_len, aio_drv_dev->aio_attr.point_num_per_frame, aio_drv_dev->aio_fifo_len);
    return TD_SUCCESS;
}

td_s32 ao_drv_set_dma_buf_addr(ot_audio_dev audio_dev_id, td_u64 dma_phy_addr, td_u8 *dma_vir_addr)
{
    aio_drv_dev_ctx *aio_drv_dev = &g_ao_drv_dev[audio_dev_id];

    /* init cir_buf addr */
    aio_drv_dev->cir_buf.phys_addr = dma_phy_addr;
    aio_drv_dev->cir_buf.vir_addr = dma_vir_addr;

    return TD_SUCCESS;
}

td_s32 ao_drv_clr_dma_buf_addr(ot_audio_dev audio_dev_id)
{
    aio_drv_dev_ctx *aio_drv_dev = &g_ao_drv_dev[audio_dev_id];

    /* clear cir_buf addr */
    aio_drv_dev->cir_buf.phys_addr = 0;
    aio_drv_dev->cir_buf.vir_addr = TD_NULL;

    return TD_SUCCESS;
}

td_s32 ao_drv_set_dev_attr(ot_audio_dev ao_dev, const ot_aio_attr *attr)
{
    td_s32 ret;
    ot_mpp_chn mpp_chn = {0};
    td_bool clk_en = TD_TRUE;
    td_bool reset = TD_FALSE;
    aio_drv_dev_ctx *aio_drv_dev = &g_ao_drv_dev[ao_dev];
    aio_export_func *aio_exp_func = TD_NULL;

    mpp_chn.mod_id = OT_ID_AIO;
    mpp_chn.dev_id = ao_dev;

    if (((td_bool)(ckfn_sys_entry()) == TD_FALSE) || (((td_bool)ckfn_sys_drv_ioctrl()) == TD_FALSE)) {
        ao_err_trace("sys module may not insert.\n");
        return OT_ERR_AO_NOT_READY;
    }

    call_sys_drv_ioctrl((hi_mpp_chn *)&mpp_chn, SYS_AO_CLK_EN, &clk_en);
    call_sys_drv_ioctrl((hi_mpp_chn *)&mpp_chn, SYS_AO_RESET_SEL, &reset);

    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if ((aio_exp_func == TD_NULL) || (aio_exp_func->pfn_set_ao_attr == TD_NULL)) {
        ao_emerg_trace("ao set_dev_attr fail, aio mode may not insmod\n");
        return OT_ERR_AO_NOT_READY;
    }

    /* 复位部分是否需要更改，需要与sys模块确认 */
    ret = aio_exp_func->pfn_set_ao_attr(ao_dev, attr);
    if (ret != TD_SUCCESS) {
        ao_err_trace("[%s: %d]pfn_set_ao_attr fail ret = %d\n", __func__, __LINE__, ret);
        return ret;
    }

    aio_drv_dev->track_mode = OT_AUDIO_TRACK_NORMAL;
    aio_drv_dev->mute = TD_FALSE;
    aio_drv_dev->fade.fade = TD_FALSE;
    aio_drv_dev->fade.fade_in_rate = OT_AUDIO_FADE_RATE_1;
    aio_drv_dev->fade.fade_out_rate = OT_AUDIO_FADE_RATE_1;
    aio_drv_dev->volume = 0; /* 0dB */

    ret = memcpy_s(&aio_drv_dev->aio_attr, sizeof(aio_drv_dev->aio_attr), attr, sizeof(*attr));
    if (ret != EOK) {
        ao_err_trace("ao_dev %d attr memcpy_s fail, ret = %d.\n", ao_dev, ret);
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

td_s32 ao_drv_clr_dev_attr(ot_audio_dev audio_dev_id)
{
    td_s32 ret;
    aio_export_func *aio_exp_func = TD_NULL;

    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if ((aio_exp_func == TD_NULL) || (aio_exp_func->pfn_clr_ao_attr == TD_NULL)) {
        ao_emerg_trace("ao_clr_cfg fail, aio mode may not insmod\n");
        return OT_ERR_AO_NOT_READY;
    }

    ret = aio_exp_func->pfn_clr_ao_attr(audio_dev_id);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

/* 所有芯片都使用该接口，启动DMA及设备 */
td_s32 ao_drv_enable_dev(ot_audio_dev audio_dev_id)
{
    aio_drv_dev_ctx *aio_drv_dev = &g_ao_drv_dev[audio_dev_id];
    aio_export_func *aio_exp_func = TD_NULL;
    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if ((aio_exp_func == TD_NULL) ||
        (aio_exp_func->pfn_set_ao_dev_buf_wptr == TD_NULL) ||
        (aio_exp_func->pfn_enable_ao_dev == TD_NULL)) {
        ao_err_trace("aio mode may not insmod.\n");
        return OT_ERR_AO_NOT_READY;
    }

    aio_exp_func->pfn_set_ao_dev_buf_wptr(audio_dev_id,
                                          aio_drv_dev->cir_buf.wptr_off_set & (~(16 - 1))); /* 16: free>=32byte */

    /* 使能中断，清除原始中断，启动设备 */
    aio_exp_func->pfn_enable_ao_dev(audio_dev_id);

    aio_drv_dev->enable = TD_TRUE;

    return TD_SUCCESS;
}

/* 所有芯片都使用这个接口 停止设备 */
td_void ao_drv_disable_dev(ot_audio_dev audio_dev_id)
{
    aio_export_func *aio_exp_func = TD_NULL;

    g_ao_drv_dev[audio_dev_id].enable = TD_FALSE;
    g_ao_drv_dev[audio_dev_id].int_cnt = 0;
    g_ao_drv_dev[audio_dev_id].u32fifo_int_cnt = 0;
    g_ao_drv_dev[audio_dev_id].u32buff_int_cnt = 0;
    g_ao_drv_dev[audio_dev_id].frm_time = 0;
    g_ao_drv_dev[audio_dev_id].max_frm_time = 0;

    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if ((aio_exp_func == TD_NULL) || (aio_exp_func->pfn_disable_ao_dev == TD_NULL)) {
        ao_err_trace("aio mode may not insmod\n");
        return;
    }

    /* 配置设备启动/停止寄存器，停止设备 */
    aio_exp_func->pfn_disable_ao_dev(audio_dev_id);
}

#ifdef CONFIG_OT_AUDIO_ALSA_SUPPORT
/* 单独设置DMA属性，供ALSA使用 */
td_s32 ao_drv_set_dma_attr(ot_audio_dev ao_dev, cir_buf cir_buf_attr, td_u32 trans_len)
{
    aio_export_func *aio_exp_func = TD_NULL;

    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if ((aio_exp_func == TD_NULL) ||
        (aio_exp_func->pfn_set_ao_dev_buf_wptr == TD_NULL) ||
        ao_err_trace("aio mode may not insmod\n");
        return OT_ERR_AO_NOT_READY;
    }

    aio_exp_func->pfn_set_ao_dev_buf_wptr(ao_dev,
        cir_buf_attr.wptr_offset & (~(16 - 1))); /* 16: free>=32byte */

    return TD_SUCCESS;
}

/* 启动DMA设备，供ALSA使用 */
td_s32 ao_drv_enable_dma(ot_audio_dev ao_dev, td_bool enable)
{
    aio_export_func *aio_exp_func = TD_NULL;

    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if ((aio_exp_func == TD_NULL) || (aio_exp_func->pfn_enable_ao_dev == TD_NULL) ||
        (aio_exp_func->pfn_disable_ao_dev == TD_NULL)) {
        ao_err_trace("aio mode may not insmod\n");
        return OT_ERR_AO_NOT_READY;
    }

    if (enable == TD_TRUE) {
        aio_exp_func->pfn_enable_ao_dev(ao_dev);
    } else {
        aio_exp_func->pfn_disable_ao_dev(ao_dev);
    }

    return TD_SUCCESS;
}
#endif

td_void ao_drv_set_dma_isr(fn_ao_dma_callback pf_callback)
{
    g_pf_dma_callback = pf_callback;
}

#ifdef CONFIG_OT_AUDIO_ALSA_SUPPORT
/* 设置DMA外部回调，供ALSA使用 */
td_void ao_drv_set_dma_ext_isr(ot_audio_dev ao_dev, fn_ao_alsa_dma_callback pf_callback, td_void *substream)
{
    aio_drv_dev_ctx *aio_drv_dev = &g_ao_drv_dev[ao_dev];
    aio_drv_dev->alsa_substream = substream;
    g_pf_dma_ext_callback = pf_callback;
}

/* 复位DMA外部回调，供ALSA使用 */
td_void ao_drv_reset_dma_ext_isr(ot_audio_dev ao_dev)
{
    aio_drv_dev_ctx *aio_drv_dev = &g_ao_drv_dev[ao_dev];
    aio_drv_dev->alsa_substream = TD_NULL;
    g_pf_dma_ext_callback = TD_NULL;
}
#endif

static td_void ao_isr_transfer_finish_process(ot_audio_dev dev_id, const aio_export_func *aio_exp_func,
                                              aio_drv_dev_ctx *aio_drv_dev)
{
    td_u64 pts;
    td_u32 len;
    struct osal_timeval time1;
    struct osal_timeval time2;

    if (aio_drv_dev->cir_buf.wptr_off_set == aio_drv_dev->cir_buf.buf_size) {
        aio_drv_dev->cir_buf.wptr_off_set = 0;
    }

    /* clac frame time */
    if (((td_bool)ckfn_sys_entry() == TD_TRUE) && ((td_bool)ckfn_sys_get_time_stamp() == TD_TRUE)) {
        pts = call_sys_get_time_stamp();
        aio_drv_dev->frm_time = (td_u32)(pts - aio_drv_dev->last_pts);
        aio_drv_dev->last_pts = pts;
    }

    /* get max frame time */
    if ((aio_drv_dev->int_cnt != 1) && (aio_drv_dev->frm_time > aio_drv_dev->max_frm_time)) {
        aio_drv_dev->max_frm_time = aio_drv_dev->frm_time;
    }

    osal_gettimeofday(&time1);

    if (g_pf_dma_ext_callback == TD_NULL) {
        if (g_pf_dma_callback == TD_NULL) {
            ao_err_trace("g_pf_dma_callback is null\n");
        } else {
            g_pf_dma_callback(dev_id);
        }
    }

    osal_gettimeofday(&time2);
    aio_drv_dev->isr_time = (td_u32)aio_time_diff_us(time2, time1);

    /* 获取最长的中断时间 */
    if (aio_drv_dev->isr_time > aio_drv_dev->max_isr_time) {
        aio_drv_dev->max_isr_time = aio_drv_dev->isr_time;
    }

    /* 更新写指针，并更新写地址寄存器中的读指针 */
    len = aio_drv_dev->cir_buf.buf_size - aio_drv_dev->cir_buf.wptr_off_set;
    if ((len != 0) && (len < aio_drv_dev->trans_len)) {
        aio_drv_dev->cir_buf.wptr_off_set = aio_drv_dev->trans_len - len;
    } else {
        aio_drv_dev->cir_buf.wptr_off_set += aio_drv_dev->trans_len;
    }
    if (aio_drv_dev->cir_buf.wptr_off_set == aio_drv_dev->cir_buf.buf_size) {
        aio_drv_dev->cir_buf.wptr_off_set = 0;
    }

    if (g_pf_dma_ext_callback == TD_NULL) {
        aio_exp_func->pfn_set_ao_dev_buf_wptr(dev_id,
            (aio_drv_dev->cir_buf.wptr_off_set & (~(16 - 1)))); /* 16:free>=32byte */
    } else {
        g_pf_dma_ext_callback(aio_drv_dev->alsa_substream); /* update write in alsa dma isr */
    }
}

static void ao_drv_isr(ot_audio_dev audio_dev_id)
{
    aio_export_func *aio_exp_func = TD_NULL;
    aio_drv_dev_ctx *aio_drv_dev = &g_ao_drv_dev[audio_dev_id];

    /* get int status */
    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if ((aio_exp_func == TD_NULL) || (aio_exp_func->pfn_get_ao_int_status == TD_NULL) ||
        (aio_exp_func->pfn_clr_ao_int_status == TD_NULL) ||
        (aio_exp_func->pfn_set_ao_dev_buf_wptr == TD_NULL)) {
        ao_emerg_trace("aio module may not insmod\n");
        return;
    }
#ifndef CONFIG_AUDIO_V200_SUPPORT
    td_u32 int_status;

    (td_void)aio_exp_func->pfn_get_ao_int_status(audio_dev_id, &int_status);

    if (int_status & 0x18) { /* bit[4][3], fifo over */
        aio_drv_dev->u32fifo_int_cnt++;
        ao_warn_trace("AO dev: %d, int status : 0x%x, fifo over cnt:%d \n",
                      audio_dev_id, int_status, aio_drv_dev->u32fifo_int_cnt);
    }

    (td_void)aio_exp_func->pfn_clr_ao_int_status(audio_dev_id);

    if ((int_status & 0x01) || (int_status & 0x02)) { /* bit[0]，transfer finish;bit[1]，buffer is empty */
        if (int_status & 0x01) {
            aio_drv_dev->int_cnt++;
        } else {
            aio_drv_dev->u32buff_int_cnt++;
            ao_err_trace("AO dev: %d, int status : 0x%x, buffer empty cnt:%d \n",
                         audio_dev_id, int_status, aio_drv_dev->u32buff_int_cnt);
        }

        /* 传输完成处理 */
        ao_isr_transfer_finish_process(audio_dev_id, aio_exp_func, aio_drv_dev);
    }
#endif
    ao_isr_transfer_finish_process(audio_dev_id, aio_exp_func, aio_drv_dev);
}

td_s32 ao_drv_dev_init(ot_audio_dev audio_dev_id)
{
    aio_export_func *aio_exp_func = TD_NULL;
    aio_drv_dev_ctx *aio_drv_dev = &g_ao_drv_dev[audio_dev_id];

    (td_void)memset_s(aio_drv_dev, sizeof(aio_drv_dev_ctx), 0, sizeof(aio_drv_dev_ctx));
    aio_drv_dev->enable = TD_FALSE;

    /* 注册DMA传输完成中断函数 */
    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if ((aio_exp_func == TD_NULL) || (aio_exp_func->pfn_set_ao_isr == TD_NULL)) {
        ao_emerg_trace("aio module may not insmod\n");
        return OT_ERR_AO_NOT_READY;
    }
    aio_exp_func->pfn_set_ao_isr(ao_drv_isr);

    return TD_SUCCESS;
}

td_s32 ao_drv_dev_deinit(ot_audio_dev audio_dev_id)
{
    aio_drv_dev_ctx *aio_drv_dev = &g_ao_drv_dev[audio_dev_id];

    aio_drv_dev->enable = TD_FALSE;
    return TD_SUCCESS;
}

td_s32 ao_drv_check_aio_export_func(td_void)
{
    aio_export_func *aio_exp_func = TD_NULL;
    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if (aio_exp_func == TD_NULL) {
        ao_emerg_trace("aio module may not insmod\n");
        return OT_ERR_AO_NOT_READY;
    }

    return TD_SUCCESS;
}

td_s32 ao_drv_set_track_mode(ot_audio_dev audio_dev_id, ot_audio_track_mode track_mode)
{
    aio_export_func *aio_exp_func = TD_NULL;
    aio_drv_dev_ctx *aio_drv_dev = TD_NULL;

    aio_drv_dev = &g_ao_drv_dev[audio_dev_id];

    if ((aio_drv_dev->aio_attr.work_mode != OT_AIO_MODE_I2S_SLAVE) &&
        (aio_drv_dev->aio_attr.work_mode != OT_AIO_MODE_I2S_MASTER)) {
        ao_err_trace("only i2s work_mode support trackmode\n");
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if (aio_exp_func == TD_NULL) {
        ao_emerg_trace("aio module may not insmod\n");
        return OT_ERR_AO_NOT_READY;
    }

    return TD_SUCCESS;
}

td_s32 ao_drv_get_track_mode(ot_audio_dev audio_dev_id, ot_audio_track_mode *track_mode)
{
    td_s32 ret;
    aio_export_func *aio_exp_func = TD_NULL;
    aio_drv_dev_ctx *aio_drv_dev = TD_NULL;
    ot_audio_track_mode tmp_track = OT_AUDIO_TRACK_NORMAL;

    aio_drv_dev = &g_ao_drv_dev[audio_dev_id];

    if ((aio_drv_dev->aio_attr.work_mode != OT_AIO_MODE_I2S_MASTER) &&
        (aio_drv_dev->aio_attr.work_mode != OT_AIO_MODE_I2S_SLAVE)) {
        ao_err_trace("only i2s work_mode support trackmode\n");
        return OT_ERR_AO_ILLEGAL_PARAM;
    }
    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if (aio_exp_func == TD_NULL) {
        ao_emerg_trace("aio module may not insmod\n");
        return OT_ERR_AO_NOT_READY;
    }

    *track_mode = tmp_track;

    return TD_SUCCESS;
}

td_s32 ao_drv_set_mute(ot_audio_dev audio_dev_id, td_bool enable, const ot_audio_fade *fade)
{
    td_s32 ret;
    aio_export_func *aio_exp_func = TD_NULL;
    aio_drv_dev_ctx *aio_drv_dev = TD_NULL;

    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if ((aio_exp_func == TD_NULL) || (aio_exp_func->pfn_set_ao_mute == TD_NULL)) {
        ao_emerg_trace("aio module may not insmod\n");
        return OT_ERR_AO_NOT_READY;
    }

    ret = aio_exp_func->pfn_set_ao_mute(audio_dev_id, enable, fade);
    if (ret == TD_SUCCESS) {
        aio_drv_dev = &g_ao_drv_dev[audio_dev_id];
        aio_drv_dev->mute = enable;
        if (fade != TD_NULL) {
            aio_drv_dev->fade.fade = fade->fade;
            aio_drv_dev->fade.fade_in_rate = fade->fade_in_rate;
            aio_drv_dev->fade.fade_out_rate = fade->fade_out_rate;
        }
    }

    return ret;
}

td_s32 ao_drv_get_mute(ot_audio_dev audio_dev_id, td_bool *enable, ot_audio_fade *fade)
{
    td_s32 ret;
    aio_export_func *aio_exp_func = TD_NULL;
    td_bool tmp_mute = TD_FALSE;
    ot_audio_fade tmp_fade = {0};

    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if (aio_exp_func == TD_NULL) {
        ao_emerg_trace("aio module may not insmod\n");
        return OT_ERR_AO_NOT_READY;
    }

    ret = aio_exp_func->pfn_get_ao_mute(audio_dev_id, &tmp_mute, &tmp_fade);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    *enable = tmp_mute;

    fade->fade = tmp_fade.fade;
    fade->fade_in_rate = tmp_fade.fade_in_rate;
    fade->fade_out_rate = tmp_fade.fade_out_rate;

    return TD_SUCCESS;
}

td_s32 ao_drv_set_volume(ot_audio_dev audio_dev_id, td_s32 volume_db)
{
    td_s32 ret;
    aio_export_func *aio_exp_func = TD_NULL;
    aio_drv_dev_ctx *aio_drv_dev = TD_NULL;

    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if ((aio_exp_func == TD_NULL) || (aio_exp_func->pfn_set_ao_volume == TD_NULL)) {
        ao_emerg_trace("aio module may not insmod\n");
        return OT_ERR_AO_NOT_READY;
    }

    ret = aio_exp_func->pfn_set_ao_volume(audio_dev_id, volume_db);
    if (ret == TD_SUCCESS) {
        aio_drv_dev = &g_ao_drv_dev[audio_dev_id];
        aio_drv_dev->volume = volume_db;
    }

    return ret;
}

td_s32 ao_drv_get_volume(ot_audio_dev audio_dev_id, td_s32 *volume_db)
{
    td_s32 ret;
    aio_export_func *aio_exp_func = TD_NULL;
    td_s32 tmp_volume;

    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if (aio_exp_func == TD_NULL) {
        ao_emerg_trace("aio module may not insmod\n");
        return OT_ERR_AO_NOT_READY;
    }

    ret = aio_exp_func->pfn_get_ao_volume(audio_dev_id, &tmp_volume);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    *volume_db = tmp_volume;

    return TD_SUCCESS;
}

td_s32 ao_drv_set_clk_dir(ot_audio_dev audio_dev_id, audio_clkdir clk_dir)
{
    aio_export_func *aio_exp_func = TD_NULL;
    aio_drv_dev_ctx *aio_drv_dev = TD_NULL;

    aio_drv_dev = &g_ao_drv_dev[audio_dev_id];

    aio_exp_func = (aio_export_func *)cmpi_get_module_func_by_id(OT_ID_AIO);
    if (aio_exp_func == TD_NULL) {
        ao_emerg_trace("aio module may not insmod\n");
        return OT_ERR_AO_NOT_READY;
    }

    return TD_SUCCESS;
}

td_s32 ao_drv_get_clk_dir(ot_audio_dev audio_dev_id, audio_clkdir *clk_dir)
{
    aio_export_func *aio_exp_func = TD_NULL;

    aio_exp_func = (aio_export_func *)cmpi_get_module_func_by_id(OT_ID_AIO);
    if (aio_exp_func == TD_NULL) {
        ao_emerg_trace("aio module may not insmod\n");
        return OT_ERR_AO_NOT_READY;
    }

    return TD_SUCCESS;
}

static td_u32 ao_aec_get_spare_point_num(const aio_drv_dev_ctx *ao_drv_dev, td_u32 rptr_off_set)
{
    td_u32 bit_cnt, len, point_num_to_end;

    if ((ao_drv_dev->aio_attr.bit_width == OT_AUDIO_BIT_WIDTH_8) &&
        (ao_drv_dev->aio_attr.expand_flag == OT_AI_EXPAND)) {
        bit_cnt = aio_get_bit_cnt(OT_AUDIO_BIT_WIDTH_16);
    } else {
        bit_cnt = aio_get_bit_cnt(ao_drv_dev->aio_attr.bit_width);
    }

    /* 计算当前读指针到循环buffer结尾的长度 */
    len = ao_drv_dev->cir_buf.buf_size - rptr_off_set;
    point_num_to_end = (len * 8) / (bit_cnt * ao_drv_dev->aio_attr.chn_cnt); /* 8: 1Byte的bit数 */
    if (point_num_to_end > ao_drv_dev->aio_attr.point_num_per_frame) {
        point_num_to_end = ao_drv_dev->aio_attr.point_num_per_frame;
    }

    return point_num_to_end;
}

static td_void ao_get_aec_data_8bit(td_u8 *dest_buf, const td_u8 *src_buf, td_u32 point_num,
    td_u32 base, td_s32 index)
{
    td_u32 excess_point, k;
    const td_u8 *tmp_buf = src_buf + index;
    td_u8 *data = dest_buf;

    if (point_num == 0) {
        /* 无须处理，直接返回 */
        return;
    }

    excess_point = point_num % 4; /* 用于处理不能被4整除的采样点 */

    /* 先处理不能被4整除的采样点 */
    for (k = 0; k < excess_point; k++) {
        data[k] = (*(tmp_buf + ((td_slong)k << base)));
    }

    /* 再处理能被4整除的采样点 */
    for (k = excess_point; k < point_num; k += 4) { /* 4: offset */
        (data)[k] = (*(tmp_buf + (k << base)));
        (data)[k + 1] = (*(tmp_buf + ((k + 1) << base)));
        (data)[k + 2] = (*(tmp_buf + ((k + 2) << base))); /* 2: offset */
        (data)[k + 3] = (*(tmp_buf + ((k + 3) << base))); /* 3: offset */
    }
}

static td_void ao_get_aec_data_16bit(td_u16 *dest_buf, const td_u8 *src_buf, td_u32 point_num,
    td_u32 base, td_s32 index)
{
    td_u32 excess_point, k;
    const td_u8 *tmp_buf = src_buf + index * 2; /* 2: 16bit使用2byte储存 */
    td_u16 *data = dest_buf;

    if (point_num == 0) {
        /* 无须处理，直接返回 */
        return;
    }

    excess_point = point_num % 4; /* 用于处理不能被4整除的采样点 */

    /* 先处理不能被4整除的采样点 */
    for (k = 0; k < excess_point; k++) {
        data[k] = (*(td_u16 *)(td_void *)(tmp_buf + (k << base)));
    }

    /* 再处理能被4整除的采样点 */
    for (k = excess_point; k < point_num; k += 4) { /* 4: offset */
        (data)[k] = (*(td_u16 *)(tmp_buf + (k << base)));
        (data)[k + 1] = (*(td_u16 *)(td_void *)(tmp_buf + ((k + 1) << base)));
        (data)[k + 2] = (*(td_u16 *)(td_void *)(tmp_buf + ((k + 2) << base))); /* 2: offset */
        (data)[k + 3] = (*(td_u16 *)(td_void *)(tmp_buf + ((k + 3) << base))); /* 3: offset */
    }
}

static td_void ao_get_aec_data_24bit(td_u32 *dest_buf, const td_u8 *src_buf, td_u32 point_num,
    td_u32 base, td_s32 index)
{
    td_u32 excess_point, k;
    const td_u8 *tmp_buf = src_buf + ((td_slong)index * 4); /* 4: 24bit使用4byte储存 */
    td_u32 *data = dest_buf;

    if (point_num == 0) {
        /* 无须处理，直接返回 */
        return;
    }

    excess_point = point_num % 4; /* 用于处理不能被4整除的采样点 */

    /* 先处理不能被4整除的采样点 */
    for (k = 0; k < excess_point; k++) {
        data[k] = (*(td_u32 *)(td_void *)(tmp_buf + (k << base)));
    }

    /* 再处理能被4整除的采样点 */
    for (k = excess_point; k < point_num; k += 4) { /* 4: offset */
        (data)[k] = (*(td_u32 *)(td_void *)(tmp_buf + (k << base)));
        (data)[k + 1] = (*(td_u32 *)(td_void *)(tmp_buf + ((k + 1) << base)));
        (data)[k + 2] = (*(td_u32 *)(td_void *)(tmp_buf + ((k + 2) << base))); /* 2: offset */
        (data)[k + 3] = (*(td_u32 *)(td_void *)(tmp_buf + ((k + 3) << base))); /* 3: offset */
    }
}

td_s32 ao_drv_get_aec_data(ot_audio_dev audio_dev_id, ot_ao_chn ao_chn, ot_audio_frame *frm, td_u32 rptr_off_set)
{
    aio_drv_dev_ctx *aio_drv_dev = &g_ao_drv_dev[audio_dev_id];
    td_s32 index;
    td_u8 *dma_buf = TD_NULL;
    td_u8 *start_buf = TD_NULL;
    td_u8 *dest_buf = TD_NULL;
    td_u32 base, point_num, point_num_to_end;

    ao_check_null_ptr_return(frm);

    if (rptr_off_set == aio_drv_dev->cir_buf.buf_size) {
        rptr_off_set = 0;
    }

    if (aio_drv_dev->cir_buf.vir_addr == TD_NULL) {
        ao_err_trace("AO CB buffer is NULL\n");
        return OT_ERR_AO_NOT_ENABLED;
    }

    dma_buf = aio_drv_dev->cir_buf.vir_addr + rptr_off_set;
    if (dma_buf > aio_drv_dev->cir_buf.vir_addr + aio_drv_dev->cir_buf.buf_size) {
        ao_err_trace("buffer addr is illegal!\n");
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    start_buf = aio_drv_dev->cir_buf.vir_addr;
    if (start_buf == TD_NULL) {
        ao_err_trace("AO not init(CB buffer is NULL)\n");
        return OT_ERR_AO_NOT_ENABLED;
    }

    /* AEC获取环形buf末端的空闲数据点数 */
    point_num_to_end = ao_aec_get_spare_point_num(aio_drv_dev, rptr_off_set);

    /* temporary variable */
    base = aio_drv_dev->fifo_len_base;
    point_num = aio_drv_dev->aio_attr.point_num_per_frame;

    index = aio_drv_dev->chn_index[ao_chn];
    dest_buf = frm->virt_addr[0];

    if ((aio_drv_dev->aio_attr.bit_width == OT_AUDIO_BIT_WIDTH_8) && (aio_drv_dev->aio_attr.expand_flag == 0)) {
        ao_get_aec_data_8bit(dest_buf, dma_buf, point_num_to_end, base, index);

        /* 必要时处理需要折回的情况 */
        ao_get_aec_data_8bit(dest_buf + point_num_to_end, start_buf, point_num - point_num_to_end, base, index);
    } else if (((aio_drv_dev->aio_attr.bit_width == OT_AUDIO_BIT_WIDTH_8) &&
               (aio_drv_dev->aio_attr.expand_flag == OT_AI_EXPAND)) ||
               (aio_drv_dev->aio_attr.bit_width == OT_AUDIO_BIT_WIDTH_16)) {
        ao_get_aec_data_16bit((td_u16 *)(td_void *)dest_buf, dma_buf, point_num_to_end, base, index);

        /* 必要时处理需要折回的情况 */
        ao_get_aec_data_16bit((td_u16 *)(td_void *)dest_buf + point_num_to_end, start_buf,
            point_num - point_num_to_end, base, index);
    } else if (aio_drv_dev->aio_attr.bit_width == OT_AUDIO_BIT_WIDTH_24) {
        ao_get_aec_data_24bit((td_u32 *)(td_void *)dest_buf, dma_buf, point_num_to_end, base, index);

        /* 必要时处理需要折回的情况 */
        ao_get_aec_data_24bit((td_u32 *)(td_void *)dest_buf + point_num_to_end, start_buf,
            point_num - point_num_to_end, base, index);
    } else {
        ao_info_trace("bit_width error\n");
        return OT_ERR_AO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

td_void ao_drv_get_dma_rdpt(ot_audio_dev audio_dev_id, td_u32 *read_offset)
{
    aio_export_func *aio_exp_func = TD_NULL;

    /* get int status */
    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if ((aio_exp_func == TD_NULL) || (aio_exp_func->pfn_get_ao_dev_buf_rptr == TD_NULL)) {
        ao_emerg_trace("aio module may not insmod\n");
        return;
    }

    *read_offset = aio_exp_func->pfn_get_ao_dev_buf_rptr(audio_dev_id);
}

td_s32 ao_drv_get_dma_buf_left_len(ot_audio_dev audio_dev_id, td_u32 *left_byte)
{
    aio_export_func *aio_exp_func = TD_NULL;
    aio_drv_dev_ctx *aio_drv_dev = &g_ao_drv_dev[audio_dev_id];
    td_u32 write_offset, read_offset;

    aio_exp_func = func_entry(aio_export_func, OT_ID_AIO);
    if ((aio_exp_func == TD_NULL) || (aio_exp_func->pfn_get_ao_dev_buf_wptr == TD_NULL) ||
        (aio_exp_func->pfn_get_ao_dev_buf_rptr == TD_NULL)) {
        ao_emerg_trace("aio module may not insmod\n");
        return OT_ERR_AO_NOT_READY;
    }

    write_offset = aio_exp_func->pfn_get_ao_dev_buf_wptr(audio_dev_id);
    read_offset = aio_exp_func->pfn_get_ao_dev_buf_rptr(audio_dev_id);
    if (write_offset >= read_offset) {
        *left_byte = write_offset - read_offset;
    } else {
        *left_byte = write_offset + aio_drv_dev->cir_buf.buf_size - read_offset;
    }

    return TD_SUCCESS;
}
