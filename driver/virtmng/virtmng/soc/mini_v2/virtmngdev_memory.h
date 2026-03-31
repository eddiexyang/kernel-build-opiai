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
* Create: 2022-08-10
*/

#ifndef __VIRTMNGDEV_MEMORY_H__
#define __VIRTMNGDEV_MEMORY_H__

#include <linux/mutex.h>

#include "virtmng_public_def.h"
#include "virtmngdev_resource.h"

int vmngd_alloc_memory_for_vf(u32 dev_id, vmngd_resource_unit_t *vf, u64 alloc_memory_size);
void vmngd_release_memory_resource(u32 dev_id, vmngd_resource_unit_t *vf);
void vmngd_get_dev_memory_info(u32 dev_id, vmngd_resource_info_t *info);

#endif
