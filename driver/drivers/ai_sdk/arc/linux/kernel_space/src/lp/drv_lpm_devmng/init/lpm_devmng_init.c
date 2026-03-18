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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/suspend.h>
#include "devdrv_manager_comm.h"
#include "lpm_devmng_debugfs.h"
#include "lpm_devmng_common.h"
#include "lpm_devmng_suspend.h"
#include "lpm_devmng_profile.h"
#include "lpm_devmng_idle.h"
#include "lpm_devmng_dsmi.h"
#include "lpm_devmng_sharemem.h"
#include "lpm_devmng_tool.h"
#include "lpm_devmng_aic_freq.h"
#include "lpm_devmng_ipc.h"
#include "lpm_devmng_flash.h"
#include "lpm_devmng_turbo.h"
#include "lpm_devmng_vrd_upgrade.h"
#include "lpm_devmng_frequency.h"
#include "lpm_devmng_temperature.h"
#include "lpm_devmng_health_status.h"
#include "lpm_devmng_volt_current.h"
#include "lpm_devmng_soc_stress.h"
#include "lpm_devmng_acg_cnt.h"
#include "lpm_devmng_temp_threshold.h"
#include "lpm_devmng_statistics.h"
#include "lpm_devmng_init.h"

#define LPM_IDLE_NOTIFY_PRO 1

STATIC struct lpm_common_init_table *lpm_devmng_get_init_table(uint32_t *table_num)
{
	static struct lpm_common_init_table lpm_devmng_init_table[] = {
		{"init debugfs",   lpm_devmng_debugfs_init, lpm_devmng_debugfs_exit},
		{"init common",    lpm_common_probe,        lpm_common_remove},
		{"init sharemem",  lpm_sharemem_probe,      lpm_sharemem_remove},
		{"init stats",     lpm_statistics_probe,    lpm_statistics_remove},
		{"init dsmi",      lpm_dsmi_probe,          lpm_dsmi_remove},
		{"init ipc",       lpm_ipc_probe,           lpm_ipc_remove},
		// the calls order of the above cannot be adjusted
		{"init idle",      lpm_idle_probe,          lpm_idle_remove},
		{"init profile",   lpm_profile_probe,       lpm_profile_remove},
		{"init suspend",   lpm_suspend_probe,       lpm_suspend_remove},
		{"init aic_freq",  lpm_aic_freq_probe,      lpm_aic_freq_remove},
		{"init stress",    lpm_soc_stress_probe,    lpm_soc_stress_remove},
		{"init lptest",    lpm_lptest_init,         lpm_lptest_exit},
		{"init turbo",     lpm_turbo_probe,         lpm_turbo_remove},
		{"init vrd_up",    lpm_vrd_upgrade_probe,   lpm_vrd_upgrade_remove},
		{"init frequency", lpm_frequency_probe,     lpm_frequency_remove},
		{"init temperature", lpm_temperature_probe, lpm_temperature_remove},
		{"init health",    lpm_health_status_probe, lpm_health_status_remove},
		{"init volt_curr", lpm_volt_current_probe,  lpm_volt_current_remove},
		{"init acg_cnt",   lpm_acg_cnt_probe,       lpm_acg_cnt_remove},
		{"init temp_thold", lpm_temp_threshold_probe, lpm_temp_threshold_remove},
		// the calls order of the following cannot be adjusted
		{"init flash",     lpm_flash_probe,         lpm_flash_remove}
	};

	*table_num = (uint32_t)ARRAY_SIZE(lpm_devmng_init_table);
	return &lpm_devmng_init_table[0];
}

STATIC int32_t lpm_devmng_init_module(uint32_t dev_num)
{
	int32_t ret;
	uint32_t table_num = 0;
	uint64_t param = dev_num;
	struct lpm_common_init_table *table_list = lpm_devmng_get_init_table(&table_num);

	ret = lpm_devmng_module_init(table_list, table_num, &param, 0x1);
	if (ret != 0) {
		lpm_log_err("lpm devmng module probe failed, ret=%d, table_num=%u\n", ret, table_num);
		return -1;
	}

	return 0;
}

