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

#ifndef DRV_MEMORY_HBM_H
#define DRV_MEMORY_HBM_H
#include "drv_profile.h"

#define DSMI_SUBCMD_MEMORY_SET_MATA_READ_ONCE 30
#define DSMI_SUBCMD_MEMORY_GET_MATA_READ_ONCE 31

#ifdef CFG_LPM_INFO_FROM_IPC
struct hbm_ipc {
	uint8_t cmd_type1;
	uint8_t cmd_type0;
	uint8_t target_id;
	uint8_t source_id;

	uint8_t cmd_para0;
	uint8_t cmd_para1;
	uint8_t cmd_para2;
	uint8_t cmd_para3;

	uint8_t cmd_data0;
	uint8_t cmd_data1;
	uint8_t cmd_data2;
	uint8_t cmd_data3;

	uint32_t data1;
	uint32_t data2;
	uint32_t data3;
	uint32_t data4;
	uint32_t data5;
};
#endif

#define MAX_MATA_PER_DIE	  8U
#define READONCE_ON		   1U
#define READONCE_OFF		  0

int memory_map_hbm_init(void);
void memory_map_hbm_exit(void);
int memory_hbm_prof_sample(struct prof_peri_para para);
int memory_hbm_prof_init_user_cfg(struct prof_peri_para para);
int get_bandwidth_util_rate(unsigned int dev_id, unsigned int *rate);
int memory_get_hbm_bandwidth_utilization_rate(void *feature, char *in, unsigned int in_len, char *out,
	unsigned int out_len);
#if defined(CFG_LPM_INFO_FROM_IPC) || defined(CFG_LPM_INFO_FROM_SHAREMEM)
int32_t memory_get_hbm_temperature(void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len);
#endif

#endif
