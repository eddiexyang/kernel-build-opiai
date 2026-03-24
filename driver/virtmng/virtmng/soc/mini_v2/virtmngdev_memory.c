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

#include <linux/version.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/securec.h>
#include <asm/uaccess.h>

#include "devdrv_interface.h"
#include "virtmngdev_resource.h"
#include "virtmngdev_memory.h"

#define VMNGD_MEMNFO_PATH_LEN 128
#define VMNGD_MEMNFO_TMP_LEN 128
#define VMNGD_DEVICE_AUTHORITY 0440
#define VMNGD_MEMNFO_MAX_LEN 2048
#define VMNGD_MEMNFO_MEMBER_NUM 2
#define VMNGD_NODE_MEMINFO_PATH "/sys/devices/system/node/node%d/meminfo"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
static ssize_t vmngd_read_file(struct file *fp, char *dst_addr, size_t fsize, loff_t *pos)
{
    return kernel_read(fp, dst_addr, fsize, pos);
}
#else
static ssize_t vmngd_read_file(struct file *fp, char *dst_addr, size_t fsize, loff_t *pos)
{
    mm_segment_t old_fs;
    ssize_t ret;

    old_fs = get_fs();
    /*lint -emacro(501,KERNEL_DS)*/
    set_fs((mm_segment_t)KERNEL_DS); /*lint!e501*/
    ret = vfs_read(fp, dst_addr, fsize, pos);
    set_fs(old_fs);
    return ret;
}
#endif

STATIC int vmngd_create_node_name(char *node_name, size_t memnfo_path_len, u32 node_index)
{
    return snprintf_s(node_name, memnfo_path_len, memnfo_path_len - 1, VMNGD_NODE_MEMINFO_PATH, node_index);
}

static int vmngd_get_num_from_string(const char *p_str, const char *form_str, u32 *size)
{
    char tmp[VMNGD_MEMNFO_TMP_LEN] = {0};
    return sscanf_s(p_str, "%s %u", tmp, VMNGD_MEMNFO_TMP_LEN - 1, size);
}

STATIC int vmngd_get_memsize_from_string(const char *file_string, const char *mem_string, u32 *size)
{
    char *p_str = NULL;
    u32 tmp_size = 0;
    int ret;

    p_str = strstr(file_string, mem_string);
    if (p_str == NULL) {
        vmng_err("Can not find string. (mem_node=%s)\n", mem_string);
        return -EINVAL;
    }

    ret = vmngd_get_num_from_string(p_str, "%s %d", &tmp_size);
    if (ret != VMNGD_MEMNFO_MEMBER_NUM) {
        vmng_err("Sscanf not right. (mem_node=%s)\n", mem_string);
        return -EINVAL;
    }

    *size = tmp_size;

    return 0;
}

static int vmngd_get_meminfo(u32 node_index, u32 *total_size_kb)
{
    char node_name[VMNGD_MEMNFO_PATH_LEN] = {0};
    char *file_string = NULL;
    struct file *fp = NULL;
    loff_t pos;
    int ret;

    ret = vmngd_create_node_name(node_name, VMNGD_MEMNFO_PATH_LEN, node_index);
    if (ret < 0) {
        return -EINVAL;
    }

    fp = filp_open(node_name, O_RDONLY, VMNGD_DEVICE_AUTHORITY);
    if (IS_ERR((void const *)fp)) {
        vmng_info("File not exist. (file=\"%s\")\n", node_name);
        return -ENOENT;
    }

    file_string = (char *)kzalloc(VMNGD_MEMNFO_MAX_LEN, GFP_KERNEL);
    if (file_string == NULL) {
        vmng_err("Kzalloc error.\n");
        goto getKallocError;
    }

    pos = 0;
    if (vmngd_read_file(fp, file_string, VMNGD_MEMNFO_MAX_LEN - 1, &pos) < 0) {
        vmng_err("Filestring not right. (file=\"%s\"; pos=%lld)\n", node_name, pos);
        goto getMemInfoError;
    }

    ret = vmngd_get_memsize_from_string(file_string, "MemTotal:", total_size_kb);
    if (ret != 0) {
        vmng_err("Can not find MemTotal. (file=\"%s\"; pos=%lld)\n", node_name, pos);
        goto getMemInfoError;
    }

    kfree(file_string);
    (void)filp_close(fp, NULL);

    return 0;

getMemInfoError:
    kfree(file_string);
getKallocError:
    (void)filp_close(fp, NULL);

    return -EINVAL;
}