STATIC int32_t lpm_devmng_exit_module(void)
{
	int32_t ret;
	uint32_t table_num = 0;
	struct lpm_common_init_table *table_list = lpm_devmng_get_init_table(&table_num);

	ret = lpm_devmng_module_uninit(table_list, table_num, NULL, 0);
	if (ret != 0) {
		lpm_log_err("lpm devmng module remove failed, ret=%d, table_num=%u\n", ret, table_num);
		return -1;
	}
	return 0;
}

STATIC int32_t lpm_devmng_suspend_prepare(void)
{
	int32_t ret;

	lpm_flash_suspend_prepare();
	lpm_statistics_suspend_prepare();
	ret = lpm_idle_suspend_prepare();
	if (ret != 0) {
		lpm_log_err("lpm devmng idle suspend failed, ret=%d\n", ret);
		return -1;
	}
	return 0;
}

STATIC int32_t lpm_devmng_post_suspend(void)
{
	int32_t ret;

	lpm_aic_freq_post_suspend();
	lpm_flash_post_suspend();
	lpm_statistics_post_suspend();
	ret = lpm_idle_post_suspend();
	if (ret != 0) {
		lpm_log_err("lpm devmng idle resume failed, ret=%d\n", ret);
		return ret;
	}
	return 0;
}

STATIC int32_t lpm_devmng_notify_nb(
	struct notifier_block *nb, unsigned long mode, void *unused)
{
	int32_t ret;

	(void)nb;
	(void)unused;

	switch (mode) {
	case PM_SUSPEND_PREPARE:
		// Going to suspend the system
		ret = lpm_devmng_suspend_prepare();
		break;
	case PM_POST_SUSPEND:
		ret = lpm_devmng_post_suspend();
		break;
	default:
		// not support
		ret = -1;
		break;
	}
	if (ret != 0) {
		// no error is returned here
		// to avoid affecting the execution of subsequent domains
		return NOTIFY_DONE;
	}

	return NOTIFY_OK;
}

STATIC struct notifier_block *lpm_devmng_get_notify_nb(void)
{
	static struct notifier_block lpm_idle_pm_nb = {
		.notifier_call = lpm_devmng_notify_nb,
		.priority = LPM_IDLE_NOTIFY_PRO,
	};
	return &lpm_idle_pm_nb;
}

STATIC int32_t lpm_devmng_register_notifier(void)
{
	int32_t ret;

	ret = register_pm_notifier(lpm_devmng_get_notify_nb());
	if (ret != 0) {
		lpm_log_err("lpm devmng register pm notifier failed, ret=%d\n", ret);
		return -1;
	}
	return 0;
}

STATIC int32_t lpm_devmng_unregister_notifier(void)
{
	int32_t ret;

	ret = unregister_pm_notifier(lpm_devmng_get_notify_nb());
	if (ret != 0) {
		lpm_log_err("lpm devmng unregister pm notifier failed, ret=%d\n", ret);
		return -1;
	}
	return 0;
}

STATIC int32_t lpm_devmng_get_devnum(uint32_t *dev_num)
{
	int32_t ret;

	ret = devdrv_get_devnum(dev_num);
	if (ret != 0) {
		lpm_log_err("devdrv_get_devnum failed, ret=%d\n", ret);
		return ret;
	}

	if (*dev_num > LPM_DEVMNG_DEV_MAX_NUM) {
		lpm_log_err("dev_num is out of range, dev_num=%u\n", *dev_num);
		return -1;
	}
	return 0;
}

// lpm_devmng ko should insmod at last
STATIC int32_t lpm_devmng_probe(struct platform_device *pdev)
{
	int32_t ret;
	uint32_t dev_num = 0;

	if (pdev == NULL) {
		lpm_log_err("lpm devmng probe parameter invalid\n");
		return -1;
	}

	ret = lpm_devmng_get_devnum(&dev_num);
	if (ret != 0) {
		lpm_log_err("get devnum failed, ret=%d\n", ret);
		return ret;
	}

	ret = lpm_devmng_init_module(dev_num);
	if (ret != 0) {
		lpm_log_err("lpm devmng init module failed, ret=%d\n", ret);
		return ret;
	}

	ret = lpm_devmng_register_notifier();
	if (ret != 0) {
		lpm_log_err("lpm idle register notifier failed, ret=%d\n", ret);
		goto register_notifier_failed;
	}

#ifndef LPM_START_DIRECT
	(void)dev_set_name(&pdev->dev, "lpmdevmng");
	lpm_log_info("lpm devmng driver probe success, dev_num=%u, dev name=%s.\n",
		dev_num, dev_name(&pdev->dev));
#else
	lpm_log_info("lpm devmng driver probe success, dev_num=%u.\n", dev_num);
#endif
	return 0;

register_notifier_failed:
	(void)lpm_devmng_exit_module();
	lpm_log_err("lpm devmng driver probe failed.\n");
	return -1;
}

