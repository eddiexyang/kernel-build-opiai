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
*
* Description:
* Author: huawei
* Create: 2019-10-15
*/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/sysfs.h>
#include <linux/securec.h>
#include <linux/io.h>
#include <linux/of_gpio.h>
#include <linux/string.h>
#include <asm/processor.h>
#include <linux/atomic.h>
#include <linux/version.h>
#include <linux/platform_device.h>
#include <linux/kallsyms.h>
#include <linux/acpi.h>

#include "ascend_ctl_dev.h"
#include "ascend_ctl_sysfs.h"

STATIC int ascend_ctl_probe(struct platform_device *pdev)
{
    int ret;

    ascend_ctl_set_platform_device(pdev);

    ret = ascend_ctl_sysfs_init();
    if (ret != 0) {
        ASCEND_CTL_ERR("Sysfs init failed. (ret=%d)\n", ret);
        return ret;
    }

    ascend_ctl_get_dts_cpu_cfg(pdev);

    return 0;
}

STATIC void ascend_ctl_remove(struct platform_device *pdev)
{
    ASCEND_CTL_INFO("drv ascend ctl remove.\n");
}

STATIC const struct of_device_id g_ascend_ctl_of_match[] = {
    { .compatible = "hisi-ascend-ctl" },
    {}
};
MODULE_DEVICE_TABLE(of, g_ascend_ctl_of_match);

STATIC const struct acpi_device_id g_ascend_ctl_of_acpi_match[] = {
    {"ACTL0000", 0},
    {}
};
MODULE_DEVICE_TABLE(acpi, g_ascend_ctl_of_acpi_match);

STATIC struct platform_driver g_ascend_ctl_platform_driver = {
    .probe = ascend_ctl_probe,
    .remove = ascend_ctl_remove,
    .driver = {
        .name = "hisi-ascend-ctl",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(g_ascend_ctl_of_match),
        .acpi_match_table = ACPI_PTR(g_ascend_ctl_of_acpi_match),
    },
};

STATIC int __init ascend_ctl_init(void)
{
    int ret;

    ret = platform_driver_register(&g_ascend_ctl_platform_driver);
    if (ret != 0) {
        ASCEND_CTL_ERR("Failed to invoke platform_driver_register. (ret=%d)\n", ret);
        return ret;
    }

    ret = ascend_ctl_sysfs_init();
    if (ret != 0) {
        ASCEND_CTL_ERR("Failed to invoke ascend_ctl_sysfs_init. (ret=%d)\n", ret);
        goto platform_register_fail;
    }

    ascend_ctl_check_dts_cpu_cfg_valid();
    return ret;

platform_register_fail:
    platform_driver_unregister(&g_ascend_ctl_platform_driver);
    return ret;
}

STATIC void __exit ascend_ctl_exit(void)
{
    ascend_ctl_sysfs_exit();
    platform_driver_unregister(&g_ascend_ctl_platform_driver);
}

module_init(ascend_ctl_init);
module_exit(ascend_ctl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("drv ascend ctl device driver");
