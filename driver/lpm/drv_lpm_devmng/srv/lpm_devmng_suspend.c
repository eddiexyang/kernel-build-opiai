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

#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/suspend.h>
#include <linux/delay.h>
#include <linux/time64.h>
#include "lpm_devmng_common.h"
#include "lpm_devmng_ipc.h"
#include "lpm_devmng_sharemem_drv.h"
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"
#endif
#include "lpm_devmng_suspend.h"

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
STATIC int32_t lpm_suspend_debugfs_init(void);
#endif

#define LPM_SUSPEND_MAX_RESUME_TIME 604800000 // unit:ms, one week
#define LPM_SUSPEND_MIN_RESUME_TIME 200 // unit:ms
#define LPM_SUSPEND_NOT_RESUME_TIME 0 // unit:ms, software does not actively resume

static struct lpm_suspend_priv g_suspend_priv;

STATIC struct lpm_suspend_priv *lpm_suspend_priv_info(void)
{
	return &g_suspend_priv;
}

STATIC void lpm_suspend_init_priv_data(void)
{
	(void)memset_s(&g_suspend_priv, sizeof(g_suspend_priv), 0, sizeof(g_suspend_priv));
	mutex_init(&g_suspend_priv.suspend_lock);
}

STATIC void lpm_suspend_init_opt_hook(void)
{
	struct lpm_suspend_priv *suspend_priv = lpm_suspend_priv_info();

	suspend_priv->opt_hook.opt_avail = false;
	suspend_priv->opt_hook.get_wakeup_count = NULL;
	suspend_priv->opt_hook.save_wakeup_count = NULL;
	suspend_priv->opt_hook.suspend_stats = NULL;
}

STATIC void lpm_suspend_put_opt_hook(void)
{
	struct lpm_suspend_priv *suspend_priv = lpm_suspend_priv_info();

	if (suspend_priv->opt_hook.get_wakeup_count != NULL) {
		__symbol_put("pm_get_wakeup_count");
		suspend_priv->opt_hook.get_wakeup_count = NULL;
	}

	if (suspend_priv->opt_hook.save_wakeup_count != NULL) {
		__symbol_put("pm_save_wakeup_count");
		suspend_priv->opt_hook.save_wakeup_count = NULL;
	}

	if (suspend_priv->opt_hook.suspend_stats != NULL) {
		__symbol_put("suspend_stats");
		suspend_priv->opt_hook.suspend_stats = NULL;
	}
}

STATIC void lpm_suspend_get_opt_hook(void)
{
	struct lpm_suspend_priv *suspend_priv = lpm_suspend_priv_info();

	lpm_suspend_init_opt_hook();

	// some kernel versions do not export these two functions
	suspend_priv->opt_hook.get_wakeup_count =
		(fn_pm_get_wakeup_count_call)(uintptr_t)__symbol_get("pm_get_wakeup_count");
	suspend_priv->opt_hook.save_wakeup_count =
		(fn_pm_save_wakeup_count_call)(uintptr_t)__symbol_get("pm_save_wakeup_count");

	// some kernel versions do not export "suspend_stats"
	suspend_priv->opt_hook.suspend_stats = (struct suspend_stats *)(uintptr_t)__symbol_get("suspend_stats");

	suspend_priv->opt_hook.opt_avail = true;
}

STATIC int32_t lpm_suspend_check_resume_time(
	enum lpm_suspend_resume_mode mode, uint32_t resume_time)
{
	if (mode >= LPM_SUSPEND_MODE_MAX) {
		return -EINVAL;
	}

#ifndef CFG_SOC_PLATFORM_MDC_V11
	if (mode > LPM_SUSPEND_MODE_TIME) {
		return -EOPNOTSUPP;
	}
#endif

	// resume_time equal to 0 means software does not actively resume
	if ((mode != LPM_SUSPEND_MODE_BUTTON) &&
		((resume_time < LPM_SUSPEND_MIN_RESUME_TIME) ||
		(resume_time > LPM_SUSPEND_MAX_RESUME_TIME))) {
		return -EINVAL;
	}

