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
*/
#ifndef LPM_FAULT_CRG_H
#define LPM_FAULT_CRG_H

/*
 * Function Name: lpm_fault_safety_crg_init
 * Decription: init crg moniter and safety irq.
 * Parameters: void
 * Return: 0 is success, otherwise failed
 */
int32_t lpm_fault_safety_crg_init(void);

/*
 * Function Name: lpm_fault_safety_crg_uninit
 * Decription: uninit crg moniter and safety irq.
 * Parameters: void
 * Return: 0 is success, otherwise failed
 */
void lpm_fault_safety_crg_uninit(void);

/*
 * Function Name: lpm_safety_irq_crg_handler
 * Decription: crg safety irq callback proc
 * Parameters: struct safety_fault_info *safety_fault
               uint32_t *event_num
               struct safety_event **event_list
 * Return: 0 is success, otherwise failed
 */
int32_t lpm_safety_irq_crg_handler(struct safety_fault_info *safety_fault,
	uint32_t *event_num, struct safety_event **event_list);

/*
 * Function Name: lpm_fault_safety_crg_sub_monitor_enable
 * Decription: get whether crg monitor enable
 * Parameters: uintptr_t base_addr
               uint32_t offset
 * Return: true is enable, false is disable
 */
bool lpm_fault_safety_crg_sub_monitor_enable(uintptr_t base_addr, uint32_t offset);

#endif /* LPM_FAULT_CRG_H */