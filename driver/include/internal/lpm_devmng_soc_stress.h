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
#ifndef LPM_DEVMNG_SOC_STRESS_H
#define LPM_DEVMNG_SOC_STRESS_H

#include <linux/types.h>

// ipc ack result
#define IPC_STRESS_RES_OK     0
#define IPC_STRESS_NOSUPPORT  1
#define IPC_STRESS_PARAM_ERR  2
#define IPC_STRESS_CFG_FAIL   3

#pragma pack(1)

struct ipc_stress_cfg_ack {
	uint8_t result;
};

struct ipc_get_cpm_ack {
	uint8_t result;
	uint8_t aic_profile;
	uint8_t cpu_profile;
	uint16_t aic_volt;
	uint16_t bus_volt;
};

#pragma pack()

#define ACG_CPM_MAGIC   0x43504D5FU  /* CPM_ */

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
#define AIC_NUM  25
#define CPU_NUM  2
#define AIC_PROFILE_NUM_MAX  7
#define CPU_PROFILE_NUM_MAX  1
#else
#define AIC_NUM  1
#define CPU_NUM  1
#define AIC_PROFILE_NUM_MAX  1
#define CPU_PROFILE_NUM_MAX  1
#endif

struct cpm_data {
	uint8_t init_rise;
	uint8_t init_fall;
	uint8_t min_rise;
	uint8_t min_fall;
	uint32_t volt_fall_by_risedata;
	uint32_t volt_fall_by_falldata;
};

struct soc_cpm_data {
	uint32_t magic;
	struct cpm_data cpu[CPU_NUM][CPU_PROFILE_NUM_MAX];
	struct cpm_data aic[AIC_NUM][AIC_PROFILE_NUM_MAX];
};

#ifdef LPM_SOC_STRESS

int32_t lpm_get_aicore_cpm(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len);
int32_t lpm_get_bus_cpm(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len);
int32_t lpm_set_soc_stress(uint32_t dev_id, const char *in, uint32_t in_len);

int32_t lpm_soc_stress_probe(uint64_t *param, uint32_t param_num);
int32_t lpm_soc_stress_remove(uint64_t *param, uint32_t param_num);

#else

static inline int32_t lpm_get_aicore_cpm(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	(void)dev_id;
	(void)in;
	(void)in_len;
	(void)out_len;
	// Operation not supported
	return -EOPNOTSUPP;
}

static inline int32_t lpm_get_bus_cpm(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	(void)dev_id;
	(void)in;
	(void)in_len;
	(void)out_len;
	// Operation not supported
	return -EOPNOTSUPP;
}

static inline int32_t lpm_set_soc_stress(uint32_t dev_id, const char *in, uint32_t in_len)
{
	(void)dev_id;
	(void)in;
	(void)in_len;
	// Operation not supported
	return -EOPNOTSUPP;
}

static inline int32_t lpm_soc_stress_probe(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}

static inline int32_t lpm_soc_stress_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}

#endif

#endif