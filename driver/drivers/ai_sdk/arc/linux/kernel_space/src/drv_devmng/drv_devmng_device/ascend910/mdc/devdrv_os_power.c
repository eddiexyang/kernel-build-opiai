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

#include <linux/securec.h>
#include <linux/securectype.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include "devdrv_common.h"
#include "devdrv_user_common.h"
#include "devdrv_manager_common.h"
#include "devdrv_parse_pdata.h"
#include "devdrv_os_power.h"

struct devdrv_sub_os_info *g_dev_sub_os_info = NULL;

int devdrv_get_power_addr_info(u32 idx, u64 *base, u32 *len)
{
    struct device_node *pw_base = NULL;
    struct resource res;
    int ret;

    if (base == NULL) {
        devdrv_drv_warn("base invalid, idx(%d).\n", idx);
        return -EINVAL;
    }

    if (len == NULL) {
        devdrv_drv_warn("len invalid, idx(%d).\n", idx);
        return -EINVAL;
    }

    if (idx >= (DEVDRV_SUB_OS_MAX * DEVDRV_SUB_OS_POWER_NUM)) {
        devdrv_drv_warn("idx(%d) invalid.\n", idx);
        return -EINVAL;
    }

    pw_base = of_find_compatible_node(NULL, NULL, "mailbox-multiple-os");
    if (pw_base == NULL) {
        devdrv_drv_warn("can't find mailbox-multiple-os device tree node.\n");
        return -ENODEV;
    }

    ret = of_address_to_resource(pw_base, idx, &res);
    if (ret != 0) {
        devdrv_drv_warn("failed to get mailbox-multiple-os idx(%d) registers,ret(%d).\n", idx, ret);
        of_node_put(pw_base);
        return -EINVAL;
    }

    *base = res.start;
    *len = resource_size(&res);

    of_node_put(pw_base);

    return 0;
}

void devdrv_parser_power_pdata(struct platform_device *pdev, struct devdrv_sub_os_info *sub_os)
{
    void __iomem *boot_vaddr = NULL;
    void __iomem *shut_vaddr = NULL;
    u64 boot_paddr = 0;
    u64 shut_paddr = 0;
    u32 boot_addr_len = 0;
    u32 shut_addr_len = 0;
    u32 os_idx;

    sub_os->os_num = 0;

    for (os_idx = 0; os_idx < DEVDRV_SUB_OS_MAX; os_idx++) {
        sub_os->pw_pdata[os_idx].valid = DEVDRV_SUB_OS_INVALID;
        if (os_idx != SUB_OS_SD) {
            continue;
        }

        if (devdrv_get_power_addr_info(DEVDRV_SUB_OS_POWER_NUM * os_idx, &boot_paddr, &boot_addr_len) != 0) {
            devdrv_drv_warn("parser unable to boot base, os_type(%d).\n", os_idx);
            continue;
        }

        if (devdrv_get_power_addr_info((DEVDRV_SUB_OS_POWER_NUM * os_idx + 1), &shut_paddr, &shut_addr_len) != 0) {
            devdrv_drv_warn("parser unable to shut base, os_type(%d).\n", os_idx);
            continue;
        }
        boot_vaddr = devm_ioremap(&pdev->dev, boot_paddr, boot_addr_len);
        if (boot_vaddr == NULL) {
            devdrv_drv_warn("ioremap unable to boot vaddr, os_type(%d)len(%d).\n", os_idx, boot_addr_len);
            continue;
        }
        shut_vaddr = devm_ioremap(&pdev->dev, shut_paddr, shut_addr_len);
        if (shut_vaddr == NULL) {
            devm_iounmap(&pdev->dev, boot_vaddr);
            devdrv_drv_warn("ioremap unable to shut vaddr, os_type(%d)len(%d).\n", os_idx, shut_addr_len);
            continue;
        }

        sub_os->pw_pdata[os_idx].boot_paddr = boot_paddr;
        sub_os->pw_pdata[os_idx].boot_vaddr = boot_vaddr;
        sub_os->pw_pdata[os_idx].boot_addr_size = boot_addr_len;
        sub_os->pw_pdata[os_idx].shut_paddr = shut_paddr;
        sub_os->pw_pdata[os_idx].shut_vaddr = shut_vaddr;
        sub_os->pw_pdata[os_idx].shut_addr_size = shut_addr_len;
        sub_os->pw_pdata[os_idx].os_id = os_idx;
        sub_os->pw_pdata[os_idx].valid = DEVDRV_SUB_OS_VALID;
        sub_os->os_num++;
        devdrv_drv_info("sub os parse success, os_type(%d)os_num(%d).\n", os_idx, sub_os->os_num);
    }
}

struct devdrv_sub_os_info *devdrv_get_sub_os_info(void)
{
    if (g_dev_sub_os_info == NULL) {
        return NULL;
    }

    return g_dev_sub_os_info;
}

int devdrv_sub_os_init(struct platform_device *pdev)
{
    struct devdrv_sub_os_info *sub_os_info = NULL;
    DRV_PRINT_START();

    if (g_dev_sub_os_info == NULL) {
        g_dev_sub_os_info = kzalloc(sizeof(struct devdrv_sub_os_info), GFP_KERNEL | __GFP_ACCOUNT);
        if (g_dev_sub_os_info == NULL) {
            devdrv_drv_warn("kmalloc for g_dev_sub_os_info failed.\n");
            return -ENOMEM;
        }
    }

    sub_os_info = g_dev_sub_os_info;
    devdrv_parser_power_pdata(pdev, sub_os_info);

    return 0;
}

void devdrv_sub_os_uninit(void)
{
    if (g_dev_sub_os_info != NULL) {
        kfree(g_dev_sub_os_info);
        g_dev_sub_os_info = NULL;
    }
}

