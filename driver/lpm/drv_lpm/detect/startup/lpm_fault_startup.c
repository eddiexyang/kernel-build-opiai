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

#include <linux/io.h>
#include "lpm_fault_common.h"
#include "lpm_fault_report.h"
#include "lpm_fault_startup.h"
#include "dms_node_type.h"
#include "dms_sensor_type.h"
#include "lpm_fault_config.h"

STATIC void lpm_fault_lp_startup_check_and_report(uint32_t dev_num)
{
	int32_t ret;
	uint32_t dev_id;
	struct lpm_inner_fault_event event = {0};
	for (dev_id = 0; dev_id < dev_num; ++dev_id) {
		if (lpm_fault_query_lp_startup_status(dev_id)) {
			continue;
		}

		// lp start failed, report faults.
		event.dev_id = dev_id;
		event.node_type = DMS_DEV_TYPE_LPM;
		event.sensor_type = DMS_SEN_TYPE_SYSTEM_FW_PROGRESS;
		event.event_type = LPM_FAULT_EVENT_FW_HANG;
		event.assertion = 0x1; // DMS_EVENT_TYPE_OCCUR
		ret = lpm_handle_fault(&event);
		if (ret != 0) {
			lpm_log_err("lpm report firmware start fault failed, ret=%d\n", ret);
		}
	}
}

// use to check some startup faults and report.
int32_t lpm_fault_startup_init(uint32_t dev_num)
{
	if (dev_num > LPM_DMS_NODE_MAX_NUM) {
		lpm_log_err("lpm firmware check init failed, dev_num=%u is out of range\n",
			dev_num);
		return -1;
	}

	// check init and report
	lpm_fault_lp_startup_check_and_report(dev_num);
	return 0;
}

int32_t lpm_fault_startup_exit(uint32_t dev_num)
{
	(void)dev_num;
	return 0;
}