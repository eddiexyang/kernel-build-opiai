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
 * Create: 2023-02-09
 */

#include "devdrv_interface.h"
#include "virtmngdev_memory.h"
#include "virtmngdev_res_mng.h"
#include "virtmngdev_common_resource.h"
#include "virtmngdev_resource_register.h"

#define VMNG_RES_STRUCT(t, n, per_bit, bnum, bmap, a) {         \
    .type = (t),                                                \
    .element.attr = (a),                                        \
    .element.num_per_bit = (per_bit),                           \
    .element.bitnum = (bnum),                                   \
    .element.num = (n),                                         \
    .element.bitmap = (bmap),                                   \
}

struct vmng_resource device_res_cloud_v1[] = {
    VMNG_RES_STRUCT(MIA_AC_AIC,     32, 0, 0, 0,    ATTR_NUM_RESOURCE)    // PG version has 30 aic
};

struct vmng_resource device_res_mini_v2[] = {
    VMNG_RES_STRUCT(MIA_AC_AIC,     8,  0, 0, 0,    ATTR_NUM_RESOURCE),   // AG version has 10 aic
    VMNG_RES_STRUCT(MIA_DVPP_JPEGE, 8,  0, 0, 0,    ATTR_NUM_RESOURCE),
    VMNG_RES_STRUCT(MIA_DVPP_VPC,   12, 0, 0, 0,    ATTR_NUM_RESOURCE),
    VMNG_RES_STRUCT(MIA_DVPP_VDEC,  12, 0, 0, 0,    ATTR_NUM_RESOURCE),
    VMNG_RES_STRUCT(MIA_DVPP_JPEGD, 16, 0, 0, 0,    ATTR_NUM_RESOURCE),
    VMNG_RES_STRUCT(MIA_DVPP_VENC,  3,  0, 0, 0,    ATTR_NUM_RESOURCE)
};

struct vmng_resource device_res_cloud_v2[] = {
    //            (res_type,            num,    num_p_bit,  bitnum,  bitmap,
    //                                  attribute)
    VMNG_RES_STRUCT(MIA_AC_AIC,             25,     1,      25,     0x1FFFFFF,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE),
    VMNG_RES_STRUCT(MIA_AC_AIV,             50,     1,      50,     0x3FFFFFFFFFFFF,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE),
    VMNG_RES_STRUCT(MIA_AC_C_CORE,          8,      1,      8,      0xFF,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE | ATTR_VFG_NECESSARY),
    VMNG_RES_STRUCT(MIA_AC_DSA,             8,      1,      8,      0xFF,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE | ATTR_VFG_NECESSARY),
    VMNG_RES_STRUCT(MIA_AC_FFTS,            16,     1,      16,     0xFFFF,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE),
    VMNG_RES_STRUCT(MIA_AC_SDMA,            32,     1,      32,     0xFFFFFFFF,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE),
    VMNG_RES_STRUCT(MIA_AC_PCIE_DMA,        12,     1,      25,     0x1FFE000,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE | ATTR_EQUAL_ALLOC),
    VMNG_RES_STRUCT(MIA_STARS_ACSQ,         128,    8,      16,     0xFFFF,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE),
    VMNG_RES_STRUCT(MIA_STARS_CDQ,          128,    8,      16,     0xFFFF,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE),
    VMNG_RES_STRUCT(MIA_STARS_RTSQ,         2048,   128,    16,     0xFFFF,
                                        ATTR_BITMAP_RESOURCE),
    VMNG_RES_STRUCT(MIA_STARS_EVENT,        65536,  4096,   16,     0xFFFF,
                                        ATTR_BITMAP_RESOURCE),
    VMNG_RES_STRUCT(MIA_STARS_NOTIFY,       8192,   512,    16,     0xFFFF,
                                        ATTR_BITMAP_RESOURCE),
    VMNG_RES_STRUCT(MIA_DVPP_JPEGD,         28,     1,      28,     0xFFFFFFF,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE),
    VMNG_RES_STRUCT(MIA_DVPP_JPEGE,         4,      1,      4,      0xF,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE),
    VMNG_RES_STRUCT(MIA_DVPP_VPC,           10,     1,      10,     0x3FF,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE),
    VMNG_RES_STRUCT(MIA_DVPP_VENC,          0,      1,      0,      0x0,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE),
    VMNG_RES_STRUCT(MIA_DVPP_VDEC,          2,      1,      2,      0x3,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE),
    VMNG_RES_STRUCT(MIA_STARS_TOPIC_ACPU_SLOT,    32,     1,      32,     0xFFFFFFFF,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE),
    VMNG_RES_STRUCT(MIA_CPU_HOST_ACPU,      64,     1,      64,     0xFFFFFFFFFFFFFFFF,
                                        ATTR_BITMAP_RESOURCE),
    VMNG_RES_STRUCT(MIA_SYS_MEM,            65536,  16384,  4,      0xF,
                                        ATTR_NUM_RESOURCE), // memory only need to set num
};

