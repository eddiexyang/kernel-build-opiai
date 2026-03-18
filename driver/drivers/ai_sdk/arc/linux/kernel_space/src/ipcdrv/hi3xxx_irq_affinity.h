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

#ifndef HI3XXX_IRQ_AFFINITY_H
#define HI3XXX_IIRQ_AFFINITY_H

#include <linux/errno.h>

#ifdef CONFIG_HI3XXX_IRQ_AFFINITY
extern void hisi_irqaffinity_status(void);
extern int hisi_irqaffinity_register(unsigned int irq, int cpu);
extern void hisi_irqaffinity_unregister(unsigned int irq);
#else
STATIC inline int hisi_irqaffinity_register(unsigned int irq, int cpu)
{
    return -ENOSYS;
}
STATIC inline void hisi_irqaffinity_unregister(unsigned int irq)
{
    return;
}
STATIC inline void hisi_irqaffinity_status(void)
{
    return;
}
#endif /* CONFIG_HI3XXX_IRQ_AFFINITY */
#endif /* HI3XXX_IRQ_AFFINITY_H */
