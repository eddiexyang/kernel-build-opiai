/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * Create: 2023-01-07
 */

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/bitmap.h>

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
#include "devdrv_interface.h"
#else
#include "user_cfg_interface.h"
#endif
#include "virtmngdev_res_common.h"
#include "virtmngdev_common_resource.h"

#define CPU_NUM_PER_CLUSTER 4
#define BASE_CLUSTER_CPU_MASK (unsigned long)((unsigned int)(~0) >> (32 - CPU_NUM_PER_CLUSTER))

STATIC unsigned long get_avaliable_aicpu_bitmap(u32 alloc_aicpu_num, unsigned long remain_aicpu_bitmap, u32 cpu_num)
{
    unsigned int cluster_num = cpu_num / CPU_NUM_PER_CLUSTER;
    unsigned int mini_cluster_aicpu_num = U32_MAX;
    unsigned int cpu_cluster_index = U32_MAX;
    unsigned int cluster_aicpu_num;
    unsigned long cluster_aicpu_bitmap;
    unsigned long cluster_cpu_mask;
    unsigned long aicpu_bitmap;
    unsigned int i;

    for (i = 0; i < cluster_num; ++i) {
        cluster_cpu_mask = BASE_CLUSTER_CPU_MASK << (i * CPU_NUM_PER_CLUSTER);
        cluster_aicpu_bitmap = remain_aicpu_bitmap & cluster_cpu_mask;
        cluster_aicpu_num = bitmap_weight(&cluster_aicpu_bitmap, cpu_num);
        if ((alloc_aicpu_num <= cluster_aicpu_num) && (cluster_aicpu_num < mini_cluster_aicpu_num)) {
            cpu_cluster_index = i;
            mini_cluster_aicpu_num = cluster_aicpu_num;
        }
    }

    if (cpu_cluster_index >= cluster_num) {
        aicpu_bitmap = remain_aicpu_bitmap;
    } else {
        cluster_cpu_mask = BASE_CLUSTER_CPU_MASK << (cpu_cluster_index * CPU_NUM_PER_CLUSTER);
        aicpu_bitmap = remain_aicpu_bitmap & cluster_cpu_mask;
    }

    return aicpu_bitmap;
}

int vmngd_alloc_aicpu_for_vf(vmngd_element_t *vf_aicpu, vmngd_element_t *remain, const u32 alloc_aicpu_num)
{
    vmngd_element_t avaliable_aicpu = {0};
    unsigned long aicpu_bitmap;
    int ret;

    if (alloc_aicpu_num > (u32)vmngd_get_element_bitnum(remain)) {
        vmng_err("aicpu is not enough. (remain=%lx, alloc_aicpu_num=%u)\n", remain->bitmap,
            alloc_aicpu_num);
        return VMNG_ERR;
    }

    vmngd_element_copy(&avaliable_aicpu, remain);
    aicpu_bitmap = get_avaliable_aicpu_bitmap(alloc_aicpu_num, remain->bitmap, remain->bitnum);
    vmngd_element_set(&avaliable_aicpu, aicpu_bitmap);
    ret = vmngd_element_alloc(vf_aicpu, &avaliable_aicpu, alloc_aicpu_num);
    if (ret != 0) {
        vmng_err("Alloc aicpu failed. (remain=%lx;avaliable=%lx;alloc_aicpu_num=%u;ret=%d)\n",
                 remain->bitmap, avaliable_aicpu.bitmap, alloc_aicpu_num, ret);
        return VMNG_ERR;
    }
    return VMNG_OK;
}

void vmngd_release_aicpu(vmngd_element_t *vf_aicpu, vmngd_element_t *remain)
{
    vmngd_element_clear(vf_aicpu);
}

void vmngd_resource_get_aicpu_info(u32 dev_id, vmngd_element_t *aicpu)
{
    u32 aicpu_start_id;
    u32 i;
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    struct agentdrv_cpu_info cpu_info = {0};
    if (agentdrv_get_cpu_info(dev_id, &cpu_info) != 0) {
        vmng_err("Get cpu info failed, use default value.(dev_id=%u)\n", dev_id);
        return;
    }
#else
    dev_cpu_nums_cfg_t cpu_info = {0};
    if (dev_user_cfg_get_cpu_number(dev_id, &cpu_info) != 0) {  // CLOUD_V2 is not support
        vmng_err("Get cpu info failed, use default value.(dev_id=%u)\n", dev_id);
        return;
    }
#endif
    aicpu->num = cpu_info.aicpu_num;
    bitmap_zero(&aicpu->bitmap, aicpu->bitnum);
    aicpu_start_id = cpu_info.ccpu_num + cpu_info.dcpu_num;
    for (i = aicpu_start_id; i < cpu_info.aicpu_num + aicpu_start_id; ++i) {
        bitmap_set(&aicpu->bitmap, i, 1);
    }
    aicpu->bitnum = cpu_info.aicpu_num + aicpu_start_id;
    aicpu->num_per_bit = 1;

    vmng_info("Get aicpu info, (aicpu_num=%u;aicpu_bitmap=%lx)\n", aicpu->num, aicpu->bitmap);
    return;
}