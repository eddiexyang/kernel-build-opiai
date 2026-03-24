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
#include <linux/types.h>
#include <linux/version.h>
#include <linux/timer.h>
#include "lpm_devmng_common.h"
#include "lpm_devmng_statistics.h"

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"

STATIC int32_t lpm_statistics_debugfs_init(void);
#endif

static struct lpm_dsmi_statistics g_dsmi_stats = {
	false, {
		{"setTops", 0, 0},
		{"setLptest", 0, 0},
		{"setIdleSwitch", 0, 0},

		{"getAicVoltCurr", 0, 0},
		{"getHybridVoltCurr", 0, 0},
		{"getCpuVoltCurr", 0, 0},
		{"getDdrVoltCurr", 0, 0},
		{"getAcgCnt", 0, 0},
		{"getIdleStatus", 0, 0},
		{"getCurrentTops", 0, 0},
		{"getAllTops", 0, 0},
		{"getAicFreqReduc", 0, 0},
		{"getPower", 0, 0},
		{"setSocStress", 0, 0},
		{"getAicCpm", 0, 0},
		{"getBusCpm", 0, 0},
		{"getLptest", 0, 0},

		{"getTempDdr", 0, 0},
		{"getTempDdrThold", 0, 0},
		{"getTempSocThold", 0, 0},
		{"getTempSocMinThold", 0, 0},

		{"setSuspend", 0, 0},
		{"getLpStatus", 0, 0},
		{"getFreq", 0, 0},
		{"getTemp", 0, 0},
		{"getVrdInfo", 0, 0},
	}
};

STATIC struct timer_list *lpm_statistics_get_timer(void)
{
	static struct timer_list stats_timer;
	return &stats_timer;
}

STATIC void lpm_statistics_show_invoking_info(void)
{
	int32_t ret;
	uint32_t i;
	char buf[LPM_STATS_MAX_BUF_LEN] = "lpm dsmi_stats:";
	size_t offset = strnlen(buf, LPM_STATS_MAX_BUF_LEN);

	for (i = 0; i < (uint32_t)LPM_DSMI_API_TYPE_MAX; i++) {
		ret = snprintf_s(buf + offset, LPM_STATS_MAX_BUF_LEN - offset, LPM_STATS_MAX_BUF_LEN - offset - 1U,
			" %s(0x%llx,0x%llx)", g_dsmi_stats.invoking_times[i].name,
			g_dsmi_stats.invoking_times[i].total, g_dsmi_stats.invoking_times[i].failed);
		if (ret < 0) {
			lpm_log_err("snprintf_s failed, type=%u, offset=%u, ret=%d\n", i, (uint32_t)offset, ret);
			return;
		}
		offset += (size_t)ret;

		if ((((i + 1U) % LPM_STATS_ITEM_IN_EACH_LINE) == 0U) || (i == ((uint32_t)LPM_DSMI_API_TYPE_MAX - 1U))) {
			lpm_log_info("%s\n", buf);
			offset = 0;
		}
	}

	g_dsmi_stats.invoking_flag = false;
}

STATIC void lpm_statistics_timer_exec(void)
{
	struct timer_list *stats_timer = lpm_statistics_get_timer();

	// print statistics only if there is new dsmi api invocation
	if (g_dsmi_stats.invoking_flag) {
		lpm_statistics_show_invoking_info();
	}

	stats_timer->expires = jiffies + ((unsigned long)LPM_STATS_PRINT_CYCLE * (unsigned long)HZ);
	add_timer_on(stats_timer, LPM_TIMER_BIND_CPU);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0)
STATIC void lpm_statistics_timer_handle(unsigned long data)
{
	(void)data;
	lpm_statistics_timer_exec();
}
#else
STATIC void lpm_statistics_timer_handle(struct timer_list *t)
{
	(void)t;
	lpm_statistics_timer_exec();
}
#endif

STATIC void lpm_statistics_cfg_init(void)
{
	struct timer_list *stats_timer = lpm_statistics_get_timer();
	(void)memset_s(stats_timer, sizeof(*stats_timer), 0, sizeof(*stats_timer));

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0)
	setup_timer(stats_timer, lpm_statistics_timer_handle, 0);
#else
	timer_setup(stats_timer, lpm_statistics_timer_handle, 0);
#endif

	stats_timer->expires = jiffies + ((unsigned long)LPM_STATS_PRINT_CYCLE * (unsigned long)HZ);
	add_timer_on(stats_timer, LPM_TIMER_BIND_CPU);
}

STATIC void lpm_statistics_cfg_uninit(void)
{
	struct timer_list *stats_timer = lpm_statistics_get_timer();
	(void)del_timer_sync(stats_timer);
}

void lpm_dsmi_update_statistics(int32_t ret, enum lpm_dsmi_api_type api_type)
{
	g_dsmi_stats.invoking_flag = true;
	++g_dsmi_stats.invoking_times[(uint32_t)api_type].total;

	if ((ret != 0) && (ret != -EOPNOTSUPP)) {
		++g_dsmi_stats.invoking_times[(uint32_t)api_type].failed;
	}
}

void lpm_statistics_suspend_prepare(void)
{
	lpm_statistics_cfg_uninit();
	lpm_log_info("lpm statistics suspend prepare success\n");
}

void lpm_statistics_post_suspend(void)
{
	lpm_statistics_cfg_init();
	lpm_log_info("lpm statistics resume success\n");
}

int32_t lpm_statistics_probe(uint64_t *param, uint32_t param_num)
{
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	int32_t ret = lpm_statistics_debugfs_init();
	if (ret != 0) {
		lpm_log_err("lpm statistics init debugfs failed, ret=%d\n", ret);
		return ret;
	}
#endif
	lpm_statistics_cfg_init();

	(void)param;
	(void)param_num;
	lpm_log_info("lpm statistics probe success\n");
	return 0;
}

int32_t lpm_statistics_remove(uint64_t *param, uint32_t param_num)
{
	lpm_statistics_cfg_uninit();

	(void)param;
	(void)param_num;
	lpm_log_info("lpm statistics remove success\n");
	return 0;
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)

STATIC int32_t lpm_statistics_debugfs_show(struct seq_file *seq, void *v)
{
	uint32_t i;

	seq_printf(seq, "lpm dsmi invoking_flag: %s\n", g_dsmi_stats.invoking_flag ? "true" : "false");
	seq_printf(seq, "lpm dsmi invoking statistics(total_times,failed_times):\n");
	for (i = 0; i < (uint32_t)LPM_DSMI_API_TYPE_MAX; i++) {
		seq_printf(seq, "  [%02u]%s(%llu,%llu)\n", i, g_dsmi_stats.invoking_times[i].name,
			g_dsmi_stats.invoking_times[i].total, g_dsmi_stats.invoking_times[i].failed);
	}

	(void)v;
	return 0;
}

STATIC int32_t lpm_statistics_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, lpm_statistics_debugfs_show, inode->i_private);
}

STATIC int32_t lpm_statistics_debugfs_init(void)
{
	int32_t ret;
	struct lpm_debugfs_init_cfg init_cfg = {
		.name = "statistics",
		.fn_open = lpm_statistics_debugfs_open,
		.fn_write = NULL
	};

	ret = lpm_debugfs_register_module(&init_cfg);
	if (ret != 0) {
		lpm_log_err("lpm statistics register debugfs failed, ret=%d\n", ret);
		return ret;
	}
	return 0;
}

#endif
