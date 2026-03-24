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
 * Create: 2022-07-27
 */

#include <linux/types.h>
#include <linux/numa.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/securec.h>
#include <linux/version.h>
#include <linux/proc_fs.h>

#include "dbl/chip_config.h"
#include "chip_config_module.h"
#include "numa_id_proc_fs.h"
#include "numa_id.h"

#define NID_MAX_DEV_NUM      1024
#define NID_MAX_NID_NUM      128
#define NID_MEMINFO_PATH_LEN 64
#define NID_MEMINFO_PATH     "/sys/devices/system/node/node%d/meminfo"
#define NID_MEMINFO_MAX_LEN  128
#define NID_MEMINFO_KEY_LEN  32

typedef struct mem_info {
    int nid_num;
    int *nid;
} mem_info_t;

typedef struct dev_nid {
    nid_info_t nid_info[NID_MAX_NID_NUM];
    mem_info_t mem[DBL_MEMTYPE_NUM][DBL_SUB_MEMTYPE_NUM];
} dev_nid_t;

STATIC dev_nid_t *g_dev_nid[NID_MAX_DEV_NUM];
STATIC spinlock_t g_dev_lock[NID_MAX_DEV_NUM];

STATIC inline u32 nid_mask_sub_memtype(u32 memtype, u32 sub_memtype)
{
    return (1 << (memtype * DBL_SUB_MEMTYPE_NUM + sub_memtype));
}

STATIC inline u32 nid_mask_memtype(u32 memtype)
{
    return (((1 << DBL_SUB_MEMTYPE_NUM) - 1) << (memtype * DBL_SUB_MEMTYPE_NUM));
}

STATIC inline void nid_set_mask_to_support(u32 *mask, u32 flag)
{
    *mask |= flag;
}

STATIC inline void nid_set_mask_not_support(u32 *mask, u32 flag)
{
    *mask &= ~flag;
}

STATIC inline bool nid_mask_is_support(u32 mask, u32 flag)
{
    return ((mask & flag) > 0);
}

STATIC ssize_t nid_kernel_read(struct file *fp, char *dst_addr, size_t fsize, loff_t *pos)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
    return kernel_read(fp, dst_addr, fsize, pos);
#else
    mm_segment_t old_fs;
    ssize_t ret;

    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);
    ret = vfs_read(fp, dst_addr, fsize, pos);
    set_fs(old_fs);
    return ret;
#endif
}

STATIC int nid_get_data_from_meminfo(const char *buffer, const char *key, size_t len, u64 *data)
{
    char *pos = NULL;
    u64 tmp;
    int ret;

    pos = strstr(buffer, key);
    if (pos == NULL) {
        ccfg_err("Can not find key from nid info. (key=%s)\n", key);
        return -EINVAL;
    }

    ret = sscanf_s(pos + len + 1, " %llu", &tmp);
    if (ret <= 0) {
        ccfg_info("Sscanf_s value not find. (key=%s)\n", key);
        tmp = 0;
    }
    *data = tmp;

    return 0;
}

STATIC int nid_set_nid_info_size(nid_info_t *nid_info)
{
    int ret;
    char path[NID_MEMINFO_PATH_LEN] = {0};
    struct file *fp = NULL;
    loff_t pos = 0;
    char *buffer = NULL;

    ret = sprintf_s(path, NID_MEMINFO_PATH_LEN, NID_MEMINFO_PATH, nid_info->nid);
    if (ret < 0) {
        ccfg_err("Sprintf_s fail. (nid=%d)\n", nid_info->nid);
        return -EINVAL;
    }

    fp = filp_open(path, O_RDONLY, 0);
    if (IS_ERR(fp)) {
        ccfg_err("File not exist. (file=\"%s\")\n", path);
        return -ENOENT;
    }

    buffer = (char *)kzalloc(NID_MEMINFO_MAX_LEN, GFP_KERNEL | __GFP_ACCOUNT);
    if (buffer == NULL) {
        ccfg_err("Kzalloc fail.\n");
        ret = -ENOMEM;
        goto out_close;
    }

    ret = nid_kernel_read(fp, buffer, NID_MEMINFO_MAX_LEN - 1, &pos);
    if (ret < 0) {
        ccfg_err("Kernel_read fail. (file=\"%s\")\n", path);
        ret = -EINVAL;
        goto out_kfree;
    }

    ret = nid_get_data_from_meminfo(buffer, "MemTotal", strlen("MemTotal"), &nid_info->size.total_size);
    if (ret != 0) {
        ccfg_err("Get nid MemTotal fail. (file=\"%s\")\n", path);
        goto out_kfree;
    }
    ret = nid_get_data_from_meminfo(buffer, "MemFree", strlen("MemFree"), &nid_info->size.free_size);
    if (ret != 0) {
        ccfg_err("Get nid MemFree fail. (file=\"%s\")\n", path);
        goto out_kfree;
    }

    ret = 0;

out_kfree:
    kfree(buffer);
out_close:
    filp_close(fp, NULL);
    return ret;
}

