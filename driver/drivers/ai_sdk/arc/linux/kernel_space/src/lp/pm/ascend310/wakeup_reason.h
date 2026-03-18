/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2022. All rights reserved.
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

#ifndef _LINUX_WAKEUP_REASON_H
#define _LINUX_WAKEUP_REASON_H

#define MAX_SUSPEND_ABORT_LEN		256
#define MAX_WAKEUP_REASON_IRQS		32
#define IRQ_NAME_LEN		(128UL)
#define IRQ_NUM_LEN		10
#define WAKEUP_DEBUG_LEN	100

/* define return value successfully */
#define RTN_SUCCESS		0

void log_wakeup_reason(int irq);
char *get_irq_name(unsigned int irq_num);
int wakeup_reason_init(void);

#endif /* _LINUX_WAKEUP_REASON_H */
