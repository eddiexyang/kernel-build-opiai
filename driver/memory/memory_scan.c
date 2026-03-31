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
#include <linux/kthread.h>
#include <linux/delay.h>

#include "dms_template.h"
#include "dms_cmd_def.h"
#include "dev_mon_cmd_def.h"
#include "urd_acc_ctrl.h"
#include "dms_timer.h"

#include "memory_log.h"
#include "memory_ecc.h"
#include "memory_ddr.h"
#include "memory_hbm.h"
#include "memory_scan.h"
#include "memory_ecc_cfg.h"
#include "dms_timer.h"

#define MEMORY_SCAN_INFO_CYCEL 2000
#define MEMORY_SCAN_DELAY 10

#ifdef CFG_FEATURE_FORCE_NO_SCAN
#define SCAN_SWITCH 0
#else
#define SCAN_SWITCH 1
#endif

STATIC unsigned int g_timer_task_id;

#ifdef CFG_MEMORY_HBM
STATIC int memory_get_hbm_bw_util_scan(u32 dev_id, struct memory_scan_info *info)
{
	int ret;

	ret = get_bandwidth_util_rate(dev_id, &info->hbm_bw_util);
	if (ret != 0) {
		memory_drv_err("Failed get hbm bw util rate. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}

	return 0;
}
#endif

#ifdef CFG_MEMORY_DDR
STATIC int memory_get_ddr_bw_util_scan(u32 dev_id, struct memory_scan_info *info)
{
	int ret;
	u32 chan_num = DDR_MAX_CHAN_NUM;
	u32 bitmap = DDR_CHAN_BITMAP;
#ifdef CFG_MEMORY_DDR_INFO_FROM_REG
	struct ddrc_info_t ddr_info = {0};

	ret = get_ddr_info_from_dts(&ddr_info);
	if (ret != 0) {
		memory_drv_err("Failed to get ddr info from dts. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}
	chan_num = ddr_info.chan_num;
	bitmap = ddr_info.bitmap;
#endif

	ret = memory_get_ddr_bw_util_rate(dev_id, chan_num, bitmap, &info->ddr_bw_util);
	if (ret != 0) {
		memory_drv_err("Failed get hbm bw util rate. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}

	return 0;
}
#endif

STATIC int memory_get_ecc_static_scan(u32 dev_id, struct memory_scan_info *info)
{
	int ret;

#ifdef CFG_MEMORY_HBM
	info->hbm_mem_info.dev_id = dev_id;
	info->hbm_mem_info.type = MEMORY_HBM;
	ret = GET_ECC_STATISTISCS(&info->hbm_mem_info);
	if (ret != 0) {
		memory_drv_err("Get ddr ecc statistics failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}
#endif
#ifdef CFG_MEMORY_DDR
	info->ddr_mem_info.dev_id = dev_id;
	info->ddr_mem_info.type = MEMORY_DDR;
	ret = GET_ECC_STATISTISCS(&info->ddr_mem_info);
	if (ret != 0) {
		memory_drv_err("Get ddr ecc statistics failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}
#endif
	return 0;
}

STATIC int memory_scan_period_check(struct memory_retry_cycle *ctrl)
{
	if (ctrl->remain_cnt != 0) {
		ctrl->remain_cnt--;
		return -EINVAL;
	}

	return 0;
}

STATIC void memory_scan_period_update(struct memory_retry_cycle *ctrl, int result)
{
	struct timeval time_current = {0};
	long times_gap = 0;

	if (result == 0) {
		ctrl->remain_cnt = 0;
		ctrl->interval_cnt = 0;
		ctrl->prev_fail_time.tv_sec = 0;
		ctrl->prev_fail_time.tv_usec = 0;
		return;
	}

	(void)do_gettimeofday(&time_current);
	if (ctrl->prev_fail_time.tv_sec != 0 || ctrl->prev_fail_time.tv_usec != 0) {
		/* 1000 ms per second */
		times_gap = (time_current.tv_sec - ctrl->prev_fail_time.tv_sec) * 1000 +
					(time_current.tv_usec - ctrl->prev_fail_time.tv_usec) / 1000; /* 1000 us per ms */
	}

	/* Stop when the interval exceeds 1 minute. The maximum interval is within 1 to 2 minutes. */
	if (times_gap < MEMORY_MAX_INTERNEL_TIME) {
		ctrl->interval_cnt = ctrl->interval_cnt == 0 ? 1 : ctrl->interval_cnt * 2; /* 2: x=x*2 */
	}
	ctrl->prev_fail_time.tv_sec = time_current.tv_sec;
	ctrl->prev_fail_time.tv_usec = time_current.tv_usec;
	ctrl->remain_cnt = ctrl->interval_cnt;
}

STATIC struct memory_scan_list g_memory_scan_list[] = {
#ifdef CFG_MEMORY_HBM
	{ (memory_get_info_handle)memory_get_hbm_bw_util_scan, MEMORY_BW_UTIL, MEMORY_HBM, {{0}}, SCAN_SWITCH, {{0}}},
	{ (memory_get_info_handle)memory_get_ecc_static_scan, MEMORY_ECC_STATIC, MEMORY_HBM, {{0}}, SCAN_SWITCH, {{0}}},
#endif
#ifdef CFG_MEMORY_DDR
	{ (memory_get_info_handle)memory_get_ddr_bw_util_scan, MEMORY_BW_UTIL, MEMORY_DDR, {{0}}, SCAN_SWITCH, {{0}}},
	{ (memory_get_info_handle)memory_get_ecc_static_scan, MEMORY_ECC_STATIC, MEMORY_DDR, {{0}}, SCAN_SWITCH, {{0}}},
#endif
};

struct memory_scan_list *find_operate_info_str(u32 type_id, u32 core_id)
{
	unsigned int i;
	unsigned int table_size = sizeof(g_memory_scan_list) / sizeof(struct memory_scan_list);

	for (i = 0; i < table_size; i++) {
		if ((type_id == g_memory_scan_list[i].type_id) && (core_id == g_memory_scan_list[i].core_id)) {
			return &g_memory_scan_list[i];
		}
	}

	return NULL;
}

STATIC int memory_scan_info(u64 arg)
{
	int ret;
	unsigned int i, dev_id;
	unsigned int num_dev = 0;
	struct memory_retry_cycle *ctrl = NULL;
	unsigned int table_size = sizeof(g_memory_scan_list) / sizeof(struct memory_scan_list);

	ret = devdrv_get_devnum(&num_dev);
	if (ret != 0) {
		dms_err("Get device num failed. (ret=%d)\n", ret);
		return -1;
	}

	for (i = 0; i < table_size; i++) {
		for (dev_id = 0; (dev_id < num_dev); dev_id++) {
			ctrl = &g_memory_scan_list[i].period_ctrl[dev_id];
			if ((g_memory_scan_list[i].scan_flag != 0) && (memory_scan_period_check(ctrl) == 0)) {
				ret = g_memory_scan_list[i].get_value_handle(dev_id, &g_memory_scan_list[i].value[dev_id]);
				memory_scan_period_update(ctrl, ret);
			}
			msleep(MEMORY_SCAN_DELAY);
		}
	}

	return 0;
}

int memory_scan_init(void)
{
	int ret;
	struct dms_timer_task memory_scan_task = {0};

	memory_scan_task.expire_ms = MEMORY_SCAN_INFO_CYCEL;
	memory_scan_task.handler_mode = COMMON_WORK;
	memory_scan_task.exec_task = memory_scan_info;

	ret = dms_timer_task_register(&memory_scan_task, &g_timer_task_id);
	if (ret != 0) {
		dms_err("Dms timer task register failed. (ret=%d)\n", ret);
		return ret;
	}

	return 0;
}

int memory_scan_uninit(void)
{
	int ret;

	ret = dms_timer_task_unregister(g_timer_task_id);
	if (ret != 0) {
		dms_err("Dms timer task unregister failed. (ret=%d)\n", ret);
		return ret;
	}

	return 0;
}