STATIC int32_t lpm_devmng_remove(struct platform_device *pdev)
{
	int32_t ret;

	if (pdev == NULL) {
		lpm_log_err("lpm devmng remove parameter invalid\n");
		return -1;
	}

	ret = lpm_devmng_exit_module();
	if (ret != 0) {
		lpm_log_err("lpm devmng exit module failed, ret=%d\n", ret);
	}

	ret = lpm_devmng_unregister_notifier();
	if (ret != 0) {
		lpm_log_err("lpm unregister notifier failed, ret=%d\n", ret);
	}

	lpm_log_info("lpm devmng driver remove end.\n");
	return 0;
}

#ifndef LPM_START_DIRECT
STATIC void lpm_devmng_shutdown(struct platform_device *pdev)
{
	if (pdev == NULL) {
		lpm_log_err("lpm devmng shutdown parameter invalid\n");
		return;
	}

	lpm_log_info("lpm devmng driver shutdown success.\n");
}

STATIC int32_t lpm_devmng_suspend(struct device *dev)
{
	if (dev == NULL) {
		lpm_log_err("lpm devmng suspend parameter invalid\n");
		return -1;
	}

	lpm_log_info("lpm devmng driver suspend success.\n");
	return 0;
}

STATIC int32_t lpm_devmng_resume(struct device *dev)
{
	if (dev == NULL) {
		lpm_log_err("lpm devmng resume parameter invalid\n");
		return -1;
	}

	lpm_log_info("lpm devmng driver resume success.\n");
	return 0;
}

SIMPLE_DEV_PM_OPS(lpm_devmng_ops, lpm_devmng_suspend, lpm_devmng_resume);

static const struct of_device_id lpm_devmng_of_match_dp[] = {
	{ .compatible = MODULE_LPM_DEVMNG_MATCH},
	{ }
};
MODULE_DEVICE_TABLE(of, lpm_devmng_of_match_dp);

static struct platform_driver lpm_devmng_driver = {
	.probe = lpm_devmng_probe,
	.remove = lpm_devmng_remove,
	.shutdown = lpm_devmng_shutdown,
	.driver = {
		.name           = MODULE_LPM_DEVMNG,
		.pm             = &lpm_devmng_ops,
		.of_match_table = of_match_ptr(lpm_devmng_of_match_dp),
	},
};
#endif

STATIC int32_t __init lpm_devmng_init(void)
{
	int32_t ret;
#ifndef LPM_START_DIRECT
	ret = platform_driver_register(&lpm_devmng_driver);
	if (ret != 0) {
		lpm_log_err("register lpm_devmng_driver failed, ret=%d\n", ret);
		return ret;
	}
#else
	struct platform_device pdev = {0};
	ret = lpm_devmng_probe(&pdev);
	if (ret != 0) {
		lpm_log_err("lpm devmng probe failed, ret=%d\n", ret);
		return ret;
	}
#endif

	lpm_log_info("lpm devmng init success\n");

	return 0;
}
STATIC void __exit lpm_devmng_exit(void)
{
#ifndef LPM_START_DIRECT
	platform_driver_unregister(&lpm_devmng_driver);
#else
	int32_t ret;
	struct platform_device pdev = {0};
	ret = lpm_devmng_remove(&pdev);
	if (ret != 0) {
		lpm_log_err("lpm devmng exit failed, ret=%d\n", ret);
		return;
	}
#endif
	lpm_log_info("lpm devmng exit success\n");
}

module_init(lpm_devmng_init);
module_exit(lpm_devmng_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("LPM DEVMNG driver");
