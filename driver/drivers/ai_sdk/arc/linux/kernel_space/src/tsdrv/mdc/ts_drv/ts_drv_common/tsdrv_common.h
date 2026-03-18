/*
 * Copyright (c) 2019-2021. Huawei Technologies Co., Ltd. All rights reserved.
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
#ifndef TSDRV_COMMON_H
#define TSDRV_COMMON_H

#include <linux/types.h>

#ifndef __GFP_ACCOUNT
#ifdef __GFP_KMEMCG
#define __GFP_ACCOUNT __GFP_KMEMCG /* for linux version 3.10 */
#endif

#ifdef __GFP_NOACCOUNT
#define __GFP_ACCOUNT 0 /* for linux version 4.1 */
#endif
#endif

#ifdef CFG_BUILD_DEBUG
#define EXPORT_SYMBOL_UNRELEASE(symbol) EXPORT_SYMBOL(symbol)
#else
#define EXPORT_SYMBOL_UNRELEASE(symbol)
#endif

extern int memset_s(void *dest, size_t destMax, int c, size_t count);

struct tsdrv_msg_sync_ssid {
    u32 hpid;
    u32 vfid;
    int ssid;
};

/**
 * get chip type
 */
#define CHIP_TYPE_MDC_ASCEND610 (1)
#define CHIP_TYPE_MDC_BS9SX1A (2)
#define CHIP_TYPE_NOT_SET (-1)

/**
 * @Return: chip type. 1: ASCEND610; 2: BS9SX1A; other: invalid value
 */
int tsdrv_get_chip_type(void);

#ifdef CFG_SOC_PLATFORM_MINIV2
#define DIE_BASEADDR_PA_OFFSET              (0x0ULL)
#elif defined (CFG_SOC_PLATFORM_CLOUD_V2)
#define DIE_BASEADDR_PA_OFFSET              (0x10000000000ULL)
#else
#define DIE_BASEADDR_PA_OFFSET              (0x0ULL)
#endif /* CFG_SOC_PLATFORM_MINIV2 */

#ifdef CFG_SOC_PLATFORM_MDC_V51
#define TS_MDC_DCACHE_BASE  __ULL(0x31800000)
#define TS_MDC_DCACHE_SIZE    __ULL(32 * 1024 * 1024)

#define TS_MDC_STL_BASE       __ULL(0x880000000)
#define TS_MDC_AIC_STL_BASE   (TS_MDC_STL_BASE + TS_MDC_DCACHE_SIZE)
#define TS_MDC_AIC_STL_SIZE       __ULL(4 * 1024 * 1024)
#define TS_MDC_AIV_STL_BASE   (TS_MDC_AIC_STL_BASE + TS_MDC_AIC_STL_SIZE)
#define TS_MDC_AIV_STL_SIZE       __ULL(4 * 1024 * 1024)
#endif

#endif
