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
*
* Description:
* Author: huawei
* Create: 2022-11-15
*/
#ifndef __MEMORY_SCAN_H__
#define __MEMORY_SCAN_H__

#include <linux/types.h>
#include <linux/time.h>

#include "ascend_hal_error.h"
#include "dms_define.h"
#include "drv_type.h"
#include "memory_ddr_cfg.h"

#define MEMORY_DEVICE_NUM_MAX 4
#define MEMORY_MAX_INTERNEL_TIME 60000

enum memory_scan_type {
	MEMORY_BW_UTIL = 0,
	MEMORY_ECC_STATIC,
	MEMORY_TYPE_MAX,
};

enum memory_scan_core {
	MEMORY_DDR = 0,
	MEMORY_HBM = 2,
};

struct memory_retry_cycle {
	unsigned int interval_cnt;
	unsigned int remain_cnt;
	struct timeval prev_fail_time;
};

struct memory_scan_info {
	u32 hbm_bw_util;
	u32 ddr_bw_util;
	struct memory_info hbm_mem_info;
	struct memory_info ddr_mem_info;
};

typedef int (*memory_get_info_handle)(u32 dev_id, struct memory_scan_info *value);

struct memory_scan_list {
	memory_get_info_handle get_value_handle;
	u32 type_id;
	u32 core_id;
	struct memory_retry_cycle period_ctrl[MEMORY_DEVICE_NUM_MAX];
	u32 scan_flag;
	struct memory_scan_info value[MEMORY_DEVICE_NUM_MAX];
};

struct memory_scan_list *find_operate_info_str(u32 type_id, u32 core_id);
int memory_scan_init(void);
int memory_scan_uninit(void);

#endif
