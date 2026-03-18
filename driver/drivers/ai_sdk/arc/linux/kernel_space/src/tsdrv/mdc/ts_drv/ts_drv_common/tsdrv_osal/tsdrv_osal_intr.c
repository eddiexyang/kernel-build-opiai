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

#include <linux/delay.h>
#ifndef AOS_LLVM_BUILD
#include <linux/irq.h>
#endif
#include <linux/module.h>

#include "devdrv_common.h"
#include "tsdrv_log.h"
#include "devdrv_interface.h"
#include "devdrv_devinit.h"
#include "tsdrv_osal_intr.h"

int tsdrv_request_irq(u32 dev_id, int vector,
    irqreturn_t (*callback_func)(int, void *), void *para, const char *name)
{
    int ret = 0;

#ifdef CFG_MANAGER_HOST_ENV
    ret = devdrv_register_irq_func_expand(dev_id, vector, callback_func, para, name);
#else
    if (tsdrv_get_pfvf_type_by_devid(dev_id) == DEVDRV_SRIOV_TYPE_PF) {
        ret = request_irq(vector, callback_func, 0, name, para);
    }
#ifdef CFG_SOC_PLATFORM_HELPER
    (void)irq_set_affinity_hint((u32)vector, get_cpu_mask(0));
#endif
#endif
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Register irq failed. (devid=%u; ret=%d)\n", dev_id, ret);
#endif
        return -EINVAL;
    }

    return 0;
}

int tsdrv_unrequest_irq(u32 dev_id, int vector, void *para)
{
#ifdef CFG_MANAGER_HOST_ENV
    int ret = devdrv_unregister_irq_func_expand(dev_id, vector, para);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Unregister irq failed. (devid=%u; ret=%d)\n", dev_id, ret);
#endif
        return -EINVAL;
    }
#else
    if (tsdrv_get_pfvf_type_by_devid(dev_id) == DEVDRV_SRIOV_TYPE_PF) {
        (void)free_irq(vector, para);
    }
#endif

    return 0;
}
