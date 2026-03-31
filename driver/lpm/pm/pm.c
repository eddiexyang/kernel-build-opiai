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
*/
#include <asm/cputype.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/suspend.h>
#include <linux/syscore_ops.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/cpu_pm.h>
#include <asm/suspend.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/console.h>

#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,2,8)
#include <asm/psci.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,15)
#include <linux/slab.h>
#include <asm/cpu_ops.h>
#include <linux/psci.h>
#endif
#include <linux/printk.h>
#include "pm.h"

#ifndef STATIC
#define STATIC static
#endif

#ifndef LPM_VPRINTK
#define lpm_log_err(...)   (void)pr_err(__VA_ARGS__)
#define lpm_log_info(...)  (void)pr_info(__VA_ARGS__)
#else
#define MODULE_LPM_PM "[DRV_LPM_PM]"
#define LPM_VPRINTK_FACILITY ((int32_t)90) // facility = kernel15
STATIC int32_t hisi_pm_vprintk_emit(int32_t level, const char *fmt, ...)
{
	va_list args;
	int32_t r;

	va_start(args, fmt);
	r = vprintk_emit(LPM_VPRINTK_FACILITY, level, NULL, fmt, args);
	va_end(args);

	return r;
}

#define lpm_log_err(...)   (void)hisi_pm_vprintk_emit(LOGLEVEL_ERR,  MODULE_LPM_PM __VA_ARGS__)
#define lpm_log_info(...)  (void)hisi_pm_vprintk_emit(LOGLEVEL_INFO, MODULE_LPM_PM __VA_ARGS__)
#endif

#ifdef HAL_KERNEL_SUSPEND_RESUME
STATIC s32 hisi_pm_prepare_late(void)
{
	return hal_kernel_high_priority_suspend();
}

STATIC void hisi_pm_wake(void)
{
	(void)hal_kernel_high_priority_resume();
}
#endif

/* PM suspend State type */
#define SR_POWER_STATE_SUSPEND        0x01010000
#define LOWER_8_BIT                   0xFFU
#define BYTE_WIDTH                    8U

/*
 * Function Name: sr_psci_suspend
 * Decription: PSCI interface, calls by suspend and resume
 */
/* cpu_suspend, cpu_resume, psci_ops are not exported in 6.x;
 * resolve them at runtime via kprobe-based lookup. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
#include <linux/kprobes.h>

typedef int (*psci_cpu_suspend_fn)(u32 state, unsigned long entry_point);
typedef int (*cpu_suspend_fn)(unsigned long arg, int (*fn)(unsigned long));

static psci_cpu_suspend_fn compat_psci_cpu_suspend;
static unsigned long compat_cpu_resume_addr;
static cpu_suspend_fn compat_cpu_suspend_fn;

static int compat_pm_resolve_symbols(void)
{
	struct psci_operations *ops;
	unsigned long addr;

	addr = compat_lookup_name("psci_ops");
	if (!addr) {
		lpm_log_err("cannot resolve psci_ops\n");
		return -ENOSYS;
	}
	ops = (struct psci_operations *)addr;
	compat_psci_cpu_suspend = ops->cpu_suspend;

	compat_cpu_resume_addr = compat_lookup_name("cpu_resume");
	if (!compat_cpu_resume_addr) {
		lpm_log_err("cannot resolve cpu_resume\n");
		return -ENOSYS;
	}

	addr = compat_lookup_name("cpu_suspend");
	if (!addr) {
		lpm_log_err("cannot resolve cpu_suspend\n");
		return -ENOSYS;
	}
	compat_cpu_suspend_fn = (cpu_suspend_fn)addr;
	return 0;
}
#endif /* >= 6.0 */

static s32 sr_psci_suspend(unsigned long suspend_arg)
{
	(void)suspend_arg;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,2,8)
	return psci_cpu_suspend_ext(SR_POWER_STATE_SUSPEND,
				virt_to_phys(cpu_resume));
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
	if (!compat_psci_cpu_suspend)
		return -ENOSYS;
	return compat_psci_cpu_suspend(SR_POWER_STATE_SUSPEND,
				compat_cpu_resume_addr);
