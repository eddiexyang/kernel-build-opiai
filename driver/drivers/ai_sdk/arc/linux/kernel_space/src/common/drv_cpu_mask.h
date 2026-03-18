/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
* Create: 2022-12-25
*/
#ifndef DRV_CPU_MASK_H
#define DRV_CPU_MASK_H

#include <linux/cpumask.h>

/*
 * drv_get_ctrlcpu_mask_from_cpuset: get cpu mask from cpuset.cpus
 *
 * Read the system file /sys/fs/cgroup/cpuset/CtrlCPU/cpuset.cpus
 * to obtain the cpu set buffer of the ctrl CPU, parse cpuset and
 * convert it to cpumask.
 */
int drv_get_ctrlcpu_mask_from_cpuset(cpumask_t *ctrl_cpumask);
#endif
