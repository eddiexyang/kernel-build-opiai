/*
 * Power managerment for HISI IC
 *
 * Copyright (C) 2019 Hisilicon, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <asm/types.h>
#include <asm/suspend.h>
#include <asm/cputype.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/suspend.h>
#include <linux/syscore_ops.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cpu_pm.h>
#include <linux/clk.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include "wakeup_reason.h"

#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,2,8)
#include <asm/psci.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,15)
#include <linux/slab.h>
#include <asm/cpu_ops.h>
#include <linux/psci.h>
#endif

#ifndef STATIC
#define STATIC static
#endif

/* PM suspend State type */
#define POWER_STATE_TYPE_SYS_SUSPEND  3
#define SR_POWER_STATE_SUSPEND        (0x01010000)

/* sofeware bakeup register 6 for SR flag */
#define cpu_sr_flag_reg(cluster)      (0x3428)

/* cpu mtcmos status for checking the cpu power down or now */
#define REG_SCPWRSTAT_OFFSET          (0x7100)
#define REG_CHIPID_OFFSET             (0xfffc)
#define AP_SUSPEND_FLAG               BIT(16)

/* IRQ group and register bit define */
#define IRQ_GRP_MAX                 20
#define IRQ_NUM_PER_WORD            32
#define SPI_GRP_START               2

/* GPIO Buffer length  */
#define GPIO_BUFFER_LEN             40
#define GPIO_GROUP0_IRQ_NO          233
#define GICD_ICPENDR_SPI_OFFSET     0x284
#define REG_BYTE_NUM                4

/* write SR tick to PMU */
#ifdef CONFIG_HISI_SR_TICK
#define pmu_write_sr_tick(offset, pos)  hisi_pmic_write_reg(offset, pos)
#else
#define pmu_write_sr_tick(offset, pos)
#endif

/* STATIC memory point for system base register */
STATIC void __iomem *g_sysctrl_base = NULL;
STATIC void __iomem *g_enable_base = NULL;
STATIC void __iomem *g_pending_base = NULL;
STATIC void __iomem *g_icpender_base = NULL;
STATIC void __iomem *g_cpu_cluster_base_addr = NULL;
STATIC void __iomem *g_chipid_addr = NULL;

/* irq totall number */
unsigned int  g_ap_irq_num;
/* Global Pointer do not need give value when define */
const char    **g_ap_irq_name;

/* define test times of cpu pd */
unsigned int  g_test_cpu_pd_times;

#ifdef CONFIG_HISI_SR_DEBUG_SLEEP

STATIC unsigned int g_irq_ctrl_value[IRQ_GRP_MAX] = {0};

/* read all irq group value*/
STATIC void pm_gic_dump(void)
{
	unsigned int i = 0;

	/* start from number 64 is SPI interrupt */
	for (i = SPI_GRP_START; i < IRQ_GRP_MAX; i++)
		g_irq_ctrl_value[i] = readl(g_enable_base + i * 4);
}

STATIC void gic_dump_group(unsigned int grp_num)
{
	unsigned int irq_bit = 0;
	unsigned int value = 0;
	unsigned int irq = 0;

	value = readl(g_pending_base + grp_num * 4);
	/* foreach irq number in each group register */
	for (irq_bit = 0; irq_bit < IRQ_NUM_PER_WORD; irq_bit++) {
		if ((value & BIT_MASK(irq_bit)) && ((value & BIT_MASK(irq_bit)) ==
			(g_irq_ctrl_value[grp_num] & BIT_MASK(irq_bit)))) {
			irq = grp_num * IRQ_NUM_PER_WORD + irq_bit;
			if (irq < g_ap_irq_num) {
				pr_info("wake up irq num: %u, irq name: %s\n",
						irq, g_ap_irq_name[irq]);
				log_wakeup_reason((int)irq);
			} else {
				pr_info("wake up irq num: %u, irq name: no name\n", irq);
			}
		}
	}
}

/* get the pending index to decide the resume irq */
STATIC void pm_gic_pending_dump(void)
{
	unsigned int grp_num = 0;
	/* we ONLY concern SPI interrupt which start from number 64 */
	for (grp_num = SPI_GRP_START; grp_num < IRQ_GRP_MAX; grp_num++)
		gic_dump_group(grp_num);
}

