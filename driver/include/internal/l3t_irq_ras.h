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
* Create: 2022-9-20
*/

#ifndef L3T_IRQ_RAS_H
#define L3T_IRQ_RAS_H

#include "drvfault_user_common.h"
#include "soc_misc_safety_conf.h"

/*
 * l3t_irq_ras_init - Initialize the "irq_ras" fault type of the l3t module.
 *
 * This interface is used to register the interrupt of ras fault to faultmng module.
 */
int l3t_irq_ras_init(void);

/*
 * l3t_irq_ras_uninit - Uninitialize the "irq_ras" fault type of the l3t module.
 *
 * This interface is used to unregister the interrupt of ras fault to faultmng module.
 */
void l3t_irq_ras_uninit(void);

 /*
 * l3t_irq_ras_handler - Reports ras fault events of l3t based on the safety status.
 * @safety_fault:fault information
 * @event_num: numbers of event
 * @event_list: fault event result, can't be null, and size must be larger
 *      than @module_info->max_event
 *
 * This function is used to receive the value of an safety fault status sent by
 * the fault management module, parse the fault status ierr, converge the fault
 * according to the ierr, and report the fault to the device specified
 * in @safety_fault.
 * During event reporting, the event queue is automatically selected based
 * on the sensor type configured in the converge table.
 */
int l3t_irq_ras_handler(struct safety_fault_info *safety_fault,
    unsigned int *event_num, struct safety_event **event_list);

#endif // L3T_IRQ_RAS_H