STATIC int nid_add_sub_memtype_nid(u32 devid, u32 memtype, u32 sub_memtype, nid_info_t nids_info[], int num)
{
    dev_nid_t *dev_nid = NULL;
    mem_info_t *mem = NULL;
    bool init_fs_flag = false;
    int *nids = NULL;
    int i;

    nids = kzalloc(sizeof(int) * num, GFP_KERNEL | __GFP_ACCOUNT);
    if (nids == NULL) {
        ccfg_err("Kzalloc fail. (devid=%u; memtype=%u; sub_memtype=%u)\n", devid, memtype, sub_memtype);
        return -ENOMEM;
    }

    spin_lock_bh(&g_dev_lock[devid]);
    if (g_dev_nid[devid] == NULL) {
        g_dev_nid[devid] = kzalloc(sizeof(dev_nid_t), GFP_ATOMIC | __GFP_ACCOUNT);
        if (g_dev_nid[devid] == NULL) {
            spin_unlock_bh(&g_dev_lock[devid]);
            ccfg_err("Kzalloc fail. (devid=%u; memtype=%u; sub_memtype=%u)\n", devid, memtype, sub_memtype);
            goto out_kfree;
        }
        init_fs_flag = true;
    }

    dev_nid = g_dev_nid[devid];
    mem = &dev_nid->mem[memtype][sub_memtype];
    if ((mem->nid_num != 0) || (mem->nid != NULL)) {
        spin_unlock_bh(&g_dev_lock[devid]);
        ccfg_err("Mem nids have been set. (devid=%u; memtype=%u; sub_memtype=%u)\n", devid, memtype, sub_memtype);
        goto out_kfree;
    }

    mem->nid = nids;
    for (i = 0; i < num; i++) {
        mem->nid[i] = nids_info[i].nid;
        if ((dev_nid->nid_info[nids_info[i].nid].size.total_size > 0)
            && (dev_nid->nid_info[nids_info[i].nid].nid_mask > 0)) {
            continue;
        }
        dev_nid->nid_info[nids_info[i].nid].size = nids_info[i].size;
        nid_set_mask_to_support(&dev_nid->nid_info[nids_info[i].nid].nid_mask,
                                nid_mask_sub_memtype(memtype, sub_memtype));
    }
    mem->nid_num = num;
    spin_unlock_bh(&g_dev_lock[devid]);
    if (init_fs_flag == true) {
        numa_id_fs_add_device(devid);
    }
    return 0;

out_kfree:
    kfree(nids);
    return -EINVAL;
}

STATIC void nid_del_dev(u32 devid)
{
    int i;
    int j;

    spin_lock_bh(&g_dev_lock[devid]);
    if (g_dev_nid[devid] == NULL) {
        spin_unlock_bh(&g_dev_lock[devid]);
        return;
    }

    for (i = 0; i < DBL_MEMTYPE_NUM; i++) {
        for (j = 0; j < DBL_SUB_MEMTYPE_NUM; j++) {
            if (g_dev_nid[devid]->mem[i][j].nid != NULL) {
                kfree(g_dev_nid[devid]->mem[i][j].nid);
            }
        }
    }
    kfree(g_dev_nid[devid]);
    g_dev_nid[devid] = NULL;
    spin_unlock_bh(&g_dev_lock[devid]);
    numa_id_fs_del_device(devid);
}

