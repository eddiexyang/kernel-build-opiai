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
#ifndef LPM_DEVMNG_STATISTICS_H
#define LPM_DEVMNG_STATISTICS_H

#include <linux/types.h>

#define LPM_STATS_PRINT_CYCLE 500U // unit: second
#define LPM_STATS_MAX_BUF_LEN 256U
#define LPM_STATS_ITEM_IN_EACH_LINE 4U

#define LPM_TIMER_BIND_CPU 0

enum lpm_dsmi_api_type {
	// dsmi_set_device_info: DSMI_MAIN_CMD_LP
	LPM_DSMI_SET_TOPS,
	LPM_DSMI_SET_LPTEST,
	LPM_DSMI_SET_IDLE_SWITCH,
	// dsmi_get_device_info: DSMI_MAIN_CMD_LP
	LPM_DSMI_VOLTAGE_CURRENT_AICORE,
	LPM_DSMI_VOLTAGE_CURRENT_HYBRID,
	LPM_DSMI_VOLTAGE_CURRENT_TAISHAN,
	LPM_DSMI_VOLTAGE_CURRENT_DDR,
	LPM_DSMI_ACG_CNT,
	LPM_DSMI_IDLE_STATUS,
	LPM_DSMI_GET_CUR_TOPS,
	LPM_DSMI_GET_ALL_TOPS,
	LPM_DSMI_AICORE_FREQREDUC_CAUSE,
	LPM_DSMI_GET_POWER_INFO,
	LPM_DSMI_SET_SOC_STRESS,
	LPM_DSMI_GET_AIC_ACG_CPM,
	LPM_DSMI_GET_BUS_ACG_CPM,
	LPM_DSMI_GET_LPTEST,
	// dsmi_get_device_info: DSMI_MAIN_CMD_TEMP
	LPM_DSMI_TEMP_DDR,
	LPM_DSMI_TEMP_DDR_THOLD,
	LPM_DSMI_TEMP_SOC_THOLD,
	LPM_DSMI_TEMP_SOC_MIN_THOLD,

	LPM_DSMI_SET_SUSPEND,
	LPM_DSMI_GET_LP_STATUS,
	LPM_DSMI_GET_FREQUENCY,
	LPM_DSMI_GET_TEMPERATURE,
	LPM_DSMI_GET_VRD_INFO,

	LPM_DSMI_API_TYPE_MAX
};

struct lpm_dsmi_invoking_times {
	const char *name;
	uint64_t total;
	uint64_t failed;
};

struct lpm_dsmi_statistics {
	// true: when there is new dsmi invocation
	// false: after printing statistics info
	bool invoking_flag;
	struct lpm_dsmi_invoking_times invoking_times[(uint32_t)LPM_DSMI_API_TYPE_MAX];
};

void lpm_dsmi_update_statistics(int32_t ret, enum lpm_dsmi_api_type api_type);

void lpm_statistics_suspend_prepare(void);
void lpm_statistics_post_suspend(void);

int32_t lpm_statistics_probe(uint64_t *param, uint32_t param_num);
int32_t lpm_statistics_remove(uint64_t *param, uint32_t param_num);

#endif
