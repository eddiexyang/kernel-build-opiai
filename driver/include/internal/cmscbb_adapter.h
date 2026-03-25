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

#ifndef CMSCBB_ADAPTER_H
#define CMSCBB_ADAPTER_H

#if ((defined CFG_SOC_PLATFORM_CLOUD) || (defined CFG_SOC_PLATFORM_MDC_V51) || (defined CFG_SOC_PLATFORM_MINI)) && \
    (!defined LINUX_KERNEL_BUILD)
#include "cmscbb_types.h"
#include "cmscbb_err_def.h"
#include "cmscbb_plt_def.h"
#include "cmscbb_sdk.h"

#else
#ifndef LINUX_KERNEL_BUILD
#include <stdio.h>
#include <stdarg.h>
#else
#include <linux/types.h>
#include <linux/acpi.h>
#endif

/* CVB: CMS Verify CBB */
typedef unsigned char   CVB_UINT8;
typedef char            CVB_INT8;
typedef unsigned short  CVB_UINT16;
typedef short           CVB_INT16;

#if defined(__LP64__) || defined(__64BIT__)
typedef unsigned int    CVB_UINT32;
typedef int             CVB_INT32;
typedef unsigned long long  CVB_ULONG;
typedef long long           CVB_LONG;
#else   /* int length is 16 for 16bit system */
typedef unsigned long   CVB_UINT32;
typedef long            CVB_INT32;
typedef unsigned long   CVB_ULONG;
typedef unsigned long   CVB_LONG;
#endif

typedef unsigned char   CVB_BYTE;
typedef char            CVB_CHAR;
typedef CVB_UINT32      CVB_BOOL;
typedef int             CVB_INT;
typedef unsigned int    CVB_UINT;
typedef size_t          CVB_SIZE_T;
typedef void*           CVB_FILE_HANDLE;
typedef va_list         cvb_va_list;

#define BIT_COUNT_OF_BYTE 8

#if defined(_MSC_VER)
typedef __int64 CVB_TIME_T;
#else
typedef long long CVB_TIME_T;
#endif

#define CMSCBB_MAX_INT  ((CVB_UINT32)-1)

#ifndef __CVB_NO_SEC_64BIT
#if defined(_MSC_VER)
typedef unsigned __int64  CVB_UINT64;
typedef __int64  CVB_INT64;
typedef CVB_ULONG       CVB_PTR;
#else
#   if defined(__linux__)
#ifndef LINUX_KERNEL_BUILD
#       include<stdint.h>
#endif
typedef uintptr_t       CVB_PTR;
#   else
typedef CVB_ULONG       CVB_PTR;
#       if defined(__vxworks__)
#           if !defined(VXWORKS_VERSION) && !defined(__INCvxTypesh)
#               error "not able to identify vxworks platform"
#           endif  /* VXWORKS_VERSION */
#       endif  /* __vxworks__ */
#       if defined(VXWORKS_VERSION) || defined(__INCvxTypesh)
#           include<vxWorks.h>
#       else
#           include<inttypes.h>
#       endif  /* VXWORKS_VERSION */
#   endif  /* __linux__ */
#   if (defined(__GNUC__) && (__GNUC__< 3))
typedef unsigned long long  CVB_UINT64;
typedef long long  CVB_INT64;
#   else
typedef uint64_t CVB_UINT64;
typedef int64_t CVB_INT64;
#   endif
#endif  /* _MSC_VER */
#endif /* __CVB_NO_SEC_64BIT */

/* define error code */
typedef CVB_UINT32 CMSCBB_ERROR_CODE;

#ifndef CVB_SUCCESS
#define CVB_SUCCESS     0
#endif

#ifndef CVB_NULL
#define CVB_NULL        0
#endif
#ifndef CVB_TRUE
#define CVB_TRUE        (CVB_BOOL)(1)
#endif
#ifndef CVB_FALSE
#define CVB_FALSE       (CVB_BOOL)(0)
#endif
#ifndef CVB_VOID
#define CVB_VOID        void
#endif

/* maximum big integer length */
#define CMSCBB_MAX_INT_DIGITS 512UL

/* big integer define, big-endian */
typedef struct cmscbb_bigint_st {
    CVB_UINT32  uiLength;
    CVB_BYTE    aVal[CMSCBB_MAX_INT_DIGITS];
} CmscbbBigInt;

