/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * Description:
 * Author: huawei
 * Create: 2020-4-1
 */

#ifndef HI_COMM_PNGD_ADAPT_H__
#define HI_COMM_PNGD_ADAPT_H__
#include "hi_type.h"
#include "hi_common.h"
#include "hi_errno.h"
#include "hi_comm_video.h"
#include "hi_defines.h"
#include "hi_common_adapt.h"
#include "hi_comm_video_adapt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif // end of #ifdef __cplusplus

typedef struct {
    void *img_address;
    hi_u32 img_size;
    hi_u32 width;
    hi_u32 height;
    hi_u32 width_stride;
    hi_u32 height_stride;
    hi_pixel_format img_format;
    hi_u32 reserved[4];
} hi_img_data;

typedef struct {
    hi_u32 stream_que_cnt;  // reserved
    hi_u64 reserved[4];
} hi_pngd_chn_attr;

typedef struct {
    hi_pngd_chn_attr chanl_attr;
    hi_bool event_notify; // RW; Event Notify
    hi_void *user_data;
} hi_pngd_chn_attr_ex;

typedef struct {
    hi_void *picture_address;
    hi_u32 picture_buffer_size;
    hi_u32 picture_width;
    hi_u32 picture_height;
    hi_u32 picture_width_stride;
    hi_u32 picture_height_stride;
    hi_pixel_format picture_format;
} hi_pic_info;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif // end of #ifdef __cplusplus

#endif // end of #ifndef  HI_COMM_PNGD_ADAPT_H__
