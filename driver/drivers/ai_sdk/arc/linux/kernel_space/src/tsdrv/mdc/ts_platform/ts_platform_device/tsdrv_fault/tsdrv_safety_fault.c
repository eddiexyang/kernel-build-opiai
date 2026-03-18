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

#include <linux/sizes.h>
#include <linux/slab.h>

#include "tsdrv_l2buff_safety.h"
#include "tsdrv_subsys_safety.h"
#include "tsdrv_kernel_common.h"
#include "tsdrv_common.h"
#include "tsdrv_log.h"
#include "tsdrv_safety_fault.h"
#include "tsdrv_ipc_fault_init.h"
#ifndef  TSDRV_UT
#include "ts_dms_init.h"
#include "aic_dms_init.h"
#include "aiv_dms_init.h"
#endif

int tsdrv_chk_safety_param(const struct safety_fault_info *safety_fault,
    unsigned int *event_num)
{
    if (safety_fault == NULL) {
        TSDRV_PRINT_ERR("safety fault is null\n");
        return -EINVAL;
    } else if (event_num == NULL) {
        TSDRV_PRINT_ERR("event_num is null\n");
        return -EINVAL;
    }

    if (safety_fault->dev_id >= TSDRV_MAX_DAVINCI_NUM) {
        TSDRV_PRINT_ERR("device id invalid\n");
        return -EINVAL;
    }
    if (safety_fault->fault_status_list == NULL) {
        TSDRV_PRINT_ERR("fault statis list is null\n");
        return -EINVAL;
    }

    return 0;
}

const struct ras_fault_converge_item *tsdrv_safety_converge(
    const struct ras_fault_converge_item *items, u32 items_num,
    u32 section_type, u64 ras_code)
{
    u32 i;

    for (i = 0; i < items_num; i++) {
        if (section_type == items[i].section_type) {
            if ((u32)ras_code == items[i].ras_code.err_status) {
                return &items[i];
            }
        }
    }
    return NULL;
}

int tsdrv_register_safety_irq(u32 devid)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    tsdrv_register_ipc_fault_event(devid);
#endif
    tsdrv_register_l2buff_safety_irq(devid);
    if (tsdrv_get_chip_type() == CHIP_TYPE_MDC_BS9SX1A) {
        tsdrv_register_subsys_safety_irq(devid);
        tsdrv_register_aic_safety_irq(devid);
        tsdrv_register_aiv_safety_irq(devid);
#ifndef  TSDRV_UT
        (void)ts_safety_manager_init(devid);
        (void)aic_safety_manager_init(devid);
        (void)aiv_safety_manager_init(devid);
#endif
    }
    return 0;
}

void tsdrv_unregister_safety_irq(u32 devid)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    tsdrv_unregister_ipc_fault_event(devid);
#endif
    tsdrv_unregister_l2buff_safety_irq(devid);
    if (tsdrv_get_chip_type() == CHIP_TYPE_MDC_BS9SX1A) {
        tsdrv_unregister_subsys_safety_irq(devid);
        tsdrv_unregister_aic_safety_irq(devid);
        tsdrv_unregister_aiv_safety_irq(devid);
#ifndef  TSDRV_UT
        aiv_safety_manager_uninit(devid);
        aic_safety_manager_uninit(devid);
        ts_safety_manager_uninit(devid);
#endif
    }
}