STATIC nid_info_t *nid_create_nids_info(nid_info_t nids_info[], int num)
{
    int total_size = num * sizeof(nid_info_t);
    nid_info_t *new_nids_info = NULL;
    int i, ret;

    new_nids_info = kmalloc(total_size, GFP_KERNEL | __GFP_ACCOUNT);
    if (new_nids_info == NULL) {
        ccfg_err("Alloc new_nids_info failed.\n");
        return NULL;
    }

    ret = memcpy_s(new_nids_info, total_size, nids_info, total_size);
    if (ret != 0) {
        kfree(new_nids_info);
        ccfg_err("Copy to new_nids_info failed. (ret=%d)\n", ret);
        return NULL;
    }

    for (i = 0; i < num; i++) {
        if (new_nids_info[i].size.total_size == 0) {
            ret = nid_set_nid_info_size(&new_nids_info[i]);
            if (ret != 0) {
                ccfg_err("Get nid size fail. (i=%d; nid=%d)\n", i, new_nids_info[i].nid);
                kfree(new_nids_info);
                return NULL;
            }
        }
    }

    return new_nids_info;
}

STATIC void nid_delete_nids_info(nid_info_t *nid_info)
{
    if (nid_info != NULL) {
        kfree(nid_info);
    }

    return;
}

int dbl_nid_add_dev(u32 devid, u32 memtype, u32 sub_memtype, nid_info_t nids_info[], int num)
{
    nid_info_t *new_nids_info = NULL;
    int ret, i;

    if ((devid >= NID_MAX_DEV_NUM) || (memtype >= DBL_MEMTYPE_NUM) || (sub_memtype >= DBL_SUB_MEMTYPE_NUM)) {
        ccfg_err("Devid or memtype is wrong. (devid=%u; memtype=%u; sub_memtype=%u)\n", devid, memtype, sub_memtype);
        return -EINVAL;
    }
    if ((nids_info == NULL) || (num <= 0) || (num > NID_MAX_NID_NUM)) {
        ccfg_err("Nids info is invalid. (devid=%u; memtype=%u; sub_memtype=%u; nid_num=%d)\n",
            devid, memtype, sub_memtype, num);
        return -EINVAL;
    }

    for (i = 0; i < num; i++) {
        if ((nids_info[i].nid < 0) || (nids_info[i].nid >= NID_MAX_NID_NUM)) {
            ccfg_err("NID must be a vaild number. (devid=%u; memtype=%u; sub_memtype=%u; nid=%d)\n",
                devid, memtype, sub_memtype, nids_info[i].nid);
            return -EINVAL;
        }
    }

    /* will copy nids_info to new_nids_info in order to not change nids_info */
    new_nids_info = nid_create_nids_info(nids_info, num);
    if (new_nids_info == NULL) {
        ccfg_err("Create new_nids_info failed. (devid=%u; memtype=%u; sub_memtype=%u)\n", devid, memtype, sub_memtype);
        return -ENOMEM;
    }

    ret = nid_add_sub_memtype_nid(devid, memtype, sub_memtype, new_nids_info, num);
    if (ret != 0) {
        ccfg_err("Add dev nid fail. (devid=%u; memtype=%u; sub_memtype=%u)\n", devid, memtype, sub_memtype);
    }

    nid_delete_nids_info(new_nids_info);
    return ret;
}
EXPORT_SYMBOL(dbl_nid_add_dev);

int dbl_nid_del_dev(u32 devid, u32 memtype, u32 sub_memtype)
{
    mem_info_t *mem = NULL;
    int i;

    if ((devid >= NID_MAX_DEV_NUM) || (memtype > DBL_MEMTYPE_NUM) || (sub_memtype > DBL_SUB_MEMTYPE_NUM)) {
        ccfg_err("Devid or memtype is wrong. (devid=%u; memtype=%u; sub_memtype=%u)\n", devid, memtype, sub_memtype);
        return -EINVAL;
    }

    if ((memtype == DBL_MEMTYPE_NUM) && (sub_memtype == DBL_SUB_MEMTYPE_NUM)) {
        nid_del_dev(devid);
        return 0;
    } else if ((memtype == DBL_MEMTYPE_NUM) || (sub_memtype == DBL_SUB_MEMTYPE_NUM)) {
        ccfg_err("Memtype is wrong. (devid=%u; memtype=%u; sub_memtype=%u)\n", devid, memtype, sub_memtype);
        return -EINVAL;
    }

    spin_lock_bh(&g_dev_lock[devid]);
    if (g_dev_nid[devid] == NULL) {
        spin_unlock_bh(&g_dev_lock[devid]);
        ccfg_err("Dev nid is not set. (devid=%u)\n", devid);
        return -ENODEV;
    }

    mem = &g_dev_nid[devid]->mem[memtype][sub_memtype];
    if ((mem->nid_num == 0) || (mem->nid == NULL)) {
        spin_unlock_bh(&g_dev_lock[devid]);
        ccfg_err("Mem nid is not set. (devid=%u; memtype=%u; sub_memtype=%u)\n", devid, memtype, sub_memtype);
        return -ENOMEM;
    }
    for (i = 0; i < mem->nid_num; i++) {
        u32 flag = nid_mask_sub_memtype(memtype, sub_memtype);
        nid_set_mask_not_support(&g_dev_nid[devid]->nid_info[mem->nid[i]].nid_mask, flag);
    }
    mem->nid_num = 0;
    kfree(mem->nid);
    mem->nid = NULL;
    spin_unlock_bh(&g_dev_lock[devid]);

    return 0;
}
EXPORT_SYMBOL(dbl_nid_del_dev);