struct vmng_resource device_res_mini_v3[] = {
    //            (res_type,            num,   num_p_bit,  bitnum,  bitmap,
    //                                  attribute)
    VMNG_RES_STRUCT(MIA_AC_AIC,             1,      1,      1,     0x1,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE | ATTR_VFG_FULL_ALLOC),
    VMNG_RES_STRUCT(MIA_AC_AIV,             1,      1,      1,     0x1,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE | ATTR_VFG_FULL_ALLOC),
    VMNG_RES_STRUCT(MIA_AC_C_CORE,          1,      1,      1,      0x1,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE | ATTR_VFG_FULL_ALLOC),
    VMNG_RES_STRUCT(MIA_AC_DSA,             1,      1,      1,      0x1,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE | ATTR_VFG_FULL_ALLOC),
    VMNG_RES_STRUCT(MIA_AC_FFTS,            2,      1,      2,      0x3,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE | ATTR_VFG_FULL_ALLOC),
    VMNG_RES_STRUCT(MIA_AC_SDMA,            4,      1,      4,      0xF,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE | ATTR_VFG_FULL_ALLOC),
    VMNG_RES_STRUCT(MIA_STARS_ACSQ,         32,     2,      16,     0xFFFF,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE | ATTR_VFG_FULL_ALLOC),
    VMNG_RES_STRUCT(MIA_STARS_RTSQ,         512,    32,     16,     0xFFFF,
                                        ATTR_BITMAP_RESOURCE),
    VMNG_RES_STRUCT(MIA_STARS_TOPIC_ACPU_SLOT,  4,  1,      4,      0xF,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_FULL_ALLOC),
    VMNG_RES_STRUCT(MIA_STARS_TOPIC_CCPU_SLOT,  4,  1,      4,      0xF,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_FULL_ALLOC),
    VMNG_RES_STRUCT(MIA_STARS_EVENT,        65536,  4096,   16,     0xFFFF,
                                        ATTR_BITMAP_RESOURCE),
    VMNG_RES_STRUCT(MIA_STARS_NOTIFY,       2048,   256,    16,     0xFFFF,
                                        ATTR_BITMAP_RESOURCE),
    VMNG_RES_STRUCT(MIA_STARS_CMO,          65536,  4096,   16,     0xFFFF,
                                        ATTR_BITMAP_RESOURCE),
    VMNG_RES_STRUCT(MIA_DVPP_JPEGD,         2,      1,      2,      0x3,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE | ATTR_VFG_FULL_ALLOC),
    VMNG_RES_STRUCT(MIA_DVPP_JPEGE,         1,      1,      1,      0x1,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE | ATTR_VFG_FULL_ALLOC),
    VMNG_RES_STRUCT(MIA_DVPP_VPC,           2,      1,      2,      0x3,
                                        ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE | ATTR_VFG_FULL_ALLOC),
    VMNG_RES_STRUCT(MIA_DVPP_VENC,          3,      1,      3,      0x7,
                                        ATTR_BITMAP_RESOURCE),
    VMNG_RES_STRUCT(MIA_DVPP_VDEC,          4,      1,      4,      0xf,
                                        ATTR_BITMAP_RESOURCE),
};

