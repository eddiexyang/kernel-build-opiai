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

#ifndef TSDRV_QUEUEWORK_AFFINITY_H
#define TSDRV_QUEUEWORK_AFFINITY_H

#include <linux/types.h>
#include <linux/workqueue.h>

#ifdef CFG_SOC_PLATFORM_MDC_V51

/**
 * set affinity of unbound type workqueue
 *
 * A workqueue of the Per cpu type whill creates one thread on each CPU.
 * Some threads may run AICORE. The workqueue type needs to be changed to unbound
 * and the affinity of the work thread needs to be set by calling this API,
 * to prevent the thread from running on the AICORE.
 */
int tsdrv_set_workqueue_affinity(struct workqueue_struct *wq, u32 flag);

#endif

#endif /* TSDRV_QUEUEWORK_AFFINITY_H */