	// trigger sleep by gpio button, resume time should be set to 0
	if ((mode == LPM_SUSPEND_MODE_BUTTON) &&
		(resume_time != LPM_SUSPEND_NOT_RESUME_TIME)) {
		return -EINVAL;
	}

	return 0;
}

STATIC int32_t lpm_suspend_trans_in_param(
	char *in, uint32_t in_len, enum lpm_suspend_power_state power_state,
	struct lpm_suspend_dsmi_power_cfg *power_cfg)
{
	int32_t ret;

	if ((in == NULL) || (in_len != sizeof(struct lpm_suspend_dsmi_power_cfg))) {
		lpm_log_err("check power_state=%u failed, input char is NULL or in_len is wrong, in_len=%u\n",
			(uint32_t)power_state, in_len);
		return -EINVAL;
	}

	ret = memcpy_s((void *)power_cfg, sizeof(struct lpm_suspend_dsmi_power_cfg), in, in_len);
	if (ret != 0) {
		lpm_log_err("check power_state=%u failed, memcpy_s input failed, ret=%d, in_len=%u\n",
			(uint32_t)power_state, ret, in_len);
		return -EINVAL;
	}

	if (!lpm_common_check_dev_id(power_cfg->dev_id)) {
		lpm_log_err("check power_state=%u failed, dev_id=%u is out of range\n",
			(uint32_t)power_state, power_cfg->dev_id);
		return -EINVAL;
	}

	if (power_state != power_cfg->type) {
		lpm_log_err("check power_state=%u failed, input type=%u\n",
			(uint32_t)power_state, (uint32_t)power_cfg->type);
		return -EINVAL;
	}

	ret = lpm_suspend_check_resume_time(power_cfg->mode, power_cfg->value);
	if (ret != 0) {
		if (ret != -EOPNOTSUPP) {
			lpm_log_err("check mode=%u, resume time=%u failed\n",
				(uint32_t)power_cfg->mode, power_cfg->value);
		}
		return ret;
	}

	return 0;
}

STATIC const char *get_suspend_step_name(enum suspend_stat_step step)
{
	// enum suspend_stat_step starts form SUSPEND_FREEZE(1) to SUSPEND_RESUME
	const char *suspend_stat_step_strs[(uint32_t)SUSPEND_RESUME + 1U] = {
		"RESV",
		"FREEZE",
		"PREPARE",
		"SUSPEND",
		"SUSPEND_LATE",
		"SUSPEND_NOIRQ",
		"RESUME_NOIRQ",
		"RESUME_EARLY",
		"RESUME"
	};

	if (step <= SUSPEND_RESUME) {
		return suspend_stat_step_strs[step];
	} else {
		return suspend_stat_step_strs[0];
	}
}

STATIC void lpm_suspend_print_suspend_stats(void)
{
	int32_t i;
	int32_t last_step, last_dev, last_err;
	struct lpm_suspend_priv *suspend_priv = lpm_suspend_priv_info();
	struct suspend_stats *suspend_stats = suspend_priv->opt_hook.suspend_stats;

	if (suspend_stats != NULL) {
		last_dev  = (suspend_stats->last_failed_dev   + REC_FAILED_NUM - 1) % REC_FAILED_NUM;
		last_err  = (suspend_stats->last_failed_errno + REC_FAILED_NUM - 1) % REC_FAILED_NUM;
		last_step = (suspend_stats->last_failed_step  + REC_FAILED_NUM - 1) % REC_FAILED_NUM;

		lpm_log_event("suspend_stats:\n");
		lpm_log_event("    %s: %d\n", "success", suspend_stats->success);
		lpm_log_event("    %s: %d\n", "fail", suspend_stats->fail);
		lpm_log_event("    %s: %d\n", "failed_freeze", suspend_stats->failed_freeze);
		lpm_log_event("    %s: %d\n", "failed_prepare", suspend_stats->failed_prepare);
		lpm_log_event("    %s: %d\n", "failed_suspend", suspend_stats->failed_suspend);
		lpm_log_event("    %s: %d\n", "failed_suspend_late", suspend_stats->failed_suspend_late);
		lpm_log_event("    %s: %d\n", "failed_suspend_noirq", suspend_stats->failed_suspend_noirq);
		lpm_log_event("    %s: %d\n", "failed_resume_noirq", suspend_stats->failed_resume_noirq);
		lpm_log_event("    %s: %d\n", "failed_resume_early", suspend_stats->failed_resume_early);
		lpm_log_event("    %s: %d\n", "failed_resume", suspend_stats->failed_resume);
		lpm_log_event("    %s: %s\n", "last_failed_dev", suspend_stats->failed_devs[last_dev]);
		lpm_log_event("    %s: %d\n", "last_failed_err", suspend_stats->errno[last_err]);
		lpm_log_event("    %s: %s\n", "last_failed_step",
			get_suspend_step_name(suspend_stats->failed_steps[last_step]));
		lpm_log_event("    failed_dev_info:\n");
		for (i = 0; i < REC_FAILED_NUM; i++) {
			lpm_log_event("        %s\n", suspend_stats->failed_devs[i]);
		}
	}
}

