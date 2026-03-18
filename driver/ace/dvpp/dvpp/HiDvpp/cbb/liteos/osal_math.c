/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2016-2020.
 * Description: osal math source file.
 * Author: Hisilicon multimedia software group
 * Create: 2016-11-11
 */

#include "hi_osal.h"

static inline hi_u64 div64_u64(hi_u64 dividend, hi_u64 divisor)
{
    return dividend / divisor;
}

static inline hi_s64 div64_s64(hi_s64 dividend, hi_s64 divisor)
{
    return dividend / divisor;
}

static inline hi_s64 div_s64_rem(hi_s64 dividend, hi_s32 divisor,
                                 hi_s32 *remainder)
{
    *remainder = dividend % divisor;
    return dividend / divisor;
}

static inline hi_u64 div64_u64_rem(hi_u64 dividend, hi_u64 divisor,
                                   hi_u64 *remainder)
{
    *remainder = dividend % divisor;
    return dividend / divisor;
}

static inline hi_u64 div_u64_rem(hi_u64 dividend, hi_u32 divisor,
                                 hi_u32 *remainder)
{
    *remainder = dividend % divisor;
    return dividend / divisor;
}

static inline hi_s64 div_s64(hi_s64 dividend, hi_s32 divisor)
{
    hi_s32 remainder;
    return div_s64_rem(dividend, divisor, &remainder);
}

static inline hi_u64 div_u64(hi_u64 dividend, hi_u32 divisor)
{
    hi_u32 remainder;
    return div_u64_rem(dividend, divisor, &remainder);
}

/* the result of u64/u32. */
hi_u64 osal_div_u64(hi_u64 dividend, hi_u32 divisor)
{
    return div_u64(dividend, divisor);
}

/* the result of s64/s32. */
hi_s64 osal_div_s64(hi_s64 dividend, hi_s32 divisor)
{
    return div_s64(dividend, divisor);
}

/* the result of u64/u64. */
hi_u64 osal_div64_u64(hi_u64 dividend, hi_u64 divisor)
{
    return div64_u64(dividend, divisor);
}

/* the result of s64/s64. */
hi_s64 osal_div64_s64(hi_s64 dividend, hi_s64 divisor)
{
    return div64_s64(dividend, divisor);
}

/* the remainder of u64/u32. */
hi_u64 osal_div_u64_rem(hi_u64 dividend, hi_u32 divisor)
{
    unsigned int remainder = 0;
    div_u64_rem(dividend, divisor, &remainder);

    return remainder;
}

/* the remainder of u64/u64. */
hi_u64 osal_div64_u64_rem(hi_u64 dividend, hi_u64 divisor)
{
    hi_u64 remainder = 0;

    div64_u64_rem(dividend, divisor, &remainder);
    return remainder;
}

hi_u32 osal_random(void)
{
    HI_TRACE_OSAL(HI_DBG_ERR, "not supported!\n");
    return 0;
}
