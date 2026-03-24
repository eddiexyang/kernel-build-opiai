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
#include <linux/fs.h>
#include <linux/kallsyms.h>

#include "dms_cmd_def.h"
#include "urd_acc_ctrl.h"
#include "dms_mem_info.h"
#include "dms_mem_common.h"
#include "kernel_version_adapt.h"
#include "devdrv_common.h"

int mem_get_hbm_node_info(u32 dev_id, u32 type, struct mem_info *info)
{
    u32 start_index;
    u32 hbm_node_num;
    u64 total_size_tmp = 0;
    u64 free_size_tmp = 0;
    int ret = 0;
    u32 dev_num = 0;
    u32 i;

    ret = devdrv_get_devnum(&dev_num);
    if (ret != 0) {
        dms_err("Get devnum failed. (devid=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    if (type == MEM_SERVICE) {
        hbm_node_num = MEM_SERVICE_HBM_NUMA_NUM;
        start_index = MEM_GET_SERVICE_HBM_NUMA_ID(dev_id, 0, dev_num);
    } else if (type == MEM_SYSTEM) {
        hbm_node_num = MEM_SYSTEM_HBM_NUMA_NUM;
        start_index = dev_id * MEM_SYSTEM_HBM_NUMA_NUM;
    } else {
        dms_err("Unsupported hbm node type. (dev_id=%u; type=%u)\n", dev_id, type);
        return -EOPNOTSUPP;
    }

    for (i = start_index; i < (start_index + hbm_node_num); i++) {
        if (node_online(i) == 0) {
            continue;
        }
        ret = mem_get_info_single(i, &total_size_tmp, &free_size_tmp);
        if (ret != 0) {
            dms_err("Get mem info failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
            return -EINVAL;
        }
        info->total_size += total_size_tmp;
        info->free_size += free_size_tmp;
    }

    info->use_size = info->total_size - info->free_size;
    if (info->total_size != 0) {
        info->mem_util = info->use_size * DIGITAL_NUM_TO_PER / info->total_size;
    } else {
        info->mem_util = 0;
    }

    return ret;
}

devmng_get_vdev_hbm_info g_devmng_get_vdev_hbm_info = NULL;
STATIC int mem_get_vdev_hbm_info_from_devmm(
    u32 devid, u32 vfid, u32 huge_flag, u64 *free_size, u64 *total_size)
{
    if (g_devmng_get_vdev_hbm_info == NULL) {
        g_devmng_get_vdev_hbm_info = (devmng_get_vdev_hbm_info)(
            uintptr_t)__kallsyms_lookup_name("devmm_get_vdev_hbm_info");
        if (g_devmng_get_vdev_hbm_info == NULL) {
            return -EFAULT;
        }
    }
    return g_devmng_get_vdev_hbm_info(devid, vfid, huge_flag, free_size, total_size);
}

int mem_get_vdev_hbm_info(u32 dev_id, u32 vfid, struct mem_info *info)
{
    u64 total_size_tmp = 0;
    u64 free_size_tmp = 0;
    int ret;

    /* read small page size */
    ret = mem_get_vdev_hbm_info_from_devmm(dev_id, vfid, 0, &free_size_tmp, &total_size_tmp);
    if (ret != 0) {
        dms_err("Read small page from devmm failed. (dev_id=%u; vfid=%u; ret=%d)\n", dev_id, vfid, ret);
        return ret;
    }
    info->total_size += total_size_tmp;
    info->free_size += free_size_tmp;

    /* read huge page size */
    ret = mem_get_vdev_hbm_info_from_devmm(dev_id, vfid, 1, &free_size_tmp, &total_size_tmp);
    if (ret != 0) {
        dms_err("Read huge page from devmm failed. (dev_id=%u; vfid=%u; ret=%d)\n", dev_id, vfid, ret);
        return ret;
    }
    info->total_size += total_size_tmp;
    info->free_size += free_size_tmp;

    info->total_size = BYTES_TO_KBYTE(info->total_size);
    info->free_size = BYTES_TO_KBYTE(info->free_size);
    info->use_size = info->total_size - info->free_size;
    info->mem_util = info->use_size * DIGITAL_NUM_TO_PER / info->total_size; // 100 change to percentage

    return 0;
}

int mem_get_hbm_medium_info(u32 dev_id, struct mem_info *hbm_info)
{
    int ret;
    struct mem_info service_mem_info = {0};
    struct mem_info system_mem_info = {0};
#ifdef CFG_FEATURE_PG
    struct devdrv_info *dev_info = NULL;
#endif

    ret = mem_get_hbm_node_info(dev_id, MEM_SERVICE, &service_mem_info);
    if (ret != 0) {
        dms_err("Failed to obtain service memory info. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = mem_get_hbm_node_info(dev_id, MEM_SYSTEM, &system_mem_info);
    if (ret != 0) {
        dms_err("Failed to obtain system memory info. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

#ifdef CFG_FEATURE_PG
    dev_info = devdrv_manager_get_devdrv_info(dev_id);
    if (dev_info == NULL) {
        dms_err("Failed to obtain dev_info data. (dev_id=%u)\n", dev_id);
        return -EFAULT;
    }

    dms_debug("Obtains the number of hbm granules. (dev_id=%u; num=%u)\n",
        dev_id, dev_info->pg_info.comPgInfo.hbmPara.minNum);
    /* One is 16 GB, 1024*1024:GB to KB */
    hbm_info->total_size = dev_info->pg_info.comPgInfo.hbmPara.minNum * 16 * 1024 * 1024;
#else
    hbm_info->total_size = service_mem_info.total_size + system_mem_info.total_size;
#endif
    hbm_info->free_size = service_mem_info.free_size;
    hbm_info->use_size = hbm_info->total_size - hbm_info->free_size;
    if (hbm_info->total_size != 0) {
        hbm_info->mem_util = hbm_info->use_size * DIGITAL_NUM_TO_PER / hbm_info->total_size;
    } else {
        hbm_info->mem_util = 0;
    }

    return 0;
}

int mem_get_dev_hbm_info(u32 dev_id, u32 type, struct mem_info *hbm_info)
{
    int ret;

    switch (type) {
        case MEM_SERVICE:
        case MEM_SYSTEM:
            ret = mem_get_hbm_node_info(dev_id, type, hbm_info);
            break;
        case MEM_MEDIUM:
            ret = mem_get_hbm_medium_info(dev_id, hbm_info);
            break;
        default:
            dms_err("Unsupported memory type. (dev_id=%u; mem_type=%u)\n", dev_id, type);
            return -EOPNOTSUPP;
    }
    if (ret != 0) {
        dms_err("Failed to obtain dev hbm info. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    return 0;
}