/* 310P
 * devid    1p:0   2P:master,slave
 * numa id
 * DDR      0      0(master),1(slave)
 * P2P DDR  1      2(master),3(slave)
 * 4G DDR   2      4(master),5(slave)
 */
#define VMNGD_MINIV2_NODE_NUM 3
void vmngd_get_dev_memory_info(u32 dev_id, vmngd_resource_info_t *info)
{
    u32 dev_num = (u32)devdrv_get_davinci_dev_num();
    unsigned int totalram_kb = 0;
    unsigned int noderam_kb = 0;
    u32 nid;
    u32 i;

    if (dev_num != 1 && dev_num != 2) { // only 1P and 2P optional
        vmng_err("Invalid device num. (dev_num=%d)\n", dev_num);
        return;
    }

    for (i = 0; i < VMNGD_MINIV2_NODE_NUM; ++i) {
        nid = dev_id + dev_num * i;
        if (vmngd_get_meminfo(nid, &noderam_kb) != 0) {
            vmng_err("Can't get node total memory. (node_id = %u)\n", i);
            return;
        }
        totalram_kb += noderam_kb;
    }

    info->vf_cfg_total.base.memory.num = totalram_kb;
    info->vf_cfg_remain.base.memory.num = info->vf_cfg_total.base.memory.num;
    vmng_info("Get total memory size = %u KB\n", totalram_kb);
}

/**
 * @param [in] u64 alloc_memory_size :  the unit of input data is MB, e.g 2048MB,
 * But the unit of data we manage is KB, because if we don't, the data isn't accurate enough.
 * So that, we need to convert input memory size from MB to KB.
 **/
int vmngd_alloc_memory_for_vf(u32 dev_id, vmngd_resource_unit_t *vf, u64 alloc_memory_size)
{
    vmngd_resource_info_t *resource_info = vmngd_get_resource_info(dev_id);
    vmngd_resource_unit_t *remain = &resource_info->vf_cfg_remain;
    vmngd_resource_unit_t *total = &resource_info->vf_cfg_total;

    alloc_memory_size <<= 10; // 10 because we need to convert from MB to KB

    if (alloc_memory_size == 0) { // adapt for VM computing group
        alloc_memory_size = total->base.memory.num * vf->accelerator.aic.num / total->accelerator.aic.num;
    }

    if (alloc_memory_size > remain->base.memory.num) {
        vmng_err("Remain memory resource is not enough, (remain_memory=%u, alloc_memory=%llu).\n",
            remain->base.memory.num, alloc_memory_size);
        return VMNG_ERR;
    }
    vf->base.memory.num = (u32)alloc_memory_size;
    remain->base.memory.num -= vf->base.memory.num;
    vmng_info("Alloc memory success, (remain_memory=%u, alloc_memory=%llu).\n", remain->base.memory.num,
        alloc_memory_size);
    return VMNG_OK;
}

void vmngd_release_memory_resource(u32 dev_id, vmngd_resource_unit_t *vf)
{
    vmngd_resource_info_t *resource_info = vmngd_get_resource_info(dev_id);
    vmngd_resource_unit_t *remain = &resource_info->vf_cfg_remain;
    remain->base.memory.num += vf->base.memory.num;
    vf->base.memory.num = 0;
}