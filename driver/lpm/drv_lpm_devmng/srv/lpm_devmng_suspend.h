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
#ifndef LPM_DEVMNG_SUSPEND_H
#define LPM_DEVMNG_SUSPEND_H

#include <linux/mutex.h>
#include <linux/suspend.h>
#include "lpm_devmng_common.h"

#ifdef LPM_SUSPEND

// Wait for 1ms each time, up to 5000 times(up to 5s)
#define LPM_SUSPEND_WAIT_RESUME_CNT 5000
#define LPM_SUSPEND_WAIT_RESUME_TIME LPM_DELAY_1000_US

typedef bool (*fn_pm_get_wakeup_count_call)(uint32_t *count, bool block);
typedef bool (*fn_pm_save_wakeup_count_call)(uint32_t count);

enum lpm_suspend_power_state {
	LPM_SUSPEND_POWER_SUSPEND,
	LPM_SUSPEND_POWER_POWEROFF,
	LPM_SUSPEND_POWER_RESET,
	LPM_SUSPEND_POWER_RESUME_TIME,
	LPM_SUSPEND_POWER_MAX
};

enum lpm_suspend_resume_mode {
	LPM_SUSPEND_MODE_BUTTON,   // resume by button
	LPM_SUSPEND_MODE_TIME,     // resume by time(Button wakeup also takes effect)
	LPM_SUSPEND_MODE_TIME_POWEROFF, // power off by time
	LPM_SUSPEND_MODE_MAX
};

struct lpm_suspend_ipc_notify {
	// timestamps
	uint32_t tv_sec_low;  // seconds low 32 bits
	uint32_t tv_sec_high; // seconds high 32 bits
	uint32_t tv_usec;     // microseconds

	// way to resume, each bit represents a resume method, bit0: gpio,bit1:timer
	// bit31: 0:Wake up on timer timeout, 1:Shutdown when timer expires
	uint32_t mode_mask;

	// unit:ms, legal range：0, or[200ms, (7*24*3600*1000)ms]
	// When the resume time is set to 0, it means that the software does not actively resume
	// if sub_cmd is resume, this value does not care, configure it to 0
	uint32_t resume_time;
};

struct lpm_suspend_ipc_ack {
	uint32_t result; // 0: success, other: failed
};

struct lpm_suspend_dsmi_power_cfg {
	uint32_t dev_id;
	// legal types are suspend and resume_time
	enum lpm_suspend_power_state type;
	// when type equal to suspend, the mode does not care
	enum lpm_suspend_resume_mode mode;
	// when mode equal to button, the value does not care
	// when mode equal to time, the value means resume time, unit:s
	uint32_t value;
	uint32_t reserve[8];
};

struct lpm_suspend_opt_hook {
	bool opt_avail;
	fn_pm_get_wakeup_count_call get_wakeup_count;
	fn_pm_save_wakeup_count_call save_wakeup_count;
	void *suspend_stats;
};

struct lpm_suspend_priv {
	struct lpm_suspend_opt_hook opt_hook;
	char last_suspend_stats[512];
	struct mutex suspend_lock;
};

int32_t lpm_suspend_set_suspend(
	char *in, uint32_t in_len, char *out, uint32_t out_len);

int32_t lpm_suspend_probe(uint64_t *param, uint32_t param_num);
int32_t lpm_suspend_remove(uint64_t *param, uint32_t param_num);

#else
static inline int32_t lpm_suspend_probe(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}
static inline int32_t lpm_suspend_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}

static inline int32_t lpm_suspend_set_suspend(
	char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	// not support
	(void)in;
	(void)in_len;
	(void)out;
	(void)out_len;
	// Operation not supported
	return -EOPNOTSUPP;
}
#endif

#endif
