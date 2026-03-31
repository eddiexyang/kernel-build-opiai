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

#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/bitmap.h>
#include <linux/cpumask.h>
#include <linux/nodemask.h>

#include "dbl/chip_config.h"
#include "devdrv_interface.h"
#include "virtmngdev_memory.h"

STATIC int vmngd_resource_memory_cfg_dbl_nid(u32 dev_id, u32 vfid, int numa_id[], u32 numa_num,
    vmngd_element_t *memory)
{
    nid_info_t nids_info[VMNGD_MEMORY_MAX_NUMA_NUM] = {0};
    unsigned long tmp_bitmap = memory->bitmap;
    u32 bitnum = memory->bitnum;
    u32 i, nid_num, idx, vdev_id;
    int ret;

    ret = vmngd_get_devid_by_pfvf_id(dev_id, vfid, &vdev_id);
    if (ret) {
        vmng_err("Config dbl get devid fail.(dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return VMNG_ERR;
    }

    for (i = 0; (i < bitnum) && (i < VMNGD_MEMORY_MAX_NUMA_NUM); i++) {
        idx = find_first_bit(&tmp_bitmap, bitnum);
        if ((idx >= bitnum) || (idx >= numa_num)) {
            break;
        }
        clear_bit(idx, &tmp_bitmap);
        nids_info[i].nid = numa_id[idx];
    }
    nid_num = i;

    ret = dbl_nid_add_dev(vdev_id, DBL_MEMTYPE_HBM, DBL_SUB_MEMTYPE_ALL, nids_info, nid_num);
    ret += dbl_nid_add_dev(vdev_id, DBL_MEMTYPE_HBM, DBL_SUB_MEMTYPE_P2P, nids_info, nid_num);
    ret += dbl_nid_add_dev(vdev_id, DBL_MEMTYPE_HBM, DBL_SUB_MEMTYPE_TS, nids_info, nid_num);
    ret += dbl_nid_add_dev(vdev_id, DBL_MEMTYPE_HBM, DBL_SUB_MEMTYPE_AI, nids_info, nid_num);
    ret += dbl_nid_add_dev(vdev_id, DBL_MEMTYPE_HBM, DBL_SUB_MEMTYPE_CTRL, nids_info, nid_num);

    ret += dbl_nid_add_dev(vdev_id, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_ALL, nids_info, nid_num);
    ret += dbl_nid_add_dev(vdev_id, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_P2P, nids_info, nid_num);
    ret += dbl_nid_add_dev(vdev_id, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_TS, nids_info, nid_num);
    ret += dbl_nid_add_dev(vdev_id, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_AI, nids_info, nid_num);
    ret += dbl_nid_add_dev(vdev_id, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_CTRL, nids_info, nid_num);
    if (ret != 0) {
        vmng_err("Config dbl nid err.(dev_id=%u; vdev_id=%u; bitmap=0x%lx; nid_num=%u)\n",
            dev_id, vdev_id, memory->bitmap, nid_num);
        return VMNG_ERR;
    }
    vmng_info("Config info success.(dev_id=%u; vdev_id=%u; bitmap=0x%lx; nid_num=%u)\n",
        dev_id, vdev_id, memory->bitmap, nid_num);

    return VMNG_OK;
}

static int vmngd_resource_memory_remap(u32 dev_id, u32 vfid, int numa_id[], u32 numa_num,
    vmngd_element_t *memory)
{
#ifndef CFG_FEATURE_SOC_VIRTMNG
    agentdrv_numa_remap_t numa_remap;
    unsigned long tmp_bitmap = memory->bitmap;
    u32 bitnum = memory->bitnum;
    size_t size;
    u64 base;
    u32 idx;
    u32 i;

    if (agentdrv_get_addr_info(dev_id, DEVDRV_ADDR_HBM_BASE, 0, &base, &size) != VMNG_OK) {
        vmng_err("Get hbm address err.(dev_id=%u)\n", dev_id);
        return VMNG_ERR;
    }

    for (i = 0; (i < bitnum) && (i < AGENTDRV_NUMA_REMAP_NUM); i++) {
        idx = find_first_bit(&tmp_bitmap, bitnum);
        if (idx >= bitnum) {
            break;
        }
        numa_remap.address[i].start = (u64)idx * VMNGD_NUMA_NODE_SIZE + base;
        numa_remap.address[i].end = numa_remap.address[i].start + VMNGD_NUMA_NODE_SIZE - 1;
        clear_bit(idx, &tmp_bitmap);
    }

    if (agentdrv_numa_addr_remap(dev_id, vfid, &numa_remap) != VMNG_OK) {
        vmng_err("Remap numa addr err.(dev_id=%u; bitmap=0x%lx; bitnum=%u)\n",
            dev_id, memory->bitmap, bitnum);
        return VMNG_ERR;
    }
    vmng_info("Resource memory remap success.(dev_id=%u; bitmap=0x%lx; bitnum=%u)\n",
        dev_id, memory->bitmap, bitnum);
#endif

    return VMNG_OK;
}

STATIC int vmngd_resource_memory_clear_dbl_nid(u32 dev_id, u32 vfid)
{
    u32 vdev_id;
    int ret;

    ret = vmngd_get_devid_by_pfvf_id(dev_id, vfid, &vdev_id);
    if (ret != 0) {
        vmng_err("Config dbl get devid fail.(dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return VMNG_ERR;
    }

    ret = dbl_nid_del_dev(vdev_id, DBL_MEMTYPE_NUM, DBL_SUB_MEMTYPE_NUM);
    if (ret != 0) {
        vmng_err("Delete dbl nid err.(dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return VMNG_ERR;
    }
    vmng_info("Clear nid info success.(dev_id=%u; vdev_id=%u)\n",
        dev_id, vdev_id);

    return VMNG_OK;
}

STATIC int vmngd_resource_memory_unmap(u32 dev_id, u32 vfid)
{
#ifndef CFG_FEATURE_SOC_VIRTMNG
    if (agentdrv_numa_addr_unmap(dev_id, vfid) != VMNG_OK) {
        vmng_err("Unremap numa addr err.(dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return VMNG_ERR;
    }
#endif
    vmng_info("Resource memory unmap success.(dev_id=%u; vfid=%u)\n",
        dev_id, vfid);
    return VMNG_OK;
}

int vmngd_resource_memory_init(u32 dev_id, u32 vfid, int numa_id[], u32 numa_num,
    vmngd_element_t *memory)
{
    int ret;

    ret = vmngd_resource_memory_remap(dev_id, vfid, numa_id, numa_num, memory);
    if (ret != 0) {
        vmng_err("Memory remap err.(dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return VMNG_ERR;
    }
    ret = vmngd_resource_memory_cfg_dbl_nid(dev_id, vfid, numa_id, numa_num, memory);
    if (ret != 0) {
        vmng_err("Cfg dbl nid err.(dev_id=%u; vfid=%u)\n", dev_id, vfid);
        (void)vmngd_resource_memory_unmap(dev_id, vfid);
        return VMNG_ERR;
    }

    return VMNG_OK;
}

int vmngd_resource_memory_uninit(u32 dev_id, u32 vfid)
{
    int ret, ret1;

    ret = vmngd_resource_memory_clear_dbl_nid(dev_id, vfid);
    if (ret != 0) {
        vmng_err("Clear dbl nid err.(dev_id=%u; vfid=%u)\n", dev_id, vfid);
    }
    ret1 = vmngd_resource_memory_unmap(dev_id, vfid);
    if (ret1 != 0) {
        vmng_err("Memory unremap err.(dev_id=%u; vfid=%u)\n", dev_id, vfid);
    }

    return ((ret != 0) || (ret1 != 0)) ? VMNG_ERR : VMNG_OK;
}

int vmngd_resource_numa_id_init(u32 dev_id, int *array, u32 maxNum, u32 *numa_num)
{
    u32 i = 0;
    int nid;

    if (array == NULL) {
        vmng_err("Array is NULL.(dev_id=%u)\n", dev_id);
        return VMNG_ERR;
    }

    for_each_online_node(nid) {
        if ((nid >= MAX_NUMA_ID_OS) && ((u32)(nid / MAX_NUMA_ID_PER_DIE) == dev_id) && (i < maxNum)) {
            array[i] = nid;
            i++;
        }
    }

    *numa_num = i;
    return VMNG_OK;
}