/* File Interface switch */
#ifndef CMSCBB_SUPPORT_FILE
#define CMSCBB_SUPPORT_FILE 1
#endif

/* Log function switch, recommended to enable */
#ifndef CMSCBB_ENABLE_LOG
#define CMSCBB_ENABLE_LOG 1
#endif /* !CMSCBB_ENABLE_LOG */

#ifdef CMSCBB_ENABLE_LOG
/* Define log output level */
#define CMSCBB_LOG_LEVEL_DEBUG 4   /* Output debugging, information, alarms, and error logs */
#define CMSCBB_LOG_LEVEL_INFO 3    /* Output information, alarms, and error logs */
#define CMSCBB_LOG_LEVEL_WARNING 2 /* Output Alarms and error logs */
#define CMSCBB_LOG_LEVEL_ERROR 1   /* Output error Log */
#define CMSCBB_LOG_LEVEL_NONE 0    /* Do not output log */

#ifndef _CMSCBB_LOG_LEVEL_
#define _CMSCBB_LOG_LEVEL_ CMSCBB_LOG_LEVEL_INFO
#endif

/* log type */
typedef enum {
    CMSCBB_LOG_TYPE_ERROR = 0,
    CMSCBB_LOG_TYPE_WARNING = 1,
    CMSCBB_LOG_TYPE_INFO = 2,
    CMSCBB_LOG_TYPE_DEBUG = 3,
} CMSCBB_LOG_TYPE;
#endif /* CMSCBB_ENABLE_LOG */

#define	CMSCBB_HASH_UNKONW (-1)
#define	CMSCBB_HASH_SHA256 43
#define	CMSCBB_HASH_SHA384 44
#define	CMSCBB_HASH_SHA512 45

typedef void* CMSCBB_CRYPTO_MD_CTX;
typedef void* CMSCBB_CRYPTO_VRF_CTX;

#ifndef CMSCBB_MAX_DIGEST_SIZE
#define CMSCBB_MAX_DIGEST_SIZE 64
#endif

#define CMSCBB_ERR_UNDEFINED (CVB_UINT32)(-1)
#define CMSCBB_ERR_SYS_BASE                   0x88000000
#define CMSCBB_ERR_SYS_MEM_ALLOC              0x88000001
#define CMSCBB_ERR_SYS_MEM_SET                0x88000003

#define CMSCBB_ERR_CONTEXT_INVALID_PARAM      0x88100001
#define CMSCBB_ERR_CONTEXT_INVALID_STRUCT     0x88100002

#define CMSCBB_ERR_PKI_CRYPTO_DIGEST_INIT     0x88200201
#define CMSCBB_ERR_PKI_CRYPTO_DIGEST_UPDATE   0x88200202
#define CMSCBB_ERR_PKI_CRYPTO_DIGEST_FINAL    0x88200203
#define CMSCBB_ERR_PKI_CMS_VERIFY_FAILED      0x88200309
#define CMSCBB_ERR_PKI_CMS_SIG_ALGO           0x88200311

CMSCBB_ERROR_CODE CmscbbCryptoVerifyCreateCtx(CMSCBB_CRYPTO_VRF_CTX* ctx);
CMSCBB_ERROR_CODE CmscbbCryptoVerifyInit(CMSCBB_CRYPTO_VRF_CTX vrf_ctx,
    const CmscbbBigInt* e, const CmscbbBigInt* n, CVB_UINT32 cmscbb_hashid);

CMSCBB_ERROR_CODE CmscbbCryptoVerifyUpdate(CMSCBB_CRYPTO_VRF_CTX vrf_ctx, const CVB_BYTE* data, CVB_UINT32 len);

CMSCBB_ERROR_CODE CmscbbCryptoVerifyFinal(CMSCBB_CRYPTO_VRF_CTX vrf_ctx,
    const CVB_BYTE* signature, CVB_UINT32 len, CVB_INT* r_result);

CVB_VOID CmscbbCryptoVerifyDestroyCtx(CMSCBB_CRYPTO_VRF_CTX vrf_ctx);
#endif /* CFG_SOC_PLATFORM_MDC_V51 */
CMSCBB_ERROR_CODE CmscbbCryptoPssVerifyFinal(CMSCBB_CRYPTO_VRF_CTX vrf_ctx, const CVB_BYTE *signature, CVB_UINT32 len,
    CVB_INT *r_result, u32 salt_len);
#endif /* CMSCBB_ADAPTER_H */
