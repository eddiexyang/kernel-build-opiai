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

#ifndef HI_DVPP_TYPE_H
#define HI_DVPP_TYPE_H

#include <linux/types.h>

typedef unsigned char HI_U8;
typedef signed char HI_S8;
typedef unsigned short HI_U16;
typedef short HI_S16;
typedef unsigned int HI_U32;
typedef int HI_S32;
typedef unsigned long long HI_U64;
typedef long long HI_S64;
typedef char HI_CHAR;
typedef double HI_DOUBLE;
typedef HI_U32 HI_FR32;
typedef float HI_FLOAT;
#define HI_VOID void
#define HI_NULL NULL
#define HI_SUCCESS 0
#define HI_FAILURE (-1)

typedef enum {
    HI_FALSE = 0,
    HI_TRUE = 1,
} HI_BOOL;

#endif // #ifndef HI_DVPP_TYPE_H