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

#ifndef SOC_MISC_CPU_INFO_H
#define SOC_MISC_CPU_INFO_H

#include "soc_misc_info.h"

int soc_misc_init_cpu_info(struct soc_misc_info_st *soc_info); /* declare for ut */
int soc_misc_init_cpu_info_from_devinfo(struct soc_misc_info_st *soc_info);
int soc_misc_init_cpu_info_from_pcie(struct soc_misc_info_st *soc_info);

#ifdef CONFIG_CPU_INFO_FROM_DEVINFO
#define soc_misc_init_cpu_info soc_misc_init_cpu_info_from_devinfo
#endif
#ifdef CONFIG_CPU_INFO_FROM_PCIE
#define soc_misc_init_cpu_info soc_misc_init_cpu_info_from_pcie
#endif
#if !defined(soc_misc_init_cpu_info) && !defined(STATIC_SKIP)
#error "not defined feature macro"
#endif

#endif