STATIC bool lpm_suspend_is_resume_succ(void)
{
	struct lpm_suspend_priv *suspend_priv = lpm_suspend_priv_info();
	struct suspend_stats *suspend_stats = suspend_priv->opt_hook.suspend_stats;
	struct suspend_stats *last_stats = &suspend_priv->last_suspend_stats;
	int32_t ret;
	bool succ_flag = true;

	if (suspend_stats == NULL) {
		return succ_flag;
	}

	if ((suspend_stats->failed_resume > last_stats->failed_resume) ||
		(suspend_stats->failed_resume_early > last_stats->failed_resume_early) ||
		(suspend_stats->failed_resume_noirq > last_stats->failed_resume_noirq)) {
		lpm_log_err("invoke os interface pm_suspend failed in resume, fail resume stats info:"
			"curr_resume:%d, last_resume:%d, curr_resume_early:%d, last_resume_early:%d, "
			"curr_resume_noirq:%d, last_resume_noirq:%d\n",
			suspend_stats->failed_resume, last_stats->failed_resume,
			suspend_stats->failed_resume_early, last_stats->failed_resume_early,
			suspend_stats->failed_resume_noirq, last_stats->failed_resume_noirq);
		succ_flag = false;
	}

	ret = memcpy_s(last_stats, sizeof(struct suspend_stats),
		suspend_stats, sizeof(struct suspend_stats));
	if (ret != 0) {
		lpm_log_warn("save suspend stats failed, ret=%d\n", ret);
	}

	return succ_flag;
}

STATIC int32_t lpm_suspend_proc(void)
{
	int32_t ret;
	uint32_t initial_count = 0;
	struct lpm_suspend_priv *suspend_priv = lpm_suspend_priv_info();

	// avoid other services occupying the wake_lock
	if (suspend_priv->opt_hook.get_wakeup_count != NULL) {
		if (!suspend_priv->opt_hook.get_wakeup_count(&initial_count, true)) {
			lpm_log_err("pm_get_wakeup_count failed!\n");
			return -EIO;
		}
		lpm_log_info("pm_get_wakeup_count succeed.\n");
	}

	if ((suspend_priv->opt_hook.get_wakeup_count != NULL) &&
		(suspend_priv->opt_hook.save_wakeup_count != NULL)) {
		if (!suspend_priv->opt_hook.save_wakeup_count(initial_count)) {
			lpm_log_err("pm_save_wakeup_count failed!\n");
			return -EIO;
		}
		lpm_log_info("pm_save_wakeup_count succeed.\n");
	}

	lpm_log_info("invoke os interface pm_suspend start.\n");

	// Delay of 10 ms to ensure that logs at the invocation boundary are saved to the disk
	msleep(LPM_DELAY_10_MS);
	ret = pm_suspend(PM_SUSPEND_MEM);
	lpm_suspend_print_suspend_stats();
	if (ret != 0) {
		lpm_log_err("invoke os interface pm_suspend failed, ret(%d).\n", ret);
		return -EIO;
	}

	if (!lpm_suspend_is_resume_succ()) {
		return -EIO;
	}

	lpm_log_info("invoke os interface pm_suspend end.\n");
	return 0;
}

