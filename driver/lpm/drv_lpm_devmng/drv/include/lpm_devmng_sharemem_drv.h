/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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

#ifndef LPM_DEVMNG_SHAREMEM_DRV_H
#define LPM_DEVMNG_SHAREMEM_DRV_H

#include <linux/types.h>
#include "lpm_devmng_common.h"

#if defined(LPM_SHAREMEM_V1) || defined(LPM_SHAREMEM_V2)

#if defined(LPM_SHAREMEM_V2)
#define LPM_SHAREMEM_TSENSOR_INFO_SIZE  128U
#define LPM_SHAREMEM_FREQ_INFO_SIZE     64U
#define LPM_SHAREMEM_POWER_INFO_SIZE    64U
#define LPM_SHAREMEM_VOLT_CURRENT_SIZE  ((uint32_t)64)

// each temperature value occupies 16 bit
enum lpm_devmng_temperature_type {
	LPM_DEVMNG_SOC_MAX_TEMP,
	LPM_DEVMNG_SOC_AIC_MAX_TEMP,
	LPM_DEVMNG_SOC_HBM_MAX_TEMP,
	LPM_DEVMNG_SOC_TOTEM_MAX_TEMP,
	LPM_DEVMNG_SOC_OTHER_TEMP_START,
	LPM_DEVMNG_SHARE_MEM_TEMP_NUM = (LPM_SHAREMEM_TSENSOR_INFO_SIZE / sizeof(uint16_t))
};

// each frequency value occupies 32 bit
enum lpm_devmng_frequency_type {
	LPM_DEVMNG_CPU_CURRENT_FREQ, // offset 0
	LPM_DEVMNG_CPU_RATE_FREQ,
	LPM_DEVMNG_AIC_CURRENT_FREQ, // offset 8
	LPM_DEVMNG_AIC_RATE_FREQ,    // offset 12
	LPM_DEVMNG_RING_CURRENT_FREQ,
	LPM_DEVMNG_RING_RATE_FREQ,
	LPM_DEVMNG_MATA_CURRENT_FREQ,
	LPM_DEVMNG_MATA_RATE_FREQ,
	LPM_DEVMNG_L2BUFF_CURRENT_FREQ,
	LPM_DEVMNG_L2BUFF_RATE_FREQ,
	LPM_DEVMNG_AIC_MIN_FREQ,
	LPM_DEVMNG_AIC_SET_FREQ,
	LPM_DEVMNG_SHARE_MEM_FREQ_NUM = (LPM_SHAREMEM_FREQ_INFO_SIZE / sizeof(uint32_t))
};

// each power value occupies 32 bit
enum lpm_devmng_power_type {
	LPM_DEVMNG_BOARD_POWER,
	LPM_DEVMNG_AI_CURRENT,
	LPM_DEVMNG_BUS_CURRENT,
	LPM_DEVMNG_BOARD_CURRENT,
	LPM_DEVMNG_POWER_LIMIT,
	LPM_SHARE_MEM_POWER_NUM = (LPM_SHAREMEM_POWER_INFO_SIZE / sizeof(uint32_t))
};

struct lpm_devmng_volt_current_info {
	uint32_t volt;
	uint32_t curr;
};

enum lpm_devmng_volt_current_type {
	LPM_DEVMNG_VOLT_CURRENT_AICORE,
	LPM_DEVMNG_VOLT_CURRENT_HYBRID,
	LPM_DEVMNG_VOLT_CURRENT_TAISHAN,
	LPM_DEVMNG_VOLT_CURRENT_DDR,
	SHARE_MEM_VOLT_CURRENT_NUM =
		(LPM_SHAREMEM_VOLT_CURRENT_SIZE / (uint32_t)sizeof(struct lpm_devmng_volt_current_info))
};

#define VRD_STATUS_MAX_NUM  8
struct lpm_vrd_status_info {
	uint8_t  valid;
	uint8_t  vrd_type;
	uint16_t slave_addr;
	uint16_t version;
	uint8_t  upgrade_remain_cnt;
	uint8_t  reserve[29];
} __attribute__((packed));

struct lpm_device_vrd_status_info {
	uint32_t num;
	struct lpm_vrd_status_info vrd_info[VRD_STATUS_MAX_NUM];
};

#define LPM_AIC_FREQ_SYSCNT_INVALID_INDEX 0xffff
#define LPM_AIC_FREQ_SYSCNT_MAC_NUM 55U

struct lpm_aic_freq_syscnt_freq {
	uint64_t syscnt; // unit: Cycle
	uint32_t freq;   // unit: MHz
	uint32_t resv;
};

struct lpm_aic_freq_syscnt_crc {
	struct lpm_aic_freq_syscnt_freq syscnt_freq;
	uint32_t freq_index;
} __attribute__((packed));