STATIC int vmngd_get_dev_res_list(int dev_id, struct vmng_resource **res_list, int *res_num)
{
    unsigned int chip_type;

    chip_type = vmngd_get_dev_chip_type(dev_id);
    if (chip_type == HISI_CLOUD_V1) {
        *res_list = device_res_cloud_v1;
        *res_num = sizeof(device_res_cloud_v1) / sizeof(struct vmng_resource);
        return 0;
    }
    if (chip_type == HISI_MINI_V2) {
        *res_list = device_res_mini_v2;
        *res_num = sizeof(device_res_mini_v2) / sizeof(struct vmng_resource);
        return 0;
    }
    if (chip_type == HISI_CLOUD_V2) {
        *res_list = device_res_cloud_v2;
        *res_num = sizeof(device_res_cloud_v2) / sizeof(struct vmng_resource);
        return 0;
    }
    if (chip_type == HISI_MINI_V3) {
        *res_list = device_res_mini_v3;
        *res_num = sizeof(device_res_mini_v3) / sizeof(struct vmng_resource);
        return 0;
    }
    return -EINVAL;
}

STATIC int vmngd_register_aicpu_resource(unsigned int dev_id)
{
    struct vmng_resource aicpu = {0};
    unsigned int chip_type;
    int ret;

    chip_type = vmngd_get_dev_chip_type(dev_id);
    if ((chip_type != HISI_MINI_V2) && (chip_type != HISI_MINI_V3) && (chip_type != HISI_CLOUD_V2)) {
        return 0;
    }

    vmngd_resource_get_aicpu_info(dev_id, &aicpu.element);
    aicpu.type = MIA_CPU_DEV_ACPU;
    aicpu.element.attr = ATTR_BITMAP_RESOURCE | ATTR_VFG_SHARE;
    if (chip_type == HISI_MINI_V3) {
        aicpu.element.attr |= ATTR_VFG_FULL_ALLOC;
    }

    vmngd_element_set(&aicpu.element, aicpu.element.bitmap);
    ret = vmngd_resource_register(dev_id, &aicpu);
    if (ret != 0) {
        vmng_err("Register aicpu resource failed.(dev_id=%u;ret=%d)\n", dev_id, ret);
        return ret;
    }
    vmng_info("Register aicpu resource success.(dev_id=%u;aicpu_bitmap=%#lx)\n", dev_id, aicpu.element.bitmap);
    return 0;
}

STATIC void vmngd_unregister_aicpu_resource(unsigned int dev_id)
{
    struct vmng_resource aicpu = {0};

    aicpu.type = MIA_CPU_DEV_ACPU;
    vmngd_resource_unregister(dev_id, &aicpu);
}

STATIC u64 vmngd_get_numa_node_size(int nid)
{
    return (__pfn_to_phys(node_end_pfn(nid)) - __pfn_to_phys(node_start_pfn(nid)));
}

STATIC bool vmngd_is_avaliable_numa_node(int nid)
{
    u64 node_size;

    node_size = vmngd_get_numa_node_size(nid);
    if (node_size <= VMNGD_AVALIABLE_NODE_SIZE) {
        return false;
    }

    return true;
}

STATIC int vmngd_set_numa_num_per_bit(int nids[], int nid_num, struct vmng_resource *numa)
{
    unsigned long idx;
    u64 node_size;
    int nid;

    idx = find_first_bit(&numa->element.bitmap, numa->element.bitnum);
    if (idx >= nid_num) {
        return -EINVAL;
    }

    nid = nids[idx];
    node_size = vmngd_get_numa_node_size(nid);
    numa->element.num_per_bit = (node_size >> 20); // 20 trans Byte to MB
    vmng_info("Get numa node size. (size=%uMB)\n", numa->element.num_per_bit);
    return 0;
}

