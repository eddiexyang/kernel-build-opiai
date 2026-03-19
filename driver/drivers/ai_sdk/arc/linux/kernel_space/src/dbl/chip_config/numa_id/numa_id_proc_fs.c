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
 * Create: 2022-10-15
 */
#include <linux/types.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/securec.h>

#include "kernel_version_adapt.h"
#include "dbl/chip_config.h"
#include "chip_config_module.h"
#include "numa_id_config.h"
#include "numa_id.h"

#ifdef CONFIG_PROC_FS
#define NUMA_ID_PROC_FS_NAME_LEN 32
#define NUMA_ID_PROC_FS_MODE 0444    /* 0xxx means octal */

struct proc_dir_entry *numa_id_proc_dev_entry = NULL;

static const char *g_memtype[DBL_MEMTYPE_NUM] = {
    [DBL_MEMTYPE_ALL] = "MEMTYPE_ALL",
    [DBL_MEMTYPE_DDR] = "MEMTYPE_DDR",
    [DBL_MEMTYPE_HBM] = "MEMTYPE_HBM",
};

static const char *g_sub_memtype[DBL_SUB_MEMTYPE_NUM] = {
    [DBL_SUB_MEMTYPE_ALL] = "SUB_MEMTYPE_ALL",
    [DBL_SUB_MEMTYPE_P2P] = "SUB_MEMTYPE_P2P",
    [DBL_SUB_MEMTYPE_TS]  = "SUB_MEMTYPE_TS",
    [DBL_SUB_MEMTYPE_AI] = "SUB_MEMTYPE_AI",
    [DBL_SUB_MEMTYPE_CTRL] = "SUB_MEMTYPE_CTRL",
};

static void numa_id_fs_format_task_dir_name(u32 dev_id, char *name, int len)
{
    if (sprintf_s(name, (unsigned int)len, "dev_%d", dev_id) <= 0) { /* if fail just name is zero */
        ccfg_err("Sprintf_s failed.\n");
    }
}

static struct proc_dir_entry *numa_id_proc_fs_mk_dev_dir(u32 dev_id, struct proc_dir_entry *parent)
{
    char name[NUMA_ID_PROC_FS_NAME_LEN] = {0};

    numa_id_fs_format_task_dir_name(dev_id, name, NUMA_ID_PROC_FS_NAME_LEN);
    return proc_mkdir((const char *)name, parent);
}

static void numa_id_proc_fs_rm_dev_dir(u32 dev_id, struct proc_dir_entry *parent)
{
    char name[NUMA_ID_PROC_FS_NAME_LEN] = {0};

    numa_id_fs_format_task_dir_name(dev_id, name, NUMA_ID_PROC_FS_NAME_LEN);
    remove_proc_subtree((const char *)name, parent);
}

static int numa_id_device_mem_show(struct seq_file *seq, void *offset)
{
    u32 dev_id = (u32)(uintptr_t)seq->private;
    int nid_id[DBL_NUMA_ID_MAX_NUM], nid_num, i;
    u32 memtype, sub_memtype;
    nid_size_t size;

    seq_printf(seq, "====show dev%u numa info====\n", dev_id);

    for (memtype = DBL_MEMTYPE_ALL; memtype < DBL_MEMTYPE_NUM; memtype++) {
        for (sub_memtype = DBL_SUB_MEMTYPE_ALL; sub_memtype < DBL_SUB_MEMTYPE_NUM; sub_memtype++) {
            nid_num = dbl_nid_get_nid(dev_id, memtype, sub_memtype, nid_id, DBL_NUMA_ID_MAX_NUM);
            if (nid_num < 0) {
                continue;
            }
            seq_printf(seq, "memtype=%s; sub_memtype=%s; nid_num=%d:\n",
                g_memtype[memtype], g_sub_memtype[sub_memtype], nid_num);

            for (i = 0; i< nid_num; i++) {
                (void)dbl_nid_get_nid_size(dev_id, nid_id[i], &size);
                seq_printf(seq, "    nid=%d, total_size=%llu, free_size=%llu\n",
                    nid_id[i], size.total_size, size.free_size);
            }
        }
    }
    return 0;
}

static int numa_id_memctrol_type_show(struct seq_file *seq, void *offset)
{
    if (dbl_nid_get_memctrl_type() == DBL_NID_MEMCTRL_SHARED_TYPE) {
        seq_printf(seq, "shared\n");
    } else {
        seq_printf(seq, "unshared\n");
    }
    return 0;
}

STATIC int numa_id_device_mem_open(struct inode *inode, struct file *file)
{
    return single_open(file, numa_id_device_mem_show, pde_data(inode));
}

STATIC int numa_id_memctrol_type_open(struct inode *inode, struct file *file)
{
    return single_open(file, numa_id_memctrol_type_show, pde_data(inode));
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static const struct proc_ops numa_id_device_mem_ops = {
    .proc_open    = numa_id_device_mem_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

static const struct proc_ops numa_id_memctrol_type = {
    .proc_open    = numa_id_memctrol_type_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};
#else
static const struct file_operations numa_id_device_mem_ops = {
    .owner = THIS_MODULE,
    .open    = numa_id_device_mem_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static const struct file_operations numa_id_memctrol_type = {
    .owner = THIS_MODULE,
    .open    = numa_id_memctrol_type_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
#endif

void numa_id_fs_add_device(u32 dev_id)
{
    struct proc_dir_entry *device_entry = numa_id_proc_fs_mk_dev_dir(dev_id, numa_id_proc_dev_entry);
    if (device_entry == NULL) {
        ccfg_err("Create task entry dir failed. (dev_id=%d)\n", dev_id);
        return;
    }
    proc_create_data("node_info", NUMA_ID_PROC_FS_MODE, device_entry,
        &numa_id_device_mem_ops, (void *)(uintptr_t)dev_id);
}

void numa_id_fs_del_device(u32 dev_id)
{
    numa_id_proc_fs_rm_dev_dir(dev_id, numa_id_proc_dev_entry);
}

void numa_id_fs_init(struct proc_dir_entry *fs_entry)
{
    numa_id_proc_dev_entry = proc_mkdir("numa_id", fs_entry);
    if (numa_id_proc_dev_entry == NULL) {
        ccfg_err("Create numa id entry dir failed.\n");
        return;
    }

    /*
     * tsd will query /proc/ccfg/numa_id/memctrol_type
     * shared:   different user share the same numa nodes
     * unshared: different user use  different numa nodes
     */
    proc_create_data("memctrol_type", NUMA_ID_PROC_FS_MODE, numa_id_proc_dev_entry,
        &numa_id_memctrol_type, NULL);

    return;
}

void numa_id_fs_uninit(struct proc_dir_entry *fs_entry)
{
    remove_proc_subtree("numa_id", fs_entry);
}
#else
void numa_id_fs_add_device(u32 dev_id)
{
}

void numa_id_fs_del_device(u32 dev_id)
{
}

void numa_id_fs_init(struct proc_dir_entry *fs_entry)
{
}

void numa_id_fs_uninit(struct proc_dir_entry *fs_entry)
{
}
#endif
