/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#ifndef _CAN_DRV_INTERRUPT_H__
#define _CAN_DRV_INTERRUPT_H__

#include <linux/jiffies.h>
#include <linux/types.h>
#include <linux/irqreturn.h>

#define CAN_ERR_PASSIVE_CNT    127

struct net_device;
struct napi_struct;

void mttcan_clear_all_interrupt_flags(const struct net_device *ndev);
int mttcan_enable_interrupts(const struct net_device *ndev, u32 intr_sel);
void mttcan_disable_interrupts(const struct net_device *ndev, u32 intr_sel);

irqreturn_t mttcan_isr0(int irq0, void *dev);
irqreturn_t mttcan_isr1(int irq1, void *dev);
void mttcan_rx_push_skb(unsigned long priv_ptr);
void mttcan_tx_push_skb(unsigned long priv_ptr);


#endif /* _CAN_DRV_INTERRUPT_H__ */