STATIC int32_t lpm_suspend_notify_suspend_ipc(
	uint32_t dev_id, uint32_t resume_time, uint32_t mode_mask)
{
	struct lpm_suspend_ipc_notify send_data = {0};
	struct lpm_suspend_ipc_ack ack_data = {0};
	struct lpm_devmng_ipc_msg ipc_msg;
	struct timespec64 stamp = {0};
	int32_t ret;

	lpm_dev_common_get_current_time(&stamp);
	send_data.tv_sec_high = (uint32_t)((((unsigned long)stamp.tv_sec) >> 32) & 0xFFFFFFFFU);
	send_data.tv_sec_low  = (uint32_t)(((unsigned long)stamp.tv_sec) & 0xFFFFFFFFU);
	send_data.tv_usec     = (uint32_t)(((unsigned long)stamp.tv_nsec / LPM_NS_TO_US) & 0xFFFFFFFFU);
	send_data.resume_time = resume_time;
	// way to resume, each bit represents a resume method, bit0: gpio, bit1:timer
	send_data.mode_mask   = mode_mask;

	ipc_msg.send_type = LPM_IPC_NOTIFY_SUSPEND;
	ipc_msg.in        = (void *)&send_data;
	ipc_msg.in_len    = (uint32_t)sizeof(send_data);
	ipc_msg.out       = (void *)&ack_data;
	ipc_msg.out_pos   = 0;
	ipc_msg.out_len   = (uint32_t)sizeof(ack_data);

	lpm_log_info("ipc notify suspend, dev_id=%u, resume_time=%u, mode_mask=0x%x, ts=%u-%u-%u\n",
		dev_id, resume_time, mode_mask, send_data.tv_sec_high, send_data.tv_sec_low, send_data.tv_usec);

	ret = lpm_ipc_send_sync_msg(dev_id, &ipc_msg);
	if (ret != 0) {
		lpm_log_err("ipc notify suspend failed, ret=%d, dev_id=%u, resume_time=%u, ts=%u-%u-%u\n",
			ret, dev_id, resume_time, send_data.tv_sec_high, send_data.tv_sec_low, send_data.tv_usec);
		return -EIO;
	}

	if (ack_data.result != 0) {
		lpm_log_err("ipc notify suspend ack failed: result=%u, dev_id=%u, resume_time=%u, ts=%u-%u-%u\n",
			ack_data.result, dev_id, resume_time,
			send_data.tv_sec_high, send_data.tv_sec_low, send_data.tv_usec);
		return -EREMOTEIO;
	}

	return 0;
}

STATIC int32_t lpm_suspend_notify_resume_ipc(uint32_t dev_id)
{
	struct lpm_suspend_ipc_notify send_data = {0};
	struct lpm_devmng_ipc_msg ipc_msg;
	struct timespec64 stamp = {0};
	int32_t ret;

	lpm_dev_common_get_current_time(&stamp);
	send_data.tv_sec_high = (uint32_t)((((unsigned long)stamp.tv_sec) >> 32) & 0xFFFFFFFFU);
	send_data.tv_sec_low  = (uint32_t)(((unsigned long)stamp.tv_sec) & 0xFFFFFFFFU);
	send_data.tv_usec     = (uint32_t)(((unsigned long)stamp.tv_nsec / LPM_NS_TO_US) & 0xFFFFFFFFU);

	ipc_msg.send_type = LPM_IPC_NOTIFY_RESUME;
	ipc_msg.in        = (void *)&send_data;
	ipc_msg.in_len    = (uint32_t)sizeof(send_data);
	ipc_msg.out       = NULL;
	ipc_msg.out_pos   = 0;
	ipc_msg.out_len   = 0;

	lpm_log_info("ipc notify resume, dev_id=%u, ts=%u-%u-%u\n",
		dev_id, send_data.tv_sec_high, send_data.tv_sec_low, send_data.tv_usec);

	ret = lpm_ipc_send_async_msg(dev_id, &ipc_msg);
	if (ret != 0) {
		lpm_log_err("ipc notify resume failed, ret=%d, dev_id=%u, ts=%u-%u-%u\n",
			ret, dev_id, send_data.tv_sec_high, send_data.tv_sec_low, send_data.tv_usec);
		return -EIO;
	}

	return 0;
}

