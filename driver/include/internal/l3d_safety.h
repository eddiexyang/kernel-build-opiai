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

#ifndef L3D_SAFETY_H
#define L3D_SAFETY_H

#include <linux/workqueue.h>

#include "soc_misc_common.h"
#include "drvfault_user_common.h"
#include "dms_define.h"
#include "fpdc.h"

/*
 * l3d_safety_init - Initialize the safety fault type of the l3d module.
 *
 * This interface is used to register the interrupt of safety fault to faultmng module.
 */
int l3d_safety_init(void);

/*
 * l3d_safety_uninit - Uninitialize the safety fault type of the l3d module.
 *
 * This interface is used to unregister the interrupt of safety fault to faultmng module.
 */
void l3d_safety_uninit(void);

/*
 * l3d_safety_handler - Reports safety fault events of l3d based on the safety status.
 * @safety_fault:fault information
 * @event_num: numbers of event
 * @event_list: fault event result, can't be null, and size must be larger
 *      than @module_info->max_event
 *
 * This function is used to receive the value of an safety fault status sent by
 * the fault management module, parse the fault status bit, converge the fault
 * according to the bit, and report the fault to the device specified
 * in @safety_fault.
 * During event reporting, the event queue is automatically selected based
 * on the sensor type configured in the converge table.
 */
int l3d_safety_handler(struct safety_fault_info *safety_fault,
    unsigned int *event_num, struct safety_event **event_list);

#endif // L3D_SAFETY_H
