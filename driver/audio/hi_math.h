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
 * Create: 2020-8-1
 */

#ifndef HI_MATH_H__
#define HI_MATH_H__

#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/******************************************************************************
** ABS(x)                 absolute value of x
** CMP(x,y)               0 if x==y; 1 if x>y; -1 if x<y
******************************************************************************/
#define ABS(x)          (((x) >= 0) ? (x) : (-(x)))
#define CMP(x, y)       (((x) == (y)) ? 0 : (((x) > (y)) ? 1 : -1))

/******************************************************************************
** MAX2(x,y)              maximum of x and y
** MIN2(x,y)              minimum of x and y
** MIN3(x,y,z)            minimun of x, y and z
******************************************************************************/
#define MAX2(x, y)       (((x) > (y)) ? (x) : (y))
#define MIN2(x, y)       (((x)<(y)) ? (x) : (y))
#define MAX3(x, y, z)     (((x)>(y)) ? MAX2(x, z) : MAX2(y, z))
#define MIN3(x, y, z)    (((x)<(y)) ? MIN2(x, z) : MIN2(y, z))
#define MEDIAN(x, y, z)   (((x)+(y)+(z) - MAX3(x, y, z)) - MIN3(x, y, z))
#define MEAN2(x, y)      (((x)+(y)) >> 1)

/******************************************************************************
** VALUE_BETWEEN(x,min.max)   True if x is between [min,max] inclusively.
******************************************************************************/
#define CLIP_MIN(x, min)          (((x) >= min) ? (x) : min)
#define CLIP3(x, min, max)         (((x)< (min)) ? (min) : ((x)>(max)?(max):(x)))
#define CLIP_MAX(x, max)          ((x)>(max)?(max):(x))
#define WRAP_MAX(x, max, min)      ((x)>=(max) ? (min) : (x))
#define WRAP_MIN(x, min, max)      ((x)<=(min) ? (max) : (x))
#define VALUE_BETWEEN(x, min, max) (((x)>=(min)) && ((x) <= (max)))

/******************************************************************************
** MULTI_OF_2_POWER(x,a)  whether x is multiple of a(a must be power of 2)
** ALIGN_DOWN(x,a)     floor x to multiple of a(a must be power of 2)
** ALIGN_UP(x, a)            align x to multiple of a
**
** Example:
** ALIGN_UP(5,4) = 8
** ALIGN_DOWN(5,4)   = 4
******************************************************************************/
static inline hi_bool multi_of_2_power(hi_u64 x, hi_u64 a)
{
    return (hi_bool)((x & (a - 1ULL)) == 0);
}
#define MULTI_OF_2_POWER(x, a)    (!((x) & ((a) - 1U)))
#define HICEILING(x, a)           (((x)+(a)-1U)/(a))

#ifndef ALIGN_DOWN
#define ALIGN_DOWN(x, a)         (((x) / (a)) * (a))
#endif
#define HI_ALIGN_UP(x, a)           ((((x) + ((a) - 1U)) / (a)) * (a))
#define HI_ALIGN_DOWN(x, a)         (((x) / (a)) * (a))
#define DIV_UP(x, a)             (((x) + ((a) - 1U)) / (a))

/******************************************************************************
** ENDIAN32(x,y)              little endian <---> big endian
** IS_LITTLE_END()            whether the system is little end mode
******************************************************************************/
#define  ENDIAN32(x)                    \
    (((x) << 24) |                        \
     (( (x) & 0x0000ff00U) << 8) |        \
     (( (x) & 0x00ff0000U) >> 8) |        \
     (((x) >> 24) & 0x000000ffU))

/******************************************************************************
** ENDIAN16(x,y)              little endian <---> big endian
** IS_LITTLE_END()            whether the system is little end mode
******************************************************************************/
#define  ENDIAN16(x)    ((((x) << 8) & 0xff00U) | (((x) >> 8) & 255U))

/******************************************************************************
** FRACTION32(de,nu)          fraction: nu(minator) / de(nominator).
** NUMERATOR32(x)              of x(x is fraction)
** DENOMINATOR32(x)           Denominator of x(x is fraction)

** represent fraction in 32 bit. LSB 16 is numerator, MSB 16 is denominator
** It is integer if denominator is 0.
******************************************************************************/
#define FRACTION32(de, nu)       (((de) << 16) | (nu))
#define NUMERATOR32(x)          ((x) & 0xffffU)
#define DENOMINATOR32(x)        ((x) >> 16)

