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
* Create: 2022-03-28
*/

#ifndef __VIRTMNGDEV_MEMORY_H__
#define __VIRTMNGDEV_MEMORY_H__

#include <linux/mutex.h>

#include "virtmng_public_def.h"
#include "virtmngdev_element.h"

/* 0~31 for os
 * 32~63 for die1
 * 64~127 for die2
*/
#define DEVMM_MAX_NUMA_NUM_OF_PER_DEV 32
#define VMNGD_MEMORY_MAX_NUMA_NUM 32
#define MAX_NUMA_ID_OS 32
#define MAX_NUMA_ID_PER_DIE 64
#define VMNGD_NUMA_NODE_SIZE 0x80000000 /* echo numa node 2GB */
#define VMNGD_AVALIABLE_NODE_SIZE 0xC0000000 /* avaliable numa node should larger than 3GB */

int vmngd_resource_memory_init(u32 dev_id, u32 vfid, int numa_id[], u32 numa_num,
                               vmngd_element_t *memory);
int vmngd_resource_memory_uninit(u32 dev_id, u32 vfid);
int vmngd_resource_numa_id_init(u32 dev_id, int *array, u32 maxNum, u32 *numa_num);

#endif
