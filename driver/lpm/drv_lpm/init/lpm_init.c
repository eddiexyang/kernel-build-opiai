/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
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
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/init.h>
#include <linux/securec.h>
#include <linux/kernel.h>
#include <linux/suspend.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include "lpm_fault_common.h"
#include "lpm_fault_sharemem.h"
#include "lpm_fault_report.h"
#include "lpm_fault_config.h"
#include "lpm_fault_heartbeat.h"
#include "lpm_fault_debugfs.h"
#include "lpm_fault_channel.h"
#include "lpm_fault_ipc.h"
#include "lpm_fault_startup.h"
#include "lpm_fault_from_ipc.h"
#include "lpm_fault_safety.h"
#include "lpm_fault_ras.h"
#include "lpm_init.h"

STATIC int32_t lpm_sharemem_init(uint32_t dev_num)
{
	(void)dev_num;
	return lpm_sharemem_probe(NULL, 0);
}
STATIC int32_t lpm_sharemem_exit(uint32_t dev_num)
{
	(void)dev_num;
	return lpm_sharemem_remove(NULL, 0);
}

STATIC struct lpm_init_module_table *lpm_get_init_table(uint32_t *table_num)
{
	static struct lpm_init_module_table lpm_fault_module_table[] = {
		// debugfs should init first
		{"debugfs",   lpm_fault_debugfs_init,   lpm_fault_debugfs_exit},
		{"common",    lpm_fault_common_init,    lpm_fault_common_exit},
		{"sharemem",  lpm_sharemem_init,        lpm_sharemem_exit},
		// config should init before report
		{"config",    lpm_fault_config_init,    lpm_fault_config_exit},
		{"report",    lpm_fault_dev_node_init,  lpm_fault_dev_node_exit},
		// channel should init before ipc
		// channel should init before modules that need to register ipc rx handle
		{"channel",   lpm_fault_channel_init,   lpm_fault_channel_exit},
		{"from_ipc",  lpm_fault_from_ipc_init,  lpm_fault_from_ipc_exit},
		{"heartbeat", lpm_fault_heartbeat_init, lpm_fault_heartbeat_exit},
		// modules that need to register ipc rx handle need to be init before ipc
		{"ipc",       lpm_ipc_receiver_init,    lpm_ipc_receiver_exit},
		{"startup",   lpm_fault_startup_init,   lpm_fault_startup_exit},
		{"safety",    lpm_fault_safety_init,    lpm_fault_safety_exit},
		{"ras",       lpm_fault_ras_init,       lpm_fault_ras_exit}
	};

	*table_num = (uint32_t)ARRAY_SIZE(lpm_fault_module_table);
	return &lpm_fault_module_table[0];
}

STATIC int32_t lpm_init_module(uint32_t dev_num)
{
	int32_t ret;
	uint32_t table_num = 0;
	uint32_t module_id;
	uint32_t i;
	struct lpm_init_module_table *table_list = lpm_get_init_table(&table_num);

	for (module_id = 0; module_id < table_num; module_id++) {
		ret = table_list[module_id].fn_init(dev_num);
		if (ret != 0) {
			lpm_log_err("lpm %s init failed, ret=%d, table_num=%u\n",
				table_list[module_id].module_name, ret, table_num);
			goto init_failed;
		}
	}
	return 0;

init_failed:
	// sequential initialization, reverse order deinitialization
	for (i = 1; i <= module_id; i++) {
		if (table_list[module_id - i].fn_exit != NULL) {
			ret = table_list[module_id - i].fn_exit(dev_num);
			if (ret != 0) {
				lpm_log_err("lpm %s exit failed, ret=%d\n",
					table_list[module_id - i].module_name, ret);
			}
		}
	}
	return -1;
}