struct lpm_aic_freq_syscnt_info {
	uint32_t magic;
	uint32_t resv1;
	struct lpm_aic_freq_syscnt_freq syscnt[LPM_AIC_FREQ_SYSCNT_MAC_NUM];
	uint32_t resv2;
	uint16_t freq_index;
	uint16_t crc;
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

#define LPM_DEVMNG_DDR_TEMP_GEAR_MAX 32U
struct lpm_temp_threshold_ddr_gear {
	uint8_t  temp_gear[LPM_DEVMNG_DDR_TEMP_GEAR_MAX];
	uint32_t gear_num;
} __attribute__((packed));

struct lpm_temp_threshold_data {
	struct lpm_temp_threshold_ddr_gear ddr_gear;
	uint32_t ddr_high;
	uint32_t soc_high;
	int32_t  soc_min;
	uint32_t crc;
} __attribute__((packed));

#define LPM_PROFILE_TOPS_MAX 10U
// query detail profile tops info
struct lpm_profile_tops_table {
	uint32_t profile_id; // profile tops index
	uint32_t aic_tops;   // unit:T
	uint32_t aic_freq;   // unit:M
	uint32_t aic_vol;    // unit:mV
	uint32_t cpu_freq;   // unit:M
	uint32_t cpu_vol;    // unit:mV
	uint8_t rsv[32];     // invalid data: 0xFF
} __attribute__((packed));

struct lpm_profile_tops_list {
	uint32_t tops_num;
	struct lpm_profile_tops_table tops_table[LPM_PROFILE_TOPS_MAX];
} __attribute__((packed));

// tops info in sharemem
struct lpm_profile_tops_info {
	uint32_t magic;
	struct lpm_profile_tops_list tops_list;
	// if user has not configured the profile, lpmcu uses init_profile by default
	uint32_t init_profile;
	uint32_t crc; // magic + size + tops_list + init_profile
} __attribute__((packed));

#define LPTEST_CMD_BUFF_LEN         24  // 8bytes aligned
#define LPTEST_CMD_ARG_MAX_NUM      6
#define LPTEST_RESULT_BUFF_LEN      4096
struct lpm_lptest_cmd {
	char cmd_name[LPTEST_CMD_BUFF_LEN];
	uint64_t args[LPTEST_CMD_ARG_MAX_NUM];
	uint32_t num_args;
} __attribute__((packed));

struct lpm_lptest_info {
	struct lpm_lptest_cmd cmd;
	char result_log[LPTEST_RESULT_BUFF_LEN];
	uint32_t length;
} __attribute__((packed));

bool lpm_profile_check_tops(uint32_t config_tops);

int32_t lpm_get_temperature_from_sharemem(
	uint32_t dev_id, enum lpm_devmng_temperature_type temp_type, uint32_t *temperature);
int32_t lpm_get_frequency_from_sharemem(
	uint32_t dev_id, enum lpm_devmng_frequency_type freq_type, uint32_t *frequency);
int32_t lpm_get_system_status_from_sharemem(
	uint32_t dev_id, struct lpm_sys_status_info *sys_status);
int32_t lpm_wait_lp_suspend_status_done(uint32_t dev_id, uint32_t wait_cnt, uint32_t wait_us,
	enum lpm_devmng_suspend_status target_status);
int32_t lpm_get_power_from_sharemem(
	uint32_t dev_id, enum lpm_devmng_power_type power_type, uint32_t *power_value);
int32_t lpm_get_profile_from_sharemem(
	uint32_t dev_id, struct lpm_profile_tops_info *tops_info);
int32_t lpm_get_pmu_volt_current_from_sharemem(
	uint32_t dev_id, enum lpm_devmng_volt_current_type volt_current_type,
	struct lpm_devmng_volt_current_info *volt_current);
int32_t lpm_get_vrd_info_from_sharemem(
	uint32_t dev_id, struct lpm_device_vrd_status_info *vrd_status);
int32_t lpm_get_temp_threshold_from_sharemem(
	uint32_t dev_id, uint32_t retry_times, uint32_t wait_us, struct lpm_temp_threshold_data *temp_data);
int32_t lpm_read_aic_freq_syscnt_from_sharemem(
	uint32_t dev_id, uint32_t retry_times, uint32_t wait_us, struct lpm_aic_freq_syscnt_info *syscnt_info);

int32_t lpm_read_lptest_result_from_sharemem(
	uint32_t dev_id, uint32_t *log_length, uint8_t *log_buffer, uint32_t buf_len);
int32_t lpm_save_lptest_cmd_to_sharemem(uint32_t dev_id, const struct lpm_lptest_cmd *lptest_cmd);

int32_t lpm_save_vrd_upgrade_to_sharemem(uint32_t dev_id, const uint8_t *firmware_buf, unsigned long firmware_size);
uint64_t lpm_get_vrd_upgrade_sharemem_addr(void);

#ifdef LPM_POWER_SAMPLING
int32_t lpm_read_power_sampling_from_sharemem(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len);
#else
static inline int32_t lpm_read_power_sampling_from_sharemem(
	uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	(void)dev_id;
	(void)in;
	(void)in_len;
	(void)out_len;
	return 0;
}
#endif // LPM_POWER_SAMPLING

#endif // v2

#define LPM_AIC_FREQ_MAX_INFO_LEN 64
struct lpm_aic_freq_adj_cause {
	uint64_t reason;
	uint64_t sys_time; // system time when adjust freq
	uint64_t utc_time; // utc time when adjust freq
} __attribute__((packed));

struct lpm_aic_freq_adj_info {
	uint32_t magic;
	uint32_t size; // sizeof(lpm_aic_freq_adj_info), should not large than 64 bytes
	struct lpm_aic_freq_adj_cause adj_cause;
	uint32_t crc; // magic + size + adj_cause
} __attribute__((packed));

int32_t lpm_get_adj_cause_from_sharemem(
	uint32_t dev_id, uint32_t retry_times, uint32_t wait_us, struct lpm_aic_freq_adj_cause *adj_cause);

#endif // v1 || v2
#endif