int dbl_nid_get_nid_num(u32 devid, u32 memtype, u32 sub_memtype)
{
    int nid_num = 0;
    int i;

    if ((devid >= NID_MAX_DEV_NUM) || (memtype >= DBL_MEMTYPE_NUM) || (sub_memtype > DBL_SUB_MEMTYPE_NUM)) {
        ccfg_err("Devid or memtype is wrong. (devid=%u; memtype=%u; sub_memtype=%u)\n", devid, memtype, sub_memtype);
        return -EINVAL;
    }

    spin_lock_bh(&g_dev_lock[devid]);
    if (g_dev_nid[devid] == NULL) {
        spin_unlock_bh(&g_dev_lock[devid]);
        ccfg_err("Get nid_num fail, dev is NULL. (devid=%u)\n", devid);
        return -ENODEV;
    }

    if (sub_memtype == DBL_SUB_MEMTYPE_NUM) {
        for (i = 0; i < NID_MAX_NID_NUM; i++) {
            if (nid_mask_is_support(g_dev_nid[devid]->nid_info[i].nid_mask, nid_mask_memtype(memtype))) {
                nid_num++;
            }
        }
    } else {
        nid_num = g_dev_nid[devid]->mem[memtype][sub_memtype].nid_num;
    }
    spin_unlock_bh(&g_dev_lock[devid]);

    return nid_num;
}
EXPORT_SYMBOL(dbl_nid_get_nid_num);

int dbl_nid_get_nid(u32 devid, u32 memtype, u32 sub_memtype, int nids[], int num)
{
    mem_info_t *mem = NULL;
    int count;
    int i;

    if ((devid >= NID_MAX_DEV_NUM) || (memtype >= DBL_MEMTYPE_NUM) || (sub_memtype > DBL_SUB_MEMTYPE_NUM)) {
        ccfg_err("Devid or memtype is wrong. (devid=%u; memtype=%u; sub_memtype=%u)\n", devid, memtype, sub_memtype);
        return -EINVAL;
    }

    spin_lock_bh(&g_dev_lock[devid]);
    if (g_dev_nid[devid] == NULL) {
        spin_unlock_bh(&g_dev_lock[devid]);
        return -ENODEV;
    }

    if (sub_memtype == DBL_SUB_MEMTYPE_NUM) {
        for (i = 0, count = 0; i < NID_MAX_NID_NUM && count < num; i++) {
            if (nid_mask_is_support(g_dev_nid[devid]->nid_info[i].nid_mask, nid_mask_memtype(memtype))) {
                nids[count] = i;
                count++;
            }
        }
    } else {
        mem = &g_dev_nid[devid]->mem[memtype][sub_memtype];
        if ((mem->nid_num == 0) || (mem->nid == NULL)) {
            spin_unlock_bh(&g_dev_lock[devid]);
            return -ENOMEM;
        }
        count = num < mem->nid_num ? num : mem->nid_num;
        for (i = 0; i < count; i++) {
            nids[i] = mem->nid[i];
        }
    }
    spin_unlock_bh(&g_dev_lock[devid]);

    return count;
}
EXPORT_SYMBOL(dbl_nid_get_nid);