STATIC int vmngd_register_numa_resource(unsigned int dev_id)
{
    int array[DEVMM_MAX_NUMA_NUM_OF_PER_DEV];
    struct vmng_resource numa = {0};
    unsigned long numa_bitmap = 0;
    unsigned int chip_type;
    int i = 0;
    int nid;
    int ret;

    chip_type = vmngd_get_dev_chip_type(dev_id);
    if (chip_type != HISI_CLOUD_V2) {   // only cloud_v2 need to manage numa resource
        return 0;
    }

    numa.type = MIA_MEM_NUMA;
    numa.element.attr = ATTR_BITMAP_RESOURCE | ATTR_RATIO_ALLOC;
    numa.element.bitnum = DEVMM_MAX_NUMA_NUM_OF_PER_DEV;
    for_each_online_node(nid) {
        if (i >= DEVMM_MAX_NUMA_NUM_OF_PER_DEV) {
            break;
        }
        if ((nid >= MAX_NUMA_ID_OS) && ((u32)(nid / MAX_NUMA_ID_PER_DIE) == dev_id)) {
            array[i] = nid;
            if (vmngd_is_avaliable_numa_node(nid)) {
                numa_bitmap |= (0x1UL << i);
            }
            i++;
        }
    }

    vmngd_element_set(&numa.element, numa_bitmap);
    ret = vmngd_set_numa_num_per_bit(array, i, &numa);
    if (ret != 0) {
        vmng_err("Set numa num per bit info failed.(dev_id=%u;ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = vmngd_resource_register(dev_id, &numa);
    if (ret != 0) {
        vmng_err("Register memory resource failed.(dev_id=%u;ret=%d)\n", dev_id, ret);
        return ret;
    }
    vmng_info("Register numa resource success.(dev_id=%u;numa_bitmap=%#lx)\n", dev_id, numa.element.bitmap);

    return VMNG_OK;
}

void vmngd_unregister_numa_resource(unsigned int dev_id)
{
    struct vmng_resource numa = {0};

    numa.type = MIA_MEM_NUMA;
    vmngd_resource_unregister(dev_id, &numa);
}

int vmngd_register_dev_all_res(unsigned int dev_id)
{
    struct vmng_pf_object *pf_node = NULL;
    struct vmng_resource *res_list = NULL;
    int res_num = 0;
    int ret;
    int i;

    pf_node = vmngd_get_pf_object(dev_id);
    if (pf_node == NULL) {
        vmng_err("Get pf node failed. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }
    pf_node->chip_type = vmngd_get_dev_chip_type(dev_id);

    ret = vmngd_get_dev_res_list(dev_id, &res_list, &res_num);
    if (ret != 0 || res_list == NULL || res_num == 0) {
        vmng_err("Get device resource list failed. (ret=%d;res_num=%d)\n", ret, res_num);
        return -EINVAL;
    }

    for (i = 0; i < res_num; ++i) {
        ret = vmngd_resource_register(dev_id, res_list + i);
        if (ret != 0) {
            vmng_err("Register resource failed. (dev_id=%u;type=%d;ret=%d)\n", dev_id, res_list[i].type, ret);
            goto failed;
        }
    }

    ret = vmngd_register_aicpu_resource(dev_id);
    if (ret != 0) {
        vmng_err("Register aicpu resource failed.(dev_id=%u;ret=%d)\n", dev_id, ret);
        goto failed;
    }

    ret = vmngd_register_numa_resource(dev_id);
    if (ret != 0) {
        vmng_err("Register numa resource failed.(dev_id=%u;ret=%d)\n", dev_id, ret);
        vmngd_unregister_aicpu_resource(dev_id);
        goto failed;
    }

    return ret;
failed:
    for (i--; i >= 0; --i) {
        vmngd_resource_unregister(dev_id, res_list + i);
    }
    return ret;
}
