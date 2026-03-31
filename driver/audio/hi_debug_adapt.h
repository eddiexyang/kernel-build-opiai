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

#ifndef HI_DEBUG_ADAPT_H__
#define HI_DEBUG_ADAPT_H__

#include "hi_type.h"
#include "hi_common_adapt.h"
#include "hi_debug.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef struct {
    hi_mod_id  mod_id;
    hi_s32  level;
    hi_char mod_name[16];
} hi_log_level_conf;

#define MPI_STATIC static inline

#ifdef __HuaweiLite__

#ifdef CONFIG_HI_LOG_TRACE_SUPPORT

#define MPI_ASSERT(expr) \
    _Static_assert((expr),  #expr " is not true")

#else

#define MPI_ASSERT(expr)

#endif

#else

#ifndef __KERNEL__

#ifdef CONFIG_HI_LOG_TRACE_SUPPORT

#define MPI_ASSERT(expr)  \
    _Static_assert((expr),  #expr " is not true")

#else

#define MPI_ASSERT(expr)

#endif

#endif

#endif

#ifdef __KERNEL__

hi_s32 hi_log(hi_s32 level, hi_mod_id mod_id, const hi_char *fmt, ...) __attribute__((format(printf, 3, 4)));

#endif /* end of __KERNEL__ */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* HI_DEBUG_ADAPT_H__ */

