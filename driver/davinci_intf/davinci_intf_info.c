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
* Create: 2022-08-16
*/
#ifndef AOSCORE_TEST_UT
#include <linux/types.h>
#include <linux/kallsyms.h>
#include <linux/pci.h>
#include <linux/kthread.h>
#include <linux/aos/cpu_domain_info.h>

#include "securec.h"
#include "devdrv_interface.h"
#include "davinci_api.h"
#include "davinci_intf_common.h"
#include "davinci_intf_info.h"

int davinci_intf_get_cpu_info(u32 dev_id, struct agentdrv_cpu_info *cpu_info)
{
    int ret;
    struct cpu_domain_info cpu_domain = {};

    if (cpu_info == NULL) {
        log_intf_err("Device ID is invalid or cpu_info is NULL.(dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    ret = get_cpudomain_info(&cpu_domain);
    if (ret != 0) {
        log_intf_err("get cpudomain info failed. (ret=%d)\n", ret);
        return ret;
    }
    cpu_info->ccpu_num = cpu_domain.ctrlcpu_num;
    cpu_info->dcpu_num = cpu_domain.datacpu_num;
    cpu_info->aicpu_num = cpu_domain.aicpu_num;

    return 0;
}
#else
void davinci_intf_info_ut_stub(void)
{
    return;
}
#endif