/* get the irq name according to irq_num */
char *get_irq_name(unsigned int irq_num)
{
	if ((irq_num >= g_ap_irq_num) || (g_ap_irq_name == NULL) ||
		(g_ap_irq_name[irq_num] == NULL))
		return NULL;

	return (char *)(g_ap_irq_name[irq_num]);
}
#endif

/* Set suepend flag into SW bakeup regiter */
STATIC void hisi_set_ap_suspend_flag(unsigned int cluster)
{
	unsigned int val = 0;

	/* do not need lock, as the core is only one now. */
	val = readl(g_sysctrl_base + cpu_sr_flag_reg(cluster));
	val |= AP_SUSPEND_FLAG;
	writel(val, g_sysctrl_base + cpu_sr_flag_reg(cluster));
}

/* clear suepend flag in SW bakeup regiter */
STATIC void hisi_clear_ap_suspend_flag(unsigned int cluster)
{
	unsigned int val = 0;

	/* do not need lock, as the core is only one now. */
	val = readl(g_sysctrl_base + cpu_sr_flag_reg(cluster));
	val &= ~AP_SUSPEND_FLAG;
	writel(val, g_sysctrl_base + cpu_sr_flag_reg(cluster));
}

/* test slave core whether in power down state
 * environment board id list table:
 *    asic: 0x00000000
 *    fpga: 0x0000xxxx
 *    emu:  0x00010000
 *    esl:  0x00020000
 * if other cores have been PD, reture 0; otherwise -1
 */
STATIC int hisi_test_pwrdn_othercores(unsigned int cluster, unsigned int core)
{
	unsigned int pwrack_stat = 0;
	unsigned int mask = 0xFE;  /* default value is asic */
	unsigned int chipid = 0x0; /* default value is asic */

	/* judge the board type */
	chipid = readl(g_chipid_addr + REG_CHIPID_OFFSET);
	if ((chipid & 0xFFFF) != 0)
		mask = 0xE;  /* fpga ONLY has 4 cores */
	else if ((chipid >> 16) == 0x1)
		mask = 0xFE; /* EMU has 8 cores */
	else if ((chipid >> 16) == 0x2)
		return RTN_SUCCESS;

	/* read the cpus power status and compare the status */
	pwrack_stat = readl(g_cpu_cluster_base_addr + REG_SCPWRSTAT_OFFSET);
	if (pwrack_stat != mask) {
		g_test_cpu_pd_times++;
		pr_err("%s: chipid is 0x%x, pwrack_stat is 0x%x, mask = 0x%x\n",
			 __func__, chipid, pwrack_stat, mask);
	}

	return (pwrack_stat == mask) ? RTN_SUCCESS : -EBUSY;
}

STATIC int sr_psci_suspend(unsigned long index)
{
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,2,8)
	return psci_cpu_suspend_ext(SR_POWER_STATE_SUSPEND,
				virt_to_phys(cpu_resume));
#else
	return psci_ops.cpu_suspend(SR_POWER_STATE_SUSPEND,
				virt_to_phys(cpu_resume));
#endif
}

void hisi_cpu_suspend(void)
{
	cpu_suspend(0, sr_psci_suspend);
}
EXPORT_SYMBOL(hisi_cpu_suspend);

/*
 * Function Name: hisi_clear_irq_pending
 * Decription: clear pending after system resume
 * Parameters: unsigned int irqno
 */
STATIC void hisi_clear_irq_pending(unsigned int irqno)
{
	unsigned int offset = 0;
	unsigned int old_reg_val = 0;
	unsigned int irq_bit = 0;
	void __iomem * irq_pender_addr = NULL;

	if (irqno > (IRQ_GRP_MAX * IRQ_NUM_PER_WORD)) {
		pr_err("irqno %u out of range\n", irqno);
		return ;
	}

	/* calc pending register offset */
	offset = irqno / IRQ_NUM_PER_WORD;
	irq_pender_addr = g_icpender_base + offset * REG_BYTE_NUM;
	old_reg_val = readl(irq_pender_addr);
	/* calc irq offset in pending register */
	irq_bit = irqno % IRQ_NUM_PER_WORD;
	/* if pending bit = 1, clear pending after system resume */
	if (test_bit(irq_bit, irq_pender_addr)) {
		writel(BIT(irq_bit), irq_pender_addr);
		pr_info("icpender 0x%pK old val = 0x%x, new val = 0x%x \n", irq_pender_addr, old_reg_val, readl(irq_pender_addr));
	}
}

