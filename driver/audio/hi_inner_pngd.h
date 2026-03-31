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

#ifndef __HI_INNER_PNGD_H__
#define __HI_INNER_PNGD_H__
#include "hi_comm_pngd_adapt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif // end of #ifdef __cplusplus

typedef struct {
    hi_s32 milli_sec;
    hi_pic_info ATTRIBUTE *png_pic_info;
    hi_img_stream ATTRIBUTE *stream;
} pngd_get_image_info;

typedef struct {
    hi_s32 milli_sec;
    hi_u32 pid;
    hi_img_stream stream;
    hi_pic_info png_pic_info;
} pngd_send_stream_info;

typedef struct {
    hi_u32 pid;
    hi_s32 is_himpi;
    hi_void *user_data;
} pngd_acl_info;

typedef struct {
    hi_bool is_acl;
    hi_s32 chan_id;
    hi_s32 user_chan_id;
    hi_pngd_chn_attr attr;
    hi_u32 logic_dev_id;
} pngd_create_info;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif // end of #ifdef __cplusplus

#endif // __HI_INNER_PNGD_H__
