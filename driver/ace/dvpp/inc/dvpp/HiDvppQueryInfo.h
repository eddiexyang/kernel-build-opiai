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

#ifndef HI_DVPP_QUERYINFO_H
#define HI_DVPP_QUERYINFO_H

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

typedef struct {
    unsigned char vdec;
    unsigned char vpc;
    unsigned char venc;
    unsigned char jpege;
    unsigned char jpegd;
} hi_subsys_status;

typedef struct {
    hi_subsys_status status[3];
} hi_dvpp_status;

typedef struct {
    int32_t vdec;
    int32_t vpc;
    int32_t venc;
    int32_t jpege;
    int32_t jpegd;
} hi_dvpp_utilization_ratio;

#endif // HI_DVPP_QUERYINFO_H