/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2023. All rights reserved.
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
 * Create: 2020-4-20
 */
#include "hi_osal.h"

/* the result of u64/u32. */
hi_u64 osal_div_u64(hi_u64 dividend, hi_u32 divisor)
{
    if (divisor == 0) {
        HI_TRACE_OSAL(HI_DBG_ERR, "error: %s divisor cannot be zero.\n", __FUNCTION__);
        return 0;
    }
    return dividend / divisor;
}

/* the result of s64/s32. */
hi_s64 osal_div_s64(hi_s64 dividend, hi_s32 divisor)
{
    if (divisor == 0) {
        HI_TRACE_OSAL(HI_DBG_ERR, "error: %s divisor cannot be zero.\n", __FUNCTION__);
        return 0;
    }
    return dividend / divisor;
}

/* the result of u64/u64. */
hi_u64 osal_div64_u64(hi_u64 dividend, hi_u64 divisor)
{
    if (divisor == 0) {
        HI_TRACE_OSAL(HI_DBG_ERR, "error: %s divisor cannot be zero.\n", __FUNCTION__);
        return 0;
    }
    return dividend / divisor;
}

/* the result of s64/s64. */
hi_s64 osal_div64_s64(hi_s64 dividend, hi_s64 divisor)
{
    if (divisor == 0) {
        HI_TRACE_OSAL(HI_DBG_ERR, "error: %s divisor cannot be zero.\n", __FUNCTION__);
        return 0;
    }
    return dividend / divisor;
}

/* the remainder of u64/u32. */
hi_u64 osal_div_u64_rem(hi_u64 dividend, hi_u32 divisor)
{
    if (divisor == 0) {
        HI_TRACE_OSAL(HI_DBG_ERR, "error: %s divisor cannot be zero.\n", __FUNCTION__);
        return 0;
    }
    return dividend % divisor;
}

/* the remainder of s64/s32. */
hi_s64 osal_div_s64_rem(hi_s64 dividend, hi_s32 divisor)
{
    if (divisor == 0) {
        HI_TRACE_OSAL(HI_DBG_ERR, "error: %s divisor cannot be zero.\n", __FUNCTION__);
        return 0;
    }
    return dividend % divisor;
}

/* the remainder of u64/u64. */
hi_u64 osal_div64_u64_rem(hi_u64 dividend, hi_u64 divisor)
{
    if (divisor == 0) {
        HI_TRACE_OSAL(HI_DBG_ERR, "error: %s divisor cannot be zero.\n", __FUNCTION__);
        return 0;
    }
    return dividend % divisor;
}

// 暂无实现随机数方案，暂时返回0
// DVPP业务暂时暂未实际使用这个接口
hi_u32 osal_random(void)
{
    return 0;
}