STATIC void lpm_suspend_wait_lp_resume_done(uint32_t dev_id)
{
	int32_t ret;
	struct timespec64 start_time = {0};
	struct timespec64 end_time = {0};
	struct timespec64 cost_time = {0};

	lpm_dev_common_get_current_time(&start_time);
	// Wait for 1ms each time, up to 1000 times
	ret = lpm_wait_lp_suspend_status_done(dev_id, LPM_SUSPEND_WAIT_RESUME_CNT,
		LPM_SUSPEND_WAIT_RESUME_TIME, LPM_SUSPEND_STATUS_RESUME);
	lpm_dev_common_get_current_time(&end_time);
	cost_time = timespec64_sub(end_time, start_time);

	// fault tolerance: if wait resume failed, record the log, and continue to execute
	lpm_log_info("wait lp resume done, ret=%d, dev_id=%u, cost_time=%lld.%09ld\n",
		ret, dev_id, cost_time.tv_sec, cost_time.tv_nsec);
}

STATIC int32_t lpm_suspend_handle_suspend(
	struct lpm_suspend_dsmi_power_cfg *power_cfg, uint32_t mode_mask)
{
	int32_t ret;
	int32_t ipc_ret;
	struct lpm_suspend_priv *suspend_priv = lpm_suspend_priv_info();

	// pay attention:
	// return -EBUSY only when the suspend operation is repeated
	// Other error scenarios are not allowed to return -EBUSY
	if (mutex_trylock(&suspend_priv->suspend_lock) == 0) {
		lpm_log_err("it is in the sleep wake-up process, please do not repeat the operation\n");
		return -EBUSY;
	}

	lpm_suspend_wait_lp_resume_done(power_cfg->dev_id);

	ret = lpm_suspend_notify_suspend_ipc(power_cfg->dev_id, power_cfg->value, mode_mask);
	if (ret != 0) {
		mutex_unlock(&suspend_priv->suspend_lock);
		lpm_log_err("notify suspend failed, ret=%d, dev_id=%u, mode=%u, type=%u, value=%u, mode_mask=%u\n",
			ret, power_cfg->dev_id, (uint32_t)power_cfg->mode, (uint32_t)power_cfg->type, power_cfg->value, mode_mask);
		return ret;
	}

	ret = lpm_suspend_proc();
	if (ret != 0) {
		lpm_log_err("suspend proc failed, ret=%d, dev_id=%u, mode=%u, type=%u, value=%u, mode_mask=%u\n",
			ret, power_cfg->dev_id, (uint32_t)power_cfg->mode,
			(uint32_t)power_cfg->type, power_cfg->value, mode_mask);
		// not return here, need to notify resume after suspend rollback
	}

	ipc_ret = lpm_suspend_notify_resume_ipc(power_cfg->dev_id);
	if (ipc_ret != 0) {
		mutex_unlock(&suspend_priv->suspend_lock);
		lpm_log_err("notify resume failed, ret=%d, dev_id=%u, mode=%u, type=%u, value=%u, mode_mask=%u\n",
			ipc_ret, power_cfg->dev_id, (uint32_t)power_cfg->mode,
			(uint32_t)power_cfg->type, power_cfg->value, mode_mask);
		return ipc_ret;
	}

	mutex_unlock(&suspend_priv->suspend_lock);
	return ret;
}