STATIC int32_t lpm_exit_module(uint32_t dev_num)
{
	int32_t ret;
	int32_t final_ret = 0;
	uint32_t table_num = 0;
	uint32_t module_id;
	struct lpm_init_module_table *table_list = lpm_get_init_table(&table_num);

	for (module_id = 1; module_id <= table_num; module_id++) {
		if (table_list[table_num - module_id].fn_exit != NULL) {
			ret = table_list[table_num - module_id].fn_exit(dev_num);
			if (ret != 0) {
				lpm_log_err("lpm %s exit failed, ret=%d\n",
					table_list[table_num - module_id].module_name, ret);
				final_ret = -1;
			}
		}
	}
	return final_ret;
}

STATIC int32_t lpm_fault_suspend(struct device *dev)
{
	if (dev == NULL) {
		lpm_log_err("lpm fault driver suspend parameter invalid\n");
		return -1;
	}

	lpm_hb_task_suspend_prepare();
	lpm_safety_suspend_prepare();
	lpm_log_info("lpm fault driver suspend success.\n");
	return 0;
}

STATIC int32_t lpm_fault_resume(struct device *dev)
{
	if (dev == NULL) {
		lpm_log_err("lpm fault driver resume parameter invalid\n");
		return -1;
	}

	lpm_hb_task_post_suspend();
	lpm_safety_post_suspend();
	lpm_log_info("lpm fault driver resume success.\n");
	return 0;
}

#if !defined(LPM_START_DIRECT)
SIMPLE_DEV_PM_OPS(lpm_fault_ops, lpm_fault_suspend, lpm_fault_resume);
#endif

#if defined(LPM_START_DIRECT)
STATIC int32_t lpm_notify_nb(
	struct notifier_block *nb, unsigned long mode, void *unused)
{
	int32_t ret = 0;
	struct device dev = {0};

	(void)nb;
	(void)unused;

	switch (mode) {
	case PM_SUSPEND_PREPARE:
		// Going to suspend the system
		ret = lpm_fault_suspend(&dev);
		lpm_log_info("PM_SUSPEND_PREPARE done\n");
		break;
	case PM_POST_SUSPEND:
		ret = lpm_fault_resume(&dev);
		lpm_log_info("PM_POST_SUSPEND done\n");
		break;
	default:
		return NOTIFY_DONE;
	}

	if (ret != 0) {
		// no error is returned here, so as to avoid affecting sleep wakeup in other fields
		lpm_log_err("lpm fault driver suspend/resume occur some error, mode=%lu, ret=%d\n",
			mode, ret);
	}

	return NOTIFY_OK;
}

STATIC struct notifier_block *lpm_get_notify_nb(void)
{
	static struct notifier_block lpm_idle_pm_nb = {
		.notifier_call = lpm_notify_nb
	};
	return &lpm_idle_pm_nb;
}

STATIC int32_t lpm_register_notifier(void)
{
	int32_t ret;

	// aos_core not support this api
	ret = register_pm_notifier(lpm_get_notify_nb());
	if (ret != 0) {
		lpm_log_err("lpm fault driver register pm notifier error, ret=%d\n", ret);
		return -1;
	}
	return 0;
}

STATIC int32_t lpm_unregister_notifier(void)
{
	int32_t ret;

	ret = unregister_pm_notifier(lpm_get_notify_nb());
	if (ret != 0) {
		lpm_log_err("lpm fault driver unregister pm notifier error, ret=%d\n", ret);
		return -1;
	}
	return 0;
}
#endif

