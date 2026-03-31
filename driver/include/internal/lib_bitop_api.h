/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
 * Create: 2019-10-15
 */

#ifndef LIB_BITOP_API_H
#define LIB_BITOP_API_H

#include <linux/types.h>

/* register read/write */
/* template */
#define REG_OFFSET_READ(_type, base, rel_offset)                \
    ((_type)(*(volatile _type *)(uintptr_t)(((uintptr_t)(base) + (uintptr_t)(rel_offset)))))
#define REG_OFFSET_WRITE(_type, base, rel_offset, data)                     \
    do {                                                                    \
        *(volatile _type *)(uintptr_t)(((uintptr_t)(base) + (uintptr_t)(rel_offset))) = (_type)(data); \
    } while (0)

/* 32 bit */
#define REG_32_READ(base, rel_offset)           REG_OFFSET_READ(u32, base, rel_offset)
#define REG_32_WRITE(base, rel_offset, data)    REG_OFFSET_WRITE(u32, base, rel_offset, data)

/* bit operation */
/* template */
/* create mask by high and low bit */
#define TEMP_BIT_MASK(suffix, hi, lo)           (((1##suffix << (((hi) - (lo)) + 1)) - 1) << (lo))
/* set the NO.y bit of x to 1 */
#define TEMP_SET_BIT(_type, suffix, x, y)           ((x) |= (_type)(1##suffix << (y)))
/* set the NO.y bit of x to 0 */
#define TEMP_CLR_BIT(_type, suffix, x, y)           ((x) = (_type)((x) & (~(1##suffix << (y)))))
/* judge the NO.y bit of x is 1 or not */
#define TEMP_TST_BIT(suffix, x, y)                  (((x) >> (y)) & 0x1##suffix)
/* get the NO.y bit of x */
#define TEMP_GET_BIT(suffix, x, y)                  TEMP_TST_BIT(suffix, x, y)
/* set the bits from low bit to high bit as val */
#define TEMP_SET_BITS(_type, suffix, dst, hi, lo, val) \
    ((dst) = (_type)(((dst) & (~(TEMP_BIT_MASK(suffix, hi, lo)))) | \
    (((val) << (lo)) & (TEMP_BIT_MASK(suffix, hi, lo)))))
/* set the bits from low bit to high bit to 0 */
#define TEMP_CLR_BITS(suffix, x, hi, lo)            TEMP_SET_BITS(suffix, x, hi, lo, 0)
/* get the val from low bit to high bit from x */
#define TEMP_GET_BITS(suffix, x, hi, lo)            (((x) >> (lo)) & ((1##suffix << (((hi) - (lo)) + 1)) - 1))
/* mask the low bit from 0 to len, len should be in (0, 64) */
#define TEMP_VALUE_MASK(_type, suffix, value, len)  \
                (((_type)(value)) & TEMP_BIT_MASK(suffix, ((_type)(len)) - 1, 0))

/* u32 bit operation */
#define SET_BIT(x, y)                   TEMP_SET_BIT(u32, U, x, y)
#define CLR_BIT(x, y)                   TEMP_CLR_BIT(u32, U, x, y)
#define TST_BIT(x, y)                   TEMP_TST_BIT(U, x, y)
#define GET_BIT(x, y)                   TST_BIT(x, y)
#define SET_BITS(dst, hi, lo, val)      TEMP_SET_BITS(u32, U, dst, hi, lo, val)
#define CLR_BITS(x, hi, lo)             SET_BITS(x, hi, lo, 0)
#define GET_BITS(x, hi, lo)             TEMP_GET_BITS(U, x, hi, lo)
#define VALUE_MASK(value, len)          TEMP_VALUE_MASK(u32, U, value, len)

#define BIT_0              0
#define BIT_31             31
#define BIT_LEN            32

#endif /* _LIB_BITOP_API_H_ */