/******************************************************************************
** RGB(r,g,b)    assemble the r,g,b to 24bit color
** RGB_R(c)      get RED   from 24bit color
** RGB_G(c)      get GREEN from 24bit color
** RGB_B(c)      get BLUE  from 24bit color
******************************************************************************/
#define RGB(r, g, b) ((((r) & 0xffU) << 16) | (((g) & 0xffU) << 8) | ((b) & 0xffU))
#define RGB_R(c)   (((c) & 0xff0000U) >> 16)
#define RGB_G(c)   (((c) & 0xff00U) >> 8)
#define RGB_B(c)   (  (c) & 0xffU)

/******************************************************************************
** YUV(y,u,v)    assemble the y,u,v to 30bit color
******************************************************************************/
#define YUV(y, u, v) ((((y) & 0x03ffU) << 20U) | (((u) & 0x03ffU) << 10U) | ((v) & 0x03ffU))
#define YUV_Y(c)   (((c) & 0x3ff00000) >> 20)
#define YUV_U(c)   (((c) & 0x000ffc00) >> 10)
#define YUV_V(c)   (  (c) & 0x000003ff)

/******************************************************************************
** YUV_8BIT(y,u,v)    assemble the y,u,v to 24bit color
** YUV_8BIT_Y(c)      get Y from 24bit color
** YUV_8BIT_U(c)      get U from 24bit color
** YUV_8BIT_V(c)      get V from 24bit color
******************************************************************************/
#define YUV_8BIT(y, u, v) ((((y) & 0xff) << 16) | (((u) & 0xff) << 8) | ((v) & 0xff))
#define YUV_8BIT_Y(c)   (((c) & 0xff0000) >> 16)
#define YUV_8BIT_U(c)   (((c) & 0xff00) >> 8)
#define YUV_8BIT_V(c)   (  (c) & 0xff)

#define ADDR_0_16_BITS(addr) ((hi_u16)(addr))
#define ADDR_32_48_BITS(addr) ((hi_u16)((addr) >> 32U))

__inline static HI_U32 GetLowAddr(HI_U64 u64Phyaddr)
{
    return (HI_U32)u64Phyaddr;
}

__inline static HI_U32 GetHighAddr(HI_U64 u64Phyaddr)
{
    return (HI_U32)(u64Phyaddr >> 32); // 32 : bit
}

__inline static HI_VOID Rgb2Yc_full_8BIT(HI_U8 r, HI_U8 g, HI_U8 b, HI_U8 *py, HI_U8 *pcb, HI_U8 *pcr)
{
    HI_S16 py_temp, pcb_temp, pcr_temp;

    py_temp = (r * 76 + g * 150 + b * 29) >> 8; // 76:algo param, 150:algo param, 29:algo param, 8:bit
    pcb_temp = (((b * 130 - r * 44) - g * 86) >> 8) + 128; // 130:algo, 44:algo, 86:algo, 8:bit, 128:offset
    pcr_temp = (((r * 130 - g * 109) - b * 21) >> 8) + 128; // 130:algo, 109:algo, 21:algo; 8:bit, 128:offset

    *py = MAX2(MIN2(py_temp, 255), 0);
    *pcb = MAX2(MIN2(pcb_temp, 255), 0);
    *pcr = MAX2(MIN2(pcr_temp, 255), 0);
}

__inline static HI_U32 Rgb2Yuv_full_8BIT(HI_U32 u32Rgb)
{
    HI_U8 y, u, v;

    Rgb2Yc_full_8BIT(RGB_R(u32Rgb), RGB_G(u32Rgb), RGB_B(u32Rgb), &y, &u, &v);

    return YUV_8BIT(y, u, v);
}

#define hi_usleep(usec) \
    do { \
        (HI_VOID)usleep(usec); \
    } while (0)

__inline static hi_u32 get_num_from_bitmap(hi_u32 bitmap)
{
    hi_u32 num = 0;
    hi_u32 i = 0;
    for (i = 0; i < 32U; i++) {
        if (((bitmap >> i) & 0x1U) == 1U) {
            num++;
        }
    }
    return num;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* HI_MATH_H__ */
