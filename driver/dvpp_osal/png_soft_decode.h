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

#ifndef DVPP_PNG_PNGDECODE_H
#define DVPP_PNG_PNGDECODE_H

#include <cstdint>
#include "HiDvppCommon.h"
#include "hi_inner_pngd.h"

typedef struct {
    uint8_t *addr;
    uint32_t len;
    uint32_t read_size;
    uint32_t left_size;
} png_buf;

extern "C" hi_s32 pngd_check_stride(const hi_img_stream *stream, hi_pic_info *png_pic_info);

#endif // DVPP_PNG_PNGDECODE_H