STATIC int32_t lpm_fault_probe(struct platform_device *pdev)
{
	int32_t ret;
	uint32_t dev_num = 0;

	if (pdev == NULL) {
		lpm_log_err("lpm fault driver probe parameter invalid\n");
		return -1;
	}

	ret = devdrv_get_devnum(&dev_num);
	if (ret != 0) {
		lpm_log_err("lpm fault driver get devnum failed, ret=%d\n", ret);
		return ret;
	}
	if (dev_num > LPM_DMS_NODE_MAX_NUM) {
		lpm_log_err("lpm fault driver dev_num=%u is out of range.\n", dev_num);
		return -EINVAL;
	}

	ret = lpm_init_module(dev_num);
	if (ret != 0) {
		lpm_log_err("lpm fault driver init module failed, ret=%d\n", ret);
		return ret;
	}

#if !defined(LPM_START_DIRECT)
	(void)dev_set_name(&pdev->dev, "lpmfault");
	lpm_log_info("lpm fault driver probe success, dev_num=%u, dev name=%s.\n",
		dev_num, dev_name(&pdev->dev));
#else
	ret = lpm_register_notifier();
	if (ret != 0) {
		lpm_log_err("lpm fault driver register notifier failed, ret=%d\n", ret);
		(void)lpm_exit_module(dev_num);
		return -1;
	}

	lpm_log_info("lpm fault driver probe success, dev_num=%u.\n", dev_num);
#endif
	return 0;
}

STATIC void lpm_fault_remove(struct platform_device *pdev)
{
	int32_t ret;
	uint32_t dev_num = lpm_common_get_dev_num();

	if (pdev == NULL) {
		lpm_log_err("lpm fault driver remove parameter invalid\n");
		/* return removed */;  // -1;
	}

#if defined(LPM_START_DIRECT)
	ret = lpm_unregister_notifier();
	if (ret != 0) {
		lpm_log_err("lpm fault driver unregister notifier failed, ret=%d, dev_num=%u\n",
			ret, dev_num);
	}
#endif
	ret = lpm_exit_module(dev_num);
	if (ret != 0) {
		lpm_log_err("lpm fault driver exit module failed, ret=%d, dev_num=%u\n",
			ret, dev_num);
	}

	lpm_log_info("lpm fault driver remove end.\n");
	/* return removed */
}

#if !defined(LPM_START_DIRECT)
STATIC void lpm_fault_shutdown(struct platform_device *pdev)
{
	if (pdev == NULL) {
		lpm_log_err("lpm fault driver shutdown parameter invalid\n");
		return;
	}

	lpm_log_info("lpm fault driver shutdown success.\n");
}


static const struct of_device_id lpm_fault_of_match_dp[] = {
	{.compatible = MODULE_LPM_MATCH},
	{}
};
MODULE_DEVICE_TABLE(of, lpm_fault_of_match_dp);

static struct platform_driver lpm_fault_driver = {
	.probe = lpm_fault_probe,
	.remove = lpm_fault_remove,
	.shutdown = lpm_fault_shutdown,
	.driver = {
		.name           = MODULE_LPM,
		.pm             = &lpm_fault_ops,
		.of_match_table = of_match_ptr(lpm_fault_of_match_dp),
	},
};
#endif

STATIC int32_t lpm_init(void)
{
	int32_t ret;
#if !defined(LPM_START_DIRECT)
	ret = platform_driver_register(&lpm_fault_driver);
	if (ret != 0) {
		lpm_log_err("register lpm fault driver failed, ret=%d\n", ret);
		return ret;
	}
#else
	struct platform_device pdev = {0};
	ret = lpm_fault_probe(&pdev);
	if (ret != 0) {
		lpm_log_err("lpm fault driver probe failed, ret=%d\n", ret);
		return ret;
	}
#endif

	lpm_log_info("lpm fault driver init success\n");
	return 0;
}

STATIC void __exit lpm_exit(void)
{
#if !defined(LPM_START_DIRECT)
	platform_driver_unregister(&lpm_fault_driver);
#else
	int32_t ret;
	struct platform_device pdev = {0};
	ret = lpm_fault_remove(&pdev);
	if (ret != 0) {
		lpm_log_err("lpm fault driver exit failed, ret=%d\n", ret);
		return;
	}
#endif
	lpm_log_info("lpm fault driver remove success\n");
}

module_init(lpm_init);
module_exit(lpm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("LPM driver");