/*
 * Function Name: hisi_pm_enter
 * Decription: called by OS to use PSCI
 * Parameters: suspend status
 * Return: 0 is success, otherwise non-zero
 */
STATIC int hisi_pm_enter(suspend_state_t state)
{
	unsigned int cluster = 0;
	unsigned int core = 0;
	unsigned long mpidr = read_cpuid_mpidr();

	/* get cluster and core value */
	cluster = (mpidr >> 8) & 0xff;
	core = mpidr & 0xff;

	pr_info("pm enter++: mpidr is 0x%lx, cluster = %u, core = %u.\n",
			mpidr, cluster, core);

#ifdef CONFIG_HISI_SR_DEBUG_SLEEP
	pm_gic_dump();
#endif

	/* wait until other cores powered down */
	g_test_cpu_pd_times = 0;
	while (RTN_SUCCESS != hisi_test_pwrdn_othercores(cluster, core)) {
		if (g_test_cpu_pd_times >= 10)
			return -EBUSY;
		mdelay(500);
	}

	/* set SR flga to PMU and SW backup register */
	pmu_write_sr_tick(PMUOFFSET_SR_TICK, KERNEL_SUSPEND_SETFLAG);
	hisi_set_ap_suspend_flag(cluster);
	cpu_cluster_pm_enter();

	/* enter real suspend function */
	hisi_cpu_suspend();

	/* set resume flag and clear SW backup register */
	cpu_cluster_pm_exit();
	hisi_clear_ap_suspend_flag(cluster);
	hisi_clear_irq_pending(GPIO_GROUP0_IRQ_NO);
	pmu_write_sr_tick(PMUOFFSET_SR_TICK, KERNEL_RESUME);

#ifdef CONFIG_HISI_SR_DEBUG_SLEEP
	pm_gic_pending_dump();
#endif

	pr_info("pm enter--\n");
	return RTN_SUCCESS;
}

STATIC const struct platform_suspend_ops hisi_pm_ops = {
	.enter = hisi_pm_enter,
	.valid = suspend_valid_only_mem,
};

/* get the irq information from the DTB */
STATIC int hisi_get_gic_base(void)
{
	void __iomem *hisi_gic_dist_base = NULL;
	struct device_node *node = NULL;
	unsigned int enable_offset = 0;
	unsigned int pending_offset = 0;
	int ret = 0;

	/* get lp_gic node from DTB */
	node = of_find_compatible_node(NULL, NULL, "hisilicon,lp_gic");
	if (node == NULL)
		return -ENODEV;

	/* get IRQ enable regiter offset */
	ret = of_property_read_u32(node, "enable-offset", &enable_offset);
	if (ret) {
		of_node_put(node);
		return -ENODEV;
	}

	/* get IRQ pending status regiter offset */
	ret = of_property_read_u32(node, "pending-offset", &pending_offset);
	if (ret) {
		of_node_put(node);
		return -ENODEV;
	}

	/* get GICD base addr */
	hisi_gic_dist_base = of_iomap(node, 0);
	if (hisi_gic_dist_base == NULL) {
		of_node_put(node);
		return -ENODEV;
	}

	of_node_put(node);

	/* get enable control and pending status physical addr */
	g_enable_base = hisi_gic_dist_base + enable_offset;
	g_pending_base = hisi_gic_dist_base + pending_offset;
	g_icpender_base = hisi_gic_dist_base + GICD_ICPENDR_SPI_OFFSET;

	return RTN_SUCCESS;
}

/* initialise the low power table */
STATIC int init_lowpm_table(struct device_node *np)
{
	int ret = 0;
	unsigned int i = 0;

	if (np == NULL)
		return -ENODEV;

	/* init ap irq total number, return value never be zero */
	ret = of_property_count_strings(np, "ap-irq-table");
	if (ret < 0)
		return ret;

	g_ap_irq_num = ret;

	/* allocate ap irq name buffer */
	g_ap_irq_name = kcalloc(g_ap_irq_num, sizeof(char *), GFP_KERNEL | __GFP_ZERO);
	if (g_ap_irq_name == NULL)
		return -ENOMEM;

	/* fill the irq name buffer */
	for (i = 0; i < g_ap_irq_num; i++) {
		ret = of_property_read_string_index(np, "ap-irq-table",
					i, &g_ap_irq_name[i]);
		if (ret)
			break;
	}

	/* check whether the for loop has been broken*/
	if (i != g_ap_irq_num) {
		kfree(g_ap_irq_name);
		g_ap_irq_name = NULL;
	}

	pr_info("init lowpm table success.\n");
	return ret;
}