int dbl_nid_get_sub_memtype_num(u32 devid, u32 memtype)
{
    mem_info_t *mem = NULL;
    int mem_num = 0;
    int i;

    if ((devid >= NID_MAX_DEV_NUM) || (memtype >= DBL_MEMTYPE_NUM)) {
        ccfg_err("Devid or memtype is wrong. (devid=%u; memtype=%u)\n", devid, memtype);
        return -EINVAL;
    }

    spin_lock_bh(&g_dev_lock[devid]);
    if (g_dev_nid[devid] == NULL) {
        spin_unlock_bh(&g_dev_lock[devid]);
        ccfg_err("Get sub_memtype num fail, dev is NULL. (devid=%u)\n", devid);
        return -ENODEV;
    }

    for (i = 0; i < DBL_SUB_MEMTYPE_NUM; i++) {
        mem = &g_dev_nid[devid]->mem[memtype][i];
        if (mem->nid_num > 0 && mem->nid != NULL) {
            mem_num++;
        }
    }
    spin_unlock_bh(&g_dev_lock[devid]);

    return mem_num;
}
EXPORT_SYMBOL(dbl_nid_get_sub_memtype_num);

int dbl_nid_get_sub_memtype(u32 devid, u32 memtype, u32 sub_memtype[], int num)
{
    mem_info_t *mem = NULL;
    int count;
    u32 i;

    if ((devid >= NID_MAX_DEV_NUM) || (memtype >= DBL_MEMTYPE_NUM)) {
        ccfg_err("Devid or memtype is wrong. (devid=%u; memtype=%u)\n", devid, memtype);
        return -EINVAL;
    }

    spin_lock_bh(&g_dev_lock[devid]);
    if (g_dev_nid[devid] == NULL) {
        spin_unlock_bh(&g_dev_lock[devid]);
        ccfg_err("Get sub_memtype fail, dev is NULL. (devid=%u)\n", devid);
        return -EINVAL;
    }

    for (i = 0, count = 0; i < DBL_SUB_MEMTYPE_NUM && count < num; i++) {
        mem = &g_dev_nid[devid]->mem[memtype][i];
        if (mem->nid_num > 0 && mem->nid != NULL) {
            sub_memtype[count] = i;
            count++;
        }
    }
    spin_unlock_bh(&g_dev_lock[devid]);

    return count;
}
EXPORT_SYMBOL(dbl_nid_get_sub_memtype);

int dbl_nid_get_nid_size(u32 devid, int nid, nid_size_t *size)
{
    if ((devid >= NID_MAX_DEV_NUM) || (nid >= NID_MAX_NID_NUM) || (nid < 0) || (size == NULL)) {
        ccfg_err("Invalid para. (devid=%u; nid=%d; size_is_null=%d)\n", devid, nid, (size == NULL));
        return -EINVAL;
    }

    spin_lock_bh(&g_dev_lock[devid]);
    if (g_dev_nid[devid] == NULL) {
        spin_unlock_bh(&g_dev_lock[devid]);
        ccfg_err("Dev nid is not set. (devid=%u)\n", devid);
        return -EINVAL;
    }

    if (g_dev_nid[devid]->nid_info[nid].nid_mask == 0) {
        spin_unlock_bh(&g_dev_lock[devid]);
        ccfg_err("Nid is not supported. (devid=%u; nid=%d)\n", devid, nid);
        return -EINVAL;
    }

    *size = g_dev_nid[devid]->nid_info[nid].size;
    spin_unlock_bh(&g_dev_lock[devid]);

    return 0;
}
EXPORT_SYMBOL(dbl_nid_get_nid_size);

int dbl_nid_mem_occupy(u32 devid, int nid, u64 memsize)
{
    nid_info_t *nid_info = NULL;
    nid_size_t size;

    if ((devid >= NID_MAX_DEV_NUM) || (nid >= NID_MAX_NID_NUM || nid < 0)) {
        ccfg_err("Devid or nid is wrong. (devid=%u; nid=%d)\n", devid, nid);
        return -EINVAL;
    }

    spin_lock_bh(&g_dev_lock[devid]);
    if (g_dev_nid[devid] == NULL) {
        spin_unlock_bh(&g_dev_lock[devid]);
        ccfg_err("Dev nid is not set. (devid=%u)\n", devid);
        return -EINVAL;
    }

    if (g_dev_nid[devid]->nid_info[nid].nid_mask == 0) {
        spin_unlock_bh(&g_dev_lock[devid]);
        ccfg_err("Nid is not supported. (devid=%u; nid=%d)\n", devid, nid);
        return -EINVAL;
    }

    nid_info = &(g_dev_nid[devid]->nid_info[nid]);
    size = nid_info->size;
    if (size.free_size < memsize) {
        spin_unlock_bh(&g_dev_lock[devid]);
        ccfg_err("Nid is out of memory. (devid=%u; nid=%d; size=(%llu/%llu); occupy_size=%llu)\n",
            devid, nid, size.free_size, size.total_size, memsize);
        return -ENOMEM;
    }

    nid_info->size.free_size -= memsize;
    spin_unlock_bh(&g_dev_lock[devid]);

    return 0;
}
EXPORT_SYMBOL(dbl_nid_mem_occupy);

