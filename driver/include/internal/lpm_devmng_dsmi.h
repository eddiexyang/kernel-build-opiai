/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
#ifndef LPM_DEVMNG_DSMI_H
#define LPM_DEVMNG_DSMI_H

#include <linux/platform_device.h>
#include "lpm_devmng_statistics.h"

typedef int32_t (*FP_LPM_DSMI_SET_DEVICE_INFO)(
	uint32_t dev_id, const char *in, uint32_t in_len);
typedef int32_t (*FP_LPM_DSMI_GET_DEVICE_INFO)(
	uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len);

struct lpm_dsmi_set_device_info {
	enum lpm_dsmi_api_type api_type;
	FP_LPM_DSMI_SET_DEVICE_INFO fn_set;
};

struct lpm_dsmi_get_device_info {
	enum lpm_dsmi_api_type api_type;
	FP_LPM_DSMI_GET_DEVICE_INFO fn_get;
};

#define LPM_DSMI_FILTER_MAX_LEN 40U

struct lpm_dsmi_feature_filter {
	char filter[LPM_DSMI_FILTER_MAX_LEN];
};

struct lpm_dsmi_sub_cmd_cfg {
	uint32_t sub_cmd;
	enum lpm_dsmi_api_type api_type;
};

typedef const struct lpm_dsmi_sub_cmd_cfg *(*FP_GET_SUBCMD_LIST)(uint32_t *cmd_cnt);

struct lpm_dsmi_cmd_cfg {
	uint32_t main_cmd;
	FP_GET_SUBCMD_LIST fn_get_sub_cmd;
};

int32_t lpm_dsmi_probe(uint64_t *param, uint32_t param_num);
int32_t lpm_dsmi_remove(uint64_t *param, uint32_t param_num);

#endif
