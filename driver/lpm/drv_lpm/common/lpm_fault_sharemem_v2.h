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

#ifndef LPM_FAULT_SHAREMEM_V2_H
#define LPM_FAULT_SHAREMEM_V2_H

#if defined(LPM_SHAREMEM_V2)

#include <linux/types.h>
#include <linux/spinlock.h>
#include "lpm_fault_common.h"
#include "lpm_fault_sharemem.h"

/* LOGBUFF */
#define LPM_SHAREMEM_LOG_BUF_ADDR_OFFSET       0ULL
#define LPM_SHAREMEM_LOG_BUF_SIZE              0x20000ULL

/* DEVICE_MANAGEMENT */
#define LPM_SHAREMEM_DEVMNG_ADDR_OFFSET        (LPM_SHAREMEM_LOG_BUF_ADDR_OFFSET + LPM_SHAREMEM_LOG_BUF_SIZE)
#define LPM_SHAREMEM_DEVMNG_SIZE               0x19000ULL

// devmng sharemem head magic num
#define LPM_SHAREMEM_MAGIC_NUM  0x4c50534d  // L P S M

#define LPM_SHAREMEM_DEVMNG_MAX_TYPE_NUM 32

/*
 * DEVMNG Content in Shared Memory
 *
 * DEVMNG     :  100K(0x19000)
 *   temprature       : 128byte(0x80)
 *   freq             : 64byte(0x40)
 *   power            : 64byte(0x40)
 *   volt             : 64byte(0x40)
 *   profile          : 1kbyte(0x400)
 *   hw_check         : 64byte(0x40)
 *   aic freq         : 64byte(0x40)
 *   system status    : 32byte(0x20)
 *   pmu volt/current : 64byte(0x40)
 *   temp threshold   : 64byte(0x40)
 *   vrd status       : 292byte(0x124)
 *   reserve          :
 */
enum lpm_sharemem_info_type {
	LPM_SHAREMEM_TYPE_TEMP,
	LPM_SHAREMEM_TYPE_FREQ,
	LPM_SHAREMEM_TYPE_POWER,
	LPM_SHAREMEM_TYPE_VOLT,
	LPM_SHAREMEM_TYPE_PROFILE,
	LPM_SHAREMEM_TYPE_HW_CHECK,
	LPM_SHAREMEM_TYPE_AIC_FREQ,
	LPM_SHAREMEM_TYPE_SYS_STATUS,
	LPM_SHAREMEM_TYPE_PMU_VOLT_CURRENT,
	LPM_SHAREMEM_TYPE_TEMP_THRESHOLD,
	LPM_SHAREMEM_TYPE_VRD_STATUS,
	LPM_SHAREMEM_TYPE_MAX
};

enum lpm_devmng_suspend_status {
	LPM_SUSPEND_STATUS_RESUME = 0, // 0-resume
	LPM_SUSPEND_STATUS_SUSPEND = 1 // 1-suspend
};

struct lpm_sys_status_info {
	uint8_t idle_status;    // 0: busy, other:idle
	uint8_t suspend_status; // enum lpm_devmng_suspend_status
	uint8_t health_status;  // 0-normal, 1-minor, 2-major, 3-critical
	uint8_t rsv[25];
	uint32_t health_status_info;
} __attribute__((packed));

struct lpm_sharemem_info_entry {
	uint32_t type;
	uint32_t offset;
	uint32_t len;
} __attribute__((packed));

struct lpm_sharemem_header {
	uint32_t magic_num;
	uint32_t version;
	uint32_t entry_num;
	struct lpm_sharemem_info_entry entry[LPM_SHAREMEM_DEVMNG_MAX_TYPE_NUM];
	uint32_t crc;
} __attribute__((packed));

int32_t lpm_get_suspend_status_from_sharemem(uint32_t dev_id, bool *in_suspend);
#endif // LPM_SHAREMEM_V2

#endif // LPM_FAULT_SHAREMEM_V2_H
