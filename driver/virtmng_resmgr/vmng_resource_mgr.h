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
* Create: 2022-09-30
*/

#ifndef VMNG_RESOURCE_MGR_H
#define VMNG_RESOURCE_MGR_H
#include <linux/types.h>
#ifndef EMU_ST
#include "drv_log.h"
#else
#include "ut_log.h"
#endif

#define module_vmng_resource "vmng_resource"
#define vmng_res_err(fmt, ...) drv_err(module_vmng_resource, \
    "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define vmng_res_info(fmt, ...) drv_info(module_vmng_resource, \
    "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)

#ifdef EMU_ST
int vmngd_get_cgroup_meminfo(u64 *size);
#endif

#endif
