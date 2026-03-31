/*
 * @file bbox_types.h
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2022. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef BBOX_TYPES_H
#define BBOX_TYPES_H

// signed type
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

// unsigned type
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef unsigned long   lock_flag_t;
typedef unsigned long   bbox_ioctl_arg_t;
typedef unsigned long   bbox_jiffies_t;
typedef unsigned long   bbox_event_t;
typedef unsigned long   bbox_bit_t;
typedef unsigned long   bbox_byte_t;
typedef void            bbox_arg_t;
typedef long            bbox_long_t;

typedef struct excep_time {
    u64 tv_sec;
    u64 tv_usec;
} excep_time;

#define BBOX_DISALLOW_REENTRANT 1   // exception report value
#define BBOX_SUCCESS            0
#define BBOX_FAILURE            (-1)

// inner return value
#define BBOX_EOK            0
#define BBOX_COMM_INVAL     (-2)    // Communication failure
#define BBOX_COMM_TIMEOUT   (-3)    // Communication timeout
#define BBOX_MSG_INVAL      (-4)    // Message invalid
#define BBOX_MSG_NONE       (-5)    // No have data
#define BBOX_NO_SUPPORT     (-10)   // No support
#define BBOX_ENODEV         (-19)   // No such device
#define BBOX_EINVAL         (-22)   // Invalid argument
#define BBOX_ENOSPC         (-28)   // No space left on device

#define BBOX_TRUE    1
#define BBOX_FALSE   0
#define BBOX_UCHAR_INVALID          0xFF
#define BBOX_UINT_INVALID           0xFFFFFFFFU
#define BBOX_MODULE_NAME_LEN        16
#define BBOX_EXCEPTIONDESC_MAXLEN   48

#endif // BBOX_TYPES_H