STATIC uint32_t lpm_suspend_get_mode_mask(enum lpm_suspend_resume_mode mode)
{
	// each bit represents a resume method
	// bit0: gpio, bit1:timer
	// bit31: 0:Wake up on timer timeout, 1:Shutdown when timer expires
	uint32_t bit_offset[(uint32_t)LPM_SUSPEND_MODE_MAX] = {0x0, 0x1, 0x1F};
	uint32_t mode_mask = 0;

	switch (mode) {
	case LPM_SUSPEND_MODE_BUTTON:
		mode_mask = (uint32_t)0x1 << bit_offset[(uint32_t)LPM_SUSPEND_MODE_BUTTON];
		break;

	case LPM_SUSPEND_MODE_TIME:
		mode_mask = ((uint32_t)0x1 << bit_offset[(uint32_t)LPM_SUSPEND_MODE_BUTTON]) |
			((uint32_t)0x1 << bit_offset[(uint32_t)LPM_SUSPEND_MODE_TIME]);
		break;

	case LPM_SUSPEND_MODE_TIME_POWEROFF:
		mode_mask = ((uint32_t)0x1 << bit_offset[(uint32_t)LPM_SUSPEND_MODE_BUTTON]) |
			((uint32_t)0x1 << bit_offset[(uint32_t)LPM_SUSPEND_MODE_TIME]) |
			((uint32_t)0x1 << bit_offset[(uint32_t)LPM_SUSPEND_MODE_TIME_POWEROFF]);
		break;

	default:
		// do nothing
		break;
	}

	return mode_mask;
}

int32_t lpm_suspend_set_suspend(
	char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	int32_t ret;
	uint32_t mode_mask;
	struct lpm_suspend_dsmi_power_cfg power_cfg = {0};

	ret = lpm_suspend_trans_in_param(in, in_len, LPM_SUSPEND_POWER_SUSPEND, &power_cfg);
	if (ret != 0) {
		if (ret != -EOPNOTSUPP) {
			lpm_log_err("suspend param check failed\n");
		}
		return ret;
	}

	mode_mask = lpm_suspend_get_mode_mask(power_cfg.mode);
	ret = lpm_suspend_handle_suspend(&power_cfg, mode_mask);
	if (ret != 0) {
		lpm_log_err("notify suspend failed, ret=%d, dev_id=%u, mode=%u, type=%u, value=%u\n",
			ret, power_cfg.dev_id, (uint32_t)power_cfg.mode, (uint32_t)power_cfg.type, power_cfg.value);
		return ret;
	}

	lpm_log_info("set suspend success, dev_id=%u, mode=%u, type=%u, value=%u\n",
		power_cfg.dev_id, (uint32_t)power_cfg.mode, (uint32_t)power_cfg.type, power_cfg.value);

	(void)out;
	(void)out_len;
	return 0;
}

int32_t lpm_suspend_probe(uint64_t *param, uint32_t param_num)
{
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	int32_t ret = lpm_suspend_debugfs_init();
	if (ret != 0) {
		lpm_log_err("lpm suspend init debugfs failed, ret=%d\n", ret);
		return ret;
	}
#endif

	lpm_suspend_init_priv_data();
	lpm_suspend_get_opt_hook();
	(void)param;
	(void)param_num;
	lpm_log_info("lpm suspend probe success\n");
	return 0;
}