void dbl_nid_mem_return(u32 devid, int nid, u64 memsize)
{
    nid_info_t *nid_info = NULL;
    nid_size_t size;

    if ((devid >= NID_MAX_DEV_NUM) || (nid >= NID_MAX_NID_NUM) || (nid < 0)) {
        ccfg_err("Devid or nid is wrong. (devid=%u; nid=%d)\n", devid, nid);
        return;
    }

    spin_lock_bh(&g_dev_lock[devid]);
    if (g_dev_nid[devid] == NULL) {
        spin_unlock_bh(&g_dev_lock[devid]);
        ccfg_err("Dev nid is not set. (devid=%u)\n", devid);
        return;
    }

    if (g_dev_nid[devid]->nid_info[nid].nid_mask == 0) {
        spin_unlock_bh(&g_dev_lock[devid]);
        ccfg_err("Nid is not supported. (devid=%u; nid=%d)\n", devid, nid);
        return;
    }

    nid_info = &(g_dev_nid[devid]->nid_info[nid]);
    size = nid_info->size;
    if (memsize > size.total_size - size.free_size) {
        spin_unlock_bh(&g_dev_lock[devid]);
        ccfg_err("Nid total size isn't enough. (devid=%u; nid=%d; size=(%llu/%llu); return_size=%llu)\n",
            devid, nid, size.free_size, size.total_size, memsize);
        return;
    }

    nid_info->size.free_size += memsize;
    spin_unlock_bh(&g_dev_lock[devid]);
}
EXPORT_SYMBOL(dbl_nid_mem_return);

int dbl_get_ts_default_nid(u32 devid)
{
    int nid_cnt = 1, nid = NUMA_NO_NODE;

    (void)dbl_nid_get_nid(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_TS, &nid, nid_cnt);
    return nid;
}
EXPORT_SYMBOL(dbl_get_ts_default_nid);

int dbl_get_ddr_ai_nid(u32 devid, int nids[], int num)
{
    return dbl_nid_get_nid(devid, DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_AI, nids, num);
}
#ifndef EMU_ST
int dbl_get_hbm_ai_nid(u32 devid, int nids[], int num)
{
    return dbl_nid_get_nid(devid, DBL_MEMTYPE_HBM, DBL_SUB_MEMTYPE_AI, nids, num);
}

int dbl_get_all_ai_nid(u32 devid, int nids[], int num)
{
    return dbl_nid_get_nid(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_AI, nids, num);
}

int dbl_get_ctrl_nid(u32 devid, int nids[], int num)
{
    return dbl_nid_get_nid(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_CTRL, nids, num);
}
EXPORT_SYMBOL(dbl_get_ctrl_nid);

int dbl_get_ts_nid(u32 devid, int nids[], int num)
{
    return dbl_nid_get_nid(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_TS, nids, num);
}
EXPORT_SYMBOL(dbl_get_ts_nid);
#endif
void numa_id_init(struct proc_dir_entry *fs_entry)
{
    int i;
    for (i = 0; i < NID_MAX_DEV_NUM; i++) {
        spin_lock_init(&g_dev_lock[i]);
    }
    numa_id_fs_init(fs_entry);
    ccfg_info("NUMA id init success.\n");
}

void numa_id_uninit(struct proc_dir_entry *fs_entry)
{
    int i;

    for (i = 0; i < NID_MAX_DEV_NUM; i++) {
        nid_del_dev(i);
    }
    numa_id_fs_uninit(fs_entry);
    ccfg_info("NUMA id uninit success.\n");
}
