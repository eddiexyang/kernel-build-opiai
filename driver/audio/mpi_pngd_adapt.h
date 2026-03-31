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

#ifndef MPI_PNGD_ADAPT_H__
#define MPI_PNGD_ADAPT_H__

#include "hi_comm_video_adapt.h"
#include "hi_debug_adapt.h"
#include "hi_comm_pngd_adapt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif // end of #ifdef __cplusplus

struct mpi_context {
    hi_s32 chan_type;
    hi_bool is_acl;
};

hi_s32 hi_mpi_pngd_create_chn(hi_pngd_chn chn_id, const hi_pngd_chn_attr *attr, hi_bool is_acl);
hi_s32 hi_mpi_pngd_create_chn_ex(hi_pngd_chn *chn_id, const hi_pngd_chn_attr_ex *attr_ex);
hi_s32 hi_mpi_pngd_create_chn2(hi_pngd_chn chn_id, const hi_pngd_chn_attr *attr, hi_u32 flag);
hi_s32 hi_mpi_pngd_destroy_chn(hi_pngd_chn chn_id);

/* milli_sec: -1 is block,0 is no block,other positive number is timeout */
hi_s32 hi_mpi_pngd_send_stream(hi_pngd_chn chn_id, const hi_img_stream *stream,
                               hi_pic_info *png_pic_info, hi_s32 milli_sec);
hi_s32 hi_mpi_pngd_get_image_data(hi_pngd_chn chn_id, hi_pic_info *png_pic_info,
                                  hi_img_stream *stream, hi_s32 milli_sec);
hi_s32 hi_mpi_png_get_image_info(const hi_img_stream *png_stream, hi_img_info *img_info);

hi_s32 pngd_soft_decode(hi_pic_info *png_pic_info, hi_img_stream *stream);

hi_s32 pngd_soft_parse_stream(const hi_img_stream *stream, hi_img_info *stImgInfo);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif // end of #ifdef __cplusplus

#endif // end of #ifndef  MPI_PNGD_ADAPT_H__