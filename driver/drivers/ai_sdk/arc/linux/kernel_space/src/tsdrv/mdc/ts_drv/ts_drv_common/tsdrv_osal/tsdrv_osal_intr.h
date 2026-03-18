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

#ifndef TSDRV_INTR_H
#define TSDRV_INTR_H
#ifndef AOS_LLVM_BUILD
#include <linux/irq.h>
#else
#include <linux/types.h>
#include <linux/irqreturn.h>
#endif
int tsdrv_request_irq(u32 dev_id, int vector,
    irqreturn_t (*callback_func)(int, void *), void *para, const char *name);
int tsdrv_unrequest_irq(u32 dev_id, int vector, void *para);

#endif
