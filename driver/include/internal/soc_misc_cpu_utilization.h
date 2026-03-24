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
* Create: 2022-11-15
*/

#ifndef SOC_MISC_CPU_UTILIZATION_H
#define SOC_MISC_CPU_UTILIZATION_H

int soc_misc_cpu_utilization_init(void);
void soc_misc_cpu_utilization_exit(void);
int soc_misc_calc_aicpu_utilization(unsigned int dev_id, unsigned int *utilization);
#endif
