/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2015-2020. All rights reserved.
 * Description: audio_osal.h
 * Author: Hisilicon multimedia software group
 * Create: 2015/4/9
 */

#ifndef __ASP_DMA_HAL_H__
#define __ASP_DMA_HAL_H__

#include "aiao_hal_comm.h"
#include "ot_type.h"

#define CHN_CNT_NUM_01 1U
#define CHN_CNT_NUM_02 2U
#define CHN_CNT_NUM_03 3U
#define CHN_CNT_NUM_04 4U
#define CHN_CNT_NUM_05 5U
#define CHN_CNT_NUM_06 6U
#define CHN_CNT_NUM_07 7U
#define CHN_CNT_NUM_08 8U
#define CHN_CNT_NUM_09 9U
#define CHN_CNT_NUM_10 10U
#define CHN_CNT_NUM_11 11U
#define CHN_CNT_NUM_12 12U
#define CHN_CNT_NUM_13 13U
#define CHN_CNT_NUM_14 14U
#define CHN_CNT_NUM_15 15U
#define CHN_CNT_NUM_16 16U

void hi3xxx_asp_dmac_prepare(ot_audio_dev audio_dev_id, td_bool is_ao);
td_s32 hi19xx_asp_dma_open(ot_audio_dev audio_dev_id, td_bool is_ao, td_u32 chn_cnt);
td_s32 hi19xx_asp_dma_init(aio_drv_dev_ctx *aio_drv_dev, td_u32 lli_offset, ot_audio_dev audio_dev_id, td_bool is_ao);
td_s32 asp_dmac_trigger(ot_audio_dev audio_dev_id, td_bool is_ao);
td_s32 asp_dmac_stop(ot_audio_dev audio_dev_id, td_bool is_ao);
void convert_24to16bit_and_interlace(const aio_drv_dev_ctx *ai_drv_dev, void *src_buf, void *dst);
void convert_24to16bit(const aio_drv_dev_ctx *ai_drv_dev, void *src_buf, void *dst);
void deinterlace_and_convert16_24(const aio_drv_dev_ctx *ai_drv_dev, void *dst, const void *src);
#endif

