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
* Create: 2022-08-08
*/
#include <linux/dmi.h>
#include <linux/cred.h>
#include <linux/types.h>

#include "runenv_config_module.h"
#include "vm_query.h"

#define DMI_VIRTUAL_SYSVENDOR "QEMU"
#define DMI_VIRTUAL_PRODUCT_NAME "KVM Virtual Machine"

bool run_in_virtual_mach(void)
{
    if (dmi_match(DMI_SYS_VENDOR, DMI_VIRTUAL_SYSVENDOR) &&
        dmi_match(DMI_PRODUCT_NAME, DMI_VIRTUAL_PRODUCT_NAME)) {
        recfg_debug("In virtual_mach.\n");
        return true;
    }
    return false;
}

