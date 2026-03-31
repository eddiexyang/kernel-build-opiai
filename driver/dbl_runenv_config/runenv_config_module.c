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
*
* Description:
* Author: huawei
* Create: 2022-6-23
*/

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/types.h>

#include "dbl/runenv_config.h"
#include "docker_query.h"
#include "vm_query.h"
#include "runenv_config_module.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("DAVINCI driver");

#define PCI_VENDOR_ID_HUAWEI            0x19e5
#define DEVDRV_DIVERSITY_PCIE_VENDOR_ID 0xFFFF
static const struct pci_device_id recfg_device_tbl[] = {
    { PCI_VDEVICE(HUAWEI, 0xd100), 0 },
    { PCI_VDEVICE(HUAWEI, 0xd105), 0 },
    { PCI_VDEVICE(HUAWEI, PCI_DEVICE_CLOUD), 0 },
    { PCI_VDEVICE(HUAWEI, 0xd801), 0 },
    { PCI_VDEVICE(HUAWEI, 0xd500), 0 },
    { PCI_VDEVICE(HUAWEI, 0xd501), 0 },
    { PCI_VDEVICE(HUAWEI, 0xd802), 0 },
    { PCI_VDEVICE(HUAWEI, 0xd803), 0 },
    { PCI_VDEVICE(HUAWEI, 0xd804), 0 },
    { DEVDRV_DIVERSITY_PCIE_VENDOR_ID, 0xd500, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
    {}
};
MODULE_DEVICE_TABLE(pci, recfg_device_tbl);

/* The name cannot be changed because it is exposed to the user. */
static u32 deployment_mode = DBL_DEVICE_DEPLOYMENT;
module_param(deployment_mode, uint, S_IRUSR);

u32 dbl_get_deployment_mode(void)
{
    return deployment_mode;
}
EXPORT_SYMBOL(dbl_get_deployment_mode);

int dbl_get_run_env(void)
{
    int mach_flag = DBL_IN_PHYSICAL_MACH;

    if (run_in_normal_docker() == true) {
        mach_flag = DBL_IN_NORMAL_DOCKER;
    } else if (run_in_admin_docker() == true) {
        mach_flag = DBL_IN_ADMIN_DOCKER;
    } else if (run_in_virtual_mach() == true) {
        mach_flag = DBL_IN_VIRTUAL_MACH;
    }

    return mach_flag;
}
EXPORT_SYMBOL(dbl_get_run_env);

STATIC int __init recfg_init(void)
{
#ifdef CFG_FEATURE_HOST_ENV
    deployment_mode = DBL_HOST_DEPLOYMENT;
#endif
    recfg_info("Module init success.\n");
    return 0;
}

STATIC void __exit recfg_exit(void)
{
    recfg_info("Module exit success.\n");
}

module_init(recfg_init);
module_exit(recfg_exit);