int32_t lpm_suspend_remove(uint64_t *param, uint32_t param_num)
{
	lpm_suspend_put_opt_hook();

	(void)param;
	(void)param_num;
	lpm_log_info("lpm suspend remove success\n");
	return 0;
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
STATIC bool lpm_suspend_debugfs_check_mode_mask(
	const struct lpm_suspend_dsmi_power_cfg *power_cfg, uint32_t mode_mask)
{
	uint32_t default_mask = lpm_suspend_get_mode_mask(power_cfg->mode);
	uint32_t mode_id;

	if ((mode_mask == 0) || (mode_mask > default_mask)) {
		lpm_log_err("mode_mask=0x%x shoule large than zero, and not large than 0x%x\n",
			mode_mask, default_mask);
		return false;
	}

	if ((mode_mask & ~default_mask) != 0) {
		lpm_log_err("mode_mask=0x%x illegal, default_mask=0x%x\n",
			mode_mask, default_mask);
		return false;
	}

	for (mode_id = 0; mode_id < (uint32_t)LPM_SUSPEND_MODE_MAX; mode_id++) {
		if ((((mode_mask >> mode_id) & (uint32_t)0x1) != 0) &&
			(((default_mask >> mode_id) & (uint32_t)0x1) == 0)) {
			lpm_log_err("mode_mask=0x%x bit%u should not set, default_mask=0x%x\n",
				mode_mask, mode_id, default_mask);
			return false;
		}
	}
	return true;
}

STATIC void lpm_suspend_debugfs_handle_suspend(char *in, uint32_t in_len, uint32_t mode_mask)
{
	int32_t ret;
	struct lpm_suspend_dsmi_power_cfg power_cfg = {0};

	ret = lpm_suspend_trans_in_param(in, in_len, LPM_SUSPEND_POWER_SUSPEND, &power_cfg);
	if (ret != 0) {
		lpm_log_err("debugfs set suspend failed, param check failed\n");
		return;
	}

	if (!lpm_suspend_debugfs_check_mode_mask(&power_cfg, mode_mask)) {
		lpm_log_err("debugfs set suspend failed, mode_mask=0x%x illegal\n", mode_mask);
		return;
	}

	ret = lpm_suspend_handle_suspend(&power_cfg, mode_mask);
	if (ret != 0) {
		lpm_log_err("debugfs notify suspend failed, ret=%d, "
			"dev_id=%u, mode=%u, type=%u, value=%u, mode_mask=0x%x\n",
			ret, power_cfg.dev_id, power_cfg.mode, power_cfg.type, power_cfg.value, mode_mask);
	} else {
		lpm_log_info("debugfs set suspend success, "
			"dev_id=%u, mode=%u, type=%u, value=%u, mode_mask=0x%x\n",
			power_cfg.dev_id, (uint32_t)power_cfg.mode,
			(uint32_t)power_cfg.type, power_cfg.value, mode_mask);
	}
}

STATIC void lpm_suspend_debugfs_set_suspend(uint32_t *user_cfg, uint32_t param_num)
{
	struct lpm_suspend_dsmi_power_cfg power_cfg = {0};

	// inner function, no need check param_num
	(void)param_num;

	// param will check when handle suspend
	power_cfg.dev_id = user_cfg[0x0];
	power_cfg.mode   = user_cfg[0x1]; // lpm_suspend_resume_mode
	power_cfg.value  = user_cfg[0x2]; // resume/power_off time, unit:ms
	power_cfg.type   = LPM_SUSPEND_POWER_SUSPEND;

	lpm_suspend_debugfs_handle_suspend((char *)&power_cfg, sizeof(power_cfg), user_cfg[0x3]);
}

STATIC ssize_t lpm_suspend_debugfs_write(
	struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
	int32_t ret;
	uint32_t user_cfg[LPM_DEBUGFS_MAX_USER_INPUT_NUM] = {0};

	(void)file;
	(void)pos;

	// dev_id, mode, resume_value, mode_mask
	ret = lpm_debugfs_get_input(buf, len, user_cfg, 0x4);
	if (ret != 0) {
		lpm_log_err("debugfs write suspend failed, ret=%d\n", ret);
		return -1;
	}

	lpm_suspend_debugfs_set_suspend(user_cfg, 0x4);

	return (ssize_t)len;
}

STATIC int32_t lpm_suspend_debugfs_init(void)
{
	int32_t ret;
	struct lpm_debugfs_init_cfg init_cfg = {
		.name = "suspend",
		.fn_open = NULL,
		.fn_write = lpm_suspend_debugfs_write
	};

	ret = lpm_debugfs_register_module(&init_cfg);
	if (ret != 0) {
		lpm_log_err("lpm suspend register debugfs failed, ret=%d\n", ret);
		return ret;
	}
	return 0;
}
#endif
