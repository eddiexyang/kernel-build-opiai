/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2022. All rights reserved.
 */

#ifndef HI_TYPE_H__
#define HI_TYPE_H__

#include "HiDvppType.h"
#ifdef __KERNEL__

#include <linux/types.h>
#elif defined(__LITEOS__)
#include "los_typedef.h"
#include <sys/types.h>
#define __iomem
#else

#include <stdint.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*--------------------------------------------------------------------------------------------------------------*
 * Defintion of basic data types. The data types are applicable to both the application layer and kernel codes. *
 *--------------------------------------------------------------------------------------------------------------*/
/*************************** Structure Definition ****************************/
/** \addtogroup      Common_TYPE */
/** @{ */  /** <!-- [Common_TYPE] */

typedef unsigned long           HI_UL;
typedef uintptr_t               HI_UINTPTR_T;

#ifndef _M_IX86

#else
    typedef unsigned __int64    HI_U64;
#endif

/*----------------------------------------------*
 * const defination                             *
 *----------------------------------------------*/

typedef unsigned char           hi_uchar;
typedef unsigned char           hi_u8;
typedef unsigned short          hi_u16;
typedef unsigned int            hi_u32;
typedef unsigned long           hi_ulong;
typedef char                    hi_char;
typedef signed char             hi_s8;
typedef short                   hi_s16;
typedef int                     hi_s32;
typedef long                    hi_slong;

typedef float                   hi_float;
typedef double                  hi_double;

#ifdef DVPP_UTST
#define hi_void void
#else
typedef void                    hi_void;
#endif

#ifndef _M_IX86
    typedef unsigned long long  hi_u64;
    typedef long long           hi_s64;
#else
    typedef unsigned __int64    hi_u64;
    typedef __int64             hi_s64;
#endif

typedef unsigned long           hi_size_t;

typedef HI_BOOL                 hi_bool;
typedef HI_UINTPTR_T            hi_uintptr_t;

typedef hi_u64                  hi_phys_addr_t;

/** @} */  /** <!-- ==== Structure Definition end ==== */

#define HI_UNUSED(x)            ((x) = (x))

#define HI_BOOL_TO_UNSIGNED(is_true) ((is_true) ? 1 : 0)

#define CHECK_DO_SOMETHING(a, something) \
    {                                    \
        if (a) {                      \
            something;                   \
        }                                \
    }

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* HI_TYPE_H__ */

