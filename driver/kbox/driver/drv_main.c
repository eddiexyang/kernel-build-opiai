/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox drv main
 * Author:
 * Create: 2019-02-14
 */
#include "drv_main.h"

#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/acpi.h>
#include <linux/platform_device.h>

#include "drv_cdev.h"
#include "drv_core.h"
#include "drv_oper.h"
#include "drv_proc.h"
#include "drv_print.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("EulerOS Kbox Driver");
MODULE_AUTHOR("HUAWEI TECHNOLOGIES CO., LTD.");
MODULE_VERSION("V2.0");

unsigned long g_kbox_va_start;

/* kbox param type1: phy_addr + size */
unsigned long g_kbox_phy_start;
unsigned long g_kbox_size;
/* kbox param type2: mem_sym + size_sym */
char *g_kbox_mem_start_sym;
char *g_kbox_mem_size_sym;
/* kbox mem type */
unsigned long g_kbox_type;
EXPORT_SYMBOL_GPL(g_kbox_type);

module_param(g_kbox_phy_start, ulong, 0400);
module_param(g_kbox_size, ulong, 0400);
module_param(g_kbox_type, ulong, 0400);
module_param(g_kbox_mem_start_sym, charp, 0400);
module_param(g_kbox_mem_size_sym, charp, 0400);

struct semaphore g_kbox_sem;

static int check_module_param(void)
{
	if (g_kbox_type == TYPE_PHY_MEM) {
		return check_mem_param();
	} else if (g_kbox_type == TYPE_PHY_SYM) {
		return check_sym_param();
	} else if (g_kbox_type == TYPE_PHY_SOTRAGE) {
		return 0;
	} else {
		return -EINVAL;
	}
}

static void __kbox_set_reserved_phymem(unsigned long phy_start, unsigned long size)
{
	g_kbox_phy_start = phy_start;
	g_kbox_size = size;
	g_kbox_type = TYPE_PHY_MEM;
}

static int kbox_get_dt(struct platform_device *pdev, u32 *phy_start, u32 *area_size)
{
	struct resource *res = NULL;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		drv_info("failed to acquire os_kbox node addr\n");
		return -EINVAL;
	}
	*phy_start = res->start;
	*area_size = resource_size(res);

	return 0;
}

static int kbox_get_acpi(struct device *dev, u32 *phy_start, u32 *area_size)
{
	int ret;

	ret = device_property_read_u32(dev, "oskbox_addr", phy_start);
	if (ret < 0) {
		drv_err("failed to acquire oskbox_addr: %d\n", ret);
		return -EINVAL;
	}

	ret = device_property_read_u32(dev, "oskbox_size", area_size);
	if (ret < 0) {
		drv_err("failed to acquire oskbox_size: %d\n", ret);
		return -EINVAL;
	}

	return 0;
}

static int __parse_kbox_param(struct platform_device *pdev)
{
	int ret = 0;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	u32 phy_start;
	u32 area_size = 0;

	if (acpi_disabled && np == NULL) {
		drv_info("failed to find os_kbox memory node\n");
		return -ENODEV;
	}

	if (np) {
		ret = kbox_get_dt(pdev, &phy_start, &area_size);
	} else {
		ret = kbox_get_acpi(dev, &phy_start, &area_size);
	}

	if (ret == 0) {
		__kbox_set_reserved_phymem(phy_start, area_size);
	}

	return ret;
}

static int kbox_device_probe(struct platform_device *pdev)
{
	int ret;

	ret = __parse_kbox_param(pdev);
	if (ret != 0) {
		return ret;
	}

	ret = check_module_param();
	if (ret != 0) {
		return ret;
	}
	drv_info("check kboxdriver param success.");
	drv_info("-------- init start --------");
	drv_info("num of online cpus: %u", num_online_cpus());

	sema_init(&g_kbox_sem, 1);
	spin_lock_init(&g_drv_write_lock);

	ret = kbox_addr_init();
	if (ret != 0) {
		return ret;
	}
	drv_info("kbox start: 0x%pK, kbox size: 0x%lx", (char *)g_kbox_va_start, g_kbox_size);
	drv_info("addr init success.");
	ret = kbox_manage_area_init();
	if (ret != 0) {
		drv_err("manage area init fail.");
		goto fail1;
	}
	drv_info("manage area init success.");
	ret = kbox_dev_init();
	if (ret != 0) {
		drv_err("dev init fail.");
		goto fail1;
	}
	drv_info("dev init success.");
	ret = kbox_proc_init();
	if (ret != 0) {
		drv_err("proc init fail.");
		goto fail2;
	}
	drv_info("-------- load OK --------");
	return 0;

fail2:
	kbox_dev_fini();
fail1:
	kbox_addr_fini();
	drv_err("load failed");
	return ret;
}

static void kbox_device_remove(struct platform_device *pdev)
{
	down(&g_kbox_sem);
	kbox_proc_fini();
	drv_err("unregister proc finish");
	kbox_dev_fini();
	drv_err("unregister dev finish");
	kbox_addr_fini();
	drv_err("unmap manage area finish");
	up(&g_kbox_sem);
	drv_info("remove kboxdriver module finish");
}

static const struct acpi_device_id kbox_acpi_match[] = {
	{ "KBOX0000", 0},
	{ }
};
MODULE_DEVICE_TABLE(acpi, kbox_acpi_match);

static const struct of_device_id kbox_of_match[] = {
	{ .compatible = "os_kbox" },
	{ }
};
MODULE_DEVICE_TABLE(of, kbox_of_match);

static struct platform_driver kbox_platform_driver = {
	.probe = kbox_device_probe,
	.remove = kbox_device_remove,
	.driver = {
		.name = "os_kbox",
		.owner = THIS_MODULE,
		.of_match_table = kbox_of_match,
		.acpi_match_table = ACPI_PTR(kbox_acpi_match),
	},
};

static int __init kbox_driver_init(void)
{
	int ret;

	ret = kallsyms_lookup_name_kprobe_init();
	if (ret != 0) {
		return ret;
	}

	ret = platform_driver_register(&kbox_platform_driver);
	if (ret != 0) {
		g_kbox_type = 0;
	}

	return 0;
}

static void __exit kbox_driver_exit(void)
{
	platform_driver_unregister(&kbox_platform_driver);
}

module_init(kbox_driver_init);
module_exit(kbox_driver_exit);
