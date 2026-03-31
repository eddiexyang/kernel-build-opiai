/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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

#ifndef LPM_DEVMNG_SHAREMEM_H
#define LPM_DEVMNG_SHAREMEM_H

#include <linux/types.h>
#include <linux/spinlock.h>
#ifndef DRV_LPM_FAULT
#include "lpm_devmng_common.h"
#else
#include "lpm_fault_common.h"
#endif

#if defined(LPM_SHAREMEM_V1) || defined(LPM_SHAREMEM_V2)

// base addr and len of the shared memory
#if defined(CFG_SOC_PLATFORM_MINIV3)
#if defined(CFG_SOC_PLATFORM_MDC_V11)
#define LPM_SHAREMEM_BASE_ADDR   0x1B00000ULL
#else
#define LPM_SHAREMEM_BASE_ADDR   0x3100000ULL
#endif
#define LPM_SHAREMEM_ADDR_LEN    0x400000ULL // 4M
#elif defined(CFG_SOC_PLATFORM_CLOUD_V2)
#define LPM_SHAREMEM_DIE0_BASE_ADDR  0x29200000ULL
#define LPM_SHAREMEM_BASE_ADDR       0x1029200000ULL
#define LPM_SHAREMEM_ADDR_LEN        0x400000ULL // 4M
#elif defined(CFG_SOC_PLATFORM_MINIV2) || defined(CFG_SOC_PLATFORM_HELPER)
#define LPM_SHAREMEM_BASE_ADDR   0xA00000ULL
#define LPM_SHAREMEM_ADDR_LEN    0x400000ULL // 4M
#elif defined(CFG_SOC_PLATFORM_MDC_V51_LITE)
#define LPM_SHAREMEM_BASE_ADDR   0xA00000ULL
#define LPM_SHAREMEM_ADDR_LEN    0x400000ULL // 4M
#endif

struct lpm_sharemem_dev_priv {
	void __iomem *viraddr;
	unsigned long viraddr_size;
	spinlock_t mem_lock;
};

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
struct lpm_sharemem_debugfs_param {
	uint32_t debug_dev_id;
	uint32_t debug_read_offset;
	uint32_t debug_read_len;
};
#endif

struct lpm_sharemem_priv {
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	// used to assist debugfs read and write functions
	struct lpm_sharemem_debugfs_param debugfs_param;
#endif
#ifndef DRV_LPM_FAULT
	struct lpm_sharemem_dev_priv dev_data[LPM_DEVMNG_DEV_MAX_NUM];
#else
	struct lpm_sharemem_dev_priv dev_data[LPM_DMS_NODE_MAX_NUM];
#endif
};

int32_t lpm_sharemem_probe(uint64_t *param, uint32_t param_num);
int32_t lpm_sharemem_remove(uint64_t *param, uint32_t param_num);

int32_t lpm_sharemem_get_value(uint32_t dev_id, unsigned long offset, uint8_t *out, unsigned long out_len);
int32_t lpm_sharemem_set_value(uint32_t dev_id, unsigned long offset, const uint8_t *out, unsigned long out_len);
int32_t lpm_sharemem_clear_value(uint32_t dev_id, unsigned long offset, unsigned long out_len);
int32_t lpm_sharemem_get_value_to_user(
	uint32_t dev_id, unsigned long offset, uint8_t __user *out, unsigned long out_len);

#else // define: v1 || v2

static inline int32_t lpm_sharemem_probe(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}
static inline int32_t lpm_sharemem_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}

#endif // undefine: v1 || v2
#endif // LPM_DEVMNG_SHAREMEM_H