#else
	return psci_ops.cpu_suspend(SR_POWER_STATE_SUSPEND,
				virt_to_phys(cpu_resume));
#endif
}

/*
 * Function Name: hisi_pm_enter
 * Decription: called by OS to use PSCI
 * Parameters: suspend status
 * Return: 0 is success, otherwise non-zero
 */
static s32 hisi_pm_enter(suspend_state_t state)
{
	u32 cluster = 0;
	u32 core = 0;
	u64 mpidr = read_cpuid_mpidr();

	(void)state;
	/* get cluster and core value */
	cluster = (u32)((mpidr >> BYTE_WIDTH) & LOWER_8_BIT);
	core = (u32)(mpidr & LOWER_8_BIT);
	lpm_log_info("pm enter++: mpidr is 0x%llx, cluster = %u, core = %u.\n",
		mpidr, cluster, core);

	(void)cpu_cluster_pm_enter();
	/* enter real suspend function */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
	if (compat_cpu_suspend_fn)
		(void)compat_cpu_suspend_fn(0, sr_psci_suspend);
#else
	(void)cpu_suspend(0, sr_psci_suspend);
#endif
	/* set resume flag and clear SW backup register */
	(void)cpu_cluster_pm_exit();

	lpm_log_info("pm enter--\n");
	return 0;
}

STATIC const struct platform_suspend_ops* hisi_pm_get_pm_ops(void)
{
	static const struct platform_suspend_ops g_hisi_pm_ops = {
#ifdef HAL_KERNEL_SUSPEND_RESUME
		.prepare_late = hisi_pm_prepare_late,
		.wake = hisi_pm_wake,
#endif
		.enter = hisi_pm_enter,
		.valid = suspend_valid_only_mem,
	};
	return &g_hisi_pm_ops;
}

/*
 * Function Name: hisi_pm_drvinit
 * Decription: PM initialization funtion
 * Parameters: void
 * Return: 0 is success
 */
STATIC s32 hisi_pm_probe(struct platform_device *pdev)
{
	(void)pdev;
	/* register pm_ops */
	suspend_set_ops(hisi_pm_get_pm_ops());
	(void)dev_set_name(&pdev->dev, "pm");
	lpm_log_info("hisi pm suspend ops register succes, device name:%s.\n", dev_name(&pdev->dev));
	return 0;
}

static const struct of_device_id hi195x_pm_of_match[] = {
	{.compatible = "hisilicon,hi195x-pm"},
	{.compatible = "hisilicon,hi191x-pm"},
	{ }
};

MODULE_DEVICE_TABLE(of, hi195x_pm_of_match);
static struct platform_driver hisi_pm_driver = {
	.probe = hisi_pm_probe,
	.driver = {
		.name  = "hisi_pm",
		.of_match_table = hi195x_pm_of_match,
	},
};

STATIC s32 __init hisi_pm_driver_init(void)
{
	s32 ret;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
	ret = compat_pm_resolve_symbols();
	if (ret != 0) {
		lpm_log_err("failed to resolve PM symbols: %d\n", ret);
		return ret;
	}
#endif
	/* disable generic psci suspend ops */
	suspend_set_ops(NULL);
	console_suspend_enabled = false;

	ret = platform_driver_register(&hisi_pm_driver);
	if (ret != 0) {
		lpm_log_err("can't register hisi pm driver: %d\n", ret);
		return ret;
	}

	lpm_log_info("hisi pm driver register succ.\n");
	return 0;
}

STATIC void __exit hisi_pm_driver_exit(void)
{
	suspend_set_ops(NULL);
	platform_driver_unregister(&hisi_pm_driver);
	lpm_log_info("hisi pm driver remove succ.\n");
}

module_init(hisi_pm_driver_init);
module_exit(hisi_pm_driver_exit);
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("Hisilicon pm driver");
MODULE_LICENSE("GPL");
