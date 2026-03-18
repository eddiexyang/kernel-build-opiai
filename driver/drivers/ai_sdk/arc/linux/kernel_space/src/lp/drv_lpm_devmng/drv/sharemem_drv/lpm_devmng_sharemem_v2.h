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

#ifndef LPM_DEVMNG_SHAREMEM_V2_H
#define LPM_DEVMNG_SHAREMEM_V2_H

#include <linux/types.h>
#include <linux/spinlock.h>
#include "lpm_devmng_common.h"
#include "lpm_devmng_sharemem_drv.h"

#if defined(LPM_SHAREMEM_V2)

#if defined(CFG_SOC_PLATFORM_MINIV3)
#define LPM_ACCESS_DDR_OFFSET    0x20000000ULL
#else
#define LPM_ACCESS_DDR_OFFSET    0
#endif

/*
 * Shared Memory Content
 * the shared memory of CFG_SOC_PLATFORM_MINIV3 or CFG_SOC_PLATFORM_CLOUD_V2
 * has the same memory design
 * except for the base address
 * ddr map table for milan (ascend910x/ascend310x)
 *
 * base 0x3100000 size 2M
 * LOGBUFF    :  128K(0x20000)
 * DEVMNG     :  100K(0x19000)
 * vrd_fw     :  16k(0x4000)
 * aicaiv usage: 4K(0x1000)
 * lptest     :  8k(0x2000)
 * syscntFreq :  7K(0x1C00)
 * heartbeat  :  1K(0x400)
 * powersampling:32K(0x8000)
 * acg cpm    :  4K(0x1000)
 * resv       : ~2M
 */

#define LPM_SHAREMEM_MAX_SIZE                  0x200000ULL

/* LOGBUFF */
#define LPM_SHAREMEM_LOG_BUF_ADDR_OFFSET       0ULL
#define LPM_SHAREMEM_LOG_BUF_SIZE              0x20000ULL

/* DEVICE_MANAGEMENT */
#define LPM_SHAREMEM_DEVMNG_ADDR_OFFSET        (LPM_SHAREMEM_LOG_BUF_ADDR_OFFSET + LPM_SHAREMEM_LOG_BUF_SIZE)
#define LPM_SHAREMEM_DEVMNG_SIZE               0x19000ULL

#define LPM_SHAREMEM_VRD_FW_OFFSET             (LPM_SHAREMEM_DEVMNG_ADDR_OFFSET + LPM_SHAREMEM_DEVMNG_SIZE)
#define LPM_SHAREMEM_VRD_FW_SIZE               0x4000ULL

/* AICAIV USAGE */
#define LPM_SHAREMEM_AICAIV_USAGE_OFFSET       (LPM_SHAREMEM_VRD_FW_OFFSET + LPM_SHAREMEM_VRD_FW_SIZE)
#define LPM_SHAREMEM_AICAIV_USAGE_SIZE         0x1000ULL

/* LPTEST */
#define LPM_SHAREMEM_LPTEST_OFFSET             (LPM_SHAREMEM_AICAIV_USAGE_OFFSET + LPM_SHAREMEM_AICAIV_USAGE_SIZE)
#define LPM_SHAREMEM_LPTEST_SIZE               0x2000ULL

/* FREQENCY */
#define LPM_SHAREMEM_FREQENCY_OFFSET           (LPM_SHAREMEM_LPTEST_OFFSET + LPM_SHAREMEM_LPTEST_SIZE)
#define LPM_SHAREMEM_FREQENCY_SIZE             0x1C00ULL

/* HEARTBEAT */
#define LPM_SHAREMEM_HEARTBEAT_OFFSET          (LPM_SHAREMEM_FREQENCY_OFFSET + LPM_SHAREMEM_FREQENCY_SIZE)
#define LPM_SHAREMEM_HEARTBEAT_SIZE            0x400ULL

/* POWER SAMPLING */
#define LPM_SHAREMEM_POWER_SAMPLING_OFFSET     (LPM_SHAREMEM_HEARTBEAT_OFFSET + LPM_SHAREMEM_HEARTBEAT_SIZE)
#define LPM_SHAREMEM_POWER_SAMPLING_SIZE       0x8000ULL

/* CPM_DATA */
#define LPM_SHAREMEM_CPM_DATA_OFFSET           (LPM_SHAREMEM_POWER_SAMPLING_OFFSET + LPM_SHAREMEM_POWER_SAMPLING_SIZE)
#define LPM_SHAREMEM_CPM_DATA_SIZE             0x1000ULL

// devmng sharemem head magic num
#define LPM_SHAREMEM_MAGIC_NUM  0x4c50534d  // L P S M

#define LPM_SHAREMEM_AIC_FREQ_ADJ_MAGIC  0x4C504152 // 'LPAR'

#define LPM_SHAREMEM_PROFILE_MAGIC  0x4C505046 // 'LPPF'

#define LPM_SHAREMEM_AIC_FREQ_SYSCNT_MAGIC  0x4C504153 // 'LPAS'

#define LPM_SHAREMEM_POWER_SAMPLING_MAGIC   0x4C505053 // 'LPPS'

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

#define POWER_SAMPLING_DATA_SIZE 500

struct lpm_power_sampling_data {
	uint32_t time_syscnt_low;
	uint32_t time_syscnt_high;
	uint32_t board_power;
	uint32_t aic_power;
	uint32_t bus_power;
	uint32_t aic_freq;
};

struct lpm_power_sampling_mem_info {
	uint32_t magic;
	uint16_t read_index;
	uint16_t write_index;
	struct lpm_power_sampling_data power_data[POWER_SAMPLING_DATA_SIZE];
};

struct lpm_power_sampling_usr_info {
	uint16_t count;
	struct lpm_power_sampling_data *power_data;
};

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


int32_t lpm_read_aic_freq_adj_from_sharemem(uint32_t dev_id, struct lpm_aic_freq_adj_info *adj_info);
#endif // LPM_SHAREMEM_V2

#endif // LPM_DEVMNG_SHAREMEM_V2_H