/* PM notify function which can get the PM notifier */
STATIC int sr_tick_pm_notify(struct notifier_block *nb,
		unsigned long mode, void *_unused)
{
	switch (mode) {
	case PM_SUSPEND_PREPARE:
		pmu_write_sr_tick(PMUOFFSET_SR_TICK, KERNEL_SUSPEND_PREPARE);
		break;
	case PM_POST_SUSPEND:
		pmu_write_sr_tick(PMUOFFSET_SR_TICK, KERNEL_RESUME_OUT);
		break;
	default:
		break;
	}

	return 0;
}

STATIC struct notifier_block sr_tick_pm_nb = {
	.notifier_call = sr_tick_pm_notify,
};

/* PM initialization funtion */
STATIC int hisi_pm_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device_node *np = NULL;

	(void)pdev;
	np = of_find_compatible_node(NULL, NULL, "hisilicon,lowpm_func");
	if (np == NULL)
		return -ENODEV;

	/* get low power table */
	ret = init_lowpm_table(np);
	if (ret) {
		of_node_put(np);
		return ret;
	}

	of_node_put(np);

	/* get chip id to judge the board type */
	np = of_find_compatible_node(NULL, NULL, "hisilicon,lp_chipid");
	if (np == NULL)
		return -ENODEV;

	g_chipid_addr = of_iomap(np, 0);
	if (g_chipid_addr == NULL) {
		of_node_put(np);
		return -EINVAL;
	}

	of_node_put(np);

	/* get sysctrl base addr */
	np = of_find_compatible_node(NULL, NULL, "hisilicon,lp_sysctrl");
	if (np == NULL)
		return -ENODEV;

	g_sysctrl_base = of_iomap(np, 0);
	if (g_sysctrl_base == NULL) {
		of_node_put(np);
		return -EINVAL;
	}

	of_node_put(np);

	/* get cpu cluster base addr, which can read cpu power status */
	np = of_find_compatible_node(NULL, NULL, "hisilicon,lp_cpu_cluster");
	if (np == NULL)
		return -ENODEV;

	g_cpu_cluster_base_addr = of_iomap(np, 0);
	if (g_cpu_cluster_base_addr == NULL) {
		of_node_put(np);
		return -EINVAL;
	}

	of_node_put(np);

	/* get gic base addr */
	if (hisi_get_gic_base())
		return -ENODEV;

	/* register pm_ops */
	suspend_set_ops(&hisi_pm_ops);

	/* register pm notifier */
	ret = register_pm_notifier(&sr_tick_pm_nb);
	if (ret)
		return ret;

	return wakeup_reason_init();
}

STATIC const struct of_device_id hisi_pm_of_match[] = {
	{.compatible = "hisilicon,hisi-pm"},
	{ }
};

MODULE_DEVICE_TABLE(of, hisi_pm_of_match);
STATIC struct platform_driver hisi_pm_driver = {
	.probe = hisi_pm_probe,
	.driver = {
		.name  = "hisi_pm",
		.of_match_table = hisi_pm_of_match,
	},
};

STATIC s32 __init hisi_pm_driver_init(void)
{
	s32 ret = platform_driver_register(&hisi_pm_driver);
	if (ret != 0) {
		pr_err("can't register hisi pm driver: %d\n", ret);
		return ret;
	}

	pr_info("hisi pm driver register succ.\n");
	return 0;
}

STATIC void __exit hisi_pm_driver_exit(void)
{
	suspend_set_ops(NULL);
	platform_driver_unregister(&hisi_pm_driver);
	pr_info("hisi pm driver remove succ.\n");
}

module_init(hisi_pm_driver_init);
module_exit(hisi_pm_driver_exit);
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("Hisilicon pm driver");
MODULE_LICENSE("GPL");
